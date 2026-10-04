"""Compile/link the exact GLES 3.0 shaders using a local ANGLE runtime, without a headset."""
from pathlib import Path
import ast, ctypes as C, os, re
ROOT=Path(__file__).resolve().parents[1]
candidates=list((Path(os.environ['LOCALAPPDATA'])/'Programs/Microsoft VS Code').glob('*/libEGL.dll'))
if not candidates:raise SystemExit('ANGLE unavailable. Pass this check on a machine with ANGLE or on the Quest.')
folder=candidates[0].parent
dll_dir=os.add_dll_directory(str(folder))
egl=C.WinDLL(str(folder/'libEGL.dll'));gl=C.WinDLL(str(folder/'libGLESv2.dll'))
P=C.c_void_p;I=C.c_int;U=C.c_uint
def bind(lib,name,restype,*args):
    f=getattr(lib,name);f.restype=restype;f.argtypes=args;return f
get_display=bind(egl,'eglGetDisplay',P,P)
initialize=bind(egl,'eglInitialize',U,P,P,P)
choose=bind(egl,'eglChooseConfig',U,P,P,P,I,P)
surface=bind(egl,'eglCreatePbufferSurface',P,P,P,P)
context=bind(egl,'eglCreateContext',P,P,P,P,P)
current=bind(egl,'eglMakeCurrent',U,P,P,P,P)
display=get_display(None)
assert initialize(display,None,None),'EGL initialization failed'
attributes=(I*13)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3038)
config=P();count=I();assert choose(display,attributes,C.byref(config),1,C.byref(count)) and count.value
surf=surface(display,config,(I*5)(0x3057,32,0x3056,32,0x3038))
ctx=context(display,config,None,(I*3)(0x3098,3,0x3038));assert surf and ctx
assert current(display,surf,surf,ctx)
create=bind(gl,'glCreateShader',U,U);source=bind(gl,'glShaderSource',None,U,I,P,P)
compile_shader=bind(gl,'glCompileShader',None,U);get=bind(gl,'glGetShaderiv',None,U,U,P)
log=bind(gl,'glGetShaderInfoLog',None,U,I,P,P)
create_program=bind(gl,'glCreateProgram',U);attach=bind(gl,'glAttachShader',None,U,U);link=bind(gl,'glLinkProgram',None,U)
get_program=bind(gl,'glGetProgramiv',None,U,U,P);log_program=bind(gl,'glGetProgramInfoLog',None,U,I,P,P)
sources=[ast.literal_eval(s) for s in re.findall(r'("#version(?:[^"\\]|\\.)*")',(ROOT/'quest/quest_gl.c').read_text())]
assert len(sources)==6
shaders=[]
for i,s in enumerate(sources):
    shader=create(0x8B31 if i%2==0 else 0x8B30);text=C.c_char_p(s.encode());source(shader,1,C.byref(text),None);compile_shader(shader)
    ok=I();get(shader,0x8B81,C.byref(ok));message=C.create_string_buffer(8192);log(shader,8192,None,message)
    assert ok.value,(i,message.value.decode());shaders.append(shader)
for i in range(0,len(shaders),2):
    program=create_program();attach(program,shaders[i]);attach(program,shaders[i+1]);link(program)
    ok=I();get_program(program,0x8B82,C.byref(ok));message=C.create_string_buffer(8192);log_program(program,8192,None,message)
    assert ok.value,message.value.decode()
renderer=bind(gl,'glGetString',C.c_char_p,U)(0x1F01).decode()
print('Exact GLES 3.0 shaders: 6 compiled, 3 programs linked; '+renderer)
current(display,None,None,None)
bind(egl,'eglDestroyContext',U,P,P)(display,ctx);bind(egl,'eglDestroySurface',U,P,P)(display,surf);bind(egl,'eglTerminate',U,P)(display)
