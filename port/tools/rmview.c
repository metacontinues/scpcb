// rmview: fly around a room (or any .b3d) using the bb runtime. run from repo root:
//   port/build/rmview GFX/map/lockroom_opt.rmesh
// wasd + mouse, shift = fast, f12 = screenshot, esc = quit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../src/bb/bb.h"
#include "../src/fmt/rmesh.h"
#include "raylib.h"
#include "rlgl.h"

// raylib's TakeScreenshot multiplies by dpi scale even when the fb isn't scaled
static void shot(const char *p) {
	int w = GetRenderWidth(), h = GetRenderHeight();
	Image im = {rlReadScreenPixels(w, h), w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	ExportImage(im, p);
	UnloadImage(im);
}

int main(int argc, char **argv) {
	if (argc < 2) { fprintf(stderr, "usage: rmview file.rmesh|file.b3d [shot.png [yaw]]\n"); return 1; }
	if (!bbGraphics3D(1280, 720, 0, 2)) return 1;
	bbAmbientLight(127, 127, 127);
	htex amb = bbCreateTexture(2, 2, TF_COLOR | TF_VRAM, 1);
	bbTextureBlend(amb, TB_MUL2);
	bbTextureFill(amb, 0, 0, 0, 255);

	ent cam = bbCreateCamera(0);
	bbCameraRange(cam, 0.05f, 60);
	bbCameraClsColor(cam, 20, 20, 28);
	rmesh rm = {0};
	size_t n = strlen(argv[1]);
	if (n > 6 && !strcasecmp(argv[1] + n - 6, ".rmesh")) {
		if (!rmesh_load(argv[1], amb, &rm)) return 1;
		bbScaleEntity(rm.obj, ROOMSCALE, ROOMSCALE, ROOMSCALE, 0);
		printf("%d point entities, %d trigger boxes\n", rm.npts, rm.ntrig);
		for (int i = 0; i < rm.ntrig; i++) bbEntityAlpha(rm.trig[i], 0); // game hides these (CheckTriggers)
		bbPositionEntity(cam, 0, 1, 0, 0);
		for (int i = 0; i < rm.npts; i++) if (rm.pts[i].kind == RP_WAYPOINT) { // a waypoint is a safe spot to start
			bbPositionEntity(cam, rm.pts[i].x, rm.pts[i].y + 1, rm.pts[i].z, 0);
			break;
		}
	} else {
		ent m = bbLoadMesh(argv[1], 0);
		if (!m) { fprintf(stderr, "load failed\n"); return 1; }
		float s = 4 / fmaxf(bbMeshWidth(m), fmaxf(bbMeshHeight(m), bbMeshDepth(m)));
		bbScaleEntity(m, s, s, s, 0);
		bbPositionEntity(cam, 0, 1, -5, 0);
	}
	DisableCursor();
	float yaw = 0, pitch = 0;
	int doshot = argc > 2, frames = 0;
	if (argc > 3) yaw = atof(argv[3]);
	while (!bbAppClosing() && !IsKeyPressed(KEY_ESCAPE)) {
		Vector2 md = frames++ > 2 ? GetMouseDelta() : (Vector2){0}; // first deltas are junk
		yaw -= md.x * 0.15f; pitch += md.y * 0.15f;
		pitch = fmaxf(-89, fminf(89, pitch));
		bbRotateEntity(cam, pitch, yaw, 0, 0);
		float sp = (IsKeyDown(KEY_LEFT_SHIFT) ? 8.0f : 2.0f) * GetFrameTime();
		bbMoveEntity(cam, (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * sp, (IsKeyDown(KEY_E) - IsKeyDown(KEY_Q)) * sp,
		             (IsKeyDown(KEY_W) - IsKeyDown(KEY_S)) * sp);
		bbRenderWorld(1);
		DrawText(TextFormat("%d fps  %d tris  pos %.2f %.2f %.2f", GetFPS(), bbTrisRendered(),
		         bbEntityX(cam, 1), bbEntityY(cam, 1), bbEntityZ(cam, 1)), 8, 8, 20, RAYWHITE);
		if (IsKeyPressed(KEY_F12)) shot("rmview.png");
		bbFlip(1);
		if (doshot && frames == 6) { shot(argv[2]); break; }
	}
	rmesh_free(&rm);
	bbEndGraphics();
	return 0;
}
