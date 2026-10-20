# Tracked player pistol

From v0.9.4, Quest and PCVR load `models/namco-gun.tcgun` by default. This is the
same blue Namco arcade gun used by Time Crisis II VR and Crisis Zone VR, with
identical geometry, texture and calibration. The previous animated CC0 pistol,
`models/player-gun.tcgun`, remains a fallback if the preferred asset cannot load.

The model contains 13,149 vertices, 17,108 triangles and a 1024-square texture.
Coordinates are metres, +Y up and the barrel facing -Z. The muzzle is
`(0, 0.054069425, -0.173751)` relative to the controller aim pose. The saved gun
angle rotates the model, muzzle and shot direction together for either hand.
Whole-gun recoil is visual only; it does not move the aiming ray.

The Android asset manifest installs and verifies the new model when updating an
existing installation. Windows packages include both models and their metadata.
See [NOTICE.md](../NOTICE.md) for Martoscar's CC BY 4.0 attribution and
`quest/assets/models/namco-gun.json` for the conversion details and checksums.
