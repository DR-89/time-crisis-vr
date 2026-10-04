"""Check swapchain negotiation and the GLES capability gate without a headset."""
from pathlib import Path
import shutil, subprocess

ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'build/color-test';out.mkdir(parents=True,exist_ok=True)
headers=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include'
for folder in ('GLES3','KHR'):shutil.copytree(headers/folder,out/'include'/folder,dirs_exist_ok=True)
compiler=ROOT/'.tools/llvm-mingw/llvm-mingw-20260922-ucrt-x86_64/bin/x86_64-w64-mingw32-clang.exe'
for mode,defines in [('GLES',[]),('desktop',['-DTCVR_PC'])]:
    exe=out/f'color-{mode}.exe'
    subprocess.run([str(compiler),'-std=c11','-O2','-Wall','-Wextra','-Wno-unused-function',
                    *defines,'-I'+str(ROOT/'quest'),'-I'+str(out/'include'),
                    '-I'+str(ROOT/'pc/vendor/glad/include'),str(ROOT/'tests/color_fixture.c'),
                    '-o',str(exe)],check=True)
    print(mode+':',flush=True);subprocess.run([str(exe)],check=True)
