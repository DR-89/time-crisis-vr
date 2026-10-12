# Quest signing-key change in v0.9.0

The original private signing key was unavailable after restoring the TC1
project. With the maintainer's approval, v0.9.0 uses a new key. Android rejects
an in-place update over v0.8.4 or earlier with `INSTALL_FAILED_UPDATE_INCOMPATIBLE`.
This is a signing mismatch, not a corrupt download. The package remains
`org.timecrisis.quest`. Windows packages are unaffected.

Do not uninstall until your backup has been saved and checked. Uninstalling
removes private settings, EEPROM, logs and recordings. The commands below
preserve the persistent settings and arcade EEPROM. Export any logs or input
recordings you also want to retain with `Export-Quest-Diagnostics.ps1` first.

## Back up before uninstalling

Connect one Quest with USB debugging allowed. Run the following in **Windows
Command Prompt (`cmd.exe`)**, with `adb` and `tar` on PATH. Do not use Windows
PowerShell 5 for the binary `>` redirection; it can corrupt the archive.
Choose an empty backup folder and keep a second copy of the resulting archive.

```bat
adb shell am force-stop org.timecrisis.quest
adb exec-out run-as org.timecrisis.quest tar -C files -cf - quest-options.cfg tc_eeprom.nv > TC1-v0.8-backup.tar
tar -tf TC1-v0.8-backup.tar
```

The listing must contain **quest-options.cfg** and **tc_eeprom.nv**, and both
commands must succeed. If `run-as` or either file cannot be read, stop and keep
the old app installed. The backup contains your saved options and arcade state;
it is not a suspend-state snapshot of a running level.

## Install and restore

Only after checking the backup:

```bat
adb uninstall org.timecrisis.quest
adb install TimeCrisisVR-v0.9.0-quest.apk
adb shell am start -n org.timecrisis.quest/.MainActivity
```

Let the new app finish its first startup, then close it and restore the archive
in the same Command Prompt folder:

```bat
adb shell am force-stop org.timecrisis.quest
adb push TC1-v0.8-backup.tar /data/local/tmp/TC1-v0.8-backup.tar
adb shell run-as org.timecrisis.quest tar -C files -xf /data/local/tmp/TC1-v0.8-backup.tar
adb shell rm /data/local/tmp/TC1-v0.8-backup.tar
adb shell am start -n org.timecrisis.quest/.MainActivity
```

Retain the backup until you have checked the restored settings in the headset.
Missing new scale options take the new defaults (100% world size, 40 cm distance).
Future APKs signed with the v0.9.0 key can use `adb install -r` normally.

The separate local **World Scale Test** app has package
`org.timecrisis.quest.worldscaletest`; it is not the production app and these
commands do not remove it. Its settings are separate from the production app.

Public certificate SHA-256 fingerprints:

- Previous TC1 key: `ea0c462442d67921a69e3848eed032828eed27392a452038fc91e7d44c16b9d9`
- v0.9.0 key: `482f044db3755b73906b8e159294a566699fa74189aefe14f47785ad4ca4670e`
