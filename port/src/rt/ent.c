// ent.c: entity pool, hierarchy, transforms
#include <stdlib.h>
#include <string.h>
#include "in.h"

E *rt_ents;
int rt_nents;
static int cap, *fl, nfl, cfl; // free list
static v3 tfo; // TFormed result

E *rt_e(ent e) {
	if (e <= 0 || e >= rt_nents || !rt_ents[e].used) die("bad entity handle %d", e);
	return &rt_ents[e];
}
int entity_exists(ent e) { return e > 0 && e < rt_nents && rt_ents[e].used; }

static void link(ent e, ent p) {
	E *x = rt_e(e);
	x->par = p;
	if (!p) return;
	E *q = rt_e(p);
	x->prev = q->last; x->next = 0;
	if (q->last) rt_e(q->last)->next = e; else q->first = e;
	q->last = e;
}
static void unlink(ent e) {
	E *x = rt_e(e);
	if (!x->par) return;
	E *q = rt_e(x->par);
	if (x->prev) rt_e(x->prev)->next = x->next; else q->first = x->next;
	if (x->next) rt_e(x->next)->prev = x->prev; else q->last = x->prev;
	x->par = x->prev = x->next = 0;
}

ent rt_new(int kind, ent par) {
	if (!rt_nents) rt_nents = 1; // slot 0 = null
	ent e;
	if (nfl) e = fl[--nfl];
	else { grow(rt_ents, rt_nents, cap); e = rt_nents++; }
	E *x = &rt_ents[e];
	*x = (E){.used = 1, .kind = kind, .scl = {1,1,1}, .rot = m3id(), .wdirty = 1, .br = rt_brdef(), .rx = 1, .ry = 1};
	if (par) link(e, par);
	return e;
}

void rt_dirty(ent e) {
	E *x = rt_e(e);
	x->wdirty = 1;
	for (ent c = x->first; c; c = rt_e(c)->next) rt_dirty(c);
}

static tf ltf(E *x) { return (tf){m3mul(x->rot, m3scale(x->scl)), x->pos}; }
tf rt_wt(ent e) {
	if (!e) return tfid();
	E *x = rt_e(e);
	if (!x->wdirty) return x->wt;
	tf l = ltf(x);
	x->wt = x->par ? tfmul(rt_wt(x->par), l) : l;
	x->wdirty = 0;
	return x->wt;
}

ent create_pivot(ent par) { return rt_new(EK_PIVOT, par); }

static void freerec(ent e) {
	E *x = rt_e(e);
	while (x->first) { freerec(x->first); x = rt_e(e); }
	unlink(e);
	x = rt_e(e);
	if (x->md) md_unref(x->md);
	br_unref(&x->br);
	free(x->cam); free(x->keys); free(x->seqs); free(x->bones); coll_free(x);
	x->used = 0;
	grow(fl, nfl, cfl);
	fl[nfl++] = e;
}
void free_entity(ent e) { if (e) freerec(e); }

static ent copyrec(ent e, ent par) {
	E src = *rt_e(e); // copy, pool may move
	ent n = rt_new(src.kind, 0), f; // linked below, once (rt_new(.., par) + link made a cycle)
	E *x = rt_e(n);
	*x = src;
	x->par = x->first = x->last = x->next = x->prev = 0; x->wdirty = 1;
	x->hits = NULL; x->nhit = x->chit = 0; x->ct = NULL; x->nct = 0; x->ohas = 0;
	link(n, par);
	if (x->md && x->md->sk) x->md = md_copy(x->md); // skinned: each copy poses its own verts
	else if (x->md) x->md->rc++;
	x->bones = NULL; x->nbones = 0;
	if (x->seqs) x->seqs = memcpy(malloc(sizeof *x->seqs * x->nseq), src.seqs, sizeof *x->seqs * x->nseq);
	br_ref(&x->br);
	if (x->cam) { cam *c = malloc(sizeof *c); *c = *src.cam; x->cam = c; }
	if (x->keys) { key *k = malloc(sizeof *k * x->nk); memcpy(k, src.keys, sizeof *k * x->nk); x->keys = k; }
	for (f = src.first; f; f = rt_e(f)->next) copyrec(f, n);
	return n;
}
// the copy itself always starts shown: the game hides its templates (GuardObj, ClassDObj...) and relies on copies
// of them being visible (it even re-hides the DTextures copies, Main.bb 8240). children keep their own flags
ent copy_entity(ent e, ent par) { ent n = copyrec(e, par); rt_e(n)->hidden = 0; return n; }

void entity_parent(ent e, ent par, int glob) {
	tf w = rt_wt(e);
	unlink(e);
	link(e, par);
	E *x = rt_e(e);
	if (glob) {
		tf l = tfmul(tfinv(rt_wt(par)), w);
		x->pos = l.v;
		x->scl = v3n(v3len(l.m.i), v3len(l.m.j), v3len(l.m.k));
		x->rot = m3ortho(l.m);
	}
	rt_dirty(e);
}
ent get_parent(ent e) { return rt_e(e)->par; }
int count_children(ent e) { int n = 0; for (ent c = rt_e(e)->first; c; c = rt_e(c)->next) n++; return n; }
ent get_child(ent e, int n) {
	for (ent c = rt_e(e)->first; c; c = rt_e(c)->next) if (n-- == 0) return c;
	return 0;
}
ent find_child(ent e, const char *nm) { // searches the whole subtree
	for (ent c = rt_e(e)->first; c; c = rt_e(c)->next) {
		if (!strcasecmp(rt_e(c)->name, nm)) return c;
		ent r = find_child(c, nm);
		if (r) return r;
	}
	return 0;
}
void name_entity(ent e, const char *n) { E *x = rt_e(e); snprintf(x->name, sizeof x->name, "%s", n); }
const char *entity_name(ent e) { return rt_e(e)->name; }
void hide_entity(ent e) { rt_e(e)->hidden = 1; }
void show_entity(ent e) { rt_e(e)->hidden = 0; }
int entity_hidden(ent e) { for (; e; e = rt_e(e)->par) if (rt_e(e)->hidden) return 1; return 0; }

// ---- transforms
static void setwpos(ent e, v3 w) {
	E *x = rt_e(e);
	x->pos = x->par ? tfp(tfinv(rt_wt(x->par)), w) : w;
	rt_dirty(e);
}
static m3 wrot(ent e) { return m3ortho(rt_wt(e).m); }
static void setwrot(ent e, m3 r) {
	E *x = rt_e(e);
	x->rot = x->par ? m3ortho(m3mul(m3t(wrot(x->par)), r)) : r;
	rt_dirty(e);
}

void position_entity(ent e, float x, float y, float z, int glob) {
	if (glob) { setwpos(e, v3n(x, y, z)); return; }
	rt_e(e)->pos = v3n(x, y, z); rt_dirty(e);
}
void move_entity(ent e, float x, float y, float z) {
	E *p = rt_e(e);
	p->pos = v3add(p->pos, m3v(p->rot, v3n(x, y, z))); rt_dirty(e);
}
void translate_entity(ent e, float x, float y, float z, int glob) {
	if (glob) { setwpos(e, v3add(rt_wt(e).v, v3n(x, y, z))); return; }
	E *p = rt_e(e);
	p->pos = v3add(p->pos, v3n(x, y, z)); rt_dirty(e);
}
void rotate_entity(ent e, float p, float y, float r, int glob) {
	m3 m = m3rot(p*DTOR, y*DTOR, r*DTOR);
	if (glob) { setwrot(e, m); return; }
	rt_e(e)->rot = m; rt_dirty(e);
}
void turn_entity(ent e, float p, float y, float r, int glob) {
	m3 m = m3rot(p*DTOR, y*DTOR, r*DTOR);
	if (glob) { setwrot(e, m3ortho(m3mul(m, wrot(e)))); return; }
	E *x = rt_e(e);
	x->rot = m3ortho(m3mul(x->rot, m)); rt_dirty(e);
}
void scale_entity(ent e, float x, float y, float z, int glob) {
	E *p = rt_e(e);
	v3 s = v3n(x, y, z);
	if (glob && p->par) { // approx: divide out parent scale
		tf w = rt_wt(p->par);
		s = v3n(x/v3len(w.m.i), y/v3len(w.m.j), z/v3len(w.m.k));
	}
	p->scl = s; rt_dirty(e);
}
void point_entity(ent e, ent t, float roll) {
	v3 d = v3sub(rt_wt(t).v, rt_wt(e).v);
	setwrot(e, m3rot(v3pitch(d), v3yaw(d), roll*DTOR));
}
void align_to_vector(ent e, float x, float y, float z, int ax, float rate) {
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

float entity_x(ent e, int g) { return g ? rt_wt(e).v.x : rt_e(e)->pos.x; }
float entity_y(ent e, int g) { return g ? rt_wt(e).v.y : rt_e(e)->pos.y; }
float entity_z(ent e, int g) { return g ? rt_wt(e).v.z : rt_e(e)->pos.z; }
static m3 grot(ent e, int g) { return g ? wrot(e) : rt_e(e)->rot; }
float entity_pitch(ent e, int g) { return m3getpitch(grot(e, g)) * RTOD; }
float entity_yaw(ent e, int g) { return m3getyaw(grot(e, g)) * RTOD; }
float entity_roll(ent e, int g) { return m3getroll(grot(e, g)) * RTOD; }
float entity_scale_x(ent e, int g) { return g ? v3len(rt_wt(e).m.i) : rt_e(e)->scl.x; }
float entity_scale_y(ent e, int g) { return g ? v3len(rt_wt(e).m.j) : rt_e(e)->scl.y; }
float entity_scale_z(ent e, int g) { return g ? v3len(rt_wt(e).m.k) : rt_e(e)->scl.z; }
float entity_distance(ent a, ent b) { return v3len(v3sub(rt_wt(a).v, rt_wt(b).v)); }
float delta_yaw(ent a, ent b) {
	float d = v3yaw(v3sub(rt_wt(b).v, rt_wt(a).v)) - m3getyaw(wrot(a));
	return wrapdeg(d * RTOD);
}
float delta_pitch(ent a, ent b) {
	float d = v3pitch(v3sub(rt_wt(b).v, rt_wt(a).v)) - m3getpitch(wrot(a));
	return wrapdeg(d * RTOD);
}

void t_form_point(float x, float y, float z, ent s, ent d) {
	tfo = tfp(tfinv(rt_wt(d)), tfp(rt_wt(s), v3n(x, y, z)));
}
void t_form_vector(float x, float y, float z, ent s, ent d) {
	tfo = m3v(m3inv(rt_wt(d).m), m3v(rt_wt(s).m, v3n(x, y, z)));
}
void t_form_normal(float x, float y, float z, ent s, ent d) {
	// normals go through inverse-transpose
	m3 a = m3t(m3inv(rt_wt(s).m)), b = m3t(rt_wt(d).m);
	tfo = v3norm(m3v(b, m3v(a, v3n(x, y, z))));
}
float t_formed_x(void) { return tfo.x; }
float t_formed_y(void) { return tfo.y; }
float t_formed_z(void) { return tfo.z; }
float get_mat_element(ent e, int r, int c) {
	tf w = rt_wt(e);
	v3 v = r == 0 ? w.m.i : r == 1 ? w.m.j : r == 2 ? w.m.k : w.v;
	return c == 0 ? v.x : c == 1 ? v.y : v.z;
}

// ---- collision / pick fields
void entity_type(ent e, int t, int rec) {
	rt_e(e)->ctype = t;
	reset_entity(e);
	if (rec) for (ent c = rt_e(e)->first; c; c = rt_e(c)->next) entity_type(c, t, 1);
}
int get_entity_type(ent e) { return rt_e(e)->ctype; }
void entity_radius(ent e, float rx, float ry) { E *x = rt_e(e); x->rx = rx; x->ry = ry ? ry : rx; }
void entity_box(ent e, float x, float y, float z, float w, float h, float d) {
	E *p = rt_e(e); p->boxo = v3n(x, y, z); p->boxs = v3n(w, h, d);
}
void entity_pick_mode(ent e, int m, int ob) { E *x = rt_e(e); x->pmode = m; x->pobsc = ob; }
float vector_yaw(float x, float y, float z) { (void)y; return v3yaw(v3n(x, y, z)) * RTOD; }
float vector_pitch(float x, float y, float z) { return v3pitch(v3n(x, y, z)) * RTOD; }
