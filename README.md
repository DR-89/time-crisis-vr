# Time Crisis VR for Meta Quest 3 and Windows

An experimental VR port of **Time Crisis** for **Meta Quest 3** and **Windows PCVR**, with a tracked 3D pistol, stereo rendering, original arcade gameplay and sound, and a choice of physical ducking or trigger-controlled cover. The Windows build also includes a mouse-controlled monitor mode.

**Built on [spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile).** This project adds the Android/OpenXR host, Quest renderer, VR aiming, player weapon and controls to that reconstruction. The original arcade logic and shared engine come from upstream; this is not an independent recreation of the game.

[Downloads](https://github.com/DR-89/time-crisis-vr/releases) · [Quest quickstart](docs/QUICKSTART.md) · [Windows / PCVR](docs/PCVR.md) · [Technical notes](docs/DEVELOPMENT.md) · [Attribution](NOTICE.md)

![Time Crisis running on Quest 3, with the tracked player pistol](docs/images/gameplay.png)

*Actual app eye capture. The screenshot is from the 0.6.0 performance test; it is not a mockup.*

## Current status

**v0.8.0 — experimental, Quest ARM64 and Windows x64.** Gameplay, the player weapon, saved laser settings, menu controls and physical ducking have been tested on Quest 3. Windows desktop rendering and recorded-gameplay replay have been checked. A Quest 3 connected through Virtual Desktop displayed stereo PCVR at 90 Hz on an RTX 3050 Laptop GPU; the tester confirmed VR and controls worked. A complete playthrough and every boss/edge-case hit have not been verified.

Changes in v0.8.0:

- Native Windows OpenXR/OpenGL port, plus a desktop mouse mode.
- Arcade timing stays independent of compositor half-rate modes such as 45 Hz reprojection.
- Laser assistance is **on by default**; B still toggles it silently and saved choices are retained.
- Fix for the reproduced Stage 1 explosive-box exit: 318 omitted original instructions are translated at build time. The recorded failing session now passes frame 12,087 and runs through frame 15,000. See the [regression details](docs/CRASH-FIX.md).
- An infinite far plane prevents distant scenery from being clipped at the former 2 km limit. This addresses distant geometry loss; other missing-texture reports and geometry outside the original arcade camera still need testing.
- Current and previous session recordings/logs for reproducible bug reports.

The direct immersive Quest launcher fix from v0.7.3 is retained. Versions v0.7.1 and v0.7.2 could open as a flat panel.

The headset is asked to run at **120 Hz**. This is a target, not a guarantee of perfectly stable 120 FPS. See the measured results below.

## Download and install

1. Download **`TimeCrisisVR-v0.8.0-quest3.apk`** from [Releases](https://github.com/DR-89/time-crisis-vr/releases).
2. Enable developer mode on the Quest, connect it by USB and allow USB debugging.
3. Install the APK with SideQuest or Android platform-tools:

   ```sh
   adb install -r TimeCrisisVR-v0.8.0-quest3.apk
   ```

4. Open **Time Crisis VR (Experimental)** from the Quest library's **Unknown sources** section.
5. After the arcade startup, press **A on the right controller**, wait briefly for the three credits, then press the **right trigger** to start.

The bundled release APK includes the supplied Time Crisis ROM set and prepares it automatically on first start. **No separate ZIP selection or ROM import is required for that APK.** First startup can take a little longer while files are verified. Original game content is distinct from the MIT-licensed port code; see [NOTICE.md](NOTICE.md).

Updates installed with `adb install -r` retain your settings and game data. Uninstalling the app removes its private data. The APK uses the project's development signing key and remains debuggable for diagnostics; this is a sideload release, not a store release. The private signing key is not in the repository.

### Windows / PCVR

Extract **`TimeCrisisVR-v0.8.0-windows-x64.zip`** to a writable folder. Connect the
headset through your PC VR software, select an OpenXR runtime with OpenGL support,
then open **Play VR.cmd**. **Play SteamVR.cmd** selects an installed SteamVR runtime
for this launch only. **Play Desktop.cmd** starts mouse/keyboard play on a monitor.
Keep the complete extracted folder together. See [PCVR setup and controls](docs/PCVR.md).

### Optional ROM-free builds

The build tools can also create an APK without game files. Only for such a build, select your own **`timecris.zip` — Time Crisis World TS2 Ver.B** in the first-run setup screen. A matching ZIP can also be copied via USB:

```powershell
./Install-ROM.ps1 -Rom 'C:\path\to\timecris.zip'
```

The importer verifies every required chip by SHA-256 and accepts checksum-matching older chip names. Other regional/program revisions are not interchangeable. No ROM files are downloaded by the app. No extra BIOS file is required.

## Controls

| Input | Action |
| --- | --- |
| Right controller | Aim the visible 3D pistol |
| Right trigger | Fire / start / confirm |
| A, right controller | Insert three credits |
| B, right controller | Toggle laser aim assistance **silently**, with no popup |
| Left menu button | Pause and open options / resume |
| Y, left controller, **while in options** | Switch cover mode |
| X, left controller | Recenter and set the upright head-height reference |
| Left trigger, in trigger mode | Hold to leave cover; release to hide and reload |
| Duck / stand upright, in physical mode | Hide and reload / leave cover |

**Laser:** on by default, saved between sessions. A previous explicit off setting is retained. Turning it off hides only the beam; aiming and shooting still work.

**Cover mode:** left trigger by default. Open the left menu and press **Y** to choose **PHYSICAL DUCKING** or **LEFT TRIGGER**. Both settings are saved together. Y has no mode-switching effect outside the menu. The in-game menu and all documentation use English.

### Physical ducking

Choose physical ducking in the menu while upright. Press **X** while standing or sitting upright to calibrate your normal head height, then resume.

- Lowering your head by about **20 cm** enters cover and reloads.
- Returning to within **12 cm** of your calibrated height leaves cover.
- The gap between those thresholds prevents rapid switching from small movements.
- X recalibrates for a different standing/seated position. Height is recalibrated each app session; the selected mode is saved.
- Missing head tracking releases the virtual pedal. Cover input does not depend on the right controller being visible.

![Options menu with physical ducking selected](docs/images/options-english.png)

## What the port includes

- Native Android/ARM64 app using **OpenXR and OpenGL ES**, running entirely on the headset.
- Native Windows x64 application using **OpenXR and OpenGL 4.3**, with stereo headset output, a PC mirror and an optional desktop mode.
- Separate eye poses/projections and head movement, with the original arcade camera progression.
- Tracked, textured player pistol, visual recoil and controller vibration.
- Original game logic, levels, textures, DSP and sound path supplied by upstream and the ROM set.
- Geometry-based controller aiming mapped to the original lightgun coordinates.
- Saved laser and cover preferences, pause, recenter and focus handling.
- A shared **multiview world pass**, bounded sprite image caching and faster sprite/text rasterization.
- Original game simulation at approximately **59.906 Hz**, with head and weapon poses updated at the headset refresh rate.

## Performance: measured, not promised

A two-minute active gameplay test of **v0.6.0** on Quest 3, with both controllers and the weapon rendered, recorded:

| Metric | Result |
| --- | --- |
| Requested / observed display rate | 120 Hz |
| Mean / median runtime FPS | 120.017 / 120 |
| Lowest one-second FPS sample | 115 |
| Repeated/late frames reported by runtime | 293 / 14,400 nominal frames — **2.035%** |
| Mean / p95 runtime CPU + GPU time | 5.21 / 6.84 ms |
| Per-eye render resolution | 1374 × 1440 |

One-second FPS counters can read slightly above 120 due to sample boundaries. They do not prove that every frame arrived on time. Short hitches remain, particularly on first loading or new textures. New textures may briefly appear coarser while the existing texture budget refines them. These results predate the menu/ducking/import changes and are **not a benchmark of v0.8.0 or PCVR**. See [performance details](docs/PERFORMANCE.md).

## Known limitations

- This is an experimental community port, not an official Namco or Meta release.
- No complete-game validation, and no guarantee of perfectly stable 120 FPS in every scene.
- The arcade game can discard geometry outside its original camera; unrestricted 360-degree viewing and roomscale traversal are not assured.
- Some sprites/HUD elements use a fixed-depth plane. World scale, stereo comfort and large sideways movements need broader testing.
- Original 2D hit logic still matters. All bosses, special targets and edge-of-screen hits have not been verified.
- The gun uses its own depth buffer and is drawn as a foreground model; level walls do not occlude it.
- No hand model, MSAA or articulated gun slide. Desktop operator/settings menus are not fully exposed in VR.
- Tested on **Quest 3**. The manifest allows Quest 3S, but that device has not been validated. Quest 2 is not a supported target.
- PCVR initially targets Quest Touch controls and has been tested through Virtual Desktop. Other runtimes and controller profiles remain unverified.

## Build from source (Windows)

You need Git, CMake, Python 3 and a JDK. Android Studio's bundled JDK is detected at its usual Windows location; otherwise set `JAVA_HOME`. Building requires your own matching ROM set even for a ROM-free APK, because the DSP and sound translations are generated locally.

```powershell
git clone --recurse-submodules https://github.com/DR-89/time-crisis-vr.git
cd time-crisis-vr

# APK with your supplied ROMs; starts without an import step:
./Build-Quest.ps1 -Rom 'C:\path\to\timecris.zip' -BundleRoms

# Optional ROM-free APK:
./Build-Quest.ps1 -Rom 'C:\path\to\timecris.zip'

# Windows PCVR and desktop package:
./Build-PC.ps1 -Rom 'C:\path\to\timecris.zip'
```

Output:

- Bundled: `artifacts/bundled/TimeCrisisVR-with-ROM.apk`
- ROM-free: `artifacts/TimeCrisisVR-quest3-debug.apk`
- Build metadata and SHA-256: `build-info.json` beside the corresponding APK.
- Windows: `artifacts/pc/TimeCrisisVR-v0.8.0-windows-x64.zip` and its `.sha256` file.

The bootstrap downloads pinned NDK r27c, API 34, Build Tools 35.0.0, SDL 2.30.11, OpenXR loader 1.1.43 and Ninja 1.12.1 into `.tools/`. The first build takes several minutes and multiple GB. It does not install a global Android SDK. CMake and the JDK must already be installed.

The Windows build needs Git, CMake and Python 3; no Android SDK or JDK is required.
It downloads LLVM/MinGW 20260922, SDL 2.30.11, OpenXR loader 1.1.43, zlib 1.3.1 and
Ninja locally. The glad 2.0.8 OpenGL loader is included in source.

Upstream is pinned as a submodule at [`6aaa90b4`](https://github.com/spacestate1/namco22-decompile/tree/6aaa90b4cbc7a23733e1c9f5f9a5e772a19fe23d). `tools/patch_upstream.py` applies the guarded Quest adaptations reproducibly; generated game code and extracted ROMs stay out of this repository.

## Tests and diagnostics

See [DEVELOPMENT.md](docs/DEVELOPMENT.md) for the complete test and diagnostic workflow. Tests cover stereo/raycast math, exact GLES shaders, multiview image equivalence, sprite caching, 256 original-vs-optimized raster cases, saved options, physical-cover hysteresis and tracking loss, model rendering, ROM import and APK contents.

```powershell
./Test-Quest.ps1
python tests/test_options.py
python tests/test_rasters.py
python tests/test_sprite_cache.py
python tests/test_patches.py
python tests/verify_apk.py
python tests/test_pc.py
./Export-Quest-Diagnostics.ps1
adb logcat -s TCVR TCVR-ROM SDL OpenXR
```

Tests that render through ANGLE currently use the ANGLE DLLs installed with VS Code on Windows and require MSVC. They do not emulate a Quest headset. Report bugs with the APK version, headset model, cover mode, scene and relevant log excerpts; do not attach ROMs or signing keys to issues.

## Credits

The foundation is **[spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile)**, whose engine, translated game code, tooling and hardware reconstruction make this port possible. Please visit that repository for the original project and its other games.

Additional components: [SDL](https://github.com/libsdl-org/SDL), [Khronos OpenXR](https://github.com/KhronosGroup/OpenXR-SDK), and a custom player pistol generated with Tripo3D. Attribution, original copyright notices and the distinction between port code and original game content are documented in [NOTICE.md](NOTICE.md).
