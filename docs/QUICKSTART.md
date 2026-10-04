# Time Crisis VR — Quickstart

A standalone experimental port for **Meta Quest 3**, built on **[spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile)**. See the [README](../README.md) for full documentation.

## Install

Download the [release APK](https://github.com/DR-89/time-crisis-vr/releases) and install it with SideQuest or `adb install -r TimeCrisisVR-v0.8.0-quest3.apk`. Open **Unknown sources → Time Crisis VR (Experimental)**. The complete APK prepares its bundled game files automatically; no ZIP selection is needed. For Windows, see [PCVR and desktop setup](PCVR.md).

After the arcade startup, press **A right**, wait briefly for three credits, then press the **right trigger** to start.

## Controls

| Input | Action |
| --- | --- |
| Right controller / trigger | Aim / shoot |
| A right | Insert three credits |
| B right | Toggle the laser silently, without a popup |
| Left menu button | Open options / resume |
| Y left, in the menu | Switch physical ducking / left-trigger cover |
| X left | Recenter and set the upright head-height reference |
| Left trigger, in trigger mode | Hold: leave cover; release: hide/reload |
| Duck, in physical mode | Hide/reload; sit or stand upright to leave cover |

For physical ducking, select **PHYSICAL DUCKING** in the menu, sit or stand upright and press **X**. Lowering your head by about 20 cm enters cover. Returning to within 12 cm of the reference height leaves cover. Cover mode and laser preference are saved; head height is calibrated for each session.

Laser assistance starts **on** with new settings. Previously saved choices are retained.

## Status and limitations

120 Hz is the target. A two-minute test of v0.6.0 averaged 120.017 FPS with about **2% repeated/late frames**. Perfectly stable 120 FPS in every scene has not been demonstrated. Physical ducking was successfully tested on Quest 3. A complete playthrough, all special-target hits and unrestricted roomscale movement remain unverified.

Original game content is not covered by the port code's MIT license; see [attribution and license scope](../NOTICE.md).
