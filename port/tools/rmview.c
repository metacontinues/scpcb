// rmview: fly around a room (or any .b3d) using the bb runtime. run from repo root:
//   port/build/rmview GFX/map/lockroom_opt.rmesh
// wasd + mouse, shift = fast, tab = walk/fly, space = jump, f12 = screenshot, esc = quit
// rmview model.b3d shot.png yaw anim [frame]  -> LoadAnimMesh, plays (or poses at frame for the shot)
// scripted: rmview room shot.png yaw walk  -> walks forward 2s w/ collisions, prints where it ended up
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "../src/rt/rt.h"
#include "../src/fmt/rmesh.h"
#include "raylib.h"
#include "rlgl.h"

// raylib's TakeScreenshot multiplies by dpi scale even when the fb isn't scaled
static void shot(const char *p) {
	int w = GetRenderWidth(), h = GetRenderHeight();
	unsigned char *px = rlReadScreenPixels(w, h);
	save_png(p, w, h, px);
	RL_FREE(px);
}

// largest mesh extent in a node tree, for auto-scaling anim meshes
static float biggest(ent e) {
	float m = 0;
	if (!strcmp(entity_class(e), "Mesh") && count_surfaces(e))
		m = fmaxf(mesh_width(e), fmaxf(mesh_height(e), mesh_depth(e))) * entity_scale_x(e, 1);
	for (int i = 1; i <= count_children(e); i++) m = fmaxf(m, biggest(get_child(e, i)));
	return m;
}

int main(int argc, char **argv) {
	if (argc < 2) { fprintf(stderr, "usage: rmview file.rmesh|file.b3d [shot.png [yaw]]\n"); return 1; }
	if (!graphics3_d(1280, 720, 0, 2)) return 1;
	ambient_light(127, 127, 127);
	ent sun = create_light(1, 0); rotate_entity(sun, 40, 30, 0, 0); // so models aren't flat black
	htex amb = create_texture(2, 2, TF_COLOR | TF_VRAM, 1);
	texture_blend(amb, TB_MUL2);
	texture_fill(amb, 0, 0, 0, 255);

	ent cam = create_camera(0);
	camera_range(cam, 0.05f, 60);
	camera_cls_color(cam, 20, 20, 28);
	rmesh rm = {0};
	int anim = argc > 4 && !strcmp(argv[4], "anim");
	float frame = argc > 5 ? atof(argv[5]) : 0;
	ent model = 0;
	size_t n = strlen(argv[1]);
	if (n > 6 && !strcasecmp(argv[1] + n - 6, ".rmesh")) {
		if (!rmesh_load(argv[1], amb, &rm)) return 1;
		scale_entity(rm.obj, ROOMSCALE, ROOMSCALE, ROOMSCALE, 0);
		printf("%d point entities, %d trigger boxes\n", rm.npts, rm.ntrig);
		for (int i = 0; i < rm.ntrig; i++) entity_alpha(rm.trig[i], 0); // game hides these (CheckTriggers)
		position_entity(cam, 0, 1, 0, 0);
		for (int i = 0; i < rm.npts; i++) if (rm.pts[i].kind == RP_WAYPOINT) { // a waypoint is a safe spot to start
			position_entity(cam, rm.pts[i].x, rm.pts[i].y + 0.4f, rm.pts[i].z, 0);
			break;
		}
	} else if (anim) {
		model = load_anim_mesh(argv[1], 0);
		if (!model) { fprintf(stderr, "load failed\n"); return 1; }
		float sz = biggest(model);
		float s = sz > 0 ? 4 / sz : 1;
		scale_entity(model, s, s, s, 0);
		position_entity(cam, 0, 1, -5, 0);
		printf("%d frames\n", anim_length(model));
	} else {
		ent m = load_mesh(argv[1], 0);
		if (!m) { fprintf(stderr, "load failed\n"); return 1; }
		float s = 4 / fmaxf(mesh_width(m), fmaxf(mesh_height(m), mesh_depth(m)));
		scale_entity(m, s, s, s, 0);
		position_entity(cam, 0, 1, -5, 0);
	}
	// walk mode: same collider as the game (Main.bb: EntityRadius Collider, 0.15, 0.30)
	enum { HIT_MAP = 1, HIT_PLAYER = 2 };
	coll_rule(HIT_PLAYER, HIT_MAP, 2, 2);
	ent col = create_pivot(0);
	entity_radius(col, 0.15f, 0.30f);
	position_entity(col, entity_x(cam, 1), entity_y(cam, 1), entity_z(cam, 1), 1);
	entity_type(col, HIT_PLAYER, 0);
	int walk = argc > 4 && !strcmp(argv[4], "walk"), auto_ = walk;
	float drop = 0;
	DisableCursor();
	float yaw = 0, pitch = 0;
	int doshot = argc > 2, frames = 0;
	if (argc > 3) yaw = atof(argv[3]);
	while (!app_closing() && !IsKeyPressed(KEY_ESCAPE)) {
		Vector2 md = frames++ > 2 ? GetMouseDelta() : (Vector2){0}; // first deltas are junk
		yaw -= md.x * 0.15f; pitch += md.y * 0.15f;
		pitch = fmaxf(-89, fminf(89, pitch));
		rotate_entity(cam, pitch, yaw, 0, 0);
		float dt = auto_ ? 1 / 60.0f : GetFrameTime();
		if (IsKeyPressed(KEY_TAB)) { walk = !walk; position_entity(col, entity_x(cam, 1), entity_y(cam, 1), entity_z(cam, 1), 1); reset_entity(col); }
		float fw = IsKeyDown(KEY_W) - IsKeyDown(KEY_S) + auto_, sd = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
		if (walk) {
			float sp = (IsKeyDown(KEY_LEFT_SHIFT) ? 3.0f : 1.5f) * dt, y = yaw * DTOR;
			// blitz yaw: fwd = (-sin, 0, cos), right = (cos, 0, sin)
			translate_entity(col, (-sinf(y) * fw + cosf(y) * sd) * sp, 0, (cosf(y) * fw + sinf(y) * sd) * sp, 1);
			drop -= 9.8f * dt * 0.5f;
			if (IsKeyPressed(KEY_SPACE)) drop = 2.5f;
			translate_entity(col, 0, drop * dt, 0, 1);
			update_world(1);
			for (int i = 1; i <= count_collisions(col); i++) if (collision_ny(col, i) > 0.7f) drop = fmaxf(drop, 0); // landed
			position_entity(cam, entity_x(col, 1), entity_y(col, 1) + 0.25f, entity_z(col, 1), 1);
		} else {
			float sp = (IsKeyDown(KEY_LEFT_SHIFT) ? 8.0f : 2.0f) * dt;
			move_entity(cam, sd * sp, (IsKeyDown(KEY_E) - IsKeyDown(KEY_Q)) * sp, fw * sp);
		}
		if (model) { if (!doshot) frame += 30 * dt; if (anim_length(model)) frame = fmodf(frame, anim_length(model)); set_anim_time(model, frame, 0); }
		render_world(1);
		DrawText(TextFormat("%d fps  %d tris  pos %.2f %.2f %.2f", GetFPS(), tris_rendered(),
		         entity_x(cam, 1), entity_y(cam, 1), entity_z(cam, 1)), 8, 8, 20, RAYWHITE);
		if (IsKeyPressed(KEY_F12)) shot("rmview.png");
		flip_frame(1);
		if (doshot && frames == (auto_ ? 120 : 6)) { shot(argv[2]); break; }
	}
	if (auto_) printf("walked to %.3f %.3f %.3f, %d hits last frame\n", entity_x(col, 1), entity_y(col, 1), entity_z(col, 1), count_collisions(col));
	rmesh_free(&rm);
	end_graphics();
	return 0;
}
