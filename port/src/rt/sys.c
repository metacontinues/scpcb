// sys.c: window, frame, timing, primitives
#include <stdlib.h>
#include "in.h"
#include "rlgl.h"

static int inframe;

void rt_frame(void) { if (!inframe) { BeginDrawing(); inframe = 1; } }

// can be called again (launcher -> game): keeps the window + gl context and
// just resizes it, so loaded stuff survives
int graphics_3d(int w, int h, int depth, int mode) {
	(void)depth;
	if (IsWindowReady()) {
		if (inframe) { EndDrawing(); inframe = 0; }
		if (IsWindowFullscreen() != (mode == 1)) ToggleFullscreen();
		SetWindowSize(w, h);
		return 1;
	}
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(mode == 1 ? FLAG_FULLSCREEN_MODE : 0);
	InitWindow(w, h, "scpcb");
	if (!IsWindowReady()) return 0;
	SetExitKey(KEY_NULL);
	rend_init();
	return 1;
}
void end_graphics(void) { snd_quit(); rend_quit(); CloseWindow(); }
int graphics_width(void) { return GetScreenWidth(); }
int graphics_height(void) { return GetScreenHeight(); }
// EndDrawing polls events. closing the window ends the program
// png writer for screenshots. no png decoder is built, but raylib always compiles stb_image_write
int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride);
int save_png(const char *p, int w, int h, const void *rgba) { return stbi_write_png(p, w, h, 4, rgba, w * 4); }

// dev hook: SCPCB_SHOT=out.png SCPCB_SHOT_FRAME=n grabs flip n to a png and exits
static void autoshot(void) {
	static int n; static const char *p; static int at = -1;
	if (at < 0) { p = getenv("SCPCB_SHOT"); const char *f = getenv("SCPCB_SHOT_FRAME"); at = f ? atoi(f) : 10; }
	if (!p || ++n != at) return;
	rlDrawRenderBatchActive();
	int w = GetRenderWidth(), h = GetRenderHeight();
	unsigned char *px = rlReadScreenPixels(w, h);
	save_png(p, w, h, px);
	exit(0);
}
void flip_frame(int vsync) {
	(void)vsync; rt_frame(); g2d_end(); autoshot(); EndDrawing(); inframe = 0; input_update(); snd_update();
	if (WindowShouldClose()) exit(0);
}

int milli_secs(void) { return (int)(GetTime() * 1000); }
int app_closing(void) { return WindowShouldClose(); }
int count_gfx_drivers(void) { return 1; }
const char *gfx_driver_name(int i) { return i == 1 ? "OpenGL ES 2 (raylib)" : ""; }
int app_focused(void) { return getenv("SCPCB_INPUT") || IsWindowFocused(); } // scripted runs never pause
void anti_alias(int on) { (void)on; } // msaa has to be picked before the window exists
int desktop_width(void) { return IsWindowReady() ? GetMonitorWidth(GetCurrentMonitor()) : 0; }
int desktop_height(void) { return IsWindowReady() ? GetMonitorHeight(GetCurrentMonitor()) : 0; }

// ---- primitives (cw front faces)
ent create_cube(ent par) {
	ent e = create_mesh(par);
	surf *s = create_surface(e, 0);
	static const float n[6][3] = {{0,0,-1},{1,0,0},{0,0,1},{-1,0,0},{0,1,0},{0,-1,0}};
	for (int f = 0; f < 6; f++) {
		v3 k = v3n(n[f][0], n[f][1], n[f][2]);
		v3 up = f < 4 ? v3n(0, 1, 0) : v3n(0, 0, f == 4 ? 1 : -1);
		v3 r = v3cross(k, up); // screen-right when facing the face, lh
		for (int i = 0; i < 4; i++) {
			float u = i == 1 || i == 2 ? 1 : -1, v = i < 2 ? 1 : -1;
			v3 p = v3add(v3add(k, v3mul(r, u)), v3mul(up, v));
			int x = add_vertex(s, p.x, p.y, p.z, (u+1)/2, (1-v)/2, 0);
			vertex_normal(s, x, k.x, k.y, k.z);
		}
		add_triangle(s, f*4, f*4+1, f*4+2); add_triangle(s, f*4, f*4+2, f*4+3);
	}
	return e;
}
ent create_sphere(int segs, ent par) {
	ent e = create_mesh(par);
	surf *s = create_surface(e, 0);
	if (segs < 2) segs = 8;
	int rs = segs, cs = segs * 2;
	for (int i = 0; i <= rs; i++) for (int j = 0; j <= cs; j++) {
		float th = PI * i / rs, ph = 2 * PI * j / cs;
		v3 p = v3n(sinf(th) * sinf(ph), cosf(th), sinf(th) * cosf(ph));
		int x = add_vertex(s, p.x, p.y, p.z, (float)j / cs, (float)i / rs, 0);
		vertex_normal(s, x, p.x, p.y, p.z);
	}
	for (int i = 0; i < rs; i++) for (int j = 0; j < cs; j++) {
		int a = i * (cs+1) + j, b = a + cs + 1;
		add_triangle(s, a, b+1, a+1); add_triangle(s, a, b, b+1);
	}
	return e;
}
ent create_plane(int segs, ent par) { // a big quad stands in for an infinite plane
	(void)segs;
	ent e = create_mesh(par);
	surf *s = create_surface(e, 0);
	float k = 1e4f;
	add_vertex(s, -k, 0, k, 0, 0, 0); add_vertex(s, k, 0, k, k, 0, 0);
	add_vertex(s, k, 0, -k, k, k, 0); add_vertex(s, -k, 0, -k, 0, k, 0);
	for (int i = 0; i < 4; i++) vertex_normal(s, i, 0, 1, 0);
	add_triangle(s, 0, 1, 2); add_triangle(s, 0, 2, 3);
	return e;
}
