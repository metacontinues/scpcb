// sys.c: window, frame, timing, primitives
#include <stdlib.h>
#include "in.h"

static int inframe;

void bb_frame(void) { if (!inframe) { BeginDrawing(); inframe = 1; } }

int bbGraphics3D(int w, int h, int depth, int mode) {
	(void)depth;
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
void bbFlip(int vsync) { (void)vsync; bb_frame(); g2d_end(); EndDrawing(); inframe = 0; input_update(); snd_update(); } // EndDrawing polled events

int bbMilliSecs(void) { return (int)(GetTime() * 1000); }
int bbAppClosing(void) { return WindowShouldClose(); }

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
