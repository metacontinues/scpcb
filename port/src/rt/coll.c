// coll.c: collisions (swept ellipsoid vs poly/sphere/box) + picking.
// one-sided: only faces you move into count (why the rooms double their collision meshes).
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "in.h"

struct ctri { v3 a, b, c; surf *s; int ti; };
struct hit { v3 p, n; ent e; float t; surf *s; int ti; };
typedef struct ctri ctri;
typedef struct hit hit;

typedef struct { int src, dst, meth, resp; } rule;
static rule *rules; static int nrules, crules;
#define EPS 1e-3f

void coll_free(E *x) { free(x->hits); free(x->ct); x->hits = NULL; x->ct = NULL; x->nhit = x->chit = x->nct = 0; }

// ---- world-space tri cache. mode 3 = EntityBox, else mesh polys
static unsigned keyof(E *x, int mode) {
	unsigned h = 2166136261u;
#define MIX(v) (h = (h ^ (unsigned)(v)) * 16777619u)
	MIX(mode); MIX((uintptr_t)x->md);
	if (mode == 3) { unsigned b[6]; memcpy(b, &x->boxo, 12); memcpy(b + 3, &x->boxs, 12); for (int i = 0; i < 6; i++) MIX(b[i]); }
	else if (x->md) { MIX(x->md->ns); for (int i = 0; i < x->md->ns; i++) { MIX(x->md->s[i]->ver); MIX(x->md->s[i]->nt); } }
	return h;
#undef MIX
}
static void addtri(E *x, int *cap, v3 a, v3 b, v3 c, surf *s, int ti) {
	grow(x->ct, x->nct, *cap);
	x->ct[x->nct++] = (ctri){a, b, c, s, ti};
}
// quad a b c d, flipped if needed so it faces `out`
static void quad(E *x, int *cap, v3 a, v3 b, v3 c, v3 d, v3 out) {
	if (v3dot(v3cross(v3sub(b, a), v3sub(c, a)), out) < 0) { v3 t = b; b = d; d = t; }
	addtri(x, cap, a, b, c, NULL, 0); addtri(x, cap, a, c, d, NULL, 0);
}
static ctri *gettris(ent e, int mode, int *n) {
	E *x = rt_e(e);
	tf w = rt_wt(e);
	unsigned k = keyof(x, mode);
	if (x->ct && x->cver == k && !memcmp(&x->ctf, &w, sizeof w)) { *n = x->nct; return x->ct; }
	free(x->ct); x->ct = NULL; x->nct = 0;
	int cap = 0;
	if (mode == 3) {
		v3 c[8];
		for (int i = 0; i < 8; i++)
			c[i] = tfp(w, v3add(x->boxo, v3n(i & 1 ? x->boxs.x : 0, i & 2 ? x->boxs.y : 0, i & 4 ? x->boxs.z : 0)));
		v3 ctr = tfp(w, v3add(x->boxo, v3mul(x->boxs, 0.5f)));
		static const int f[6][4] = {{0,1,3,2},{4,5,7,6},{0,1,5,4},{2,3,7,6},{0,2,6,4},{1,3,7,5}};
		for (int i = 0; i < 6; i++) {
			v3 fc = v3mul(v3add(v3add(c[f[i][0]], c[f[i][1]]), v3add(c[f[i][2]], c[f[i][3]])), 0.25f);
			quad(x, &cap, c[f[i][0]], c[f[i][1]], c[f[i][2]], c[f[i][3]], v3sub(fc, ctr));
		}
	} else if (x->md) {
		for (int i = 0; i < x->md->ns; i++) {
			surf *s = x->md->s[i];
			for (int t = 0; t < s->nt; t++) {
				v3 p[3];
				for (int j = 0; j < 3; j++) { float *q = s->pos + s->idx[t*3+j]*3; p[j] = tfp(w, v3n(q[0], q[1], q[2])); }
				addtri(x, &cap, p[0], p[1], p[2], s, t);
			}
		}
	}
	v3 lo = {FLT_MAX, FLT_MAX, FLT_MAX}, hi = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	for (int i = 0; i < x->nct; i++) {
		v3 *p = &x->ct[i].a;
		for (int j = 0; j < 3; j++) {
			lo = v3n(fminf(lo.x, p[j].x), fminf(lo.y, p[j].y), fminf(lo.z, p[j].z));
			hi = v3n(fmaxf(hi.x, p[j].x), fmaxf(hi.y, p[j].y), fmaxf(hi.z, p[j].z));
		}
	}
	x->clo = lo; x->chi = hi; x->cver = k; x->ctf = w;
	*n = x->nct;
	return x->ct;
}
static int boxhit(v3 alo, v3 ahi, v3 blo, v3 bhi) {
	return alo.x <= bhi.x && ahi.x >= blo.x && alo.y <= bhi.y && ahi.y >= blo.y && alo.z <= bhi.z && ahi.z >= blo.z;
}

// ---- primitives
static int lowroot(float a, float b, float c, float max, float *r) {
	float d = b*b - 4*a*c;
	if (d < 0 || fabsf(a) < 1e-12f) return 0;
	float sd = sqrtf(d), r1 = (-b - sd) / (2*a), r2 = (-b + sd) / (2*a);
	if (r1 > r2) { float t = r1; r1 = r2; r2 = t; }
	if (r1 > 0 && r1 < max) { *r = r1; return 1; }
	if (r2 > 0 && r2 < max) { *r = r2; return 1; }
	return 0;
}
static int intri(v3 p, v3 a, v3 b, v3 c, v3 n) {
	return v3dot(v3cross(v3sub(b, a), v3sub(p, a)), n) >= 0 && v3dot(v3cross(v3sub(c, b), v3sub(p, b)), n) >= 0
	    && v3dot(v3cross(v3sub(a, c), v3sub(p, c)), n) >= 0;
}
// unit sphere at base moving by vel vs tri. on an earlier hit: *t, *cp (contact) updated
static int sweeptri(v3 base, v3 vel, v3 p1, v3 p2, v3 p3, float *t, v3 *cp) {
	v3 n = v3cross(v3sub(p2, p1), v3sub(p3, p1));
	float nl = v3len(n);
	if (nl < 1e-12f) return 0;
	n = v3mul(n, 1 / nl);
	float ndv = v3dot(n, vel);
	if (ndv >= 0) return 0; // back face / moving away
	float sd = v3dot(n, v3sub(base, p1));
	float t0 = (1 - sd) / ndv, t1 = (-1 - sd) / ndv;
	if (t0 > t1) { float x = t0; t0 = t1; t1 = x; }
	if (t0 > 1 || t1 < 0) return 0;
	t0 = fmaxf(t0, 0);
	v3 pip = v3add(v3sub(base, n), v3mul(vel, t0));
	if (intri(pip, p1, p2, p3, n)) {
		if (t0 >= *t) return 0;
		*t = t0; *cp = pip; return 1;
	}
	float a = v3dot(vel, vel), nt, tt = *t;
	int f = 0;
	v3 c, ps[3] = {p1, p2, p3};
	for (int i = 0; i < 3; i++) {
		v3 d = v3sub(base, ps[i]);
		if (lowroot(a, 2 * v3dot(vel, d), v3dot(d, d) - 1, tt, &nt)) { tt = nt; c = ps[i]; f = 1; }
	}
	for (int i = 0; i < 3; i++) {
		v3 e = v3sub(ps[(i+1)%3], ps[i]), btv = v3sub(ps[i], base);
		float el2 = v3dot(e, e), edv = v3dot(e, vel), edb = v3dot(e, btv);
		float qa = el2 * -a + edv * edv, qb = el2 * (2 * v3dot(vel, btv)) - 2 * edv * edb;
		float qc = el2 * (1 - v3dot(btv, btv)) + edb * edb;
		if (lowroot(qa, qb, qc, tt, &nt)) {
			float g = (edv * nt - edb) / el2;
			if (g >= 0 && g <= 1) { tt = nt; c = v3add(ps[i], v3mul(e, g)); f = 1; }
		}
	}
	if (f) { *t = tt; *cp = c; }
	return f;
}
// ray p + d*t vs tri, one-sided
static int raytri(v3 p, v3 d, v3 a, v3 b, v3 c, float *t) {
	v3 e1 = v3sub(b, a), e2 = v3sub(c, a), n = v3cross(e1, e2);
	if (v3dot(n, d) >= 0) return 0;
	v3 h = v3cross(d, e2);
	float det = v3dot(e1, h);
	if (fabsf(det) < 1e-12f) return 0;
	float f = 1 / det;
	v3 s = v3sub(p, a);
	float u = f * v3dot(s, h);
	if (u < 0 || u > 1) return 0;
	v3 q = v3cross(s, e1);
	float v = f * v3dot(d, q);
	if (v < 0 || u + v > 1) return 0;
	float tt = f * v3dot(e2, q);
	if (tt < 0 || tt >= *t) return 0;
	*t = tt; return 1;
}
// point p moving by d vs sphere (c, r). inside + moving in = hit at 0
static int raysph(v3 p, v3 d, v3 c, float r, float *t) {
	v3 m = v3sub(p, c);
	float b = v3dot(m, d), cc = v3dot(m, m) - r*r;
	if (cc <= 0) { if (b < 0 && 0 < *t) { *t = 0; return 1; } return 0; }
	float a = v3dot(d, d), nt;
	if (!lowroot(a, 2*b, cc, *t, &nt)) return 0;
	*t = nt; return 1;
}

// ---- collisions
void coll_rule(int src, int dst, int meth, int resp) {
	for (int i = 0; i < nrules; i++)
		if (rules[i].src == src && rules[i].dst == dst) { rules[i].meth = meth; rules[i].resp = resp; return; }
	grow(rules, nrules, crules);
	rules[nrules++] = (rule){src, dst, meth, resp};
}
void clear_collisions(void) { nrules = 0; }

void reset_entity(ent e) {
	E *x = rt_e(e);
	x->opos = rt_wt(e).v; x->ohas = 1; x->nhit = 0;
}

typedef struct { ent e; int meth, resp; } cand;
static cand *cs; static int ncs, ccs;

static void record(ent e, hit h) { E *x = rt_e(e); grow(x->hits, x->nhit, x->chit); x->hits[x->nhit++] = h; }

static void collide(ent e) {
	E *x = rt_e(e);
	v3 rad = v3n(x->rx, x->ry, x->rx), inv = v3n(1 / x->rx, 1 / x->ry, 1 / x->rx);
	v3 wf = x->opos, wt = rt_wt(e).v;
	float mv = v3len(v3sub(wt, wf));
	if (mv < 1e-7f) return;
	// broad phase once: everything the slide could reach
	v3 m = v3add(rad, v3n(mv, mv, mv)), lo = v3sub(wf, m), hi = v3add(wf, m);
	int me = x->ctype;
	ncs = 0;
	for (int r = 0; r < nrules; r++) {
		if (rules[r].src != me) continue;
		for (ent t = 1; t < rt_nents; t++) {
			E *y = &rt_ents[t];
			if (t == e || !y->used || y->ctype != rules[r].dst || entity_hidden(t)) continue;
			if (rules[r].meth == 1) {
				v3 c = rt_wt(t).v, rr = v3n(y->rx, y->rx, y->rx);
				if (!boxhit(lo, hi, v3sub(c, rr), v3add(c, rr))) continue;
			} else {
				int n; gettris(t, rules[r].meth == 3 ? 3 : 2, &n);
				y = &rt_ents[t];
				if (!n || !boxhit(lo, hi, y->clo, y->chi)) continue;
			}
			grow(cs, ncs, ccs);
			cs[ncs++] = (cand){t, rules[r].meth, rules[r].resp};
		}
	}
	if (!ncs) return;
	v3 pos = v3scl(wf, inv), dst = v3scl(wt, inv);
	for (int it = 0; it < 8; it++) {
		v3 vel = v3sub(dst, pos);
		if (v3len(vel) < 1e-6f) break;
		float bt = 1; v3 bcp = {0}; int bc = -1; surf *bs = NULL; int bti = 0;
		for (int i = 0; i < ncs; i++) {
			if (cs[i].meth == 1) { // sphere vs sphere, in world space; t is the same fraction
				v3 c = rt_wt(cs[i].e).v;
				float rt = rt_e(cs[i].e)->rx, tt = bt;
				v3 wp = v3scl(pos, rad), wv = v3scl(vel, rad);
				if (raysph(wp, wv, c, x->rx + rt, &tt)) {
					v3 hp = v3add(wp, v3mul(wv, tt));
					bt = tt; bc = i; bs = NULL; bti = 0;
					bcp = v3scl(v3add(c, v3mul(v3norm(v3sub(hp, c)), rt)), inv);
				}
				continue;
			}
			int n; ctri *tr = gettris(cs[i].e, cs[i].meth == 3 ? 3 : 2, &n);
			for (int k = 0; k < n; k++)
				if (sweeptri(pos, vel, v3scl(tr[k].a, inv), v3scl(tr[k].b, inv), v3scl(tr[k].c, inv), &bt, &bcp)) {
					bc = i; bs = tr[k].s; bti = tr[k].ti;
				}
		}
		if (bc < 0) { pos = dst; break; }
		v3 np = v3add(pos, v3mul(vel, bt));
		v3 sn = v3norm(v3sub(np, bcp));
		np = v3add(np, v3mul(sn, EPS)); // back off a hair
		record(e, (hit){v3scl(bcp, rad), v3norm(v3scl(sn, inv)), cs[bc].e, bt, bs, bti});
		if (cs[bc].resp == 1) { pos = np; break; }
		// slide: keep dst 1 unit (+eps) off the contact plane
		float d = v3dot(v3sub(dst, bcp), sn);
		v3 nd = v3sub(dst, v3mul(sn, d - 1 - EPS));
		if (cs[bc].resp == 3 && sn.y > 0) nd.y = fmaxf(nd.y, np.y); // no sliding down slopes
		pos = np; dst = nd;
	}
	position_entity(e, pos.x * rad.x, pos.y * rad.y, pos.z * rad.z, 1);
}

void update_world(float anim) {
	anim_step(anim); // animate first, then collide
	for (ent e = 1; e < rt_nents; e++) if (rt_ents[e].used) rt_ents[e].nhit = 0;
	for (ent e = 1; e < rt_nents; e++) {
		E *x = &rt_ents[e];
		if (!x->used || !x->ctype || !x->ohas || entity_hidden(e)) continue;
		int src = 0;
		for (int r = 0; r < nrules && !src; r++) src = rules[r].src == x->ctype;
		if (src) collide(e);
	}
	for (ent e = 1; e < rt_nents; e++) {
		E *x = &rt_ents[e];
		if (x->used && x->ctype) { x->opos = rt_wt(e).v; x->ohas = 1; }
	}
}

static hit *gethit(ent e, int i) { E *x = rt_e(e); static hit z; return i >= 0 && i < x->nhit ? &x->hits[i] : &z; }
int count_collisions(ent e) { return rt_e(e)->nhit; }
float collision_x(ent e, int i) { return gethit(e, i)->p.x; }
float collision_y(ent e, int i) { return gethit(e, i)->p.y; }
float collision_z(ent e, int i) { return gethit(e, i)->p.z; }
float collision_nx(ent e, int i) { return gethit(e, i)->n.x; }
float collision_ny(ent e, int i) { return gethit(e, i)->n.y; }
float collision_nz(ent e, int i) { return gethit(e, i)->n.z; }
float collision_time(ent e, int i) { return gethit(e, i)->t; }
ent collision_entity(ent e, int i) { return gethit(e, i)->e; }
surf *collision_surface(ent e, int i) { return gethit(e, i)->s; }
int collision_triangle(ent e, int i) { return gethit(e, i)->ti; }
ent entity_collided(ent e, int type) {
	E *x = rt_e(e);
	for (int i = 0; i < x->nhit; i++) if (entity_exists(x->hits[i].e) && rt_e(x->hits[i].e)->ctype == type) return x->hits[i].e;
	return 0;
}

// ---- picking
static hit pk;

ent rt_pick(v3 from, v3 d, float rad, ent ex1, ent ex2, int obsonly) {
	pk = (hit){0};
	float bt = 1;
	v3 to = v3add(from, d), rr = v3n(rad, rad, rad);
	v3 lo = v3sub(v3n(fminf(from.x, to.x), fminf(from.y, to.y), fminf(from.z, to.z)), rr);
	v3 hi = v3add(v3n(fmaxf(from.x, to.x), fmaxf(from.y, to.y), fmaxf(from.z, to.z)), rr);
	for (ent e = 1; e < rt_nents; e++) {
		E *x = &rt_ents[e];
		if (!x->used || !x->pmode || e == ex1 || e == ex2 || (obsonly && !x->pobsc) || entity_hidden(e)) continue;
		if (x->pmode == 1) {
			v3 c = rt_wt(e).v;
			float tt = bt;
			if (raysph(from, d, c, rt_e(e)->rx + rad, &tt)) {
				v3 hp = v3add(from, v3mul(d, tt)), n = v3norm(v3sub(hp, c));
				bt = tt; pk = (hit){v3sub(hp, v3mul(n, rad)), n, e, tt, NULL, 0};
			}
			continue;
		}
		int n; ctri *tr = gettris(e, x->pmode == 3 ? 3 : 2, &n);
		x = &rt_ents[e];
		if (!n || !boxhit(lo, hi, x->clo, x->chi)) continue;
		for (int k = 0; k < n; k++) {
			ctri *c = &tr[k];
			if (rad <= 0) {
				if (!raytri(from, d, c->a, c->b, c->c, &bt)) continue;
				pk = (hit){v3add(from, v3mul(d, bt)), v3norm(v3cross(v3sub(c->b, c->a), v3sub(c->c, c->a))), e, bt, c->s, c->ti};
			} else {
				float iv = 1 / rad; v3 cp;
				if (!sweeptri(v3mul(from, iv), v3mul(d, iv), v3mul(c->a, iv), v3mul(c->b, iv), v3mul(c->c, iv), &bt, &cp)) continue;
				v3 hp = v3mul(v3add(v3mul(from, iv), v3mul(v3mul(d, iv), bt)), rad);
				cp = v3mul(cp, rad);
				pk = (hit){cp, v3norm(v3sub(hp, cp)), e, bt, c->s, c->ti};
			}
		}
	}
	return pk.e;
}

ent line_pick(float x, float y, float z, float dx, float dy, float dz, float r) { return rt_pick(v3n(x, y, z), v3n(dx, dy, dz), r, 0, 0, 0); }
ent entity_pick(ent e, float range) {
	tf w = rt_wt(e);
	return rt_pick(w.v, v3mul(v3norm(w.m.k), range), 0, e, 0, 0);
}
int entity_visible(ent a, ent b) {
	v3 p = rt_wt(a).v;
	return !rt_pick(p, v3sub(rt_wt(b).v, p), 0, a, b, 1);
}
float picked_x(void) { return pk.p.x; }
float picked_y(void) { return pk.p.y; }
float picked_z(void) { return pk.p.z; }
float picked_nx(void) { return pk.n.x; }
float picked_ny(void) { return pk.n.y; }
float picked_nz(void) { return pk.n.z; }
float picked_time(void) { return pk.t; }
ent picked_entity(void) { return pk.e; }
surf *picked_surface(void) { return pk.s; }
int picked_triangle(void) { return pk.ti; }
