# next: scpcb on raylib, no blitz baggage

same game to play + look at, new tech. `port/` stays as the visual/behaviour reference (+ `SCPCB_SHOT` screenshots
to diff against) until this reaches parity, then it goes.

## build + run

same toolchain as port/ (`./setup.sh && ninja -C build`). muon: `BUILD=build-muon MESON=./muon ./setup.sh && ./muon -C build-muon samu`
(setup.sh hands it gnu ar + the system's pkg-config path, see the comments there). linux builds glfw w/ x11 + wayland (picked at
runtime), the wayland protocol glue comes from `wayland-scanner`. `./setup.sh -Dangle=/opt/angle/usr/lib` runs on angle:
- glfw gets angle's unversioned sonames (it'd dlopen the system's libEGL.so.1 otherwise) + an rpath to that dir
- src/glfw.c replaces raylib's rglfw.c so glfw's wayland backend asks for EGL_PLATFORM_ANGLE_ANGLE (angle refuses
  plain EGL_PLATFORM_WAYLAND_EXT), and angle builds ask for a "transparent" fb (every angle config has alpha, glfw's
  wayland backend skips those otherwise; angle's swapchain presents opaque)
- angle dlopens libvulkan.so.1 from its own dir only, so that dir needs one (`ln -s /usr/lib/libvulkan.so.1` there)
- check w/ `VIEW_LOG=1 next/build/view`: the renderer line should say ANGLE

other platforms: `TARGET=windows ./setup.sh && ninja -C build-windows` cross builds w/ clang + the mingw-w64 sysroot
(`cross/windows.ini`), es2 via angle's dlls next to the exe. mac has its meson branch (cocoa as objective-c +
frameworks, angle's dylibs) but needs an apple sdk, so it's untested; from linux that'd be osxcross + your own sdk.

style: `.clang-format` (yours, `clang-format -i` on src/ + tools/; vendor/ and src/glfw.c are in .clang-format-ignore,
glfw.c's include order is load-bearing). lint: `next/tools/lint.sh [build dir]` runs clang-tidy w/ `.clang-tidy`
(bug-finding checks; the ones that fight the house style are off, see the comment there). both clean right now.

from the repo root:
- `next/build/bake [x.rmesh|.b3d|.x ...]` bakes into `next/data` (only stale outputs; all of GFX/map takes ~2s)
- `next/build/scpcb` the game so far (reads options.ini: size, keys, mouse, fog, map size). dev env: `SCPCB_SEED=n` (skips the menu),
  `SCPCB_SHOT=a.png SCPCB_SHOT_FRAME=n` (w/o a seed: the menu, `SCPCB_TAB`/`SCPCB_OPT` pick the page), `SCPCB_PAUSE=1`, `SCPCB_WALK=ticks`, `SCPCB_YAW=deg`, `SCPCB_NOFOG=1`, `SCPCB_DOOR=1|2|3` +
  `SCPCB_CLICK=frame` (door tests), see the top of src/game.c
- `next/build/view [id]` flies around a baked room (eg `gfx/map/room2cafeteria_opt`) or orbits a model (eg
  `gfx/npcs/scp096`: space play, , . step, b bones, `VIEW_POSE=f` / `VIEW_YAW=deg` for shots). f fog, g ambient,
  t/l/p trigger boxes, lights, waypoints. `VIEW_CAM="x y z tx ty tz" VIEW_SHOT=a.png VIEW_FRAME=n VIEW_FOG=0` for
  scripted screenshots (port/build/rmview takes the same shot w/ `rmview GFX/map/x.rmesh a.png 0` from the first waypoint)

## rules

- strict c99 (+ posix in tools) for our code; only the vendored libs get gnu11.

- raylib types all the way down: `Vector3`/`Matrix`/`Quaternion` + raymath, `Model`/`Mesh`/`Material`/`Shader`,
  `Camera3D`, `Texture2D`/`RenderTexture2D`, `Font`, `Sound`/`Music`, `Color`, `BoundingBox`/`Ray`. no int handles, no
  entity-tree emulation, no blitz api names or semantics (no SetBuffer, no brushes, no TextureBlend, no black-key masks).
- raylib's space: right-handed, y up. converted once at bake time, the game never flips anything.
- the game only loads what raylib loads natively. anything else is baked offline.
- fixed 70hz sim tick (keeps the original's tuning, which was all in 1/70s units), render every frame. no FPSfactor.
- gameplay is ported from what the .bb *does*, not how it's written: real structs + state enums instead of
  EventState/EventState2/EventState3 floats, no global soup, no `Dim n+1`, no iround/Not-precedence rules.

## raylib module -> what we use it for

- **core**: window, input (keybinds stored as `KeyboardKey`), `Camera3D`, `BeginMode3D`, timing, `TakeScreenshot` for tests.
- **models**: rooms, props, npcs, items as `.glb` via `LoadModel`/`LoadModelAnimations` + `UpdateModelAnimation`
  (frame ranges = the old ExtractAnimSeq clips). picking w/ `GetRayCollisionMesh/Triangle/Box`. billboards
  (`DrawBillboardPro`) for sprites, particles, decals that face the camera.
- **shaders**: glsl es 100 in `next/shaders/`, read at startup (`SCPCB_SHADERS=dir` to point elsewhere), variants by
  `#define`s src/shd.c prepends. one world shader on every material: diffuse (uv0) x lightmap (`MATERIAL_MAP_OCCLUSION`, uv1) or vertex
  light, up to n point lights, linear fog, alpha cutout. plus small post shaders (blur, nvg, gasmask, vignette).
- **textures**: qoi via `LoadTexture`, mipmaps via `GenTextureMipmaps`, `RenderTexture2D` for screen fx + 3d icons.
- **text**: `LoadFontEx` on GFX/font's ttfs, `DrawTextEx`/`MeasureTextEx`. no AAText.
- **shapes**: menu frames, bars, cursor, debug draw (`DrawBoundingBox`, `DrawLine3D` for waypoints/triggers).
- **audio**: `Sound` + `LoadSoundAlias` voices, `Music` for ambience/tracks. 3d = falloff + pan from the camera's
  right vector -> `SetSoundVolume/Pan/Pitch`.

## what raylib doesn't have (so it's ours, kept small)

- **collision response**: swept ellipsoid vs triangles (player + npcs), static tris per room in a uniform grid; doors
  and other movers are `BoundingBox`es. rays go through the same grid -> `GetRayCollisionTriangle`.
- **attachments**: raylib flattens gltf node trees, so attach points come from bones (`model.bones` by name) or the
  bake sidecar, and parenting is just `MatrixMultiply` in the owner's update. no generic hierarchy.
- **config/saves**: options as key=value text, saves in our own format (no Save.bb compat).

## bake (`tools/bake`, offline, reuses port/'s fmt parsers)

- `.rmesh` -> `room.glb` (uv0 diffuse, uv1 + occlusion texture = lightmap) + `room.col` (collision tris) +
  `room.txt` sidecar (lights, spawns, waypoints, triggerboxes, sound emitters, props, screens), one record per line.
- `.b3d`/`.x` -> `.glb` w/ skin + one anim track, writer = raylib's vendored `cgltf_write.h`. lh -> rh + winding here.
- images -> qoi w/ real alpha (masks + "alpha from brightness" baked in), sounds -> qoa (as now).

## milestones

0. portability should be the goal, not strictly only for x11 because the game should be able to run on windows/mac later
1. bake: every room + model -> glb, checked in a tiny viewer (orbit cam, anim scrub).
   rooms done: 112 baked, screenshots match port/'s rmview. models done: all 168 .b3d/.x -> glb (skin + anim
   when they have keys/bones, keyframe n = blitz frame n), npc poses match port/'s rmview. rooms draw their props.
   left: dds textures (2, the forest), a few textures missing in the repo itself.
2. one room rendered w/ the world shader + fog, screenshot-diffed against `port/`. done (rooms + models).
3. map gen + rooms + doors + player controller/collision: walkable facility. done: `next/build/scpcb` generates the
   map (the original's algorithm, raylib's rng), drops you in the start room, wasd/mouse w/ the original's speeds,
   sprint/crouch/gravity, swept-ellipsoid collision (src/coll.c), doors between rooms open from their buttons.
   FillRoom done (src/fill*.c, drafted from port/'s fillroom*.c by a throwaway translator, then fixed by hand):
   every room's own doors (all 4 kinds, keycard/scanner/code buttons, moved/removed buttons, locks, linked +
   timed doors), Objects[] (models + pivots, hidden hitboxes collide), Levers[], RoomDoors[], and item spawns
   kept as records until items exist. cams, emitters, decals, sprites + extra waypoints come w/ their systems.
   `SCPCB_ROOM=name` starts you in a room. not yet: room culling beyond distance.
4. hud, menus, text, audio, options. hud done (blink + stamina meters, hand icon, messages, blinking itself),
   pixel-matched against port/'s. ui images bake 1:1 (tools/bake/ui.c, `.mu` = masked ui), fonts are copied as ttf.
   the meters only show when they matter: options.ini `hud mode = auto|always|off` (missing -> the original's
   `HUD enabled`, 1 = auto). auto: stamina once it's below full (+2 s), blink in the last third before a blink or
   while something's watching (173, once npcs exist).
   audio done: every sound under SFX/ bakes to qoa (tools/bake/snd.c, ~30 s the first time, 247 MB), src/snd.c =
   cache + 64-voice pool (raylib sound aliases) + the original's 3d (linear falloff to `range`, pan from the view
   angle) + music crossfades (out over 250 ticks, in 10%/tick). src/sfx.c: footsteps by floor material
   (Data/materials.ini stepsound, ray vs the room's meshes), landing, breathing when tired, the zone's ambience +
   music. doors + buttons play theirs. options.ini [audio] volumes, keys from [binds] (or [options]).
   menus done: main menu (173 flicker, whispers, new game w/ name/seed/intro/difficulties incl. custom), loading
   screens (Loadingscreens/loadingscreens.ini pages), pause menu, the options tabs in both layouts (only options
   next/ has: vsync, volumes, mouse, key rebinding, hud mode, show fps, framelimit; saved to options.ini on BACK).
   src/ui.c = the original's widgets, immediate mode. load game / achievements are greyed out until saves exist.
   left: the lamps' glow sprites, eye strain before a blink, the intro.
5. 173/106/guard/class-d, items + inventory, the events port/ already has.
6. the rest of port/README's "left to port" list, written straight against this.

## risks to check early

- gpu skinning: done. the model shader's skinned variant takes up to 128 bones (most is 966's 121): 512 vec4
  uniforms, mesa + angle give 4096 (es2's bare minimum is 128, so very old gles2-only gpus would need bone
  palettes). raylib's rlSetUniformMatrices hands es2 transposed matrices, src/model.c transposes first.
- lightmap look: blitz's TextureBlend multiply (+ the 2x on some) has to match in the shader, verify on milestone 2.
- room meshes w/ alpha surfaces need sorting; raylib's DrawModel doesn't sort, so the draw list does.
