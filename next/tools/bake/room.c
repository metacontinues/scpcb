// room.c: .rmesh -> <id>.glb (what you see), <id>.col.glb (what you bump into), <id>.txt (everything else).
// coords leave here in game units, right-handed (z flipped, so triangle order flips too).
//
// the .txt sidecar, one record per line, '#' comments:
//   mat i kind diff lm          gltf material i (raylib: model.materials[i + 1]); '-' = no texture.
//                               kind lit:   (vcolor * ambient * 2 + lm(uv1)) * diff(uv0) * 2
//                                    vert:  vcolor * diff * 2
//                                    flat:  vcolor
//                                    alpha: vcolor * diff, diff's alpha, double-sided, drawn sorted
//   light x y z range r g b
//   spot x y z range r g b dx dy dz inner outer
//   screen x y z img
//   waypoint x y z
//   sound x y z id range
//   start x y z dx dy dz
//   prop model m0 m4 m8 m12 m1 m5 m9 m13 m2 m6 m10 m14 m3 m7 m11 m15   (raylib Matrix field order, room space)
//   trig name minx miny minz maxx maxy maxz   (room space)
#define _POSIX_C_SOURCE 200809L
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "bake.h"

enum { K_LIT, K_VERT, K_FLAT, K_ALPHA };
static const char *kname[] = {"lit", "vert", "flat", "alpha"};

typedef struct { float p[3], t0[2], t1[2]; uint8_t c[4]; } vtx;
typedef struct { vtx *v; int nv, cv; uint16_t *ix; int ni, ci; } chunk;
typedef struct { int kind; char diff[256], lm[256]; chunk *ch; int nch; } group;

static group *gs; static int ngs;
static group col;

#define GROW(p, n, c) do { if ((n) >= (c)) { (c) = (c) ? (c) * 2 : 256; (p) = realloc((p), sizeof *(p) * (c)); } } while (0)

static group *grp(int kind, const char *diff, const char *lm) {
	for (int i = 0; i < ngs; i++)
		if (gs[i].kind == kind && !strcmp(gs[i].diff, diff) && !strcmp(gs[i].lm, lm)) return &gs[i];
	gs = realloc(gs, sizeof *gs * (ngs + 1));
	group *g = &gs[ngs++];
	*g = (group){.kind = kind};
	snprintf(g->diff, sizeof g->diff, "%s", diff);
	snprintf(g->lm, sizeof g->lm, "%s", lm);
	return g;
}

// a surface's verts + tris into g, flipped copies too when dbl. raylib meshes index w/ u16
static void put(group *g, const vtx *v, int nv, const int *t, int nt, int dbl) {
	if (nv > 65535) { warn("surface w/ %d verts, dropped", nv); return; }
	if (!g->nch || g->ch[g->nch - 1].nv + nv > 65535) { g->ch = realloc(g->ch, sizeof *g->ch * (g->nch + 1)); g->ch[g->nch++] = (chunk){0}; }
	chunk *c = &g->ch[g->nch - 1];
	int b = c->nv;
	for (int i = 0; i < nv; i++) { GROW(c->v, c->nv, c->cv); c->v[c->nv++] = v[i]; }
	for (int i = 0; i < nt; i++) {
		int a = t[i * 3], bb = t[i * 3 + 1], cc = t[i * 3 + 2];
		if (a < 0 || bb < 0 || cc < 0 || a >= nv || bb >= nv || cc >= nv) { warn("bad triangle"); continue; }
		int tri[2][3] = {{a, cc, bb}, {a, bb, cc}}; // flipped z -> flipped winding; [1] = back side
		for (int s = 0; s <= dbl; s++)
			for (int k = 0; k < 3; k++) { GROW(c->ix, c->ni, c->ci); c->ix[c->ni++] = b + tri[s][k]; }
	}
}

static FILE *f;
static int ri(void) { int v = 0; if (fread(&v, 4, 1, f) != 1) v = 0; return v; }
static float rf(void) { float v = 0; if (fread(&v, 4, 1, f) != 1) v = 0; return v; }
static int rb(void) { int c = fgetc(f); return c < 0 ? 0 : c; }
static char *rs(char *o, int n) {
	int l = ri();
	if (l < 0 || l > 1 << 16) l = 0;
	int k = 0;
	for (int i = 0; i < l; i++) { int c = rb(); if (k < n - 1) o[k++] = c; }
	o[k] = 0;
	return o;
}
static void rp(float *p) { float x = rf(), y = rf(), z = rf(); p[0] = x * ROOMSCALE; p[1] = y * ROOMSCALE; p[2] = -z * ROOMSCALE; }

// plain geometry block (hidden walls, trigger boxes): positions + tris only
static void geom(vtx **v, int *nv, int **t, int *nt) {
	*nv = ri();
	*v = calloc(*nv > 0 ? *nv : 1, sizeof **v);
	for (int i = 0; i < *nv; i++) rp((*v)[i].p);
	*nt = ri();
	*t = calloc(*nt > 0 ? *nt * 3 : 1, sizeof **t);
	for (int i = 0; i < *nt * 3; i++) (*t)[i] = ri();
}

static void freegroup(group *g) {
	for (int i = 0; i < g->nch; i++) { free(g->ch[i].v); free(g->ch[i].ix); }
	free(g->ch);
}

static int writeglb(const char *path, group *g, int n, int render) {
	glb b = {0};
	for (int i = 0; i < n; i++) {
		int mat = render ? gb_mat(&b, kname[g[i].kind]) : -1;
		gbprim *ps = calloc(g[i].nch, sizeof *ps);
		for (int c = 0; c < g[i].nch; c++) {
			chunk *ch = &g[i].ch[c];
			float *p = malloc(sizeof(float) * 3 * ch->nv), *t0 = malloc(sizeof(float) * 2 * ch->nv), *t1 = malloc(sizeof(float) * 2 * ch->nv);
			uint8_t *cl = malloc(4 * ch->nv);
			for (int k = 0; k < ch->nv; k++) {
				memcpy(p + k * 3, ch->v[k].p, 12); memcpy(t0 + k * 2, ch->v[k].t0, 8); memcpy(t1 + k * 2, ch->v[k].t1, 8);
				memcpy(cl + k * 4, ch->v[k].c, 4);
			}
			ps[c] = gb_prim0();
			ps[c].pos = gb_vec3(&b, p, ch->nv); ps[c].idx = gb_idx16(&b, ch->ix, ch->ni); ps[c].mat = mat;
			if (render) {
				ps[c].uv0 = gb_vec2(&b, t0, ch->nv);
				if (g[i].kind == K_LIT) ps[c].uv1 = gb_vec2(&b, t1, ch->nv);
				ps[c].col = gb_rgba8(&b, cl, ch->nv);
			}
			free(p); free(t0); free(t1); free(cl);
		}
		if (g[i].nch) { char j[32]; snprintf(j, sizeof j, "\"mesh\":%d", gb_mesh(&b, ps, g[i].nch)); gb_node(&b, 1, j); }
		free(ps);
	}
	return gb_write(&b, path);
}

// blitz's RotateEntity order (yaw * pitch * roll, left-handed), rows
static void rot(float p, float y, float r, float m[3][3]) {
	p *= D2R; y *= D2R; r *= D2R;
	float cp = cosf(p), sp = sinf(p), cy = cosf(y), sy = sinf(y), cr = cosf(r), sr = sinf(r);
	float Y[3][3] = {{cy, 0, -sy}, {0, 1, 0}, {sy, 0, cy}}, P[3][3] = {{1, 0, 0}, {0, cp, -sp}, {0, sp, cp}},
	      R[3][3] = {{cr, -sr, 0}, {sr, cr, 0}, {0, 0, 1}}, T[3][3];
	for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) { T[i][j] = 0; for (int k = 0; k < 3; k++) T[i][j] += Y[i][k] * P[k][j]; }
	for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) { m[i][j] = 0; for (int k = 0; k < 3; k++) m[i][j] += T[i][k] * R[k][j]; }
}

static void piece(const char *s, float *o, int n) { // "a b c" -> floats
	for (int i = 0; i < n; i++) { char *e; o[i] = strtof(s, &e); s = e; }
}

int bake_room(const char *path) {
	char id[1024], out[4096], dir[4096], b1[512], b2[512];
	mkid(path, id, sizeof id);
	snprintf(out, sizeof out, OUT "%s.txt", id);
	if (!stale(path, out)) return 1;
	if (!(f = fopen(path, "rb"))) { warn("can't open %s", path); return 0; }
	snprintf(dir, sizeof dir, "%s", path);
	char *sl = strrchr(dir, '/');
	sl ? (sl[1] = 0) : (dir[0] = 0);
	rs(b1, sizeof b1);
	int hastrig = !strcmp(b1, "RoomMesh.HasTriggerBox");
	if (!hastrig && strcmp(b1, "RoomMesh")) { warn("%s: not an rmesh (%s)", path, b1); fclose(f); return 0; }

	sb txt = {0};
	sbf(&txt, "# %s\n", path);
	int count = ri();
	for (int i = 0; i < count; i++) {
		int t[2]; char name[2][512]; const char *tid[2] = {0, 0}; char tidb[2][1024];
		for (int j = 0; j < 2; j++) {
			t[j] = rb(); name[j][0] = 0;
			if (!t[j]) continue;
			rs(name[j], sizeof name[j]);
			if (!name[j][0]) continue;
			snprintf(b2, sizeof b2, "%s%s", dir, name[j]);
			const char *r = bake_tex(b2, t[j] == 3 ? IM_ALPHA : IM_OPAQUE);
			if (!r) { warn("%s: texture %s missing", path, name[j]); continue; }
			snprintf(tidb[j], sizeof tidb[j], "%s", r);
			tid[j] = tidb[j];
		}
		// what LoadRMesh ends up drawing, by which slots loaded (slot 0 = lightmap on uv1, slot 1 = diffuse on uv0)
		int kind = K_FLAT;
		const char *diff = "-", *lm = "-";
		int alpha = (tid[0] && t[0] == 3) || (tid[1] && t[1] == 3);
		if (tid[1] && t[1] == 3) { kind = K_ALPHA; diff = tid[1]; }
		else if (alpha) kind = K_ALPHA; // alpha in slot 0 only: blitz painted it blank white
		else if (tid[0] && tid[1]) {
			kind = K_LIT; lm = tid[0]; diff = tid[1];
			if (!hasci(name[0], "_lm") || t[1] != 1) warn("%s: unusual lit surface (%s %d, %s %d)", path, name[0], t[0], name[1], t[1]);
		} else if (tid[1]) {
			kind = K_VERT; diff = tid[1];
			if (t[1] != 1) warn("%s: vert surface w/ slot type %d", path, t[1]);
		} else if (tid[0]) warn("%s: lightmap %s w/o a diffuse, drawn flat", path, name[0]);

		int nv = ri();
		vtx *v = calloc(nv > 0 ? nv : 1, sizeof *v);
		for (int k = 0; k < nv; k++) {
			rp(v[k].p);
			for (int s = 0; s < 2; s++) { float u = rf(), w = rf(); float *o = s ? v[k].t1 : v[k].t0; o[0] = u; o[1] = w; }
			v[k].c[0] = rb(); v[k].c[1] = rb(); v[k].c[2] = rb(); v[k].c[3] = 255;
		}
		int nt = ri();
		int *tr = calloc(nt > 0 ? nt * 3 : 1, sizeof *tr);
		for (int k = 0; k < nt * 3; k++) tr[k] = ri();
		put(grp(kind, diff, lm), v, nv, tr, nt, kind == K_ALPHA);
		if (kind != K_ALPHA) put(&col, v, nv, tr, nt, 0); // collision is double-sided by definition, no copies
		free(v); free(tr);
	}

	count = ri();
	for (int i = 0; i < count; i++) {
		vtx *v; int nv, *tr, nt;
		geom(&v, &nv, &tr, &nt);
		put(&col, v, nv, tr, nt, 0);
		free(v); free(tr);
	}

	if (hastrig) {
		int n = ri();
		for (int i = 0; i < n; i++) {
			float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
			int ns = ri();
			for (int s = 0; s < ns; s++) {
				vtx *v; int nv, *tr, nt;
				geom(&v, &nv, &tr, &nt);
				for (int k = 0; k < nv; k++) for (int a = 0; a < 3; a++) { mn[a] = fminf(mn[a], v[k].p[a]); mx[a] = fmaxf(mx[a], v[k].p[a]); }
				free(v); free(tr);
			}
			rs(b1, sizeof b1);
			for (char *c = b1; *c; c++) if (*c == ' ') *c = '_';
			sbf(&txt, "trig %s %g %g %g %g %g %g\n", b1, mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]);
		}
	}

	count = ri();
	for (int i = 0; i < count; i++) {
		float p[3];
		rs(b1, sizeof b1);
		if (!strcmp(b1, "screen")) {
			rp(p); rs(b2, sizeof b2); mkid(b2, b1, sizeof b1);
			sbf(&txt, "screen %g %g %g %s\n", p[0], p[1], p[2], b1);
		} else if (!strcmp(b1, "waypoint")) {
			rp(p); sbf(&txt, "waypoint %g %g %g\n", p[0], p[1], p[2]);
		} else if (!strcmp(b1, "light") || !strcmp(b1, "spotlight")) {
			int spot = b1[0] == 's';
			rp(p);
			float range = rf() / 2000, c[3];
			rs(b2, sizeof b2); piece(b2, c, 3);
			float in = fminf(rf() * 0.8f, 1);
			int r = roundf((int)c[0] * in), g = roundf((int)c[1] * in), bl = roundf((int)c[2] * in);
			if (!spot) { sbf(&txt, "light %g %g %g %g %d %d %d\n", p[0], p[1], p[2], range, r, g, bl); continue; }
			float a[3], m[3][3];
			rs(b2, sizeof b2); piece(b2, a, 2);
			int inner = ri(), outer = ri();
			rot(a[0], a[1], 0, m);
			sbf(&txt, "spot %g %g %g %g %d %d %d %g %g %g %d %d\n", p[0], p[1], p[2], range, r, g, bl, m[0][2], m[1][2], -m[2][2], inner, outer);
		} else if (!strcmp(b1, "soundemitter")) {
			rp(p);
			int snd = ri(); float range = rf();
			sbf(&txt, "sound %g %g %g %d %g\n", p[0], p[1], p[2], snd, range);
		} else if (!strcmp(b1, "playerstart")) {
			rp(p);
			float a[3], m[3][3];
			rs(b2, sizeof b2); piece(b2, a, 3);
			rot(a[0], a[1], a[2], m);
			sbf(&txt, "start %g %g %g %g %g %g\n", p[0], p[1], p[2], m[0][2], m[1][2], -m[2][2]);
		} else if (!strcmp(b1, "model")) {
			rs(b2, sizeof b2);
			float v[9];
			for (int k = 0; k < 9; k++) v[k] = rf();
			if (!b2[0]) continue;
			char mid[1024], mp[1024];
			snprintf(mp, sizeof mp, "GFX/Map/Props/%s", b2);
			mkid(mp, mid, sizeof mid);
			// room space = ROOMSCALE * (T * R * S), then mirrored: F M F, F = diag(1, 1, -1)
			float m[3][3];
			rot(v[3], v[4], v[5], m);
			for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) m[r][c] *= v[6 + c] * ROOMSCALE * ((r == 2) != (c == 2) ? -1 : 1);
			float t[3] = {v[0] * ROOMSCALE, v[1] * ROOMSCALE, -v[2] * ROOMSCALE};
			sbf(&txt, "prop %s", mid);
			for (int r = 0; r < 3; r++) sbf(&txt, " %g %g %g %g", m[r][0], m[r][1], m[r][2], t[r]);
			sbf(&txt, " 0 0 0 1\n");
		} else { warn("%s: unknown point entity '%s', stopping", path, b1); break; }
	}
	fclose(f);

	for (int i = 0; i < ngs; i++) sbf(&txt, "mat %d %s %s %s\n", i, kname[gs[i].kind], gs[i].diff, gs[i].lm);
	int ok = 1;
	snprintf(b2, sizeof b2, OUT "%s.glb", id);
	if (!writeglb(b2, gs, ngs, 1)) { warn("can't write %s", b2); ok = 0; }
	snprintf(b2, sizeof b2, OUT "%s.col.glb", id);
	if (!writeglb(b2, &col, 1, 0)) { warn("can't write %s", b2); ok = 0; }
	if (ok) {
		FILE *o = fopen(out, "w");
		if (!o || fwrite(txt.s, 1, txt.n, o) != txt.n) { warn("can't write %s", out); ok = 0; }
		if (o) fclose(o);
	}
	for (int i = 0; i < ngs; i++) freegroup(&gs[i]);
	free(gs); gs = NULL; ngs = 0;
	freegroup(&col); col = (group){0};
	free(txt.s);
	return ok;
}
