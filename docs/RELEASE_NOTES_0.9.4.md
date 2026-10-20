# Time Crisis VR v0.9.4

The **blue Namco arcade pistol** is now the default tracked weapon on Quest and
Windows PCVR, matching Time Crisis II VR and Crisis Zone VR.

## Changes

- Uses the same textured Namco model and calibrated grip/muzzle as the other
  two ports, with whole-gun recoil and support for either hand.
- Saved gun angle rotates the model, muzzle and shot direction together.
  Recoil is visual and does not move the aiming ray.
- APK updates automatically install and verify the new model. The earlier
  animated pistol remains a fallback.
- Model attribution and conversion metadata are included in both packages.
- Resolution presets, default-on optional texture filtering, launcher artwork,
  world-size/view-distance settings and existing controls are retained.

The Namco Arcade Gun model is by
[Martoscar](https://sketchfab.com/3d-models/namco-arcade-gun-15fbd5b9add94a34b5c21746e3dd32be),
licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
See [NOTICE.md](https://github.com/DR-89/time-crisis-vr/blob/v0.9.4/NOTICE.md)
for attribution and modifications.

## Downloads and installation

- **TimeCrisisVR-v0.9.4-quest3.apk** — Quest ARM64; package
  `org.timecrisis.quest`, version name **0.9.4**, version code **26**.
  Install with SideQuest or `adb install -r TimeCrisisVR-v0.9.4-quest3.apk`.
- **TimeCrisisVR-v0.9.4-windows-x64.zip** — extract the entire archive, connect
  an OpenXR headset and run **Play VR.cmd**. **Play SteamVR.cmd** selects an
  installed SteamVR runtime; **Play Desktop.cmd** starts monitor play.
- **SHA256SUMS.txt** — checksums for the APK, Windows ZIP and upgrade guide.
- **TC1-Quest-Upgrade.md** — signing-key migration instructions for older apps.

Game content and weapon models are included. Updates from v0.9.0 or newer use
the same signing key and support in-place installation. **For v0.8.4 or earlier,
back up settings and EEPROM before uninstalling** and follow the attached
[migration guide](https://github.com/DR-89/time-crisis-vr/blob/v0.9.4/docs/SIGNING-MIGRATION.md).

## Validation

Production GLES rendering passes stereo, both hands, recoil/return to rest,
stable rotated muzzle and fallback-pistol animation checks. Shared-host input
tests pass gun-angle alignment, hand switching and existing graphics controls.
APK verification checks model/ROM hashes, metadata, attribution, ARM64 libraries
and the immersive launcher; signature and alignment checks pass.

The clean Windows ZIP passes internal hashes and a desktop startup/render test
through **601 simulation frames**, loading the Namco model and shutting down
without CPU traps or graphics errors. A new Quest or PCVR headset playtest of
v0.9.4 has not been performed.

[README and controls](https://github.com/DR-89/time-crisis-vr/tree/v0.9.4#readme)

Support development: [https://www.patreon.com/c/OpenSourceDevDirk](https://www.patreon.com/c/OpenSourceDevDirk)
