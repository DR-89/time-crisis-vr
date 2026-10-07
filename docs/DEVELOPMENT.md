# Development and validation

## Layout

- `quest/`: Android entry points, OpenXR host, GLES renderer, controller/input/UI and weapon asset.
- `upstream/`: pinned [namco22-decompile](https://github.com/spacestate1/namco22-decompile) Git submodule.
- `pc/`: Windows entry, OpenGL loader and CMake build; shares the Quest renderer/host.
- `tools/patch_upstream.py`: guarded, idempotent changes to seven engine files and the Time Crisis dispatcher.
- `tools/translate_crate.py`: ROM-verified build-time translation of 318 missing explosion-coroutine instructions.
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
python tests/test_handedness.py
python tests/test_color.py
python tests/verify_apk.py
python tests/verify_apk.py --apk artifacts/bundled/TimeCrisisVR-with-ROM.apk
python tests/test_pc.py
python tests/test_pc.py --replay 'C:\path\to\last-session.inputs' --frames 15000
```

`verify_apk.py` checks either APK variant: ROM hashes (bundled chip bytes or local build inputs), weapon bytes, ARM64 ELF files, DEX and build hash. It also checks the compiled manifest: the complete APK launches `MainActivity` directly, that activity retains both VR categories in every variant, and the optional 2D setup uses a separate task. Packaging verifies APK signing and 16 KiB ZIP alignment.

`test_handedness.py` uses the LLVM/MinGW toolchain and headers installed by `Build-PC.ps1`. It compiles the real shared host input code with mocked OpenXR calls and checks both grip inputs, trigger handoffs, short taps, simultaneous presses, arcade fire edges, pose/haptic routing, stable face buttons, saved defaults and focus/tracking/action loss. `test_options.py` renders both menu layouts and checks all eight saved preference combinations. `test_gun_render.py` checks eight gun views, separate slide/trigger motion, a stationary grip, the 150 ms return and stable aim origin. These checks do not replace an in-headset playtest.

`test_color.py` checks sRGB capability detection, format ordering and the RGBA8 fallback. For an actual Quest GPU comparison, run `python tests/test_color_device.py --adb C:\platform-tools\adb.exe --serial YOUR_SERIAL`. It compares raw output across RGBA8, sRGB stereo and sRGB multiview, including the arcade gamma pass. ANGLE without `GL_EXT_sRGB_write_control` cannot replace that device check.

For launch changes, test opening the app from the Quest library and confirm immersive display in the headset. OpenXR initialization alone does not establish that the app actually opened in VR; the v0.7.1/v0.7.2 launch regression passed that weaker check.

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

`Export-Quest-Diagnostics.ps1` copies current and previous logs/input recordings,
saved options and Android exit details into a local ignored artifact directory.
It also creates a ZIP for sharing, selects a Quest automatically when exactly
one is connected, and accepts `-Adb` and `-Serial` overrides. Missing optional
files are listed in the report. See [Stage 2 report collection](STAGE2-REPORTS.md).
Input recordings preserve the starting EEPROM and arcade controls. They do not
record video, audio or headset poses. Windows writes equivalent recordings beside
the executable. Replays preserve those session files. See [the explosive-box
regression](CRASH-FIX.md) for the reproduced failure and repair.

For desktop render diagnostics, `TCVR_FAST=1` disables real-time pacing,
`TCVR_CAPTURE_FRAME=600` saves a PPM on that rendered frame, and
`TCVR_SCENE_VIEW=1` uses reconstructed world geometry instead of the normal flat
camera. `TCVR_GEOMETRY_DIAGNOSTICS=1` reports scene distances. These are developer
environment variables, not player menu settings. `--no-dialog` suppresses startup
error dialogs for automated tests. Use `--headless --replay FILE --frames N` for
simulation-only regression tests without a headset.

## Reproducibility and packaging

Upstream commit: `6aaa90b4cbc7a23733e1c9f5f9a5e772a19fe23d`. The sound coverage supplement `quest/snd_extra.cov` adds a ROM-backed return instruction observed at `00DABE` during hardware testing. The translator regenerates it from the local ROM.

Packaging uses a fresh asset staging directory. ROM-free builds include only manifests, the generated player model and license notices; bundled builds also include the supplied ROM directory. Models and every required game chip are verified at startup. Settings and EEPROM data are private app files and are preserved on APK updates.

APK signing uses a local development keystore under `.tools/`. Keep that key private and backed up if distributing updates; another key cannot replace an existing installation with `adb install -r`.
