# Time Crisis VR — Kurzanleitung

Eigenständiger, experimenteller Port für **Meta Quest 3**, aufgebaut auf **[spacestate1/namco22-decompile](https://github.com/spacestate1/namco22-decompile)**. Die vollständige englische Dokumentation steht in der [README](../README.md).

## Installieren

Die [Release-APK](https://github.com/DR-89/time-crisis-vr/releases/latest) herunterladen und mit SideQuest oder `adb install -r TimeCrisisVR-v0.7.1-quest3.apk` installieren. Danach unter **Unbekannte Quellen → Time Crisis VR (Experimental)** öffnen. Diese vollständige APK richtet die integrierten Spiel-Dateien automatisch ein; keine ZIP-Auswahl nötig.

Nach dem Arcade-Start **A rechts** drücken, kurz auf die drei Credits warten und mit dem **rechten Trigger** starten.

## Steuerung

| Taste / Bewegung | Funktion |
| --- | --- |
| Rechter Controller / Trigger | Zielen / Schießen |
| A rechts | Drei Münzen einwerfen |
| B rechts | Laser lautlos und ohne Einblendung umschalten |
| Linke Menütaste | Optionen öffnen / weiterspielen |
| Y links im Menü | Physisch ducken oder linker Trigger |
| X links | Blick und aufrechte Sitz-/Stehhöhe neu setzen |
| Linker Trigger im Trigger-Modus | Halten: heraus; loslassen: Deckung/Nachladen |
| Abducken im physischen Modus | Deckung/Nachladen; aufrichten: heraus |

Für physisches Ducken im Menü den Modus auswählen, aufrecht stehen oder sitzen und **X** drücken. Ab etwa 20 cm Absenkung gilt Deckung; beim Aufrichten bis 12 cm unter die Ausgangshöhe wird sie verlassen. Modus und Laser-Auswahl bleiben gespeichert, die Höhe wird pro Sitzung neu erfasst.

## Stand und Grenzen

120 Hz sind die Zielrate. Ein zweiminütiger Test der 0.6.0 erreichte durchschnittlich 120,017 FPS, allerdings mit rund **2 % verspäteten/wiederholten Bildern**. Vollständig stabile 120 FPS in allen Szenen sind nicht nachgewiesen. Physisches Ducken wurde auf Quest 3 erfolgreich ausprobiert. Ein kompletter Spieldurchlauf, alle Spezialtreffer und uneingeschränktes Roomscale sind noch nicht verifiziert.

Die Original-Spielinhalte fallen nicht unter die MIT-Lizenz des Port-Codes; siehe [Hinweise und Quellen](../NOTICE.md).
