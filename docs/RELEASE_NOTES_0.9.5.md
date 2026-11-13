# Time Crisis VR v0.9.5

Explosions, fire and other world sprite effects now appear at their arcade depth
in VR instead of sitting on the flat HUD plane. This improves their stereo depth
and alignment as you move your head.

Thanks to **[RichardGG](https://github.com/RichardGG)** for
[PR #10](https://github.com/DR-89/time-crisis-vr/pull/10) and the reported Quest 3
playtest. Built on [spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile).

## Changes

- World effects use the low 21 depth bits of the arcade sprite key; priority
  bits are excluded from the physical distance.
- Sprites at or below the game's HUD threshold (`0x10`) stay on the HUD plane.
  The timer, health, ammunition, banners and score keep their existing placement.
- Shot marks retain their separate nearest-surface placement.
- World effects follow saved world size and view distance, including changes
  while paused. Normal desktop projection is unchanged.
- Both the Quest and Windows PCVR builds include the fix. Existing controls,
  graphics options and the blue Namco pistol are retained.

## Downloads and installation

- **TimeCrisisVR-v0.9.5-quest3.apk** — Quest ARM64, `org.timecrisis.quest`, version
  name **0.9.5**, version code **27**. Install with SideQuest or
  `adb install -r TimeCrisisVR-v0.9.5-quest3.apk`.
- **TimeCrisisVR-v0.9.5-windows-x64.zip** — extract fully, connect an OpenXR
  headset and run **Play VR.cmd**. **Play SteamVR.cmd** selects an installed
  SteamVR runtime; **Play Desktop.cmd** starts monitor play.
- **SHA256SUMS.txt** — checksums for both packages and the upgrade guide.
- **TC1-Quest-Upgrade.md** — signing-key migration instructions for old APKs.

Game content and weapon models are included. The signing key is unchanged from
v0.9.0–v0.9.4, allowing an in-place update that retains settings. For v0.8.4 or
earlier, back up settings and EEPROM before uninstalling; follow the attached
[migration guide](https://github.com/DR-89/time-crisis-vr/blob/v0.9.5/docs/SIGNING-MIGRATION.md).

## Validation

- VR math tests pass HUD-threshold boundaries, priority-bit masking, shot marks,
  world scale, view distance and unchanged desktop projection.
- Production GLES tests compare world sprites against equal-depth polygons in
  36 scale/distance/head/eye combinations per rendering path. Both normal stereo
  and Quest multiview match pixel-for-pixel, with no GL errors.
- The Windows ZIP passes internal checksums, a 601-frame startup/render test and
  a 2,730-frame recorded-input replay with zero CPU traps or graphics errors.
  The desktop capture at frame 2,100 is pixel-identical to v0.9.4 with adaptive
  texture-bake scheduling disabled for a deterministic comparison.
- APK verification passes 31 game-chip hashes, weapon assets and attribution,
  ARM64 libraries, immersive launch metadata, signing and alignment. The signing
  certificate matches v0.9.4.

The contributor reports successful testing on Quest 3. The maintainer's checks
for this release are automated; a new headset playtest and complete-game
validation have not been performed. This release does not claim to fix the
unreproduced Stage 2 chandelier/stairs crash reports.

[README and controls](https://github.com/DR-89/time-crisis-vr/tree/v0.9.5#readme)

Support development: [https://www.patreon.com/c/OpenSourceDevDirk](https://www.patreon.com/c/OpenSourceDevDirk)
