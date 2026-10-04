"""Verify ROM integrity, the ARM64 ELF headers, manifest identity and exported entry point."""
from pathlib import Path
import hashlib,json,struct,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1]
apk=ROOT/'artifacts/TimeCrisisVR-quest3-debug.apk'
with zipfile.ZipFile(apk) as z:
    assert z.testzip() is None
    manifest=z.read('assets/roms.sha256').decode().splitlines()
    assert len(manifest)==31, len(manifest)
    assert not any(n.startswith('assets/roms/') or n.endswith('timecris.zip') for n in z.namelist()),'Public APK must not bundle ROMs'
    for line in manifest:
        digest,name=line.split('  ',1)
        assert hashlib.sha256((ROOT/'build/assets/roms'/name).read_bytes()).hexdigest()==digest,name
    for line in z.read('assets/models.sha256').decode().splitlines():
        digest,name=line.split('  ',1);model=z.read('assets/models/'+name)
        assert hashlib.sha256(model).hexdigest()==digest,name
        assert model==(ROOT/'quest/assets/models'/name).read_bytes(),name
        assert model[:8]==b'TCGUN001'
        vertices,indices,w,h=struct.unpack_from('<4I',model,8)
        assert 0<vertices<=100000 and 0<indices<=300000 and indices%3==0 and 0<w<=2048 and 0<h<=2048
        assert len(model)==36+vertices*32+indices*4+w*h*4
    libs=[n for n in z.namelist() if n.startswith('lib/')]
    assert sorted(libs)==['lib/arm64-v8a/libSDL2.so','lib/arm64-v8a/libmain.so','lib/arm64-v8a/libopenxr_loader.so']
    for lib in libs:
        elf=z.read(lib);assert elf[:5]==b'\x7fELF\x02';assert struct.unpack_from('<H',elf,18)[0]==183,lib
    assert z.read('classes.dex')[:4]==b'dex\n'
    for name in ['namco22-LICENSE.txt','SDL2-LICENSE.txt','OpenXR-LICENSE.txt']:assert z.read('assets/licenses/'+name)
badging=subprocess.check_output([str(ROOT/'.tools/buildtools/android-15/aapt.exe'),'dump','badging',str(apk)],text=True)
assert "package: name='org.timecrisis.quest'" in badging
assert "launchable-activity: name='org.timecrisis.quest.LauncherActivity'" in badging
assert "native-code: 'arm64-v8a'" in badging
readelf=ROOT/'.tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-readelf.exe'
symbols=subprocess.check_output([str(readelf),'--dyn-syms',str(ROOT/'build/package-libs/libmain.so')],text=True)
assert ' SDL_main' in symbols
info=json.loads((ROOT/'artifacts/build-info.json').read_text());assert info['sha256']==hashlib.sha256(apk.read_bytes()).hexdigest()
assert info['roms_bundled'] is False
print('APK verified: NO bundled ROMs, 31 expected chip hashes, gun model/hash, 3 ARM64 libraries, SDL_main, DEX, setup launcher, licenses, artifact hash')
