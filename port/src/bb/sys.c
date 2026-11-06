// sys.c: window, frame, timing, primitives
#include <stdlib.h>
#include "in.h"
#include "rlgl.h"

static int inframe;

void bb_frame(void) { if (!inframe) { BeginDrawing(); inframe = 1; } }

// blitz lets you call this again (launcher -> game). we keep the window + gl context and
// just resize it, so loaded stuff survives (blitz would free it, the game reloads anyway)
int bbGraphics3D(int w, int h, int depth, int mode) {
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
void bbEndGraphics(void) { snd_quit(); rend_quit(); CloseWindow(); }
int bbGraphicsWidth(void) { return GetScreenWidth(); }
int bbGraphicsHeight(void) { return GetScreenHeight(); }
// EndDrawing polls events. closing the window ends the program, like blitz w/o a close prompt
// png writer for screenshots. no png decoder is built, but raylib always compiles stb_image_write
int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride);
int bbSavePNG(const char *p, int w, int h, const void *rgba) { return stbi_write_png(p, w, h, 4, rgba, w * 4); }

// dev hook: BB_SHOT=out.png BB_SHOT_FRAME=n grabs flip n to a png and exits
static void autoshot(void) {
	static int n; static const char *p; static int at = -1;
	if (at < 0) { p = getenv("BB_SHOT"); const char *f = getenv("BB_SHOT_FRAME"); at = f ? atoi(f) : 10; }
	if (!p || ++n != at) return;
	rlDrawRenderBatchActive();
	int w = GetRenderWidth(), h = GetRenderHeight();
	unsigned char *px = rlReadScreenPixels(w, h);
	bbSavePNG(p, w, h, px);
	exit(0);
}
void bbFlip(int vsync) {
	(void)vsync; bb_frame(); g2d_end(); autoshot(); EndDrawing(); inframe = 0; input_update(); snd_update();
	if (WindowShouldClose()) exit(0);
}

int bbMilliSecs(void) { return (int)(GetTime() * 1000); }
int bbAppClosing(void) { return WindowShouldClose(); }
int bbCountGfxDrivers(void) { return 1; }
const char *bbGfxDriverName(int i) { return i == 1 ? "OpenGL ES 2 (raylib)" : ""; }
void bbAntiAlias(int on) { (void)on; } // msaa has to be picked before the window exists
int bbDesktopWidth(void) { return IsWindowReady() ? GetMonitorWidth(GetCurrentMonitor()) : 0; }
int bbDesktopHeight(void) { return IsWindowReady() ? GetMonitorHeight(GetCurrentMonitor()) : 0; }

// ---- primitives (blitz layouts, cw front faces)
ent bbCreateCube(ent par) {
	ent e = bbCreateMesh(par);
	surf *s = bbCreateSurface(e, 0);
	static const float n[6][3] = {{0,0,-1},{1,0,0},{0,0,1},{-1,0,0},{0,1,0},{0,-1,0}};
	for (int f = 0; f < 6; f++) {
		v3 k = v3n(n[f][0], n[f][1], n[f][2]);
		v3 up = f < 4 ? v3n(0, 1, 0) : v3n(0, 0, f == 4 ? 1 : -1);
		v3 r = v3cross(k, up); // screen-right when facing the face, lh
		for (int i = 0; i < 4; i++) {
			float u = i == 1 || i == 2 ? 1 : -1, v = i < 2 ? 1 : -1;
			v3 p = v3add(v3add(k, v3mul(r, u)), v3mul(up, v));
			int x = bbAddVertex(s, p.x, p.y, p.z, (u+1)/2, (1-v)/2, 0);
			bbVertexNormal(s, x, k.x, k.y, k.z);
		}
		bbAddTriangle(s, f*4, f*4+1, f*4+2); bbAddTriangle(s, f*4, f*4+2, f*4+3);
	}
	return e;
}
ent bbCreateSphere(int segs, ent par) {
	ent e = bbCreateMesh(par);
	surf *s = bbCreateSurface(e, 0);
	if (segs < 2) segs = 8;
	int rs = segs, cs = segs * 2;
	for (int i = 0; i <= rs; i++) for (int j = 0; j <= cs; j++) {
		float th = PI * i / rs, ph = 2 * PI * j / cs;
		v3 p = v3n(sinf(th) * sinf(ph), cosf(th), sinf(th) * cosf(ph));
		int x = bbAddVertex(s, p.x, p.y, p.z, (float)j / cs, (float)i / rs, 0);
		bbVertexNormal(s, x, p.x, p.y, p.z);
	}
	for (int i = 0; i < rs; i++) for (int j = 0; j < cs; j++) {
		int a = i * (cs+1) + j, b = a + cs + 1;
		bbAddTriangle(s, a, b+1, a+1); bbAddTriangle(s, a, b, b+1);
	}
	return e;
}
ent bbCreatePlane(int segs, ent par) { // blitz planes are infinite; a big quad will do
	(void)segs;
	ent e = bbCreateMesh(par);
	surf *s = bbCreateSurface(e, 0);
	float k = 1e4f;
	bbAddVertex(s, -k, 0, k, 0, 0, 0); bbAddVertex(s, k, 0, k, k, 0, 0);
	bbAddVertex(s, k, 0, -k, k, k, 0); bbAddVertex(s, -k, 0, -k, 0, k, 0);
	for (int i = 0; i < 4; i++) bbVertexNormal(s, i, 0, 1, 0);
	bbAddTriangle(s, 0, 1, 2); bbAddTriangle(s, 0, 2, 3);
	return e;
}
