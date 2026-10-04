"""Render the real gun with its production GLES code on ANGLE, including stereo."""
from pathlib import Path
import ctypes as C, os, re, shutil, subprocess
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'build/gun-test';out.mkdir(parents=True,exist_ok=True)
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
run(['lib /nologo /def:gles.def /machine:x64 /out:gles.lib',f'cl /nologo /std:c11 /O2 /LD /I"{out / "include"}" /I"{ROOT / "quest"}" "{ROOT / "quest/quest_gun.c"}" "{ROOT / "tests/gun_fixture.c"}" gles.lib /Fe:gun.dll'])
directory=os.add_dll_directory(str(angle));egl=C.WinDLL(str(angle/'libEGL.dll'));lib=C.CDLL(str(out/'gun.dll'))
P=C.c_void_p;I=C.c_int;U=C.c_uint
def bind(lib,name,result,*args):
    f=getattr(lib,name);f.restype=result;f.argtypes=args;return f
display=bind(egl,'eglGetDisplay',P,P)(None);assert bind(egl,'eglInitialize',U,P,P,P)(display,None,None)
attributes=(I*15)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3025,24,0x3038)
config=P();count=I();assert bind(egl,'eglChooseConfig',U,P,P,P,I,P)(display,attributes,C.byref(config),1,C.byref(count)) and count.value
current=bind(egl,'eglMakeCurrent',U,P,P,P,P);render=bind(lib,'gun_fixture',I,C.c_char_p,I,I,I,P)
images=[];w=h=640
for variant in range(5):
    surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,w,0x3056,h,0x3038))
    context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038));assert surface and context and current(display,surface,surface,context)
    pixels=(C.c_ubyte*(w*h*4))();status=render(str(ROOT/'quest/assets/models/player-gun.tcgun').encode(),variant,w,h,pixels);assert status==1,(variant,status)
    image=Image.frombytes('RGBA',(w,h),bytes(pixels)).transpose(Image.Transpose.FLIP_TOP_BOTTOM).convert('RGB');images.append(image)
    changed=sum(p!=image.getpixel((0,0)) for p in image.get_flattened_data());assert changed>1500,(variant,changed)
    image.save(ROOT/f'artifacts/gun-preview-{variant}.png')
    current(display,None,None,None);bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
assert images[1].tobytes()!=images[2].tobytes(),'Missing stereo disparity'
assert images[2].tobytes()!=images[3].tobytes(),'Missing recoil/pose motion'
bind(egl,'eglTerminate',U,P)(display)
print('PASS: textured gun visible in all five views, stereo disparity and recoil; no GLES errors.')
