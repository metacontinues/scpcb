// rmesh.c: port of LoadRMesh (MapSystem.bb)
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../rt/in.h"
#include "rmesh.h"

// GetTextureFromCache / CreatePropObj equivalents
typedef struct { char name[128]; htex t; } tcache;
static tcache *tcs; static int ntc, ctc;
typedef struct { char file[256]; ent e; } pcache;
static pcache *pcs; static int npc, cpc;

// clear_world took everything these point at
void rmesh_clear_cache(void) { ntc = npc = 0; }

static const char *strip(const char *p) {
	const char *b = p;
	for (; *p; p++) if (*p == '\\' || *p == '/') b = p + 1;
	return b;
}
static htex tc_get(const char *n) {
	for (int i = 0; i < ntc; i++) if (!strcasecmp(tcs[i].name, strip(n))) return tcs[i].t;
	return 0;
}
static void tc_add(htex t) {
	grow(tcs, ntc, ctc);
	snprintf(tcs[ntc].name, sizeof tcs[ntc].name, "%s", strip(texture_name(t)));
	tcs[ntc++].t = t;
}
static ent prop(const char *file) {
	for (int i = 0; i < npc; i++) if (!strcasecmp(pcs[i].file, file)) return copy_entity(pcs[i].e, 0);
	ent e = load_mesh(file, 0);
	if (!e) return 0;
	grow(pcs, npc, cpc);
	snprintf(pcs[npc].file, sizeof pcs[npc].file, "%s", file);
	pcs[npc++].e = e;
	hide_entity(e); // template stays hidden, hand out copies
	return copy_entity(e, 0);
}

static void rgeom(FILE *f, surf *s, int dbl) {
	int nv = read_int(f);
	for (int j = 0; j < nv; j++) { float x = read_float(f), y = read_float(f), z = read_float(f); add_vertex(s, x, y, z, 0, 0, 0); }
	int nt = read_int(f);
	for (int j = 0; j < nt; j++) {
		int a = read_int(f), b = read_int(f), c = read_int(f);
		add_triangle(s, a, b, c);
		if (dbl) add_triangle(s, a, c, b);
	}
}
static void piece(const char *s, int n, char *o, int on) { // Piece(s, n, " ")
	int i = 1;
	while (*s && i < n) if (*s++ == ' ') i++;
	int k = 0;
	while (*s && *s != ' ' && k < on - 1) o[k++] = *s++;
	o[k] = 0;
}
static float piecef(const char *s, int n) { char b[64]; piece(s, n, b, sizeof b); return atof(b); }
static char *rs(FILE *f, char *o, int n) { char *s = read_string(f); snprintf(o, n, "%s", s); return o; } // gc string, no free

int rmesh_load(const char *file, htex amb, rmesh *out) {
	*out = (rmesh){0};
	FILE *f = read_file(file);
	if (!f) { fprintf(stderr, "rmesh: can't open %s\n", file); return 0; }
	char hdr[64], dir[512], b1[256], b2[256];
	rs(f, hdr, sizeof hdr);
	int trig = !strcmp(hdr, "RoomMesh.HasTriggerBox");
	if (!trig && strcmp(hdr, "RoomMesh")) { fprintf(stderr, "rmesh: %s is not rmesh (%s)\n", file, hdr); fclose(f); return 0; }
	snprintf(dir, sizeof dir, "%.*s", (int)(strip(file) - file), file);

	htex blank = create_texture(4, 4, TF_COLOR, 1);
	texture_fill(blank, 255, 255, 255, 255);
	ent coll = create_pivot(0), opaque = create_mesh(0), alpha = create_mesh(0);

	int count = read_int(f);
	for (int i = 0; i < count; i++) {
		ent child = create_mesh(0);
		hbrush br = create_brush(255, 255, 255);
		htex tx[2] = {0, 0};
		int isa = 0;
		for (int j = 0; j < 2; j++) {
			int t = read_byte(f);
			if (!t) continue;
			rs(f, b1, sizeof b1);
			tx[j] = tc_get(b1);
			if (!tx[j]) {
				snprintf(b2, sizeof b2, "%s%s", dir, b1);
				tx[j] = load_texture(b2, t < 3 ? TF_COLOR | TF_MIP : TF_COLOR | TF_ALPHA | TF_MIP);
				if (tx[j]) {
					if (t == 1) texture_blend(tx[j], TB_MUL2);
					if (strcasestr(b1, "_lm")) texture_blend(tx[j], TB_ADD);
					tc_add(tx[j]);
				}
			}
			if (tx[j]) { isa = t == 3 ? 1 : 2; texture_coords(tx[j], 1 - j); }
		}
		if (isa == 1) {
			if (tx[1]) { texture_blend(tx[1], TB_MUL); brush_texture(br, tx[1], 0, 0); }
			else brush_texture(br, blank, 0, 0);
		} else if (tx[0] && tx[1]) {
			for (int j = 0; j < 2; j++) brush_texture(br, tx[j], 0, j + 1); // bump maps: TODO
			brush_texture(br, amb, 0, 0);
		} else for (int j = 0; j < 2; j++) brush_texture(br, tx[j] ? tx[j] : blank, 0, j);

		surf *s = create_surface(child, 0);
		if (isa > 0) paint_surface(s, br);
		free_brush(br);
		int nv = read_int(f);
		for (int j = 0; j < nv; j++) {
			float x = read_float(f), y = read_float(f), z = read_float(f);
			int v = add_vertex(s, x, y, z, 0, 0, 0);
			for (int k = 0; k < 2; k++) { float u = read_float(f), w = read_float(f); vertex_tex_coords(s, v, u, w, 0, k); }
			int r = read_byte(f), g = read_byte(f), b = read_byte(f);
			vertex_color(s, v, r, g, b, 1);
		}
		int nt = read_int(f);
		for (int j = 0; j < nt; j++) { int a = read_int(f), b = read_int(f), c = read_int(f); add_triangle(s, a, b, c); }

		if (isa == 1) { add_mesh(child, alpha); free_entity(child); continue; }
		add_mesh(child, opaque);
		entity_parent(child, coll, 1);
		entity_alpha(child, 0);
		entity_type(child, 1, 0); // HIT_MAP
		entity_pick_mode(child, 2, 1);
		ent fl = copy_mesh(child, 0); // double-sided collision
		flip_mesh(fl); add_mesh(fl, child); free_entity(fl);
	}

	ent hidden = create_mesh(0);
	count = read_int(f);
	for (int i = 0; i < count; i++) rgeom(f, create_surface(hidden, 0), 1);

	if (trig) {
		out->ntrig = read_int(f);
		for (int t = 0; t < out->ntrig; t++) {
			ent m = create_mesh(0);
			if (t < 32) out->trig[t] = m;
			count = read_int(f);
			for (int i = 0; i < count; i++) rgeom(f, create_surface(m, 0), 1);
			rs(f, b1, sizeof b1);
			if (t < 32) snprintf(out->trigname[t], 64, "%s", b1);
		}
		if (out->ntrig > 32) out->ntrig = 32;
	}

	count = read_int(f);
	int cap = 0;
	for (int i = 0; i < count; i++) {
		rs(f, b1, sizeof b1);
		rpoint p = {0};
		float S = ROOMSCALE;
		if (!strcmp(b1, "screen")) {
			p.kind = RP_SCREEN; p.x = read_float(f)*S; p.y = read_float(f)*S; p.z = read_float(f)*S;
			rs(f, p.str, sizeof p.str);
		} else if (!strcmp(b1, "waypoint")) {
			p.kind = RP_WAYPOINT; p.x = read_float(f)*S; p.y = read_float(f)*S; p.z = read_float(f)*S;
		} else if (!strcmp(b1, "light") || !strcmp(b1, "spotlight")) {
			p.kind = b1[0] == 'l' ? RP_LIGHT : RP_SPOT;
			p.x = read_float(f)*S; p.y = read_float(f)*S; p.z = read_float(f)*S;
			p.range = read_float(f) / 2000;
			rs(f, b2, sizeof b2);
			float in = fminf(read_float(f) * 0.8f, 1);
			p.r = roundf((int)piecef(b2, 1) * in); // rounds to nearest
			p.g = roundf((int)piecef(b2, 2) * in);
			p.b = roundf((int)piecef(b2, 3) * in);
			if (p.kind == RP_SPOT) {
				rs(f, b2, sizeof b2);
				p.pitch = piecef(b2, 1); p.yaw = piecef(b2, 2);
				p.inner = read_int(f); p.outer = read_int(f);
			}
		} else if (!strcmp(b1, "soundemitter")) {
			p.kind = RP_SOUND; p.x = read_float(f)*S; p.y = read_float(f)*S; p.z = read_float(f)*S;
			p.snd = read_int(f); p.range = read_float(f);
		} else if (!strcmp(b1, "playerstart")) {
			p.kind = RP_START; p.x = read_float(f); p.y = read_float(f); p.z = read_float(f);
			rs(f, b2, sizeof b2);
			p.pitch = piecef(b2, 1); p.yaw = piecef(b2, 2); p.roll = piecef(b2, 3);
		} else if (!strcmp(b1, "model")) {
			p.kind = RP_MODEL;
			rs(f, p.str, sizeof p.str);
			float v[9];
			for (int k = 0; k < 9; k++) v[k] = read_float(f);
			if (p.str[0]) {
				snprintf(b2, sizeof b2, "GFX\\Map\\Props\\%s", p.str);
				ent m = prop(b2);
				if (m) {
					position_entity(m, v[0], v[1], v[2], 0); rotate_entity(m, v[3], v[4], v[5], 0);
					scale_entity(m, v[6], v[7], v[8], 0);
					entity_parent(m, opaque, 1); entity_type(m, 1, 0); entity_pick_mode(m, 2, 1);
					p.ent = m;
				} else fprintf(stderr, "rmesh: prop %s failed\n", b2);
			}
			p.x = v[0]; p.y = v[1]; p.z = v[2];
		} else { fprintf(stderr, "rmesh: unknown point entity '%s', stopping\n", b1); break; }
		grow(out->pts, out->npts, cap);
		out->pts[out->npts++] = p;
	}
	fclose(f);

	ent fl = copy_mesh(alpha, 0); // alpha faces double-sided
	flip_mesh(fl); add_mesh(fl, alpha); free_entity(fl);
	add_mesh(alpha, opaque); free_entity(alpha);
	entity_fx(opaque, FX_FULLBRIGHT | FX_VCOLOR);
	entity_alpha(hidden, 0); entity_type(hidden, 1, 0);
	free_texture(blank);

	ent obj = create_pivot(0);
	create_pivot(obj); // slot 0 ("meshes"): the opaque mesh is child 1, the game looks it up by that index
	entity_parent(opaque, obj, 1); entity_parent(hidden, obj, 1);
	create_pivot(obj); create_pivot(obj);
	entity_parent(coll, obj, 1);
	for (int t = 0; t < out->ntrig; t++) entity_parent(out->trig[t], obj, 1);
	out->obj = obj; out->coll = coll; out->hidden = hidden;
	return 1;
}

void rmesh_free(rmesh *r) { if (r->obj) free_entity(r->obj); free(r->pts); *r = (rmesh){0}; }
