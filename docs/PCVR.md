# Time Crisis VR for Windows

**Unreleased 0.8.3-aim-test.1:** this test build aligns flat layers and fallback
aiming with the arcade camera, and places shot marks on hit surfaces. The normal
monitor projection stays unchanged. See `docs/AIM-PROJECTION.md` in the source
tree for validation and limitations. The controls below are unchanged from v0.8.2.

**v0.8.2:** cover now uses either grip button. Both triggers select their controller and fire. Physical ducking remains optional.

Experimental native **Windows x64 / OpenXR / OpenGL 4.3** port, built on
[spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile).
The same original game simulation, tracked pistol, laser option and cover modes
are shared with the standalone Quest version.

## Start in VR

1. Extract the complete Windows ZIP into a writable folder. Keep `roms`, `models`
   and `openxr_loader.dll` beside `TimeCrisisVR.exe`.
2. Connect the headset to the PC using your PC VR software. Being attached to ADB
   by USB alone does not establish a PC VR session.
3. Start an OpenXR runtime that provides **OpenGL** graphics support and make it
   the active runtime in your PC VR software. SteamVR is one option.
4. Open **Play VR.cmd**. The headset receives separate stereo views; the PC
   window mirrors the left eye.
5. Press **A right** to insert three credits (or **X left** with DEFAULT HAND set
   to LEFT), then press **either trigger** to select that hand and start.

If SteamVR is installed in a standard Steam library, **Play SteamVR.cmd** selects
its OpenXR runtime for this launch only. It does not change the Windows default.
For a custom location, set `XR_RUNTIME_JSON` to SteamVR's `steamxr_win64.json`
before starting the executable.

This initial port targets **Quest Touch controllers**. Index, Vive and other
controller layouts have not been implemented or validated. The PC determines
the frame rate and streaming performance; standalone Quest measurements do not
describe the PC version. The runtime controls the headset refresh rate where
the refresh-rate extension is unavailable. Eye resolution follows the runtime
recommendation, capped at 2160 pixels on the longer edge.

## VR controls

| Action | Default hand: right | Default hand: left |
| --- | --- | --- |
| Select weapon hand / fire | Press either controller trigger | Press either controller trigger |
| Insert three credits | A right | X left |
| Toggle laser silently | B right | Y left |
| Pause / options / resume | Left menu / keyboard Escape | Left menu / keyboard Escape |
| Set default hand, in options | Right thumbstick click / keyboard H | Right thumbstick click / keyboard H |
| Change cover mode, in options | Y left | B right |
| Recenter / calibrate upright height | X left / keyboard R | A right / keyboard R |
| Grip cover: hold to leave cover; release both to hide/reload | Either grip button | Either grip button |
| Physical cover: hide/reload; leave cover | Duck; return upright | Duck; return upright |

Press a trigger to select that controller and fire. Aim and recoil vibration follow
that hand. Hold either grip to leave cover; release both to hide and reload.
The two triggers are no longer cover inputs. Physical ducking remains optional.

Open options and **click the right thumbstick** to change **DEFAULT HAND**.
This saved preference sets the starting hand and the face-button layout above.
Automatic trigger handoffs do not change the face buttons or rewrite preferences.
Held triggers cannot repeatedly switch hands. Release a trigger after tracking or
focus loss before firing again. The left menu button always resumes.

The laser is **on by default**. Existing saved choices are retained.
For physical ducking, choose that mode in options, sit or stand upright, then
press **X** (right-handed) or **A** (left-handed). A drop of approximately 20 cm enters cover; returning to within 12 cm
of the reference leaves cover. Height is calibrated each session.

## Play on a monitor

Open **Play Desktop.cmd**. This mode uses the original flat camera and a mouse;
it does not need a headset or an active OpenXR runtime.

| Input | Action |
| --- | --- |
| Mouse | Aim |
| Left mouse button | Fire / start |
| Right mouse button or Space | Hold to leave cover; release to hide/reload |
| C | Insert three credits |
| L | Toggle the laser |
| Escape | Pause / resume |
| H, while paused | Change the saved VR default hand |

Monitor mode always uses the mouse/keyboard cover input. Physical ducking and
the tracked 3D pistol are VR features. The original game advances at approximately
59.906 Hz; VR head and controller updates follow the headset display rate.

## Troubleshooting and diagnostics

- Read `timecris.log` beside the executable after a startup failure. The previous
  run is retained as `timecris-previous.log`.
- `XR_ERROR_FORM_FACTOR_UNAVAILABLE` means the selected runtime does not currently
  expose a connected headset. Connect the headset in the PC VR software first.
- An extension or graphics-device error can mean the runtime lacks OpenGL support
  or uses a different GPU. Use the same GPU for the runtime and game.
- Keep the whole extracted folder together. The executable alone is insufficient.
- Options are saved to `quest-options.cfg`. The name is shared with the Quest port.
- `last-session.inputs` records arcade controls and the starting EEPROM, not
  headset video or microphone audio. One prior session is retained. These files
  allow a crash to be replayed. No diagnostics are uploaded automatically.

The supplied portable package contains the game files. Original game content is
separate from the MIT-licensed port code; see `NOTICE.md` and `licenses`.

## Validation scope

Windows compilation, desktop OpenGL rendering, and replay of the recorded Stage 1
explosive-box crash have been tested. The corrected replay reaches 15,000 frames
without a CPU trap, including the former failure at frame 12,087. A Quest 3 was
tested through Virtual Desktop at 90 Hz, with 2005 x 2160 pixels per eye on an
RTX 3050 Laptop GPU. Eye captures show the stereo scene and tracked pistol, and
the tester confirmed that VR and controls worked. Compositor half-rate modes
are handled by advancing additional simulation ticks without extra XR submissions.
The subsequent timing adjustment passed clock tests at 30/45/60/72/90/120 Hz;
the headset disconnected before a second live PCVR check of that adjustment.
Short hitches and audio underruns were observed; perfectly stable frame delivery
has not been established. This remains an experimental build. Other runtimes,
a complete playthrough and all missing-scene reports remain unverified.

The v0.8.1 left-handed controls pass automated tests against the shared host's
OpenXR input code, including pose/haptic routing, both cover modes, held-button
handling and tracking/focus loss. Both menu layouts pass stereo rendering tests.
The tester confirmed left-handed play on standalone Quest 3. A separate live PCVR
test of the new layout is still pending.

The v0.8.2 gun/control changes additionally pass shared-host input tests for
either-grip cover, trigger handoffs, short taps, fire edges, simultaneous presses
and tracking/focus loss. GLES rendering tests cover separate slide and trigger
motion, stereo and a fixed aim origin. Live Quest and PCVR confirmation of these
changes is still pending. Its sRGB colour correction passed byte-for-byte output
checks on the Quest 3 GPU with both stereo render paths; the tester confirmed the
improved Quest colour appearance. A live PCVR colour comparison remains pending.
