# Time Crisis VR 0.9.2 â€” local Quest build

20 October 2026 Â· Android version code 24.

- Add **TEXTURE FILTERING: ON / OFF** to the VR pause/options menu. Default is **ON** for new installations and existing configs that have no filtering setting.
- Squeeze the physical **left grip** while paused to toggle; the setting applies immediately and is saved. Grip controls during gameplay are unchanged. Keyboard **F** performs the same action while paused in the shared PC host.
- ON uses bilinear world-texture sampling; OFF restores point sampling. HUD/text and stencil masks retain their existing behavior. Bilinear filtering reduces abrupt texel transitions; it does not add mipmaps or eliminate all distance shimmer.
- Add guard texels around atlas slots so bilinear sampling cannot pull in colours from an adjacent slot, including coarse placeholder textures.
- Preserve the launcher icons and banners from the preceding local build.

## Validation

Saved defaults, old-config migration, malformed values and ON/OFF persistence pass. The actual host input fixture covers both handedness layouts, held controls at menu entry, focus loss, disconnected controllers and one toggle per press. All 26 menu previews render without GLES errors; live changes refresh the label.

Real GLES tests cover 2D and atlas textures, live ON/OFF/ON changes without re-upload, queued-draw barriers, sampler isolation and multiview equivalence. The production texture baker also passes adjacent-slot border, allocation, cached-coordinate and filtered-pixel checks.

Quest ARM64 APK built and signed locally. This update has not been installed or performance-tested on a headset and has not been published to GitHub.
