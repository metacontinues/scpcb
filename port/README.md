# scpcb c port

c rewrite of scpcb on raylib, rendering through gles2 so it runs on [ANGLE](https://chromium.googlesource.com/angle/angle).

plan: write a blitz3d-alike runtime first (`src/bb`), then port the game `.bb` files on top of it
almost line by line. runtime fns are named after the blitz commands with a `bb` prefix (that's what
blitz3d's own runtime calls them): `PositionEntity e,x,y,z` -> `bbPositionEntity(e, x, y, z, 0)`.
handles are ints and 0 is null, same as blitz, so game code that stores entities in fields keeps working.

## layout

```
port/
  meson.build      meson build (plain syntax, muon works too), builds raylib + bdwgc + mimalloc from vendor/ too
  src/rlconfig.h   raylib's config.h trimmed to what the game loads (png/jpg/bmp, ogg, ttf)
  vendor/raylib    submodule, pinned to 5.5
  vendor/bdwgc     submodule, boehm gc v8.2.12 (for blitz strings + Type objects)
  vendor/mimalloc  submodule, v3.5.4, overrides malloc for the whole process (-Dmimalloc=false to skip)
  setup.sh         configures port/build w/ the llvm toolchain (clang, llvm-ar, lld)
  src/bb/          blitz runtime
    bb.h           public api, the only header game code should include
    in.h           internals shared w/ src/fmt
    m3.h           lh math (blitz conventions: yaw*pitch*roll, +z fwd)
    ent.c          entity pool, hierarchy, transforms, TForm*
    mesh.c         brushes, surfaces, mesh ops
    tex.c          textures + blitz flags, refcounted so FreeTexture on a used tex is fine
    rend.c         cameras, lights, sprites, RenderWorld
    shd.h          the fixed-function-ish glsl es 100 shader
    fs.c           blitz paths (backslashes, any case) + ReadInt/ReadString/etc
    sys.c          window/frame/time, primitives
    coll.c         Collisions/UpdateWorld (swept ellipsoid), picks, EntityVisible
    anim.c         keyframes (lerp + slerp per channel), cpu skinning, SetAnimTime/Animate/seqs
    snd.c          sounds/channels (raylib aliases) + music streams for the FSOUND_Stream_* calls
    g2d.c          buffers, images, Text/Rect/Line/Oval, fonts, CopyRect, Read/WritePixel
    input.c        keyboard (directinput scancodes) + mouse, hit/speed counters like blitz
    misc.c         blitz's exact rng, file writing, dirs, dates
    str.h/.c       blitz strings: gc'd char*, Left/Mid/Instr/Str/Int/... + cat(...)
    tl.h/.c        blitz Types: bbNew/bbDelete/each/First/After/Insert + Handle/Object
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
    entities.c     LoadEntities (most of it)
    keyname.c difficulty.c achv.c             the small .bb files
  tools/rmview.c   room/model viewer, the current test bed (tab = walk w/ the game's collider,
                   `rmview GFX/npcs/classd.b3d out.png 0 anim 200` poses an npc)
  tools/colltest.c collision + pick checks, `meson test -C port/build`
  tools/sndtest.c  sound/channel/stream checks (muted). w/o an audio device it only checks silent mode
  tools/g2dtest.c  every 2d path once + pixel io; `port/build/g2dtest out.png` to eyeball it
  tools/misctest.c rng + file io
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

dev hooks: `BB_SHOT=out.png BB_SHOT_FRAME=n` saves the nth Flip to a png and exits. `BB_INPUT="11:c13,14:m250x225,15:l"`
plays input at given flips: `m<x>x<y>` virtual mouse move, `l` left click, `k<scancode>` key hit, `c<ascii>` typed char.
eg `BB_INPUT="11:c13,14:m250x365,15:l" BB_SHOT=o.png BB_SHOT_FRAME=22` gets past the loading screen and opens options at 720p. to try the game w/o
touching the repo's options.ini, run it from a dir that symlinks GFX, SFX, Data, Loadingscreens and has its own options.ini
(the launcher rewrites options.ini on LAUNCH, like the original).

## how the blitz stuff maps

- **2d**: SetBuffer on an image/texture lazily turns it into a render texture drawn w/ a y-flipped ortho,
  so row 0 stays the top row and blitz uvs keep working. images are masked like blitz (black = clear,
  alpha ignored) and get re-keyed after being drawn into. AAText.bb's sprite-camera font hack isn't
  needed: raylib text is antialiased already, so AAText -> bbTextA (text w/ alpha). CopyRect and pixel
  ops go through cpu readback (TODO: glCopyTexSubImage2D for the per-frame Dreamfilter grab).
- **strings**: `str` is a gc'd `char *` (bdwgc), NULL reads as "". results are fresh, never freed.
  `cat(a, b, ...)` is blitz `+`, `bbStr`/`bbInt`/`bbFloat` dispatch on the arg type. gc pointers must sit in
  globals, locals, or gc memory (Type objects), never in malloc'd memory, or they get collected. Chr(0) can't
  exist in a c string, so the INI code that used it as a separator is rewritten w/ a line array.
- **Types**: `typedef struct T { lnode n; ...fields } T;` + `TLIST(T)` in one .c. objects are gc'd, Delete just
  unlinks, so dangling refs stay readable (blitz would error). `bbObject(T, h)` is Null after Delete, like blitz.
- **game porting rules**: float -> int goes through `bbInt` (blitz rounds to nearest, c truncates), Min/Max are
  float like blitz, `Dim a(n)` is `a[n+1]`, globals are declared once w/ `G(type, names)` in g.h, function
  default args become explicit. closing the window exits in bbFlip, like blitz.
- **rng**: Rand/Rnd/SeedRnd use blitz3d's park-miller generator. the step is checked against c++'s
  minstd_rand (10000th value). the float mapping + default seed 0x1234 are from blitz3d's source as i
  remember it, not checked against a real blitz build yet. worth verifying w/ a known map seed.
- **audio**: the game does its own 3d (PlaySound2/UpdateSoundOrigin compute vol + pan from DeltaYaw), so
  only the plain blitz api is needed. each PlaySound gets its own voice (a raylib sound alias); channel
  handles carry a generation so stale ones go dead instead of steering a recycled voice. fmod streams
  map to raylib Music, fed in bbFlip. no audio device -> LoadSound still returns handles, everything's silent.
  all 894 referenced oggs decode (the 895th, SFX/SCP/970/fcveny.ogg, is a text-prefixed easter egg nobody loads).

- **coords**: everything stays in blitz's left-handed space. the projection matrix maps lh view space
  straight to gl clip space, and since blitz front faces are cw we cull gl "front" faces. no mirroring anywhere.
- **materials**: one shader does up to 4 texture layers, each w/ blitz TextureBlend (0-5), coord set,
  ScaleTexture/PositionTexture, masking. plus brush color/alpha, EntityFX (fullbright, vertex color,
  vertex alpha, nofog, 2-sided), up to 8 hw-style lights, linear fog.
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
generator), CreateRoom, CreateDoor. START builds the facility from the seed and drops you in a stand-in fly cam
(wasd + mouse, esc back to the menu). dev env for it: `SCPCB_SEED=x` fixed seed, `SCPCB_TOPDOWN=1` whole map from
above (ceilings are culled), `SCPCB_DUMP=1` room list on stderr.
heads up: FillRoom's per-room part (the ~3400 line Select) isn't ported. it calls Rand too, so until it is, a seed
gives the same corridor layout as blitz but different room picks after the first special room.
stubbed: CheckForUpdates (needs http), PlayStartupVideos (needs a video decoder), Dreamfilter,
borderless windowed + ScreenGamma (blitz did these w/ the fresize_* camera, not ported), ResizeImage2 is a plain ResizeImage,
TextureLodBias is a no-op (es2), "antialiased text" only flips the ini value, bump maps, LoadWorld (.b3d rooms, unused),
light cones, the blur cam overlays, items/particles/NPC/event setup, loading saves.
next up: FillRoom's per-room cases (needs Items.bb's CreateItem, NPCs, emitters, levers, security cams), UpdateRooms/
UpdateDoors, then the player (MovePlayer/MouseLook) so the fly cam can go.

known gaps: Animate's transition blending is ignored (game only uses SetAnimTime anyway), skinned
CopyEntity copies get their own vertex buffers (needed, each npc poses separately), collision response 3 (no sliding down slopes) is approximated, moving targets are tested
where they are now (not swept), CameraClsMode only does both-or-neither, ortho proj mode is ignored, light attenuation
is a guess (`1/(1+d/range)`), bump maps from materials.ini are skipped. .x SkinWeights (21 files) + AnimationSet (6) are skipped, so those render in bind pose.
