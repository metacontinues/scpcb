# scpcb c port. muon (or meson) + its built-in samurai:
#   muon setup build && muon -C build samu      build everything
#   muon -C build test                          run the tests
#   muon setup -Dangle=/path/to/angle/out build  rpath angle's libEGL.so/libGLESv2.so so they win at runtime
project('scpcb', 'c', default_options: ['c_std=gnu11', 'buildtype=debugoptimized', 'warning_level=2'])

cc = meson.get_compiler('c')
add_project_arguments('-Wno-unused-parameter', language: 'c')

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
bb = declare_dependency(link_with: [bblib, raylib, gc], dependencies: sys, compile_args: rldefs,
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
