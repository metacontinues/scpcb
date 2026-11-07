# scpcb on raylib. plain meson syntax only (no native files, no b_* opts), so muon builds it too.
#   ./setup.sh && ninja -C build       (or: muon -C build samu)   configure w/ the llvm toolchain, build
#   ./setup.sh -Dangle=/path/to/angle/out    rpath angle's libEGL.so/libGLESv2.so so they win at runtime
#   build/bake                         (from the repo root) source assets -> next/data
project('scpcb-next', 'c', default_options: ['c_std=c99', 'buildtype=release', 'warning_level=2'])

cc = meson.get_compiler('c')
add_project_arguments('-Wno-unused-parameter', '-pedantic', language: 'c')
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

# same opt flags as port/
opt = ['-fno-plt', '-fno-semantic-interposition', '-fno-math-errno', '-ffunction-sections', '-fdata-sections']
optl = ['-Wl,-O2', '-Wl,--gc-sections', '-Wl,--as-needed']
if get_option('march') != ''
	opt += '-march=' + get_option('march')
endif
lto = get_option('lto')
if lto != ''
	opt += '-flto=' + lto
	optl += '-flto=' + lto
endif
if ld == 'lld'
	optl += '-Wl,--icf=all'
endif
add_project_arguments(cc.get_supported_arguments(opt), language: 'c')
add_project_link_arguments(optl, language: 'c')

# raylib twice: the game's (gles2 so it runs on angle, only the formats the game loads: glb, qoi, qoa, ttf)
# and the tools' (+ the source formats the baker reads). src/rlconfig*.h get force-included ahead of config.h
rl = 'vendor/raylib/src'
plat = ['-DPLATFORM_DESKTOP_GLFW', '-DGRAPHICS_API_OPENGL_ES2', '-D_GLFW_X11']
rldefs = plat + ['-include', meson.current_source_dir() / 'src/rlconfig.h']
tooldefs = plat + ['-include', meson.current_source_dir() / 'src/rlconfig_tool.h']
sys = [cc.find_library('m'), cc.find_library('dl'), cc.find_library('rt'), cc.find_library('X11'), dependency('threads')]
rlsrc = files(rl / 'rcore.c', rl / 'rshapes.c', rl / 'rtextures.c', rl / 'rtext.c', rl / 'rmodels.c',
	rl / 'raudio.c', rl / 'rglfw.c')
rlinc = include_directories(rl / 'external/glfw/include')
# our code is strict c99; the vendored libs need gnu extensions (glfw, mimalloc), so only they get gnu11
raylib = static_library('raylib', rlsrc, c_args: rldefs + ['-w'], include_directories: rlinc, override_options: ['c_std=gnu11'])
rltool = static_library('raylib_tool', rlsrc, c_args: tooldefs + ['-w'], include_directories: rlinc,
	override_options: ['c_std=gnu11'])

mi = []
if get_option('mimalloc')
	mi = static_library('mimalloc', 'vendor/mimalloc/src/static.c',
		c_args: ['-w', '-DNDEBUG', '-DMI_MALLOC_OVERRIDE=1'], include_directories: 'vendor/mimalloc/include', override_options: ['c_std=gnu11'])
endif

link = []
angle = get_option('angle')
if angle != ''
	link = ['-L' + angle, '-Wl,-rpath,' + angle]
endif

rlg = declare_dependency(link_with: raylib, link_whole: mi, dependencies: sys, compile_args: rldefs,
	include_directories: include_directories(rl, 'src'), link_args: link)
rlt = declare_dependency(link_with: rltool, link_whole: mi, dependencies: sys, compile_args: tooldefs,
	include_directories: include_directories(rl, 'src'), link_args: link)

executable('bake', files('tools/bake/bake.c', 'tools/bake/glb.c', 'tools/bake/img.c', 'tools/bake/room.c'),
	dependencies: rlt)
# the viewer runs the game's room code but links the tools' raylib so screenshots can be png
executable('view', files('tools/view.c', 'src/asset.c', 'src/room.c', 'src/shd.c'), dependencies: rlt)
