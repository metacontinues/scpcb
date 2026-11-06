# scpcb c port. muon (or meson) + its built-in samurai:
#   ./setup.sh && muon -C build samu            build everything (clang + mold)
#   muon -C build test                          run the tests
#   ./setup.sh -Dangle=/path/to/angle/out       rpath angle's libEGL.so/libGLESv2.so so they win at runtime
project('scpcb', 'c', default_options: ['c_std=gnu11', 'buildtype=debugoptimized', 'warning_level=2'])

cc = meson.get_compiler('c')
add_project_arguments('-Wno-unused-parameter', language: 'c')
if cc.get_id() != 'clang'
	warning('not clang (got ' + cc.get_id() + '), configure w/ ./setup.sh or CC=clang')
endif
ld = get_option('linker')
if ld != ''
	if not cc.has_link_argument('-fuse-ld=' + ld)
		error(ld + ' not usable as the linker; install it or pass -Dlinker= for the default')
	endif
	add_project_link_arguments('-fuse-ld=' + ld, language: 'c')
endif

rldefs = ['-DPLATFORM_DESKTOP_GLFW', '-DGRAPHICS_API_OPENGL_ES2', '-D_GLFW_X11',
	'-DSUPPORT_FILEFORMAT_JPG', '-DSUPPORT_FILEFORMAT_BMP']
sys = [cc.find_library('m'), cc.find_library('dl'), cc.find_library('rt'), cc.find_library('X11'), dependency('threads')]

# vendored libs, built quiet + optimized whatever the buildtype
rl = 'vendor/raylib/src'
raylib = static_library('raylib',
	files(rl / 'rcore.c', rl / 'rshapes.c', rl / 'rtextures.c', rl / 'rtext.c', rl / 'rmodels.c',
		rl / 'raudio.c', rl / 'utils.c', rl / 'rglfw.c'),
	c_args: rldefs + ['-w', '-O2'], include_directories: rl / 'external/glfw/include')
# bdwgc as one tu, single threaded (only the main thread touches gc memory)
gc = static_library('gc', 'vendor/bdwgc/extra/gc.c',
	c_args: ['-w', '-O2', '-DNO_EXECUTE_PERMISSION'], include_directories: 'vendor/bdwgc/include')

# mimalloc replaces malloc/free for the whole process: one tu w/ the override, linked whole so it wins over libc.
# bdwgc keeps its own heap (mmap), so the two don't overlap
mi = []
if get_option('mimalloc')
	mi = static_library('mimalloc', 'vendor/mimalloc/src/static.c',
		c_args: ['-w', '-O2', '-DNDEBUG', '-DMI_MALLOC_OVERRIDE=1'], include_directories: 'vendor/mimalloc/include')
endif

link = []
angle = get_option('angle')
if angle != ''
	link = ['-L' + angle, '-Wl,-rpath,' + angle]
endif

# blitz runtime + file formats. c_args carry rldefs so rlgl.h/config.h match the lib
bblib = static_library('bb',
	files('src/bb/ent.c', 'src/bb/mesh.c', 'src/bb/tex.c', 'src/bb/rend.c', 'src/bb/fs.c', 'src/bb/sys.c',
		'src/bb/coll.c', 'src/bb/anim.c', 'src/bb/snd.c', 'src/bb/g2d.c', 'src/bb/input.c', 'src/bb/misc.c',
		'src/bb/str.c', 'src/bb/tl.c', 'src/fmt/b3d.c', 'src/fmt/x.c', 'src/fmt/rmesh.c'),
	c_args: rldefs, include_directories: [rl, 'vendor/bdwgc/include'])
bb = declare_dependency(link_with: [bblib, raylib, gc], link_whole: mi, dependencies: sys, compile_args: rldefs,
	include_directories: include_directories(rl, 'vendor/bdwgc/include'), link_args: link)

executable('scpcb', files('src/game/achv.c', 'src/game/difficulty.c', 'src/game/ini.c', 'src/game/keyname.c',
	'src/game/loading.c', 'src/game/main.c', 'src/game/menu.c', 'src/game/strict.c', 'src/game/widgets.c'),
	dependencies: bb)

foreach t : ['rmview', 'colltest', 'sndtest', 'g2dtest', 'misctest', 'strtest']
	exe = executable(t, 'tools' / t + '.c', dependencies: bb)
	if t in ['colltest', 'misctest', 'strtest']
		test(t, exe)
	elif t == 'sndtest'
		test(t, exe, workdir: meson.project_source_root() / '..') # needs the repo's SFX
	endif
endforeach
