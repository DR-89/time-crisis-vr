"""Saved settings, physical cover traces and GLES stereo menu rendering on ANGLE."""
from pathlib import Path
import ctypes as C, itertools, os, re, shutil, subprocess, tempfile
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'build/options-test';out.mkdir(parents=True,exist_ok=True)
vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs=Path(subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
angle=next((Path(os.environ['LOCALAPPDATA'])/'Programs/Microsoft VS Code').glob('*/libEGL.dll')).parent
headers=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include'
for folder in ('GLES3','KHR'):shutil.copytree(headers/folder,out/'include'/folder,dirs_exist_ok=True)
def run(lines):
    script=out/'compile.cmd';script.write_text('@echo off\ncall "'+str(vs/'VC/Auxiliary/Build/vcvars64.bat')+'" >nul\n'+'\n'.join(lines)+'\n')
    subprocess.run([os.environ['ComSpec'],'/c',str(script)],cwd=out,check=True)
run([f'dumpbin /nologo /exports "{angle / "libGLESv2.dll"}" > exports.txt'])
exports=re.findall(r'^\s*\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(gl\w+)\s*$',(out/'exports.txt').read_text(),re.M);assert len(exports)>100
(out/'gles.def').write_text('LIBRARY libGLESv2.dll\nEXPORTS\n'+'\n'.join(exports))
run(['lib /nologo /def:gles.def /machine:x64 /out:gles.lib',f'cl /nologo /std:c11 /O2 /LD /I"{out / "include"}" /I"{ROOT / "quest"}" /I"{ROOT / "upstream/include"}" "{ROOT / "quest/quest_ui.c"}" "{ROOT / "quest/quest_options.c"}" "{ROOT / "quest/quest_cover.c"}" "{ROOT / "tests/options_fixture.c"}" gles.lib /Fe:options.dll /link /EXPORT:qoptions_load /EXPORT:qoptions_save /EXPORT:qcover_calibrate /EXPORT:qcover_pedal'])
directory=os.add_dll_directory(str(angle));egl=C.WinDLL(str(angle/'libEGL.dll'));lib=C.CDLL(str(out/'options.dll'))
P=C.c_void_p;I=C.c_int;U=C.c_uint
def bind(lib,name,result,*args):
    f=getattr(lib,name);f.restype=result;f.argtypes=args;return f
class Options(C.Structure):
    _fields_=[('laser_enabled',C.c_bool),('physical_crouch',C.c_bool),('left_handed',C.c_bool)]
class Cover(C.Structure):
    _fields_=[('upright_y',C.c_float),('calibrated',C.c_bool),('ducked',C.c_bool)]
load=bind(lib,'qoptions_load',None,C.c_char_p,C.POINTER(Options));save=bind(lib,'qoptions_save',C.c_bool,C.c_char_p,C.POINTER(Options))
def read_options(path):
    result=Options();load(path,C.byref(result));return tuple(getattr(result,k) for k,_ in Options._fields_)
with tempfile.TemporaryDirectory(prefix='tcvr-options-') as tmp:
    config=Path(tmp)/'quest-options.cfg';path=str(config).encode()
    assert read_options(path)==(True,False,False),'Default: laser ON, trigger cover, right-handed'
    config.write_text('laser_enabled=1\n');assert read_options(path)==(True,False,False)
    config.write_text('laser_enabled=0\nphysical_crouch=1\n');assert read_options(path)==(False,True,False),'Old settings survive upgrade; existing players stay right-handed'
    for values in itertools.product((False,True),repeat=3):
        options=Options(*values);assert save(path,C.byref(options))
        assert read_options(path)==values,'All three settings must survive saving either choice'
    keys=[key for key,_ in Options._fields_]
    for index,key in enumerate(keys):
        for invalid in ('2','1garbage','banana'):
            config.write_text(''.join(f'{k}={invalid if k==key else 1}\n' for k in keys))
            expected=[True]*3;expected[index]=index==0
            assert read_options(path)==tuple(expected)
    assert not save(str(Path(tmp)/'missing/options.cfg').encode(),C.byref(Options(True,True))),'Write failure must be reported'
calibrate=bind(lib,'qcover_calibrate',None,C.POINTER(Cover),C.c_float)
pedal=bind(lib,'qcover_pedal',C.c_bool,C.POINTER(Cover),C.c_float,C.c_bool)
cover=Cover();cp=C.byref(cover)
assert not pedal(cp,1.7,True),'No exposure before calibration'
calibrate(cp,1.7);assert pedal(cp,1.7,True),'Upright leaves cover'
for y in [1.69,1.6,1.51]:assert pedal(cp,y,True),'Small movements must not duck'
assert not pedal(cp,1.49,True),'Physical duck releases arcade pedal'
for y in [1.495,1.505,1.49,1.54,1.57]:assert not pedal(cp,y,True),'Noise near the duck threshold must not switch back'
assert pedal(cp,1.59,True),'Rising above the separate return threshold leaves cover'
assert not pedal(cp,1.0,False),'Tracking loss forces cover without recalibrating'
assert pedal(cp,1.7,True),'Tracking recovery preserves the upright reference'
assert not pedal(cp,float('nan'),True),'Invalid poses cannot expose the player'
calibrate(cp,float('nan'));assert pedal(cp,1.7,True),'Invalid calibration preserves reference'
calibrate(cp,1.2);assert pedal(cp,1.2,True) and not pedal(cp,.97,True),'Recalibration supports seated play'
display=bind(egl,'eglGetDisplay',P,P)(None);assert bind(egl,'eglInitialize',U,P,P,P)(display,None,None)
attributes=(I*13)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3038)
config=P();count=I();assert bind(egl,'eglChooseConfig',U,P,P,P,I,P)(display,attributes,C.byref(config),1,C.byref(count)) and count.value
current=bind(egl,'eglMakeCurrent',U,P,P,P,P);render=bind(lib,'options_fixture',I,I,I,I,P)
images=[];w=h=800
for variant in range(9):
    surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,w,0x3056,h,0x3038))
    context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038));assert surface and context and current(display,surface,surface,context)
    pixels=(C.c_ubyte*(w*h*4))();status=render(variant,w,h,pixels);assert status==1,(variant,status)
    image=Image.frombytes('RGBA',(w,h),bytes(pixels)).transpose(Image.Transpose.FLIP_TOP_BOTTOM).convert('RGB');images.append(image)
    assert sum(p!=image.getpixel((0,0)) for p in image.get_flattened_data())>10000
    image.save(ROOT/f'artifacts/options-preview-{variant}.png')
    current(display,None,None,None);bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
assert images[0].tobytes()!=images[1].tobytes(),'ON and OFF must be visually distinct'
assert images[1].tobytes()!=images[2].tobytes(),'Both cover modes must be visually distinct'
assert images[2].tobytes()!=images[3].tobytes(),'The panel must have stereo disparity'
assert images[2].tobytes()!=images[4].tobytes(),'Save errors must be visible'
assert images[1].tobytes()!=images[5].tobytes(),'Left-handed trigger/laser/credit labels must change'
assert images[2].tobytes()!=images[6].tobytes(),'Left-handed physical/recenter labels must change'
assert images[6].tobytes()!=images[7].tobytes(),'Left-handed panel must also have stereo disparity'
assert images[5].tobytes()!=images[8].tobytes(),'Changing hands must invalidate the cached menu texture'
bind(egl,'eglTerminate',U,P)(display)
print('PASS: all eight option combinations, old-config migration, malformed settings, cover traces, nine GLES menu views, hand-switch texture refresh and stereo; no GL errors.')
