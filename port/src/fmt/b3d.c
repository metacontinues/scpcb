// b3d.c: .b3d loader. LoadAnimMesh keeps the node tree, LoadMesh bakes it flat.
// skinning (BONE) is parsed but not applied yet.
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../bb/in.h"

typedef struct { FILE *f; long end[32]; int d; } rd;

static int tag(rd *r, char *t) { // enter next chunk if any left in the current one
	if (ftell(r->f) >= r->end[r->d]) return 0;
	if (fread(t, 1, 4, r->f) != 4) return 0;
	t[4] = 0;
	int n = bbReadInt(r->f);
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
static float rf(rd *r) { return bbReadFloat(r->f); }
static int ri(rd *r) { return bbReadInt(r->f); }

// blitz Quat::getMatrix
static m3 qmat(float w, float x, float y, float z) {
	float xx = x*x, yy = y*y, zz = z*z, xy = x*y, xz = x*z, yz = y*z, wx = w*x, wy = w*y, wz = w*z;
	return (m3){{1-2*(yy+zz), 2*(xy-wz), 2*(xz+wy)}, {2*(xy+wz), 1-2*(xx+zz), 2*(yz-wx)}, {2*(xz-wy), 2*(yz+wx), 1-2*(xx+yy)}};
}

typedef struct {
	rd r;
	char dir[512];
	htex *tx; int ntx;
	brush *br; int nbr;
} ctx;

static htex loadtex(ctx *c, const char *name, int flags) {
	char p[1024];
	const char *b = strrchr(name, '\\'), *b2 = strrchr(name, '/');
	if (b2 > b) b = b2;
	snprintf(p, sizeof p, "%s%s", c->dir, name);
	htex t = bbPath(p) ? bbLoadTexture(p, flags) : 0;
	if (!t && b) { snprintf(p, sizeof p, "%s%s", c->dir, b + 1); t = bbLoadTexture(p, flags); }
	if (!t) t = bbLoadTexture(name, flags);
	return t;
}

static void texs(ctx *c) {
	while (more(&c->r)) {
		char n[256]; cstr(&c->r, n, sizeof n);
		int fl = ri(&c->r), bl = ri(&c->r);
		float px = rf(&c->r), py = rf(&c->r), sx = rf(&c->r), sy = rf(&c->r); rf(&c->r);
		htex t = loadtex(c, n, fl | TF_MIP);
		if (t) { bbTextureBlend(t, bl); bbScaleTexture(t, sx, sy); bbPositionTexture(t, px, py); }
		c->tx = realloc(c->tx, sizeof *c->tx * (c->ntx + 1));
		c->tx[c->ntx++] = t;
	}
}
static void brus(ctx *c) {
	int nt = ri(&c->r);
	while (more(&c->r)) {
		char n[256]; cstr(&c->r, n, sizeof n);
		brush b = bb_brdef();
		b.r = rf(&c->r); b.g = rf(&c->r); b.b = rf(&c->r); b.a = rf(&c->r); b.shine = rf(&c->r);
		b.blend = ri(&c->r); b.fx = ri(&c->r);
		for (int i = 0; i < nt; i++) {
			int id = ri(&c->r);
			if (i < BB_NTEX && id >= 0 && id < c->ntx) b.tex[i] = c->tx[id];
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
	E *x = bb_e(e);
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
			brush b = bi >= 0 && bi < c->nbr ? c->br[bi] : bb_brdef();
			surf *s = md_addsurf(bb_md(e), b);
			int *map = malloc(sizeof *map * (nv ? nv : 1));
			for (int i = 0; i < nv; i++) map[i] = -1;
			while (more(&c->r)) {
				int ix[3];
				for (int k = 0; k < 3; k++) {
					int v = ri(&c->r);
					if (v < 0 || v >= nv) v = 0;
					if (map[v] < 0) {
						vtx *p = &vs[v];
						map[v] = bbAddVertex(s, p->p[0], p->p[1], p->p[2], p->uv[0][0], p->uv[0][1], 0);
						bbVertexTexCoords(s, map[v], p->uv[1][0], p->uv[1][1], 0, 1);
						bbVertexNormal(s, map[v], p->n[0], p->n[1], p->n[2]);
						bbVertexColor(s, map[v], p->c[0]*255, p->c[1]*255, p->c[2]*255, p->c[3]);
					}
					ix[k] = map[v];
				}
				bbAddTriangle(s, ix[0], ix[1], ix[2]);
			}
			free(map);
		}
		leave(&c->r);
	}
	free(vs);
	if (!hasn) bbUpdateNormals(e);
}

static void keys(ctx *c, ent e) {
	int fl = ri(&c->r);
	while (more(&c->r)) {
		E *x = bb_e(e);
		x->keys = realloc(x->keys, sizeof *x->keys * (x->nk + 1));
		key *k = &x->keys[x->nk++];
		*k = (key){.t = ri(&c->r), .has = fl, .s = {1, 1, 1}, .r = m3id()};
		if (fl & 1) k->p = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
		if (fl & 2) k->s = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
		if (fl & 4) { float w = rf(&c->r), a = rf(&c->r), b = rf(&c->r), d = rf(&c->r); k->r = qmat(w, a, b, d); }
	}
}

static ent node(ctx *c, ent par) {
	char n[256]; cstr(&c->r, n, sizeof n);
	ent e = bb_new(EK_PIVOT, par);
	bbNameEntity(e, n);
	E *x = bb_e(e);
	x->pos = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
	x->scl = v3n(rf(&c->r), rf(&c->r), rf(&c->r));
	float w = rf(&c->r), a = rf(&c->r), b = rf(&c->r), d = rf(&c->r);
	x->rot = m3ortho(qmat(w, a, b, d));
	char t[5];
	while (tag(&c->r, t)) {
		if (!strcmp(t, "MESH")) mesh(c, e);
		else if (!strcmp(t, "KEYS")) keys(c, e);
		else if (!strcmp(t, "NODE")) node(c, e);
		leave(&c->r); // ANIM/BONE skipped for now
	}
	bb_dirty(e);
	return e;
}

static int isb3d(const char *p) { size_t n = strlen(p); return n > 4 && !strcasecmp(p + n - 4, ".b3d"); }

static ent load(const char *p, ent par) {
	const char *rp = bbPath(p);
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
	for (int i = 0; i < c.ntx; i++) bbFreeTexture(c.tx[i]); // brushes keep refs
	free(c.tx); free(c.br);
	return root;
}

ent bbLoadAnimMesh(const char *p, ent par) {
	if (!isb3d(p)) { fprintf(stderr, "bb: LoadAnimMesh %s: only .b3d so far\n", p); return 0; }
	ent e = load(p, par);
	if (e && !bb_e(e)->md) { E *x = bb_e(e); x->kind = EK_MESH; x->md = md_new(); }
	return e;
}

// flatten subtree meshes into dst, in dst-local space
static void bake(ent e, ent dst, tf t) {
	E *x = bb_e(e);
	if (x->md) {
		ent tmp = bbCopyMesh(e, 0);
		mdat *m = bb_md(tmp);
		m3 nm = m3t(m3inv(t.m));
		for (int i = 0; i < m->ns; i++) for (int v = 0; v < m->s[i]->nv; v++) {
			float *p = m->s[i]->pos + v*3, *n = m->s[i]->nrm + v*3;
			v3 a = tfp(t, v3n(p[0], p[1], p[2])), b = v3norm(m3v(nm, v3n(n[0], n[1], n[2])));
			memcpy(p, &a, 12); memcpy(n, &b, 12);
		}
		bbAddMesh(tmp, dst);
		bbFreeEntity(tmp);
	}
	for (ent ch = bb_e(e)->first; ch; ch = bb_e(ch)->next) {
		E *y = bb_e(ch);
		bake(ch, dst, tfmul(t, (tf){m3mul(y->rot, m3scale(y->scl)), y->pos}));
	}
}
ent bbLoadMesh(const char *p, ent par) {
	if (!isb3d(p)) { fprintf(stderr, "bb: LoadMesh %s: only .b3d so far (.x is TODO)\n", p); return 0; }
	ent root = load(p, 0);
	if (!root) return 0;
	ent m = bbCreateMesh(par);
	E *r = bb_e(root);
	bake(root, m, (tf){m3mul(r->rot, m3scale(r->scl)), r->pos});
	bbFreeEntity(root);
	return m;
}
