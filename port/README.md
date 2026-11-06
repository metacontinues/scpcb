# scpcb c port

c rewrite of scpcb on raylib, rendering through gles2 so it runs on [ANGLE](https://chromium.googlesource.com/angle/angle).

plan: write a blitz3d-alike runtime first (`src/bb`), then port the game `.bb` files on top of it
almost line by line. runtime fns are named after the blitz commands with a `bb` prefix (that's what
blitz3d's own runtime calls them): `PositionEntity e,x,y,z` -> `bbPositionEntity(e, x, y, z, 0)`.
handles are ints and 0 is null, same as blitz, so game code that stores entities in fields keeps working.

## layout

```
port/
  Makefile         plain make, builds raylib from vendor/ too
  vendor/raylib    submodule, pinned to 5.5
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
  src/fmt/         file formats: b3d.c (+ LoadMesh dispatch), x.c, rmesh.c
  src/game/        game code goes here (empty rn)
  tools/rmview.c   room/model viewer, the current test bed (tab = walk w/ the game's collider,
                   `rmview GFX/npcs/classd.b3d out.png 0 anim 200` poses an npc)
  tools/colltest.c collision + pick checks, `make -C port test`
  tools/sndtest.c  sound/channel/stream checks (muted). w/o an audio device it only checks silent mode
  tools/g2dtest.c  every 2d path once + pixel io; `port/build/g2dtest out.png` to eyeball it
  tools/misctest.c rng + file io
```

## build + run

needs a c compiler, make, x11 dev headers. run stuff from the repo root so asset paths resolve.

```sh
git submodule update --init
make -C port
port/build/rmview GFX/map/173_opt.rmesh     # wasd/qe + mouse, shift fast, f12 screenshot
port/build/rmview GFX/apache.b3d
```

by default it uses the system `libEGL`/`libGLESv2` (mesa). to run on ANGLE, point it at a dir with
ANGLE's `libEGL.so` + `libGLESv2.so` (chromium/electron ship them, or build angle w/ gn):

```sh
make -C port ANGLE=/path/to/angle/out   # adds an rpath
# or without rebuilding:
LD_LIBRARY_PATH=/path/to/angle/out port/build/rmview ...
```

glfw dlopens `libEGL.so.1`, so if the angle build only has `libEGL.so` symlink it.
x11 only for now (works under xwayland). native wayland needs glfw's generated protocol headers.

## how the blitz stuff maps

- **2d**: SetBuffer on an image/texture lazily turns it into a render texture drawn w/ a y-flipped ortho,
  so row 0 stays the top row and blitz uvs keep working. images are masked like blitz (black = clear,
  alpha ignored) and get re-keyed after being drawn into. AAText.bb's sprite-camera font hack isn't
  needed: raylib text is antialiased already, so AAText -> bbTextA (text w/ alpha). CopyRect and pixel
  ops go through cpu readback (TODO: glCopyTexSubImage2D for the per-frame Dreamfilter grab).
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

next up: the game itself. Main.bb (12k), UpdateEvents (10k), MapSystem (8.7k), NPCs (7.5k), ...
the runtime covers the blitz commands those files use (see the usage counts in git history); the open
design questions are strings (blitz `$` is a gc'd value type) and blitz `Type` lists (New/Each/Delete/Before/After).

known gaps: Animate's transition blending is ignored (game only uses SetAnimTime anyway), skinned
CopyEntity copies get their own vertex buffers (needed, each npc poses separately), collision response 3 (no sliding down slopes) is approximated, moving targets are tested
where they are now (not swept), CameraClsMode only does both-or-neither, ortho proj mode is ignored, light attenuation
is a guess (`1/(1+d/range)`), bump maps from materials.ini are skipped. .x SkinWeights (21 files) + AnimationSet (6) are skipped, so those render in bind pose.
