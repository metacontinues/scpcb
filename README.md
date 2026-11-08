# SCP - Containment Breach

The game is based on the works of the SCP Foundation community (http://www.scp-wiki.net/).

This game and the source code are licensed under Creative Commons Attribution-ShareAlike 3.0 License.

http://creativecommons.org/licenses/by-sa/3.0/

Requirements:
•	Blitz3D v1.108

Beware - the source code is perhaps more horrifying than the game itself!

## layout

- `bb/`: the original blitz3d source (`Main.bb` + its includes, the `.decls`, the IDEal project), original docs
- `bb/bin/`: the original game + tool exes and the dlls they load
- `port/`: c port of the .bb code (reference only now), `next/`: the raylib-native rewrite
- `GFX/`, `SFX/`, `Data/`, `Loadingscreens/`, `Map Creator/`, `options.ini`: game data, shared by all of the above

everything loads data relative to the working dir, so run from the repo root, eg
`wine "bb/bin/SCP - Containment Breach.exe"` (double clicking the exe won't find `GFX/`).
