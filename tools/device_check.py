"""Save evidence from this app only; optional capture of its two swapchain images."""
from pathlib import Path
import argparse,hashlib,json,re,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--serial');p.add_argument('--capture',action='store_true');args=p.parse_args()
adb=['adb']+(['-s',args.serial] if args.serial else [])
def call(*cmd):return subprocess.check_output(adb+list(cmd))
def shell(*cmd):return call('shell',*cmd).decode(errors='replace').strip()
pid=shell('pidof','org.timecrisis.quest')
if not pid:raise SystemExit('Time Crisis VR is not running.')
out=ROOT/'artifacts/device-check';out.mkdir(parents=True,exist_ok=True)
log=call('exec-out','run-as','org.timecrisis.quest','cat','files/timecris-vr.log').decode(errors='replace');(out/'timecris-vr.log').write_text(log)
cat=call('logcat','-d','--pid='+pid,'-v','brief').decode(errors='replace');(out/'android.log').write_text(cat)
apk=ROOT/'artifacts/TimeCrisisVR-quest3-debug.apk'
remote=shell('pm','path','org.timecrisis.quest').removeprefix('package:')
installed_hash=shell('sha256sum',remote).split()[0]
local_hash=hashlib.sha256(apk.read_bytes()).hexdigest();assert installed_hash==local_hash,'Installed APK differs from artifact'
frames=re.findall(r'\[TC\] frame (\d+)',log)
errors=[l for l in log.splitlines() if re.search(r'GL error|fault [1-9]|traps [1-9]|initialization failed',l)]
rates=[]
for line in cat.splitlines():
    sample=re.search(r'FPS=(\d+)/(\d+)',line);scale=re.search(r'SF=([\d.]+)',line)
    if sample and scale and float(scale[1])>0:rates.append(sample.groups())
rates=rates[-60:]
fps=[int(n) for n,_ in rates]
report={'apk_sha256':local_hash,'installed_apk_matches':True,'model':shell('getprop','ro.product.model'),'pid':pid,'openxr_initialized':'[XR] Quest initialized:' in log,'openxr_reached_focused':'[XR] session state 5' in log,'last_game_frame':int(frames[-1]) if frames else 0,'errors':errors,'fps_samples':fps,'full_playthrough_verified':False,'stereo_comfort_verified':False}
report['gun_model_loaded']=any(marker in log for marker in ('[GUN] Tripo model loaded:','[GUN] Arcade pistol model loaded:'))
report['refresh_rate_samples']=[int(hz) for _,hz in rates]
if args.capture:
    shell('run-as','org.timecrisis.quest','touch','files/capture.request')
    for _ in range(100):
        result=subprocess.run(adb+['shell','run-as','org.timecrisis.quest','test','-e','files/capture.request'],capture_output=True)
        if result.returncode:break
        time.sleep(.1)
    else:raise SystemExit('Capture not completed; headset may be asleep or not tracking.')
    from PIL import Image
    for eye in range(2):
        file=out/f'eye-{eye}.ppm';file.write_bytes(call('exec-out','run-as','org.timecrisis.quest','cat',f'files/eye-{eye}.ppm'))
        Image.open(file).save(file.with_suffix('.png'))
    report['captured_both_eyes']=True
(out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
info_file=ROOT/'artifacts/build-info.json'
info=json.loads(info_file.read_text());info['hardware_tested']=report['openxr_reached_focused'] and report['last_game_frame']>=120 and not errors
info['hardware_test_scope']='Quest 3 startup/attract smoke test; no full playthrough or comfort validation' if info['hardware_tested'] else 'Installed and OpenXR startup checked; active gameplay test pending'
info['hardware_report']='device-check/verification.json';info_file.write_text(json.dumps(info,indent=2)+'\n')
print(json.dumps(report,indent=2))
