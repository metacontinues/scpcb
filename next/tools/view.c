// view: look at a baked room. run from the repo root: next/build/view [room id]   (default gfx/map/room2_opt)
// wasd + mouse fly, space/ctrl up/down, shift fast. f fog, g ambient 0/15, t trigger boxes, l lights, p waypoints
// env: VIEW_CAM="x y z tx ty tz" start camera, VIEW_SHOT=out.png + VIEW_FRAME=n screenshot frame n and quit
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "asset.h"
#include "room.h"
#include "shd.h"

int main(int argc, char **argv) {
	const char *id = argc > 1 ? argv[1] : "gfx/map/room2_opt";
	const char *shot = getenv("VIEW_SHOT");
	int shotf = getenv("VIEW_FRAME") ? atoi(getenv("VIEW_FRAME")) : 3;
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(1280, 720, "scpcb view");
	SetTargetFPS(60);
	shd_init();
	RoomMesh r;
	if (!room_load(&r, id)) { fprintf(stderr, "view: can't load %s\n", asset_path(id, ".*")); CloseWindow(); return 1; }
	printf("%s: %d meshes, %d mats, %d lights, %d waypoints, %d props, %d trigs, %d col meshes\n", id, r.vis.meshCount,
		r.vis.materialCount - 1, r.nlights, r.nwps, r.nprops, r.ntrigs, r.col.meshCount);

	// blitz's CameraZoom 1 = 90 deg across, raylib wants the vertical fov
	float fovy = 2 * atanf(tanf(45 * DEG2RAD) * 720 / 1280.f) * RAD2DEG;
	Camera3D cam = {{0, 1, 0}, {0, 1, -1}, {0, 1, 0}, fovy, CAMERA_PERSPECTIVE};
	if (getenv("VIEW_CAM")) sscanf(getenv("VIEW_CAM"), "%f %f %f %f %f %f", &cam.position.x, &cam.position.y,
		&cam.position.z, &cam.target.x, &cam.target.y, &cam.target.z);
	int fog = !getenv("VIEW_FOG") || atoi(getenv("VIEW_FOG")), amb = 0, trig = 0, lights = 0, wps = 0;
	if (!shot) DisableCursor();

	for (int frame = 0; !WindowShouldClose(); frame++) {
		if (!shot) UpdateCamera(&cam, CAMERA_FREE);
		if (IsKeyPressed(KEY_F)) fog ^= 1;
		if (IsKeyPressed(KEY_G)) amb ^= 1;
		if (IsKeyPressed(KEY_T)) trig ^= 1;
		if (IsKeyPressed(KEY_L)) lights ^= 1;
		if (IsKeyPressed(KEY_P)) wps ^= 1;
		shd_fog(fog ? 0.5f : 0, fog ? 6 : 0, BLACK);
		shd_ambient(amb ? 15 / 255.f : 0);

		BeginDrawing();
		ClearBackground(BLACK);
		BeginMode3D(cam);
		room_draw(&r, MatrixIdentity(), 0);
		BeginBlendMode(BLEND_ALPHA);
		rlDisableDepthMask();
		room_draw(&r, MatrixIdentity(), 1);
		rlEnableDepthMask();
		EndBlendMode();
		if (trig) for (int i = 0; i < r.ntrigs; i++) DrawBoundingBox(r.trigs[i].box, YELLOW);
		if (lights) for (int i = 0; i < r.nlights; i++) {
			DrawSphere(r.lights[i].p, 0.05f, r.lights[i].c);
			if (r.lights[i].spot) DrawLine3D(r.lights[i].p, Vector3Add(r.lights[i].p, Vector3Scale(r.lights[i].dir, 0.5f)), r.lights[i].c);
		}
		if (wps) for (int i = 0; i < r.nwps; i++) DrawCubeWires(r.wps[i], 0.1f, 0.1f, 0.1f, GREEN);
		EndMode3D();
		if (!shot) DrawText(TextFormat("%s  %d fps  fog %d amb %d  (%.2f %.2f %.2f)", id, GetFPS(), fog, amb,
			cam.position.x, cam.position.y, cam.position.z), 10, 10, 20, RAYWHITE);
		EndDrawing();
		if (shot && frame == shotf) { TakeScreenshot(shot); break; }
	}
	room_free(&r);
	tex_clear();
	CloseWindow();
	return 0;
}
