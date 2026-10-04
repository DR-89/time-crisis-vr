"""Pixel-exact reference comparison of optimized sprite/text rasterization."""
from pathlib import Path
import ctypes as C, json, os, subprocess, time
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'build/raster-test';out.mkdir(parents=True,exist_ok=True)
vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs=Path(subprocess.check_output([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
lines=['@echo off',f'call "{vs}/VC/Auxiliary/Build/vcvars64.bat" >nul']
for kind in ('sprite','text'):
    for fast in (0,1):
        stem=f'{kind}-{fast}'
        (out/f'{stem}.c').write_text(('#define TCVR\n' if fast else '')+('#define TEST_TEXT\n' if kind=='text' else '')+f'#define RASTER_SOURCE "{ROOT.as_posix()}/upstream/engine/{kind}_hw.c"\n#include "{ROOT.as_posix()}/tests/raster_fixture.c"\n')
        lines.append(f'cl /nologo /std:c11 /O2 /LD /I"{ROOT}/quest" "{stem}.c" /Fe:{stem}.dll')
        lines.append('if errorlevel 1 exit /b 1')
(out/'compile.cmd').write_text('\n'.join(lines)+'\n')
subprocess.run([os.environ['ComSpec'],'/c',str(out/'compile.cmd')],cwd=out,check=True)
results={}
for kind in ('sprite','text'):
    arrays=[];times=[]
    for fast in (0,1):
        lib=C.CDLL(str(out/f'{kind}-{fast}.dll'));run=lib.raster_fixture;run.argtypes=[C.c_uint,C.c_void_p]
        buffer=(C.c_uint8*(640*480*5))();images=[];start=time.perf_counter()
        for variant in range(128):run(variant,buffer);images.append(bytes(buffer))
        times.append(time.perf_counter()-start);arrays.append(images)
    for i,(a,b) in enumerate(zip(*arrays)):
        if a!=b:
            diffs=[j for j,(x,y) in enumerate(zip(a,b)) if x!=y]
            raise AssertionError((kind,i,len(diffs),[(j,a[j],b[j]) for j in diffs[:12]]))
    results[kind]=dict(cases=128,pixel_identical=True,reference_seconds=times[0],optimized_seconds=times[1])
    print(f'PASS: {kind}, 128 pixel-exact cases; {times[0]:.3f}s -> {times[1]:.3f}s (desktop including fixture setup).')
(out/'verification.json').write_text(json.dumps(results,indent=2)+'\n')
