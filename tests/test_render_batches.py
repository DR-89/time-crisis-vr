"""Compare deferred GLES draws with the immediate reference on local ANGLE.

By default, compare queued draws with the same adapter flushing immediately.
An older renderer can also be passed explicitly, e.g. build/quest_gl-baseline.c.
The fixture covers texture mutation, blend/mask/fog, gamma, laser and two eye
transforms. Requires MSVC and the bootstrapped NDK headers.
"""
from pathlib import Path
import argparse, ctypes as C, json, os, re, shutil, subprocess

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--reference',type=Path);p.add_argument('--multiview',action='store_true');p.add_argument('--srgb',action='store_true',help='Compare a raw sRGB eye target with the original RGBA8 target');a=p.parse_args()
reference=a.reference.resolve() if a.reference else ROOT/'quest/quest_gl.c';assert reference.is_file()
out=ROOT/'build/render-test';out.mkdir(parents=True,exist_ok=True)
vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs=Path(subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
vcvars=vs/'VC/Auxiliary/Build/vcvars64.bat'
angle=next((Path(os.environ['LOCALAPPDATA'])/'Programs/Microsoft VS Code').glob('*/libEGL.dll')).parent
headers=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include'
if not headers.is_dir():
    headers=next((ROOT/'.tools/android-sdk/ndk').glob('*/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include'))
for folder in ('GLES3','KHR'):shutil.copytree(headers/folder,out/'include'/folder,dirs_exist_ok=True)
(out/'include/android').mkdir(exist_ok=True)
(out/'include/android/log.h').write_text('#define ANDROID_LOG_ERROR 6\nstatic int __android_log_print(int p,const char*t,const char*f,...){return 0;}\n')
def run_script(lines):
    path=out/'compile.cmd';path.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\n'+'\n'.join(lines)+'\n')
    subprocess.run([os.environ['ComSpec'],'/c',str(path)],cwd=out,check=True)
run_script([f'dumpbin /nologo /exports "{angle / "libGLESv2.dll"}" > exports.txt'])
exports=re.findall(r'^\s*\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(gl\w+)\s*$',(out/'exports.txt').read_text(),re.M)
assert len(exports)>100
(out/'gles.def').write_text('LIBRARY libGLESv2.dll\nEXPORTS\n'+'\n'.join(exports))
run_script(['lib /nologo /def:gles.def /machine:x64 /out:gles.lib'])
for name,source in [('reference',reference),('deferred',ROOT/'quest/quest_gl.c')]:
    # An include wrapper avoids command-line quoting for macro string literals.
    wrapper=out/f'{name}.c'
    define=''
    if name=='reference':
        if not a.reference:define='#define QGL_TEST_IMMEDIATE\n'
        elif 'void qgl_flush(void)' not in reference.read_text():define='#define IMMEDIATE_REFERENCE\n'
    if 'void qgl_set_texture_filter(' in source.read_text():define+='#define TEST_TEXTURE_FILTER\n'
    if 'void qgl_scene_begin(' in source.read_text():define+='#define TEST_SCENE_TARGET\n'
    if 'void qgl_atlas_texture(' in source.read_text():define+='#define TEST_ATLAS_ARRAY\n'
    if name=='deferred' and a.multiview:define+='#define TEST_MULTIVIEW\n'
    if name=='deferred' and a.srgb:define+='#define TEST_SRGB_TARGET\n'
    wrapper.write_text(define+f'#define RENDERER_SOURCE "{source.as_posix()}"\n#include "{(ROOT / "tests/render_fixture.c").as_posix()}"\n')
    atlas_source='\"'+str(ROOT/'tests/atlas_filter_fixture.c')+'\"' if 'void qgl_set_texture_filter(' in source.read_text() else ''
    run_script([f'cl /nologo /std:c11 /O2 /LD /I"{out / "include"}" /I"{ROOT / "quest/include"}" /I"{ROOT / "quest"}" /I"{ROOT / "upstream/engine"}" "{wrapper}" "{ROOT / "quest/quest_scene.c"}" {atlas_source} gles.lib /Fe:{name}.dll'])
directory=os.add_dll_directory(str(angle));egl=C.WinDLL(str(angle/'libEGL.dll'))
P=C.c_void_p;I=C.c_int;U=C.c_uint
def bind(lib,name,result,*args):
    f=getattr(lib,name);f.restype=result;f.argtypes=args;return f
display=bind(egl,'eglGetDisplay',P,P)(None)
assert bind(egl,'eglInitialize',U,P,P,P)(display,None,None)
attributes=(I*13)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3038)
config=P();count=I();assert bind(egl,'eglChooseConfig',U,P,P,P,I,P)(display,attributes,C.byref(config),1,C.byref(count)) and count.value
make_current=bind(egl,'eglMakeCurrent',U,P,P,P,P)
results={}
for name in ('reference','deferred'):
    lib=C.CDLL(str(out/f'{name}.dll'));render=bind(lib,'render_fixture',I,I,I,I,P)
    if name=='deferred' and a.multiview:
        proc=bind(egl,'eglGetProcAddress',P,C.c_char_p)(b'glFramebufferTextureMultiviewOVR')
        assert proc,'ANGLE has no multiview entry point'
        bind(lib,'render_fixture_proc',None,P)(proc)
    for variant in range(4):
        surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,256,0x3056,256,0x3038))
        context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038))
        assert surface and context and make_current(display,surface,surface,context)
        pixels=(C.c_ubyte*(256*256*4))();status=render(variant,256,256,pixels);assert status==1,(name,variant,status)
        results[name,variant]=bytes(pixels)
        make_current(display,None,None,None)
        bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
for variant in range(4):
    expected=results['reference',variant];actual=results['deferred',variant]
    differences=sum(x!=y for x,y in zip(expected,actual))
    assert not differences,f'Eye {variant}: {differences} channel differences'
assert results['deferred',0]!=results['deferred',1],'Eye transforms produced identical images'
if not a.reference:
    oracle=bind(lib,'projection_fixture',I,I,I,I,P);projected=[]
    for variant in range(3):
        surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,256,0x3056,256,0x3038))
        context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038));assert make_current(display,surface,surface,context)
        pixels=(C.c_ubyte*(256*256*4))();assert oracle(variant,256,256,pixels)==1
        projected.append(bytes(pixels));make_current(display,None,None,None)
        bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
    assert projected[0]!=projected[1],'Camera uniform did not change the flat layer'
    assert projected[1]==projected[2],'Flat sprites and reconstructed camera rays do not align off-centre'
    print('PASS: flat sprite grid matches reconstructed polygons pixel-for-pixel; old 500 px projection differs.')
if not a.reference:
    scale_render=bind(lib,'world_scale_fixture',I,C.c_float,C.c_float,C.c_float,I,I,I,P)
    centres={};widths={};measurements=[]
    for size in (.25,1,4):
      for distance in (0,.4,1):
        for head in (0,.12):
            for eye in (0,1):
                w=h=512
                surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,w,0x3056,h,0x3038))
                context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038))
                assert surface and context and make_current(display,surface,surface,context)
                pixels=(C.c_ubyte*(w*h*4))();assert scale_render(size,distance,head,eye,w,h,pixels)==1
                for channel in (0,1):
                    hits=[n//4 for n in range(0,len(pixels),4) if pixels[n+channel]>200]
                    assert hits,'Missing world or HUD marker'
                    centre=sum(n%w for n in hits)/len(hits);centres[size,distance,head,eye,channel]=centre
                    base=15337/12000*size
                    depth=base+distance if channel==1 else 2.5
                    expected=(w-1)/2-(head+(.032 if eye else -.032))*w/2/depth
                    assert abs(centre-expected)<.6,(size,distance,head,eye,channel,centre,expected)
                    cy=sum(n//w for n in hits)/len(hits)
                    expected_y=(h-1)/2+(40*base/depth if channel==1 else -80)/772.5625*h/2
                    assert abs(cy-expected_y)<.6,(cy,expected_y)
                    widths[size,distance,head,eye,channel]=max(n%w for n in hits)-min(n%w for n in hits)+1
                make_current(display,None,None,None)
                bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
            disparity=centres[size,distance,head,0,1]-centres[size,distance,head,1,1]
            measurements.append(dict(scale=size,view_distance_metres=distance,head_metres=head,world_disparity_pixels=disparity))
    assert widths[1,1,0,0,1]<widths[1,.4,0,0,1]<widths[1,0,0,0,1],'Camera distance did not visibly shrink the cover-depth marker'
    for head in (0,.12):
        for eye in (0,1):
            for size in (.25,1,4):
                for distance in (0,.4,1):
                    assert centres[size,distance,head,eye,0]==centres[1,0,head,eye,0],'HUD moved with world settings'
                    assert widths[size,distance,head,eye,0]==widths[1,0,head,eye,0],'HUD resized with world settings'
    (out/('scale-multiview.json' if a.multiview else 'scale-stereo.json')).write_text(json.dumps(measurements,indent=2))
    print('PASS: measured cover depth, 25/100/400% stereo and 12 cm head parallax; view distance visibly shrinks cover without changing HUD.')
if not a.reference:
    filter_render=bind(lib,'texture_filter_fixture',I,I,I,P)
    for atlas in (0,1):
        for world in (0,1):
            surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,256,0x3056,256,0x3038))
            context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038))
            assert make_current(display,surface,surface,context)
            colours=(C.c_ubyte*12)();assert filter_render(atlas,world,colours)==1
            for step in range(3):
                expected=128 if world and step!=1 else 255
                assert all(abs(colours[step*4+c]-expected)<=1 for c in range(3)),(atlas,world,step,list(colours))
                assert colours[step*4+3]==255
            make_current(display,None,None,None)
            bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
    print('PASS: world filtering ON/OFF/ON without re-upload, 2D and array textures, unchanged flat layers, queued-state barriers and sampler isolation.')
if not a.reference:
    surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,64,0x3056,64,0x3038))
    context=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038))
    assert make_current(display,surface,surface,context)
    result=bind(lib,'atlas_filter_fixture',I)();assert result==1,result
    make_current(display,None,None,None)
    bind(egl,'eglDestroyContext',U,P,P)(display,context);bind(egl,'eglDestroySurface',U,P,P)(display,surface)
    print('PASS: production coarse atlas bakes have isolated colour borders, correct cache offsets/allocation and no neighbour bleed under linear sampling.')
bind(egl,'eglTerminate',U,P)(display)
print('PASS: '+('raw sRGB target, ' if a.srgb else '')+('multiview' if a.multiview else 'deferred')+' both eyes, identity and non-identity gamma are byte-identical to reference; no GLES errors.')
