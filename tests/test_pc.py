"""Validate a clean Windows release and optionally replay a real crash report.

python tests/test_pc.py [--replay path/to/last-session.inputs --frames 15000]
Always uses a temporary extraction; the developer's settings and logs survive.
"""
from pathlib import Path
import argparse, hashlib, json, os, re, shutil, subprocess, tempfile, zipfile

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--zip',type=Path,default=ROOT/'artifacts/pc/TimeCrisisVR-v0.8.4-windows-x64.zip')
p.add_argument('--replay',type=Path)
p.add_argument('--frames',type=int,default=15000)
a=p.parse_args()
with tempfile.TemporaryDirectory(prefix='tcvr-pc-test-') as temp:
    temp=Path(temp)
    with zipfile.ZipFile(a.zip) as z:
        assert not any(n.endswith(('.inputs','.log','.nv','.cfg','.keystore')) for n in z.namelist()),'Private state in release'
        z.extractall(temp)
    game=temp/'TimeCrisisVR-PC'
    for line in (game/'SHA256SUMS.txt').read_text().splitlines():
        digest,name=line.split('  ',1)
        assert hashlib.sha256((game/name).read_bytes()).hexdigest()==digest,name
    assert (game/'TimeCrisisVR.exe').read_bytes()[:2]==b'MZ'
    info=json.loads((game/'build-info.json').read_text())
    assert hashlib.sha256((game/'TimeCrisisVR.exe').read_bytes()).hexdigest()==info['exe_sha256']
    env=os.environ.copy();env.update(TCVR_FAST='1',TCVR_CAPTURE_FRAME='600')
    subprocess.run([game/'TimeCrisisVR.exe','--desktop','--no-dialog','--frames','601'],env=env,check=True,timeout=45)
    log=(game/'timecris.log').read_text()
    assert '[OPTIONS] laser ON' in log
    assert 'stop at frame 601, 0 traps' in log
    assert not re.search(r'\[TRAP\]|\[GL\].*error|Shader:|fault [1-9]',log)
    image=(game/'eye-0.ppm').read_bytes().split(b'\n',3)[3]
    assert len(image)==1280*960*3 and sum(v>20 for v in image)>len(image)//3,'Blank desktop render'
    print('PASS: clean Windows ZIP, hashes, default laser, desktop GL scene, 601 simulation frames')
    if a.replay:
        shutil.copy2(a.replay,game/'regression.inputs')
        subprocess.run([game/'TimeCrisisVR.exe','--headless','--replay','regression.inputs','--frames',str(a.frames)],check=True,timeout=180)
        log=(game/'timecris.log').read_text()
        assert f'stop at frame {a.frames}, 0 traps' in log
        assert not re.search(r'\[TRAP\]|fault [1-9]',log)
        (ROOT/'build/pc-regression.log').write_text(log)
        print(f'PASS: recorded crash replay reaches {a.frames} frames without traps or DSP/sound faults')
