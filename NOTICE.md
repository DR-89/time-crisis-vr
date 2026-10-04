# Attribution and license scope

Time Crisis VR is an unofficial community port for Meta Quest 3. It is not
affiliated with or endorsed by Namco, Bandai Namco, or Meta.

- **Quest port code:** DR-89 and contributors, MIT; see [LICENSE](LICENSE).
- **Underlying game reconstruction and shared engine:**
  [spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile),
  pinned at `6aaa90b4cbc7a23733e1c9f5f9a5e772a19fe23d`.
  Its MIT notice credits **Copyright (c) 2026 cmcrann** and is preserved in
  the submodule and APK's `assets/licenses/namco22-LICENSE.txt`.
- **SDL 2.30.11:** zlib license; original notice included in the APK.
- **Khronos OpenXR loader 1.1.43:** its original license notice is included in the APK.
- **Player weapon model:** generated for this project using Tripo3D; asset provenance,
  geometry and checksums are recorded in `quest/assets/models/player-gun.json`.
  It is a custom player-held model, not a model extracted from Time Crisis.
- **Time Crisis, original game program, graphics, sound, ROM contents and trademarks:**
  belong to their respective original rights holders. They are not covered by this
  project's MIT license. A bundled-ROM APK includes these original game files;
  a ROM-free APK requires the user to supply them separately.

The repository contains the port, build/import tools and a reference to upstream.
ROM archives, extracted ROMs, signing keys, generated build output and APKs are
excluded from Git. APKs are distributed separately as release assets.
