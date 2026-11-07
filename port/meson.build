# scpcb c port. plain meson syntax only (no native files, no b_* opts), so muon builds it too.
#   ./setup.sh && ninja -C build       (or: muon -C build samu)   configure w/ the llvm toolchain, build
#   meson test -C build                (or: muon -C build test)
#   ./setup.sh -Dangle=/path/to/angle/out    rpath angle's libEGL.so/libGLESv2.so so they win at runtime
project('scpcb', 'c', default_options: ['c_std=gnu11', 'buildtype=release', 'warning_level=2'])

cc = meson.get_compiler('c')
add_project_arguments('-Wno-unused-parameter', language: 'c')
if cc.get_id() != 'clang'
	warning('not clang (got ' + cc.get_id() + '), configure w/ ./setup.sh or CC=clang AR=llvm-ar')
endif
ld = get_option('linker')
if ld != ''
	if not cc.has_link_argument('-fuse-ld=' + ld)
		error(ld + ' not usable as the linker; install it or pass -Dlinker= for the default')
	endif
	add_project_link_arguments('-fuse-ld=' + ld, language: 'c')
endif

# optimizations, as plain flags. buildtype=release gives -O3. no -ffast-math: the game
# leans on inf/nan (Main.bb's INFINITY/NAN consts) and blitz's ieee float results
opt = ['-fno-plt', '-fno-semantic-interposition', '-fno-math-errno', '-ffunction-sections', '-fdata-sections']
optl = ['-Wl,-O2', '-Wl,--gc-sections', '-Wl,--as-needed']
if get_option('march') != ''
	opt += '-march=' + get_option('march')
endif
lto = get_option('lto')
if lto != ''
	opt += '-flto=' + lto # static libs then hold bitcode: AR has to be llvm-ar
	optl += '-flto=' + lto
endif
if ld == 'lld'
	optl += '-Wl,--icf=all'
endif
add_project_arguments(cc.get_supported_arguments(opt), language: 'c')
add_project_link_arguments(optl, language: 'c')

# src/rlconfig.h overrides raylib's config.h flags (only the formats the game loads), force-included ahead of it
rldefs = ['-DPLATFORM_DESKTOP_GLFW', '-DGRAPHICS_API_OPENGL_ES2', '-D_GLFW_X11',
	'-include', meson.current_source_dir() / 'src/rlconfig.h']
sys = [cc.find_library('m'), cc.find_library('dl'), cc.find_library('rt'), cc.find_library('X11'), dependency('threads')]

# vendored libs, built quiet
rl = 'vendor/raylib/src'
raylib = static_library('raylib',
	files(rl / 'rcore.c', rl / 'rshapes.c', rl / 'rtextures.c', rl / 'rtext.c', rl / 'rmodels.c',
		rl / 'raudio.c', rl / 'rglfw.c'),
	c_args: rldefs + ['-w'], include_directories: rl / 'external/glfw/include')

# mimalloc replaces malloc/free for the whole process: one tu w/ the override, linked whole so it wins over libc.
mi = []
if get_option('mimalloc')
	mi = static_library('mimalloc', 'vendor/mimalloc/src/static.c',
		c_args: ['-w', '-DNDEBUG', '-DMI_MALLOC_OVERRIDE=1'], include_directories: 'vendor/mimalloc/include')
endif

link = []
angle = get_option('angle')
if angle != ''
	link = ['-L' + angle, '-Wl,-rpath,' + angle]
endif

# blitz runtime + file formats. c_args carry rldefs so rlgl.h/config.h match the lib
rtlib = static_library('rt',
	files('src/rt/ent.c', 'src/rt/mesh.c', 'src/rt/tex.c', 'src/rt/rend.c', 'src/rt/fs.c', 'src/rt/sys.c',
		'src/rt/coll.c', 'src/rt/anim.c', 'src/rt/snd.c', 'src/rt/g2d.c', 'src/rt/input.c', 'src/rt/misc.c',
		'src/rt/str.c', 'src/rt/tl.c', 'src/fmt/b3d.c', 'src/fmt/x.c', 'src/fmt/rmesh.c'),
	c_args: rldefs, include_directories: rl)
rt = declare_dependency(link_with: [rtlib, raylib], link_whole: mi, dependencies: sys, compile_args: rldefs,
	include_directories: include_directories(rl), link_args: link)

executable('scpcb', files(
	'src/game/achv.c', 'src/game/blur.c', 'src/game/difficulty.c', 'src/game/doors.c', 'src/game/entities.c',
	'src/game/fillroom1.c', 'src/game/fillroom2.c', 'src/game/fillroom3.c', 'src/game/forest.c',
	'src/game/fx.c', 'src/game/hud.c', 'src/game/ini.c', 'src/game/items.c', 'src/game/keyname.c',
	'src/game/loading.c', 'src/game/loop.c', 'src/game/main.c', 'src/game/mainmenu.c', 'src/game/mapgen.c',
	'src/game/menu.c', 'src/game/music.c', 'src/game/newgame.c', 'src/game/options.c', 'src/game/player.c', 'src/game/npcs.c', 'src/game/npc173.c', 'src/game/npcguard.c', 'src/game/waypoints.c', 'src/game/events.c', 'src/game/evlib.c', 'src/game/ev173.c', 'src/game/ev1.c', 'src/game/ev2.c', 'src/game/evpd.c', 'src/game/ev3.c', 'src/game/ev4.c', 'src/game/inv.c', 'src/game/use.c',
	'src/game/rooms.c', 'src/game/save.c', 'src/game/sliders.c', 'src/game/sounds.c', 'src/game/strict.c',
	'src/game/tooltip.c', 'src/game/widgets.c'),
	dependencies: rt)

# assets -> qoi/qoa (port/data). has its own decoders (stb_image, stb_vorbis), the game doesn't
executable('assetconv', 'tools/assetconv.c', c_args: ['-w'], include_directories: rl, dependencies: cc.find_library('m'))

foreach t : ['rmview', 'colltest', 'sndtest', 'g2dtest', 'misctest', 'strtest']
	exe = executable(t, 'tools' / t + '.c', dependencies: rt)
	if t in ['colltest', 'misctest', 'strtest']
		test(t, exe)
	elif t == 'sndtest'
		test(t, exe, workdir: meson.project_source_root() / '..') # needs the repo's SFX
	endif
endforeach
