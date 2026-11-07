# scpcb c port

c rewrite of scpcb on raylib, rendering through gles2 so it runs on [ANGLE](https://chromium.googlesource.com/angle/angle).

the game (`src/game`, ported from the original `.bb` files) runs on a small runtime of its own (`src/rt`):
an entity tree w/ collisions, picks and animation, textures, 2d, sound, input and files, on raylib.
entities are int handles (0 = null), angles are degrees, indices count from 0, `irand`/`frnd` take (lo, hi).

## layout

```
port/
  meson.build      meson build (plain syntax, muon works too), builds raylib + mimalloc from vendor/ too
  src/rlconfig.h   overrides for raylib's config.h: only the formats the game loads (qoi, qoa, ttf)
  data/            converted assets (gitignored), written by tools/assetconv.c
  vendor/raylib    submodule, pinned to 6.2
  vendor/mimalloc  submodule, v3.5.4, overrides malloc for the whole process (-Dmimalloc=false to skip)
  setup.sh         configures port/build w/ the llvm toolchain (clang, llvm-ar, lld)
  src/rt/          the runtime
    rt.h           public api, the only header game code should include
    in.h           internals shared w/ src/fmt
    m3.h           lh math (yaw*pitch*roll, +z fwd)
    ent.c          entity pool, hierarchy, transforms, TForm*
    mesh.c         brushes, surfaces, mesh ops
    tex.c          textures + flags, refcounted so freeing a used tex is fine
    rend.c         cameras, lights, sprites, RenderWorld
    shd.h          the fixed-function-ish glsl es 100 shader
    fs.c           game paths (backslashes, any case) + read_int/read_string/etc
    sys.c          window/frame/time, primitives
    coll.c         Collisions/UpdateWorld (swept ellipsoid), picks, EntityVisible
    anim.c         keyframes (lerp + slerp per channel), cpu skinning, SetAnimTime/Animate/seqs
    snd.c          sounds/channels (raylib aliases) + music streams for the FSOUND_Stream_* calls
    g2d.c          buffers, images, Text/Rect/Line/Oval, fonts, CopyRect, Read/WritePixel
    input.c        keyboard (directinput scancodes) + mouse, hit/speed counters
    misc.c         rng, file writing, dirs, dates
    str.h/.c       strings: frame-arena temps + sset for kept ones, s_left/s_mid/s_find/... + cat(...)
    tl.h/.c        blitz Types: lnew/ldel/each/First/After/Insert + Handle/Object
  src/fmt/         file formats: b3d.c (+ LoadMesh dispatch), x.c, rmesh.c
  src/game/        the game, hand-ported. g.h has every shared global/type/fn
    main.c         Main.bb top level (init order), console msgs, misc Main.bb fns
    ini.c          Main.bb INI-functions      strict.c  StrictLoads.bb (+ LoadTempSound)
    menu.c         launcher                   loading.c DrawLoading
    widgets.c      buttons/ticks/frames/RowText/InputBox   sliders.c SlideBar/Slider3-7/Button/scrollbar
    mainmenu.c     UpdateMainMenu + Menu.bb top level      options.c the options tabs
    tooltip.c      option/map tooltips, GetLineAmount      save.c    Save.bb (save + map lists so far)
    loop.c         Main.bb's main loop (+ a temp fly cam)  music.c   UpdateMusic + the Music list
    map.h          MapSystem types (rooms, templates, doors, waypoints, screens) + their globals
    rooms.c        materials, rmesh glue, room templates, CreateRoom, FillRoom's generic part, lights, overlap
    mapgen.c       CreateMap / SetRoom / GetZone           doors.c   CreateDoor, CreateButton
    entities.c     LoadEntities (most of it)              items.c   Items.bb: templates, CreateItem
    fillroom1-3.c  FillRoom's 75 per-room cases (drafted by a throwaway script, reviewed by hand)
    forest.c       SCP-860-1's forest: grid gen, PlaceForest, load_terrain
    fx.c           particles, emitters, decals, security cams, elevator objs
    npcs.h/.c      NPCs.bb: the NPCs type, CreateNPC (all 22 types), RemoveNPC, the UpdateNPCs frame + gravity, helpers
    npc173.c       UpdateNPCs' 173 + 106 cases          waypoints.c InitWayPoints + FindPath (a*)
    events.c       Events type, CreateEvent/InitEvents (UpdateEvents.bb itself isn't ported yet)
    inv.c          DrawGUI's inventory screen (drag, drop, combine into clipboard/wallet, batteries), the inventory key
    use.c          DrawGUI's item-in-hand part: documents, kits, wearables, pills... (not yet: radio, nav, 1499, 294 cup, 1025)
    ev173.c ev1-4.c evpd.c  UpdateEvents.bb's events (ev2.c also has DrawQuickLoading/QuickLoadEvents) (evlib.c: their helpers), one name -> fn table per file
    keyname.c difficulty.c achv.c             the small .bb files (achv.c also has the unlock popups)
  tools/rmview.c   room/model viewer, the current test bed (tab = walk w/ the game's collider,
                   `rmview GFX/npcs/classd.b3d out.png 0 anim 200` poses an npc)
  tools/colltest.c collision + pick checks, `meson test -C port/build`
  tools/sndtest.c  sound/channel/stream checks (muted). w/o an audio device it only checks silent mode
  tools/g2dtest.c  every 2d path once + pixel io; `port/build/g2dtest out.png` to eyeball it
  tools/misctest.c rng + file io
  tools/assetconv.c images -> qoi, oggs -> qoa into port/data (the only place w/ png/jpg/bmp/dds/ogg decoders)
```

## build + run

needs clang + lld + llvm-ar, meson + ninja (or [muon](https://muon.build), the file sticks to plain meson so
both work), x11 dev headers. run stuff from the repo root so asset paths resolve.

defaults: `buildtype=release` (-O3), thin lto, `-march=native`, `-fno-plt -fno-semantic-interposition
-fno-math-errno`, section gc + `--icf=all` at link. no `-ffast-math`, the game relies on inf/nan.
knobs: `-Dlto=full|thin|` `-Dmarch=x86-64-v3|` (native builds only run on cpus like yours) `-Dlinker=mold|`
`-Dmimalloc=false`. w/ muon use `MESON=muon AR=ar port/setup.sh`: muon 0.8.0-dev misdetects llvm-ar, and gnu ar
handles the lto bitcode fine through LLVMgold.so.

```sh
git submodule update --init
port/setup.sh && ninja -C port/build
port/build/assetconv                         # once (and after asset changes): GFX/SFX/... -> port/data, ~5s
meson test -C port/build
port/build/scpcb                             # the game (launcher -> loading screen so far)
port/build/rmview GFX/map/173_opt.rmesh     # wasd/qe + mouse, shift fast, f12 screenshot
port/build/rmview GFX/apache.b3d
```

by default it uses the system `libEGL`/`libGLESv2` (mesa). to run on ANGLE, point it at a dir with
ANGLE's `libEGL.so` + `libGLESv2.so` (chromium/electron ship them, or build angle w/ gn):

```sh
port/setup.sh -Dangle=/path/to/angle/out   # adds an rpath
# or without rebuilding:
LD_LIBRARY_PATH=/path/to/angle/out port/build/rmview ...
```

glfw dlopens `libEGL.so.1`, so if the angle build only has `libEGL.so` symlink it.
x11 only for now (works under xwayland). native wayland needs glfw's generated protocol headers.

dev hooks: `SCPCB_SHOT=out.png SCPCB_SHOT_FRAME=n` saves the nth Flip to a png and exits. `SCPCB_INPUT="11:c13,14:m250x225,15:l"`
plays input at given flips: `m<x>x<y>` virtual mouse move, `l` left click, `d` left button held this flip (drags), `k<scancode>` key hit, `c<ascii>` typed char.
eg `SCPCB_INPUT="11:c13,14:m250x365,15:l" SCPCB_SHOT=o.png SCPCB_SHOT_FRAME=22` gets past the loading screen and opens options at 720p. to try the game w/o
touching the repo's options.ini, run it from a dir that symlinks GFX, SFX, Data, Loadingscreens and has its own options.ini
(the launcher rewrites options.ini on LAUNCH, like the original).

## how the runtime works

- **2d**: SetBuffer on an image/texture lazily turns it into a render texture drawn w/ a y-flipped ortho,
  so row 0 stays the top row and blitz uvs keep working. images are masked like blitz (black = clear,
  alpha ignored) and get re-keyed after being drawn into. AAText.bb's sprite-camera font hack isn't
  needed: raylib text is antialiased already, so AAText -> draw_text_a (text w/ alpha). CopyRect and pixel
  ops go through cpu readback (TODO: glCopyTexSubImage2D for the per-frame Dreamfilter grab).
- **strings**: `str` is a plain `char *`, NULL reads as "". results of `cat`/`sf`/`s_*`/ini reads are temporaries from a
  frame arena, valid until the main loop's `tmp_reset()` (so through a whole load too). anything kept (fields, globals,
  tables) is an owned heap copy set w/ `sset(&field, value)`. `s_mid`/`s_find` use 0-based positions.
  debug: build w/ `-Dc_args=-DTMP_POISON` (plus `-Db_sanitize=address -Dmimalloc=false`) to make stale temps visible.
- **Types**: `typedef struct T { lnode n; ...fields } T;` + `TLIST(T)` (or `TLIST_DTOR(T, fn)` when it owns strings) in one .c.
  objects are calloc'd. `ldel` unlinks at once, the memory goes at the next `tl_flush()` (main loop, once a frame), so
  deleting while iterating is fine and `llive(p)` spots a dead object for the rest of that frame. refs kept across frames
  get cleared by the Remove* fns (RemoveNPC/Item/Event/Door/Waypoint/Room/SecurityCam). `lobject(T, h)` is NULL after delete.
- **assets**: the game only reads qoi (images) and qoa (sound), so raylib is built w/o its png/jpg/bmp/ogg decoders.
  tools/assetconv.c converts every asset once into `port/data/<path>.qoi|qoa` (full name kept, so foo.png and
  foo.jpg can't collide; files are recognised by content, the game hides jpgs as .dc/.pt/.it/.s and an ogg as
  .cwm). the runtime maps every image/sound load there (`bb_media`, SCPCB_DATA overrides the dir). originals stay as
  they are. qoi is lossless; the rgb/rgba channel count is kept since texture alpha depends on it. qoa is lossy
  (~3.2 bits/sample, transcoded from vorbis). dxt dds are decoded in the converter. sizes: ~207MB of originals
  -> ~508MB in port/data (qoa ~2x the oggs, qoi of jpgs is most of the rest).
- **textures**: LoadAnimTexture keeps one gl texture per frame, EntityTexture/BrushTexture pick it.
  ReadPixel outside a Lock reads a cached download (invalidated by any draw), so per-pixel loops stay fast.
- **game porting rules**: float -> int goes through `iround` (blitz rounds to nearest, c truncates), Min/Max are
  float like blitz, `Dim a(n)` is `a[n+1]`, And/Or/Xor share one precedence level (left to right)
  and `Not` is the lowest of all (blitz3d compiler/parser.cpp), globals are declared once w/ `G(type, names)` in g.h, function
  default args become explicit. closing the window exits in flip_frame, like blitz.
- **rng**: Rand/Rnd/SeedRnd match blitz3d's bbruntime/bbmath.cpp exactly (park-miller, same float mapping,
  default seed 0x1234; also checked against c++'s minstd_rand). note `Rnd(x)` is `Rnd(x, 0)` (= x - u*x)
  and `Rand(x)` is `Rand(x, 1)`, so ports keep those argument orders.
- **rng**: Rand/Rnd use blitz3d's generator (park-miller), but nothing tries to match blitz's call order any more, so seeds don't reproduce the original game's maps.
- **audio**: the game does its own 3d (PlaySound2/UpdateSoundOrigin compute vol + pan from DeltaYaw), so
  only the plain blitz api is needed. each PlaySound gets its own voice (a raylib sound alias, 256 of them; when all are busy the oldest
  one-shot is stolen, like fmod); channel
  handles carry a generation so stale ones go dead instead of steering a recycled voice. fmod streams
  map to raylib Music, fed in flip_frame. no audio device -> LoadSound still returns handles, everything's silent.
  all 894 referenced oggs convert (the 895th, SFX/SCP/970/fcveny.ogg, is a text-prefixed easter egg nobody loads).

- **coords**: everything stays in blitz's left-handed space. the projection matrix maps lh view space
  straight to gl clip space, and since blitz front faces are cw we cull gl "front" faces. no mirroring anywhere.
- **materials**: one shader does up to 4 texture layers, each w/ blitz TextureBlend (0-5), coord set,
  ScaleTexture/PositionTexture, masking. plus brush color/alpha, EntityFX (fullbright, vertex color,
  vertex alpha, nofog, 2-sided), up to 8 hw-style lights, linear fog.
- **cameras**: CameraProjMode 0 skips the camera, 2 is blitz's ortho (x*zoom/near across, like gxscene's
  setOrthoProj). the game's screen overlays (fog vignette, gas mask, nv, blur) sit on an ortho camera.
- **sorting**: per surface, not per entity. opaque near->far, alpha far->near w/o depth writes,
  EntityOrder != 0 draws w/o depth test.
- **textures**: resized to pow2 and mipmapped like blitz did (also needed for es2 wrap/mips).
  alpha flag w/o an alpha channel derives alpha from brightness, mask flag keys out pure black.
- **rooms**: `rmesh_load` follows `LoadRMesh` exactly (same child pivot layout, collision copies,
  AmbientLightRoomTex slot, FX 3 on the opaque mesh). game-side point entities come back as `rpoint` records.

## status

works: entities/hierarchy/transforms, b3d animation + skinning, sound + music streams, 2d, input, files, rng, meshes, brushes, textures, cameras, lights, sprites, fog,
collisions + picking, b3d + .x meshes and node trees (.x: text, binary, and single-block mszip "bzip"), rmesh rooms w/ props.
every .x in the repo loads. the only warnings are 2 broken asset paths (294.x wants a missing png, mug.x
points at a folder on the modder's desktop), and blitz chokes on those too.

game: hand-ported (no transpiler) into `src/game/`. done so far: Main.bb's top level up to the main loop, the main
loop's menu side, INI, StrictLoads, launcher, loading screens, the whole main menu, save/map lists, UpdateMusic,
KeyName, Difficulty, Achievements init, LoadEntities (most), room templates + rmesh rooms, CreateMap (the full map
generator), CreateRoom, CreateDoor, FillRoom, the player (MovePlayer/MouseLook/hud), UpdateRooms/UpdateDoors, waypoints +
FindPath, CreateNPC (every type) + UpdateNPCs w/ 173, 106, guards and class-d/clerks, InitEvents + UpdateEvents w/ 52
events (173 intro, alarm, pocket dimension, most of the plain room events, room2closets' quickload), decals, particles,
achievement popups, the inventory (UpdateItems, inv.c) + most item use (use.c), DrawMenu (pause/death screen, in-game
options, achievements, quit to menu).
START builds the facility from the seed and you walk it; 173 and 106 hunt you, 106's pocket dimension works. dev env for it: `SCPCB_SEED=x` fixed seed, `SCPCB_TOPDOWN=1` whole map from
above (ceilings are culled), `SCPCB_EV=name` prints that event's state once a second, `SCPCB_ROOM=name` start in the first room w/ that template name, `SCPCB_173=d` / `SCPCB_106=d` drop 173 / an already risen
106 d units in front of you after loading (note u start mid-blink, so 173 at 2 snaps ur neck on the first frames, like blitz would).
stubbed: CheckForUpdates (needs http), PlayStartupVideos (needs a video decoder), Dreamfilter,
borderless windowed + ScreenGamma (blitz did these w/ the fresize_* camera, not ported), ResizeImage2 is a plain ResizeImage,
TextureLodBias is a no-op (es2), "antialiased text" only flips the ini value, bump maps, LoadWorld (.b3d rooms, unused),
light cones, the 1499 chunks, loading saves, EntityAutoFade (1499-1 only), MeshCullBox (no per-entity culling here).
left to port (all hand-ported), roughly in this order:
- npc ai: 096, 049 + zombies, MTF (UpdateMTFUnit, TeleportMTFGroup, camera-check announcements), 939, 966, 860-2, 066,
  372, 513-1, 1048-a, 1499-1, 008, apache, tentacles; ManipulateNPCBones. most missing events need these first
- events: room106, room079, room035, room049, room205, room860, room966, room1123, room1162, room2test1074, room2storage,
  room2servers, room2sl, room2tunnel, room3storage, room3pit1048, room_gw, 914, 1048a, the 2nd select (096spawn,
  room3pit/room4pit/room3z2, 939-*_fix), Update096ElevatorEvent
- endings: exit1, gatea, UpdateEndings/DrawEnding, credits
- dimension1499 + its chunks, UpdateForest/DestroyForest
- items: radio, navigator, 1499, 294 (Using294 is a stub), 1025, Use914, Use427, the clipboard/wallet screen, dropping a
  half-worn vest/hazmat
- room stuff: keypad, SelectedScreen/UpdateScreens, security cams, UpdateRoomLights, UpdateDeafPlayer, devil emitters,
  the nvg "REFRESHING DATA" readout
- Save.bb: saving + loading (blocks Save & Quit, Load Game, quicksave)
- the console, the debug hud

known gaps: Animate's transition blending is ignored (game only uses SetAnimTime anyway), skinned
CopyEntity copies get their own vertex buffers (needed, each npc poses separately), collision response 3 (no sliding down slopes) is approximated, moving targets are tested
where they are now (not swept), CameraClsMode only does both-or-neither, light attenuation
is a guess (`1/(1+d/range)`), bump maps from materials.ini are skipped. .x SkinWeights (21 files) + AnimationSet (6) are skipped, so those render in bind pose.
