# Time Crisis VR 0.9.1 — local test build

Added launcher artwork at five Android densities, an adaptive icon and a banner.
Application, game activity and setup activity declare the artwork, including the
ROM-free launcher variant. APK packaging now compiles and links the resources.
Version code 23 retains the existing package identity and signing key.

The icon update repackages the existing native game libraries and supplied game
content. The rendering, controls and game behaviour are unchanged.

APK signature and 16 KiB ZIP alignment pass. All five legacy icons are opaque
and nonblank; adaptive foreground/background references and the banner resolve.
All 37 DEX/native/model/ROM files are byte-identical to 0.9.0.

Headset/QGO validation remains pending. This build is not published.
