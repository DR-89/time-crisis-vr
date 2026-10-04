"""Compare actual engine sprite caching with its original path on ANGLE.

Repeated stereo poses, new frames, resized/removed sprites, text priority masks
and the bounded-memory fallback must all produce identical pixels.
"""
from pathlib import Path
import ctypes as C, os, re, shutil, subprocess
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'build/sprite-cache-test';out.mkdir(parents=True,exist_ok=True)
vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs=Path(subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
angle=next((Path(os.environ['LOCALAPPDATA'])/'Programs/Microsoft VS Code').glob('*/libEGL.dll')).parent
headers=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include'
for folder in ('GLES3','KHR'):shutil.copytree(headers/folder,out/'include'/folder,dirs_exist_ok=True)
(out/'include/android').mkdir(exist_ok=True)
(out/'include/android/log.h').write_text('#define ANDROID_LOG_ERROR 6\nstatic int __android_log_print(int p,const char*t,const char*f,...){return 0;}\n')
def compile(lines):
    path=out/'compile.cmd';path.write_text('@echo off\ncall "'+str(vs/'VC/Auxiliary/Build/vcvars64.bat')+'" >nul\n'+'\n'.join(lines)+'\n')
    subprocess.run([os.environ['ComSpec'],'/c',str(path)],cwd=out,check=True)
compile([f'dumpbin /nologo /exports "{angle/"libGLESv2.dll"}" > exports.txt'])
exports=re.findall(r'^\s*\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(gl\w+)\s*$',(out/'exports.txt').read_text(),re.M)
(out/'gles.def').write_text('LIBRARY libGLESv2.dll\nEXPORTS\n'+'\n'.join(exports));compile(['lib /nologo /def:gles.def /machine:x64 /out:gles.lib'])
engine=(ROOT/'upstream/engine/ss22_gl.c').read_text()
block=engine[engine.index('static GLuint  spr_tex, txt_tex;'):engine.index('/* the text tilemap, drawn last:')]
macros='\n'.join(re.findall(r'^#define gl\w+ qgl\w+$',(ROOT/'quest/include/GL/gl.h').read_text(),re.M))
(out/'cache.c').write_text(macros+'\n'+block)
for name in ('reference','cached','budget'):
    define='' if name=='reference' else '#define TCVR\n'
    if name=='budget':define+='#define QVR_SPRITE_CACHE_BYTES 8192u\n'
    wrapper=out/f'{name}.c';wrapper.write_text(define+f'#define RENDERER_SOURCE "{ROOT.as_posix()}/quest/quest_gl.c"\n#define CACHE_SOURCE "{(out/"cache.c").as_posix()}"\n#include "{ROOT.as_posix()}/tests/sprite_cache_fixture.c"\n')
    compile([f'cl /nologo /std:c11 /O2 /LD /I"{out}/include" /I"{ROOT}/quest/include" /I"{ROOT}/quest" /I"{ROOT}/upstream/engine" "{wrapper}" gles.lib /Fe:{name}.dll'])
directory=os.add_dll_directory(str(angle));egl=C.WinDLL(str(angle/'libEGL.dll'))
P=C.c_void_p;I=C.c_int;U=C.c_uint
def bind(lib,name,result,*args):
    f=getattr(lib,name);f.restype=result;f.argtypes=args;return f
display=bind(egl,'eglGetDisplay',P,P)(None);assert bind(egl,'eglInitialize',U,P,P,P)(display,None,None)
config=P();count=I();attrs=(I*13)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3038)
assert bind(egl,'eglChooseConfig',U,P,P,P,I,P)(display,attrs,C.byref(config),1,C.byref(count)) and count.value
current=bind(egl,'eglMakeCurrent',U,P,P,P,P);results={}
for name in ('reference','cached','budget'):
    surf=bind(egl,'eglCreatePbufferSurface',P,P,P,P)(display,config,(I*5)(0x3057,256,0x3056,256,0x3038))
    ctx=bind(egl,'eglCreateContext',P,P,P,P,P)(display,config,None,(I*3)(0x3098,3,0x3038));assert surf and ctx and current(display,surf,surf,ctx)
    lib=C.CDLL(str(out/f'{name}.dll'));run=bind(lib,'render_fixture',I,I,I,I,P)
    pixels=(C.c_ubyte*(16*(256*256*4+640*480)))();status=run(int(name=='budget'),256,256,pixels);assert status==1,(name,status)
    results[name]=bytes(pixels);current(display,None,None,None)
    bind(egl,'eglDestroyContext',U,P,P)(display,ctx);bind(egl,'eglDestroySurface',U,P,P)(display,surf)
for name in ('cached','budget'):
    differences=sum(a!=b for a,b in zip(results['reference'],results[name]))
    assert not differences,(name,differences)
bind(egl,'eglTerminate',U,P)(display)
print('PASS: 16 stereo images + priority masks match exactly; frame reuse, resize, removal and memory-budget fallback verified.')
