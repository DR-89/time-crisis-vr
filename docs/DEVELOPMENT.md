# Development and validation

## Layout

- `quest/`: Android entry points, OpenXR host, GLES renderer, controller/input/UI and weapon asset.
- `upstream/`: pinned [namco22-decompile](https://github.com/spacestate1/namco22-decompile) Git submodule.
- `tools/patch_upstream.py`: guarded, idempotent changes to seven engine files.
- `tools/build.py`: local ROM preparation, DSP/sound translation, CMake build, asset packaging, signing and alignment.
- `tests/`: native math, GLES image comparisons, settings/cover, ROM import and package validation.

Use `git submodule update --init` after a non-recursive clone. Do not commit generated upstream sources or extracted game files. ROM files are inputs to the local build, not Git source files.

## Tests

Windows tests use Python 3. Rendering fixtures additionally need Pillow, MSVC and the ANGLE DLLs from VS Code. `Test-Quest.ps1` locates MSVC through vswhere.

```powershell
./Test-Quest.ps1
python tests/test_shaders.py
python tests/test_render_batches.py
python tests/test_render_batches.py --multiview
python tests/test_rasters.py
python tests/test_sprite_cache.py
python tests/test_patches.py
python tests/test_gun_render.py
python tests/test_options.py
python tests/verify_apk.py
```

`verify_apk.py` checks the default ROM-free output, ROM manifest against local build inputs, weapon bytes, ARM64 ELF files, DEX, setup launcher and build hash. Packaging also verifies APK signing and 16 KiB ZIP alignment. Bundled APKs must additionally have their chip contents checked against `assets/roms.sha256`.

The pure Java importer can be tested without Android:

```powershell
New-Item -ItemType Directory -Force build/rom-import-test
javac -d build/rom-import-test quest/java/org/timecrisis/quest/RomInstaller.java tests/RomInstallerTest.java
java -cp build/rom-import-test RomInstallerTest
```

Optional arguments are a matching local `timecris.zip` and the `roms.sha256` manifest extracted from the APK; they enable a full real-ROM import round trip. The fixture removes only its own temporary test directory.

## Device diagnostics

```sh
adb logcat -s TCVR TCVR-ROM SDL OpenXR
adb shell run-as org.timecrisis.quest cat files/timecris-vr.log
adb shell run-as org.timecrisis.quest cat files/quest-options.cfg
adb shell run-as org.timecrisis.quest touch files/capture.request
```

Capture writes `eye-0.ppm` and `eye-1.ppm` in private app files and may cause a brief hitch. Do it outside timed performance windows.

```powershell
python tools/device_check.py --serial YOUR_SERIAL --capture
python tools/perf_report.py artifacts/perf-window --capture --seconds 120
```

The device check targets the default ROM-free artifact. For a bundled install, compare the installed package hash to `artifacts/bundled/build-info.json` separately. Reports are local and ignored by Git. Current historical measurements are summarized in [PERFORMANCE.md](PERFORMANCE.md).

A private `files/refresh-rate.txt` can request another supported refresh rate on next startup; default is 120 Hz. `files/profile.request` enables detailed engine timers. Neither setting is shown in the player menu.

## Reproducibility and packaging

Upstream commit: `6aaa90b4cbc7a23733e1c9f5f9a5e772a19fe23d`. The sound coverage supplement `quest/snd_extra.cov` adds a ROM-backed return instruction observed at `00DABE` during hardware testing. The translator regenerates it from the local ROM.

Packaging uses a fresh asset staging directory. ROM-free builds include only manifests, the generated player model and license notices; bundled builds also include the supplied ROM directory. Models and every required game chip are verified at startup. Settings and EEPROM data are private app files and are preserved on APK updates.

APK signing uses a local development keystore under `.tools/`. Keep that key private and backed up if distributing updates; another key cannot replace an existing installation with `adb install -r`.
