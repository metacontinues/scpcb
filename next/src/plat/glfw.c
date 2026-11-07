// glfw.c: glfw as one tu, same as raylib's rglfw.c (built instead of it), plus one fix: glfw's wayland backend
// only asks egl for EGL_PLATFORM_WAYLAND_EXT, and angle only takes wayland through EGL_PLATFORM_ANGLE_ANGLE w/
// the native platform set to wayland (glfw's x11 backend already does that dance). so wl_window.c's version gets
// renamed while it's included and ours below takes its place, falling back to it when egl isn't angle.
// vendored code: gnu11 + -w like the rest of raylib.
#if (defined(__linux__) || defined(PLATFORM_WEB)) && (_POSIX_C_SOURCE < 199309L)
	#undef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 199309L
#endif
#if (defined(__linux__) || defined(PLATFORM_WEB)) && !defined(_GNU_SOURCE)
	#define _GNU_SOURCE
#endif
#if defined(__linux__)
	#include <features.h>
	#if defined(__GLIBC__)
		#undef __GLIBC_USE_C2X_STRTOL
		#define __GLIBC_USE_C2X_STRTOL 0
		#undef __GLIBC_USE_C23_STRTOL
		#define __GLIBC_USE_C23_STRTOL 0
	#endif
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
	#define _GLFW_WIN32
#endif
#if defined(__linux__) && !defined(_GLFW_WAYLAND) && !defined(_GLFW_X11)
	#error "Cannot disable Wayland and X11 at the same time"
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	#define _GLFW_X11
#endif
#if defined(__APPLE__)
	#define _GLFW_COCOA
	#define _GLFW_USE_MENUBAR
	#define _GLFW_USE_RETINA
#endif

#include "external/glfw/src/init.c"
#include "external/glfw/src/platform.c"
#include "external/glfw/src/context.c"
#include "external/glfw/src/monitor.c"
#include "external/glfw/src/window.c"
#include "external/glfw/src/input.c"
#include "external/glfw/src/vulkan.c"
#include "external/glfw/src/egl_context.c"
#include "external/glfw/src/osmesa_context.c"

GLFWbool _glfwConnectNull(int platformID, _GLFWplatform *platform) { return GLFW_FALSE; } // raylib doesn't use it

#if defined(_WIN32) || defined(__CYGWIN__)
	#include "external/glfw/src/win32_init.c"
	#include "external/glfw/src/win32_module.c"
	#include "external/glfw/src/win32_monitor.c"
	#include "external/glfw/src/win32_window.c"
	#include "external/glfw/src/win32_joystick.c"
	#include "external/glfw/src/win32_time.c"
	#include "external/glfw/src/win32_thread.c"
	#include "external/glfw/src/wgl_context.c"
#endif

#if defined(__linux__)
	#include "external/glfw/src/posix_module.c"
	#include "external/glfw/src/posix_thread.c"
	#include "external/glfw/src/posix_time.c"
	#include "external/glfw/src/posix_poll.c"
	#include "external/glfw/src/linux_joystick.c"
	#include "external/glfw/src/xkb_unicode.c"
	#if defined(_GLFW_WAYLAND)
		#define createKeyTables createKeyTablesWayland
		#define translateKey translateKeyWayland
		#define acquireMonitor acquireMonitorWayland
		#define releaseMonitor releaseMonitorWayland
		#include "external/glfw/src/wl_init.c"
		#include "external/glfw/src/wl_monitor.c"
		#define _glfwGetEGLPlatformWayland glfwStockEGLPlatformWayland
		#include "external/glfw/src/wl_window.c"
		#undef _glfwGetEGLPlatformWayland
		#undef createKeyTables
		#undef translateKey
		#undef acquireMonitor
		#undef releaseMonitor

EGLenum _glfwGetEGLPlatformWayland(EGLint **attribs) {
	if (!_glfw.egl.ANGLE_platform_angle) return glfwStockEGLPlatformWayland(attribs);
	int type = 0, n = 0;
	if (_glfw.hints.init.angleType == GLFW_ANGLE_PLATFORM_TYPE_OPENGL && _glfw.egl.ANGLE_platform_angle_opengl)
		type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
	else if (_glfw.egl.ANGLE_platform_angle_vulkan) type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
	*attribs = _glfw_calloc(5, sizeof(EGLint));
	if (type) { (*attribs)[n++] = EGL_PLATFORM_ANGLE_TYPE_ANGLE; (*attribs)[n++] = type; }
	(*attribs)[n++] = EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE;
	(*attribs)[n++] = EGL_PLATFORM_WAYLAND_EXT;
	(*attribs)[n] = EGL_NONE;
	return EGL_PLATFORM_ANGLE_ANGLE;
}
	#endif
	#if defined(_GLFW_X11)
		#define createKeyTables createKeyTablesX11
		#define translateKey translateKeyX11
		#define acquireMonitor acquireMonitorX11
		#define releaseMonitor releaseMonitorX11
		#include "external/glfw/src/x11_init.c"
		#include "external/glfw/src/x11_monitor.c"
		#include "external/glfw/src/x11_window.c"
		#include "external/glfw/src/glx_context.c"
		#undef createKeyTables
		#undef translateKey
		#undef acquireMonitor
		#undef releaseMonitor
	#endif
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
	#include "external/glfw/src/posix_module.c"
	#include "external/glfw/src/posix_thread.c"
	#include "external/glfw/src/posix_time.c"
	#include "external/glfw/src/posix_poll.c"
	#include "external/glfw/src/null_joystick.c"
	#include "external/glfw/src/xkb_unicode.c"
	#include "external/glfw/src/x11_init.c"
	#include "external/glfw/src/x11_monitor.c"
	#include "external/glfw/src/x11_window.c"
	#include "external/glfw/src/glx_context.c"
#endif

#if defined(__APPLE__)
	#include "external/glfw/src/posix_module.c"
	#include "external/glfw/src/posix_thread.c"
	#include "external/glfw/src/cocoa_init.m"
	#include "external/glfw/src/cocoa_joystick.m"
	#include "external/glfw/src/cocoa_monitor.m"
	#include "external/glfw/src/cocoa_window.m"
	#include "external/glfw/src/macos_time.c"
	#include "external/glfw/src/nsgl_context.m"
#endif
