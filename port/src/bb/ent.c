// ent.c: entity pool, hierarchy, transforms
#include <stdlib.h>
#include <string.h>
#include "in.h"

E *bb_ents;
int bb_nents;
static int cap, *fl, nfl, cfl; // free list
static v3 tfo; // TFormed result

E *bb_e(ent e) {
	if (e <= 0 || e >= bb_nents || !bb_ents[e].used) die("bad entity handle %d", e);
	return &bb_ents[e];
}
int bbEntityExists(ent e) { return e > 0 && e < bb_nents && bb_ents[e].used; }

static void link(ent e, ent p) {
	E *x = bb_e(e);
	x->par = p;
	if (!p) return;
	E *q = bb_e(p);
	x->prev = q->last; x->next = 0;
	if (q->last) bb_e(q->last)->next = e; else q->first = e;
	q->last = e;
}
static void unlink(ent e) {
	E *x = bb_e(e);
	if (!x->par) return;
	E *q = bb_e(x->par);
	if (x->prev) bb_e(x->prev)->next = x->next; else q->first = x->next;
	if (x->next) bb_e(x->next)->prev = x->prev; else q->last = x->prev;
	x->par = x->prev = x->next = 0;
}

ent bb_new(int kind, ent par) {
	if (!bb_nents) bb_nents = 1; // slot 0 = null
	ent e;
	if (nfl) e = fl[--nfl];
	else { grow(bb_ents, bb_nents, cap); e = bb_nents++; }
	E *x = &bb_ents[e];
	*x = (E){.used = 1, .kind = kind, .scl = {1,1,1}, .rot = m3id(), .wdirty = 1, .br = bb_brdef()};
	if (par) link(e, par);
	return e;
}

void bb_dirty(ent e) {
	E *x = bb_e(e);
	x->wdirty = 1;
	for (ent c = x->first; c; c = bb_e(c)->next) bb_dirty(c);
}

static tf ltf(E *x) { return (tf){m3mul(x->rot, m3scale(x->scl)), x->pos}; }
tf bb_wt(ent e) {
	if (!e) return tfid();
	E *x = bb_e(e);
	if (!x->wdirty) return x->wt;
	tf l = ltf(x);
	x->wt = x->par ? tfmul(bb_wt(x->par), l) : l;
	x->wdirty = 0;
	return x->wt;
}

ent bbCreatePivot(ent par) { return bb_new(EK_PIVOT, par); }

static void freerec(ent e) {
	E *x = bb_e(e);
	while (x->first) { freerec(x->first); x = bb_e(e); }
	unlink(e);
	x = bb_e(e);
	if (x->md) md_unref(x->md);
	br_unref(&x->br);
	free(x->cam); free(x->keys);
	x->used = 0;
	grow(fl, nfl, cfl);
	fl[nfl++] = e;
}
void bbFreeEntity(ent e) { if (e) freerec(e); }

ent bbCopyEntity(ent e, ent par) {
	E src = *bb_e(e); // copy, pool may move
	ent n = bb_new(src.kind, par);
	E *x = bb_e(n);
	ent p = x->par, f;
	*x = src;
	x->par = x->first = x->last = x->next = x->prev = 0; x->wdirty = 1;
	link(n, p);
	if (x->md) x->md->rc++;
	br_ref(&x->br);
	if (x->cam) { cam *c = malloc(sizeof *c); *c = *src.cam; x->cam = c; }
	if (x->keys) { key *k = malloc(sizeof *k * x->nk); memcpy(k, src.keys, sizeof *k * x->nk); x->keys = k; }
	for (f = src.first; f; f = bb_e(f)->next) bbCopyEntity(f, n);
	return n;
}

void bbEntityParent(ent e, ent par, int glob) {
	tf w = bb_wt(e);
	unlink(e);
	link(e, par);
	E *x = bb_e(e);
	if (glob) {
		tf l = tfmul(tfinv(bb_wt(par)), w);
		x->pos = l.v;
		x->scl = v3n(v3len(l.m.i), v3len(l.m.j), v3len(l.m.k));
		x->rot = m3ortho(l.m);
	}
	bb_dirty(e);
}
ent bbGetParent(ent e) { return bb_e(e)->par; }
int bbCountChildren(ent e) { int n = 0; for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) n++; return n; }
ent bbGetChild(ent e, int n) {
	for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) if (--n == 0) return c;
	return 0;
}
ent bbFindChild(ent e, const char *nm) { // recursive like blitz
	for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) {
		if (!strcasecmp(bb_e(c)->name, nm)) return c;
		ent r = bbFindChild(c, nm);
		if (r) return r;
	}
	return 0;
}
void bbNameEntity(ent e, const char *n) { E *x = bb_e(e); snprintf(x->name, sizeof x->name, "%s", n); }
const char *bbEntityName(ent e) { return bb_e(e)->name; }
void bbHideEntity(ent e) { bb_e(e)->hidden = 1; }
void bbShowEntity(ent e) { bb_e(e)->hidden = 0; }
int bbEntityHidden(ent e) { for (; e; e = bb_e(e)->par) if (bb_e(e)->hidden) return 1; return 0; }

// ---- transforms
static void setwpos(ent e, v3 w) {
	E *x = bb_e(e);
	x->pos = x->par ? tfp(tfinv(bb_wt(x->par)), w) : w;
	bb_dirty(e);
}
static m3 wrot(ent e) { return m3ortho(bb_wt(e).m); }
static void setwrot(ent e, m3 r) {
	E *x = bb_e(e);
	x->rot = x->par ? m3ortho(m3mul(m3t(wrot(x->par)), r)) : r;
	bb_dirty(e);
}

void bbPositionEntity(ent e, float x, float y, float z, int glob) {
	if (glob) { setwpos(e, v3n(x, y, z)); return; }
	bb_e(e)->pos = v3n(x, y, z); bb_dirty(e);
}
void bbMoveEntity(ent e, float x, float y, float z) {
	E *p = bb_e(e);
	p->pos = v3add(p->pos, m3v(p->rot, v3n(x, y, z))); bb_dirty(e);
}
void bbTranslateEntity(ent e, float x, float y, float z, int glob) {
	if (glob) { setwpos(e, v3add(bb_wt(e).v, v3n(x, y, z))); return; }
	E *p = bb_e(e);
	p->pos = v3add(p->pos, v3n(x, y, z)); bb_dirty(e);
}
void bbRotateEntity(ent e, float p, float y, float r, int glob) {
	m3 m = m3rot(p*DTOR, y*DTOR, r*DTOR);
	if (glob) { setwrot(e, m); return; }
	bb_e(e)->rot = m; bb_dirty(e);
}
void bbTurnEntity(ent e, float p, float y, float r, int glob) {
	m3 m = m3rot(p*DTOR, y*DTOR, r*DTOR);
	if (glob) { setwrot(e, m3ortho(m3mul(m, wrot(e)))); return; }
	E *x = bb_e(e);
	x->rot = m3ortho(m3mul(x->rot, m)); bb_dirty(e);
}
void bbScaleEntity(ent e, float x, float y, float z, int glob) {
	E *p = bb_e(e);
	v3 s = v3n(x, y, z);
	if (glob && p->par) { // approx: divide out parent scale
		tf w = bb_wt(p->par);
		s = v3n(x/v3len(w.m.i), y/v3len(w.m.j), z/v3len(w.m.k));
	}
	p->scl = s; bb_dirty(e);
}
void bbPointEntity(ent e, ent t, float roll) {
	v3 d = v3sub(bb_wt(t).v, bb_wt(e).v);
	setwrot(e, m3rot(v3pitch(d), v3yaw(d), roll*DTOR));
}
void bbAlignToVector(ent e, float x, float y, float z, int ax, float rate) {
	v3 v = v3norm(v3n(x, y, z));
	if (v3len(v) == 0) return;
	m3 r = wrot(e);
	v3 *a = ax == 1 ? &r.i : ax == 2 ? &r.j : &r.k;
	v3 cur = *a, nv = v3norm(v3add(v3mul(cur, 1-rate), v3mul(v, rate)));
	v3 c = v3cross(cur, nv);
	float s = v3len(c), co = v3dot(cur, nv);
	if (s < 1e-6f) return;
	c = v3mul(c, 1/s);
	float an = atan2f(s, co), cs = cosf(an), sn = sinf(an), t = 1-cs;
	m3 q = {{t*c.x*c.x+cs, t*c.x*c.y+sn*c.z, t*c.x*c.z-sn*c.y},
	        {t*c.x*c.y-sn*c.z, t*c.y*c.y+cs, t*c.y*c.z+sn*c.x},
	        {t*c.x*c.z+sn*c.y, t*c.y*c.z-sn*c.x, t*c.z*c.z+cs}};
	setwrot(e, m3ortho(m3mul(q, r)));
}

float bbEntityX(ent e, int g) { return g ? bb_wt(e).v.x : bb_e(e)->pos.x; }
float bbEntityY(ent e, int g) { return g ? bb_wt(e).v.y : bb_e(e)->pos.y; }
float bbEntityZ(ent e, int g) { return g ? bb_wt(e).v.z : bb_e(e)->pos.z; }
static m3 grot(ent e, int g) { return g ? wrot(e) : bb_e(e)->rot; }
float bbEntityPitch(ent e, int g) { return m3getpitch(grot(e, g)) * RTOD; }
float bbEntityYaw(ent e, int g) { return m3getyaw(grot(e, g)) * RTOD; }
float bbEntityRoll(ent e, int g) { return m3getroll(grot(e, g)) * RTOD; }
float bbEntityScaleX(ent e, int g) { return g ? v3len(bb_wt(e).m.i) : bb_e(e)->scl.x; }
float bbEntityScaleY(ent e, int g) { return g ? v3len(bb_wt(e).m.j) : bb_e(e)->scl.y; }
float bbEntityScaleZ(ent e, int g) { return g ? v3len(bb_wt(e).m.k) : bb_e(e)->scl.z; }
float bbEntityDistance(ent a, ent b) { return v3len(v3sub(bb_wt(a).v, bb_wt(b).v)); }
float bbDeltaYaw(ent a, ent b) {
	float d = v3yaw(v3sub(bb_wt(b).v, bb_wt(a).v)) - m3getyaw(wrot(a));
	return wrapdeg(d * RTOD);
}
float bbDeltaPitch(ent a, ent b) {
	float d = v3pitch(v3sub(bb_wt(b).v, bb_wt(a).v)) - m3getpitch(wrot(a));
	return wrapdeg(d * RTOD);
}

void bbTFormPoint(float x, float y, float z, ent s, ent d) {
	tfo = tfp(tfinv(bb_wt(d)), tfp(bb_wt(s), v3n(x, y, z)));
}
void bbTFormVector(float x, float y, float z, ent s, ent d) {
	tfo = m3v(m3inv(bb_wt(d).m), m3v(bb_wt(s).m, v3n(x, y, z)));
}
void bbTFormNormal(float x, float y, float z, ent s, ent d) {
	// normals go through inverse-transpose
	m3 a = m3t(m3inv(bb_wt(s).m)), b = m3t(bb_wt(d).m);
	tfo = v3norm(m3v(b, m3v(a, v3n(x, y, z))));
}
float bbTFormedX(void) { return tfo.x; }
float bbTFormedY(void) { return tfo.y; }
float bbTFormedZ(void) { return tfo.z; }
float bbGetMatElement(ent e, int r, int c) {
	tf w = bb_wt(e);
	v3 v = r == 0 ? w.m.i : r == 1 ? w.m.j : r == 2 ? w.m.k : w.v;
	return c == 0 ? v.x : c == 1 ? v.y : v.z;
}

// ---- collision / pick fields
void bbEntityType(ent e, int t, int rec) {
	bb_e(e)->ctype = t;
	if (rec) for (ent c = bb_e(e)->first; c; c = bb_e(c)->next) bbEntityType(c, t, 1);
}
int bbGetEntityType(ent e) { return bb_e(e)->ctype; }
void bbEntityRadius(ent e, float rx, float ry) { E *x = bb_e(e); x->rx = rx; x->ry = ry ? ry : rx; }
void bbEntityBox(ent e, float x, float y, float z, float w, float h, float d) {
	E *p = bb_e(e); p->boxo = v3n(x, y, z); p->boxs = v3n(w, h, d);
}
void bbEntityPickMode(ent e, int m, int ob) { E *x = bb_e(e); x->pmode = m; x->pobsc = ob; }
