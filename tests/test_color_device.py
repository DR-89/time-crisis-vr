"""Compare raw sRGB with RGBA8 on the connected Quest's real GLES driver.
No app install or settings changes. Only a uniquely named /data/local/tmp folder
is used on the headset; local comparison buffers stay in ignored artifacts/.
"""
from pathlib import Path
import argparse, json, subprocess, uuid

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--adb',default='adb');p.add_argument('--serial');a=p.parse_args()
adb=[a.adb]+(['-s',a.serial] if a.serial else [])
compiler=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/bin/aarch64-linux-android29-clang.cmd'
out=ROOT/'artifacts/private-color-test/gpu-validation';out.mkdir(parents=True,exist_ok=True)
remote='/data/local/tmp/tcvr-color-'+uuid.uuid4().hex
subprocess.run(adb+['shell','mkdir',remote],check=True)
results={};logs={}
try:
    for mode,defines in [('rgba8',['-DQGL_TEST_IMMEDIATE']),('srgb',['-DTEST_SRGB_TARGET']),('srgb-multiview',['-DTEST_SRGB_TARGET','-DTEST_MULTIVIEW'])]:
        exe=out/f'color-{mode}'
        subprocess.run([str(compiler),'-std=c11','-O2',*defines,
                        *['-I'+str(ROOT/f) for f in ('quest','quest/include','upstream/engine')],
                        str(ROOT/'tests/color_gles_main.c'),'-lEGL','-lGLESv3','-llog','-landroid','-lm','-o',str(exe)],check=True)
        subprocess.run(adb+['push',str(exe),remote+'/test'],check=True,capture_output=True)
        subprocess.run(adb+['shell','chmod','700',remote+'/test'],check=True)
        run=subprocess.run(adb+['shell',remote+'/test',remote],capture_output=True,text=True,timeout=30)
        logs[mode]=run.stdout+run.stderr;print(mode+': '+logs[mode],flush=True)
        (out/f'{mode}.log').write_text(logs[mode]);run.check_returncode()
        for variant in range(4):
            data=subprocess.check_output(adb+['exec-out','cat',f'{remote}/frame-{variant}.rgba'])
            assert len(data)==256*256*4
            (out/f'{mode}-{variant}.rgba').write_bytes(data);results[mode,variant]=data
    for mode in ('srgb','srgb-multiview'):
        for variant in range(4):
            baseline=results['rgba8',variant];current=results[mode,variant]
            differences=sum(x!=y for x,y in zip(baseline,current))
            assert not differences,f'{mode}, eye/gamma variant {variant}: {differences} channel differences'
    report={'passed':True,'byte_identical_to_rgba8':True,'modes':list(logs),'eye_gamma_variants':4,'logs':logs}
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: Quest GPU raw sRGB matches RGBA8 byte-for-byte for both eyes, gamma/identity/copy/blend paths, mono and multiview; no GLES errors.')
finally:
    # Exact known files only; never recursively remove a computed remote path.
    subprocess.run(adb+['shell','rm','-f',remote+'/test',*[f'{remote}/frame-{i}.rgba' for i in range(4)]],check=False)
    subprocess.run(adb+['shell','rmdir',remote],check=False)
