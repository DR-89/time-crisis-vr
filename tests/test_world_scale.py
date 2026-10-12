"""Exercise production scene projection, aim and world effects without a headset."""
from pathlib import Path
import subprocess, tempfile

ROOT = Path(__file__).resolve().parents[1]
compiler = ROOT / '.tools/llvm-mingw/llvm-mingw-20260922-ucrt-x86_64/bin/x86_64-w64-mingw32-clang.exe'
with tempfile.TemporaryDirectory(prefix='tcvr-scale-') as temp:
    exe = Path(temp) / 'scene.exe'
    subprocess.run([str(compiler), '-std=c11', '-O2', '-Wall',
                    '-I'+str(ROOT/'quest'), '-I'+str(ROOT/'upstream/engine'),
                    '-I'+str(ROOT/'upstream/include'),
                    str(ROOT/'tests/test_vr.c'), str(ROOT/'quest/quest_scene.c'),
                    '-o', str(exe)], check=True)
    subprocess.run([str(exe)], cwd=temp, check=True, timeout=15)
