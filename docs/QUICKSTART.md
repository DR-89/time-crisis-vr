# Time Crisis VR — Quickstart

A standalone experimental port for **Meta Quest 3**, built on **[spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile)**. See the [README](../README.md) for full documentation.

## Install

Download the [release APK](https://github.com/DR-89/time-crisis-vr/releases) and install it with SideQuest or `adb install -r TimeCrisisVR-v0.8.1-quest3.apk`. Open **Unknown sources → Time Crisis VR (Experimental)**. The complete APK prepares its bundled game files automatically; no ZIP selection is needed. For Windows, see [PCVR and desktop setup](PCVR.md).

After the arcade startup, press **A right**, wait briefly for three credits, then press the **right trigger** to start. In left-handed mode, use **X left** and the **left trigger** instead.

## Controls

| Action | Right-handed (default) | Left-handed |
| --- | --- | --- |
| Aim / shoot | Right controller / trigger | Left controller / trigger |
| Insert three credits | A right | X left |
| Toggle laser silently | B right | Y left |
| Open options / resume | Left menu button | Left menu button |
| Change weapon hand, in options | Right thumbstick click | Right thumbstick click |
| Change cover mode, in options | Y left | B right |
| Recenter / calibrate upright height | X left | A right |
| Hold: leave cover; release: hide/reload | Left trigger | Right trigger |
| Physical cover: hide/reload; leave cover | Duck; return upright | Duck; return upright |

To play left-handed, open options with the **left menu button**, then click the **right thumbstick** until **WEAPON HAND: LEFT** appears. The pistol and recoil move to the left controller, and the other controls follow the table above. Resume with the left menu button. Release the new firing trigger once after switching.

For physical ducking, select **PHYSICAL DUCKING** in the menu, sit or stand upright and press **X** (right-handed) or **A** (left-handed). Lowering your head by about 20 cm enters cover. Returning to within 12 cm of the reference height leaves cover. Weapon hand, cover mode and laser preference are saved; head height is calibrated for each session.

Laser assistance starts **on** with new settings. Previously saved choices are retained.

## Status and limitations

120 Hz is the target. A two-minute test of v0.6.0 averaged 120.017 FPS with about **2% repeated/late frames**. Perfectly stable 120 FPS in every scene has not been demonstrated. Physical ducking was successfully tested on Quest 3. A complete playthrough, all special-target hits and unrestricted roomscale movement remain unverified.

Original game content is not covered by the port code's MIT license; see [attribution and license scope](../NOTICE.md).
