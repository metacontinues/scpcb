// b3d.c: .b3d loader + LoadMesh/LoadAnimMesh dispatch. LoadAnimMesh keeps the node tree, LoadMesh bakes it flat.
// ANIM/KEYS/BONE get turned into anim + skin data for anim.c.
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../rt/in.h"

typedef struct { FILE *f; long end[32]; int d; } rd;

static int tag(rd *r, char *t) { // enter next chunk if any left in the current one
	if (ftell(r->f) >= r->end[r->d]) return 0;
	if (fread(t, 1, 4, r->f) != 4) return 0;
	t[4] = 0;
	int n = read_int(r->f);
	r->end[++r->d] = ftell(r->f) + n;
	return 1;
}
static void leave(rd *r) { fseek(r->f, r->end[r->d--], SEEK_SET); }
static int more(rd *r) { return ftell(r->f) < r->end[r->d]; }
static void cstr(rd *r, char *o, int n) {
	int c, i = 0;
	while ((c = fgetc(r->f)) > 0) if (i < n - 1) o[i++] = c;
	o[i] = 0;
}
static float rf(rd *r) { return read_float(r->f); }
static int ri(rd *r) { return read_int(r->f); }

typedef struct { ent m; surf *s; int *map, nv; } smap; // MESH vertex id -> surface-local idx
typedef struct { ent b, m; int n, *vid; float *w; } bone;

typedef struct {
	rd r;
	char dir[512];
	htex *tx; int ntx;
	brush *br; int nbr;
	smap *sm; int nsm;
	bone *bn; int nbn;
} ctx;

static htex loadtex(ctx *c, const char *name, int flags) {
	char p[1024];
	const char *b = strrchr(name, '\\'), *b2 = strrchr(name, '/');
	if (b2 > b) b = b2;
	snprintf(p, sizeof p, "%s%s", c->dir, name);
	htex t = res_path(p) ? load_texture(p, flags) : 0;
	if (!t && b) { snprintf(p, sizeof p, "%s%s", c->dir, b + 1); t = load_texture(p, flags); }
	if (!t) t = load_texture(name, flags);
	return t;
}

static void texs(ctx *c) {
	while (more(&c->r)) {
		char n[256]; cstr(&c->r, n, sizeof n);
		int fl = ri(&c->r), bl = ri(&c->r);
		float px = rf(&c->r), py = rf(&c->r), sx = rf(&c->r), sy = rf(&c->r); rf(&c->r);
		htex t = loadtex(c, n, fl | TF_MIP);
		if (t) { texture_blend(t, bl); scale_texture(t, sx, sy); position_texture(t, px, py); }
		c->tx = realloc(c->tx, sizeof *c->tx * (c->ntx + 1));
		c->tx[c->ntx++] = t;
	}
}
static void brus(ctx *c) {
	int nt = ri(&c->r);
	while (more(&c->r)) {
		char n[256]; cstr(&c->r, n, sizeof n);
		brush b = rt_brdef();
		b.r = rf(&c->r); b.g = rf(&c->r); b.b = rf(&c->r); b.a = rf(&c->r); b.shine = rf(&c->r);
		b.blend = ri(&c->r); b.fx = ri(&c->r);
		for (int i = 0; i < nt; i++) {
			int id = ri(&c->r);
			if (i < RT_NTEX && id >= 0 && id < c->ntx) b.tex[i] = c->tx[id];
		}
		c->br = realloc(c->br, sizeof *c->br * (c->nbr + 1));
		c->br[c->nbr++] = b;
	}
}

typedef struct { float p[3], n[3], c[4], uv[2][2]; } vtx;

static void mesh(ctx *c, ent e) {
	int mbr = ri(&c->r);
	vtx *vs = NULL; int nv = 0, hasn = 0;
	char t[5];
	E *x = rt_e(e);
	x->kind = EK_MESH; x->md = md_new();
	while (tag(&c->r, t)) {
		if (!strcmp(t, "VRTS")) {
			int fl = ri(&c->r), sets = ri(&c->r), sz = ri(&c->r);
			hasn = fl & 1;
			while (more(&c->r)) {
				vs = realloc(vs, sizeof *vs * (nv + 1));
				vtx *v = &vs[nv++];
				*v = (vtx){.c = {1, 1, 1, 1}};
				for (int i = 0; i < 3; i++) v->p[i] = rf(&c->r);
				if (fl & 1) for (int i = 0; i < 3; i++) v->n[i] = rf(&c->r);
				if (fl & 2) for (int i = 0; i < 4; i++) v->c[i] = rf(&c->r);
				for (int s = 0; s < sets; s++) for (int k = 0; k < sz; k++) {
					float f = rf(&c->r);
					if (s < 2 && k < 2) v->uv[s][k] = f;
				}
			}
		} else if (!strcmp(t, "TRIS")) {
			int bi = ri(&c->r);
			if (bi < 0) bi = mbr;
			brush b = bi >= 0 && bi < c->nbr ? c->br[bi] : rt_brdef();
			surf *s = md_addsurf(rt_md(e), b);
			int *map = malloc(sizeof *map * (nv ? nv : 1));
			for (int i = 0; i < nv; i++) map[i] = -1;
			while (more(&c->r)) {
				int ix[3];
				for (int k = 0; k < 3; k++) {
					int v = ri(&c->r);
					if (v < 0 || v >= nv) v = 0;
					if (map[v] < 0) {
						vtx *p = &vs[v];
						map[v] = add_vertex(s, p->p[0], p->p[1], p->p[2], p->uv[0][0], p->uv[0][1], 0);
						vertex_tex_coords(s, map[v], p->uv[1][0], p->uv[1][1], 0, 1);
						vertex_normal(s, map[v], p->n[0], p->n[1], p->n[2]);
						vertex_color(s, map[v], p->c[0]*255, p->c[1]*255, p->c[2]*255, p->c[3]);
					}
					ix[k] = map[v];
				}
				add_triangle(s, ix[0], ix[1], ix[2]);
			}
			c->sm = realloc(c->sm, sizeof *c->sm * (c->nsm + 1));
			c->sm[c->nsm++] = (smap){e, s, map, nv}; // kept for BONE
		}
		leave(&c->r);
	}
	free(vs);
	if (!hasn) update_normals(e);
}

static void keys(ctx *c, ent e) {
	int fl = ri(&c->r);
	while (more(&c->r)) {
		E *x = rt_e(e);
		x->keys = realloc(x->keys, sizeof *x->keys * (x->nk + 1));
		key *k = &x->keys[x->nk++];
		*k = (key){.t = ri(&c->r), .has = fl, .s = {1, 1, 1}, .q = {1, 0, 0, 0}};
		if (fl & 1) k->p = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
		if (fl & 2) k->s = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
		if (fl & 4) for (int i = 0; i < 4; i++) k->q[i] = rf(&c->r);
	}
}

static ent node(ctx *c, ent par) {
	char n[256]; cstr(&c->r, n, sizeof n);
	ent e = rt_new(EK_PIVOT, par);
	name_entity(e, n);
	E *x = rt_e(e);
	x->pos = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
	x->scl = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
	float w = rf(&c->r), a = rf(&c->r), b = rf(&c->r), d = rf(&c->r);
	x->rot = m3ortho(qmat(w, a, b, d));
	char t[5];
	while (tag(&c->r, t)) {
		if (!strcmp(t, "MESH")) mesh(c, e);
		else if (!strcmp(t, "KEYS")) keys(c, e);
		else if (!strcmp(t, "NODE")) node(c, e);
		else if (!strcmp(t, "ANIM")) {
			ri(&c->r); int fr = ri(&c->r); rf(&c->r); // flags, frames, fps
			E *y = rt_e(e);
			y->anim = 1; y->nseq = 1;
			y->seqs = malloc(sizeof *y->seqs);
			y->seqs[0].a = 0; y->seqs[0].b = fr;
		} else if (!strcmp(t, "BONE")) {
			bone b = {e, 0, 0, NULL, NULL};
			for (ent q = rt_e(e)->par; q && !b.m; q = rt_e(q)->par) if (rt_e(q)->md) b.m = q;
			while (more(&c->r)) {
				b.vid = realloc(b.vid, sizeof(int) * (b.n + 1)); b.w = realloc(b.w, sizeof(float) * (b.n + 1));
				b.vid[b.n] = ri(&c->r); b.w[b.n++] = rf(&c->r);
			}
			if (b.m) { c->bn = realloc(c->bn, sizeof *c->bn * (c->nbn + 1)); c->bn[c->nbn++] = b; }
			else { free(b.vid); free(b.w); }
		}
		leave(&c->r);
	}
	rt_dirty(e);
	return e;
}


// bones -> per-vertex weights (4 max, normalized) + bind inverses, per skinned mesh
static void skins(ctx *c) {
	for (int i = 0; i < c->nbn; i++) {
		ent m = c->bn[i].m;
		mdat *md = rt_md(m);
		if (md->sk) continue; // done via an earlier bone
		skin *k = md->sk = calloc(1, sizeof *k);
		tf mi = tfinv(rt_wt(m));
		for (int j = i; j < c->nbn; j++) {
			if (c->bn[j].m != m) continue;
			k->name = realloc(k->name, 64 * (k->nb + 1)); k->binv = realloc(k->binv, sizeof(tf) * (k->nb + 1));
			snprintf(k->name[k->nb], 64, "%s", entity_name(c->bn[j].b));
			k->binv[k->nb++] = tfinv(tfmul(mi, rt_wt(c->bn[j].b)));
		}
		for (int si = 0; si < md->ns; si++) {
			surf *s = md->s[si];
			s->bpos = memcpy(malloc(s->nv * 12), s->pos, s->nv * 12);
			s->bnrm = memcpy(malloc(s->nv * 12), s->nrm, s->nv * 12);
			s->bw = calloc(s->nv * 4, sizeof(float)); s->bi = calloc(s->nv * 4, 1);
		}
		int bi = 0;
		for (int j = i; j < c->nbn; j++) {
			if (c->bn[j].m != m) continue;
			for (int sm = 0; sm < c->nsm; sm++) {
				if (c->sm[sm].m != m) continue;
				surf *s = c->sm[sm].s;
				for (int w = 0; w < c->bn[j].n; w++) {
					int v = c->bn[j].vid[w];
					if (v < 0 || v >= c->sm[sm].nv || c->sm[sm].map[v] < 0) continue;
					int l = c->sm[sm].map[v];
					float *ww = s->bw + l*4; unsigned char *ii = s->bi + l*4;
					int slot = 0; // empty slot, else replace the smallest
					for (int q = 1; q < 4; q++) if (ww[q] < ww[slot]) slot = q;
					if (c->bn[j].w[w] > ww[slot]) { ww[slot] = c->bn[j].w[w]; ii[slot] = bi; }
				}
			}
			bi++;
		}
		for (int si = 0; si < md->ns; si++) {
			surf *s = md->s[si];
			for (int v = 0; v < s->nv; v++) {
				float *ww = s->bw + v*4, t = ww[0] + ww[1] + ww[2] + ww[3];
				if (t > 0) for (int q = 0; q < 4; q++) ww[q] /= t;
			}
		}
	}
}

ent b3d_load(const char *p, ent par) {
	const char *rp = res_path(p);
	if (!rp) return 0;
	ctx c = {.r = {.f = fopen(rp, "rb")}};
	if (!c.r.f) return 0;
	snprintf(c.dir, sizeof c.dir, "%s", p);
	char *sl = NULL;
	for (char *q = c.dir; *q; q++) if (*q == '\\' || *q == '/') sl = q;
	if (sl) sl[1] = 0; else c.dir[0] = 0;
	fseek(c.r.f, 0, SEEK_END); c.r.end[0] = ftell(c.r.f); rewind(c.r.f);
	char t[5]; ent root = 0;
	if (tag(&c.r, t) && !strcmp(t, "BB3D")) {
		ri(&c.r); // version
		while (tag(&c.r, t)) {
			if (!strcmp(t, "TEXS")) texs(&c);
			else if (!strcmp(t, "BRUS")) brus(&c);
			else if (!strcmp(t, "NODE") && !root) root = node(&c, par);
			leave(&c.r);
		}
	}
	fclose(c.r.f);
	if (root) skins(&c);
	for (int i = 0; i < c.nsm; i++) free(c.sm[i].map);
	for (int i = 0; i < c.nbn; i++) { free(c.bn[i].vid); free(c.bn[i].w); }
	free(c.sm); free(c.bn);
	for (int i = 0; i < c.ntx; i++) free_texture(c.tx[i]); // brushes keep refs
	free(c.tx); free(c.br);
	return root;
}

// dispatch on extension
static ent load(const char *p, ent par) {
	size_t n = strlen(p);
	if (n > 2 && !strcasecmp(p + n - 2, ".x")) return x_load(p, par);
	if (n > 4 && !strcasecmp(p + n - 4, ".b3d")) return b3d_load(p, par);
	fprintf(stderr, "rt: don't know how to load %s\n", p);
	return 0;
}

ent load_anim_mesh(const char *p, ent par) {
	ent e = load(p, par);
	if (e && !rt_e(e)->md) { E *x = rt_e(e); x->kind = EK_MESH; x->md = md_new(); }
	return e;
}

// flatten subtree meshes into dst, in dst-local space
static void bake(ent e, ent dst, tf t) {
	E *x = rt_e(e);
	if (x->md) {
		ent tmp = copy_mesh(e, 0);
		mdat *m = rt_md(tmp);
		m3 nm = m3t(m3inv(t.m));
		for (int i = 0; i < m->ns; i++) for (int v = 0; v < m->s[i]->nv; v++) {
			float *p = m->s[i]->pos + v*3, *n = m->s[i]->nrm + v*3;
			v3 a = tfp(t, v3n(p[0], p[1], p[2])), b = v3norm(m3v(nm, v3n(n[0], n[1], n[2])));
			memcpy(p, &a, 12); memcpy(n, &b, 12);
		}
		add_mesh(tmp, dst);
		free_entity(tmp);
	}
	for (ent ch = rt_e(e)->first; ch; ch = rt_e(ch)->next) {
		E *y = rt_e(ch);
		bake(ch, dst, tfmul(t, (tf){m3mul(y->rot, m3scale(y->scl)), y->pos}));
	}
}
ent load_mesh(const char *p, ent par) {
	ent root = load(p, 0);
	if (!root) return 0;
	ent m = create_mesh(par);
	E *r = rt_e(root);
	bake(root, m, (tf){m3mul(r->rot, m3scale(r->scl)), r->pos});
	free_entity(root);
	return m;
}
