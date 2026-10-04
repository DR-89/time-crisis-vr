# Time Crisis VR for Windows

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
5. Press **A** on the right controller to insert three credits, then the **right
   trigger** to start.

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

| Input | Action |
| --- | --- |
| Right controller / trigger | Aim the 3D pistol / fire |
| A | Insert three credits |
| B | Toggle the laser silently |
| Left menu / keyboard Escape | Pause and open options / resume |
| Y, while in options | Switch physical ducking / left-trigger cover |
| X / keyboard R | Recenter and calibrate upright head height |
| Left trigger, in trigger mode | Hold to leave cover; release to hide/reload |
| Duck, in physical mode | Hide/reload; return upright to leave cover |

The laser is **on by default**. Existing saved choices are retained.
For physical ducking, choose that mode in options, sit or stand upright, then
press X. A drop of approximately 20 cm enters cover; returning to within 12 cm
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
