// x.c: directx .x loader. text + binary, plus mszip ("bzip") if it's one block.
// one tokenizer over both encodings, one parser on top. dx is lh/cw like blitz, so no conversion.
// skin weights / AnimationSet are skipped.
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "../bb/in.h"

enum { T_EOF, T_NAME, T_STR, T_NUM, T_OB, T_CB };
typedef struct { int k; double n; char s[256]; } tok;

typedef struct {
	unsigned char *b; size_t n, i;
	int bin, f64;
	int lk; size_t ln; // pending binary list: kind (6 int, 7 float), remaining
	tok pk; int haspk;
	char dir[512];
	struct { char name[128]; brush br; } *mats; int nmat;
} xr;

// ---- tokenizer
static int delim(int c) { return !c || isspace(c) || strchr(",;{}\"<", c); }

static uint32_t u32(xr *r) { uint32_t v = 0; if (r->i + 4 <= r->n) memcpy(&v, r->b + r->i, 4); r->i += 4; return v; }
static uint16_t u16(xr *r) { uint16_t v = 0; if (r->i + 2 <= r->n) memcpy(&v, r->b + r->i, 2); r->i += 2; return v; }

static tok bnext(xr *r) {
	tok t = {0};
	for (;;) {
		if (r->ln) { // drain lists one number at a time
			r->ln--;
			t.k = T_NUM;
			if (r->lk == 6) t.n = u32(r);
			else if (r->f64) { double d = 0; if (r->i + 8 <= r->n) memcpy(&d, r->b + r->i, 8); r->i += 8; t.n = d; }
			else { float f; uint32_t v = u32(r); memcpy(&f, &v, 4); t.n = f; }
			return t;
		}
		if (r->i + 2 > r->n) return t;
		int c = u16(r);
		switch (c) {
		case 1: case 2: { // name, string (+ terminator token)
			uint32_t l = u32(r);
			if (r->i + l > r->n) return t;
			snprintf(t.s, sizeof t.s, "%.*s", (int)l, r->b + r->i);
			r->i += l;
			if (c == 2) r->i += 2;
			t.k = c == 1 ? T_NAME : T_STR;
			return t;
		}
		case 3: t.k = T_NUM; t.n = (int)u32(r); return t;
		case 5: r->i += 16; break; // guid
		case 6: case 7: r->lk = c; r->ln = u32(r); break;
		case 10: t.k = T_OB; return t;
		case 11: t.k = T_CB; return t;
		case 31: strcpy(t.s, "template"); t.k = T_NAME; return t;
		default:
			if (c >= 40 && c <= 52) { t.k = T_NAME; strcpy(t.s, "_type"); return t; } // template body types
			break; // punctuation
		}
	}
}

static tok tnext(xr *r) {
	tok t = {0};
	for (;;) {
		while (r->i < r->n && (isspace(r->b[r->i]) || r->b[r->i] == ',' || r->b[r->i] == ';')) r->i++;
		if (r->i >= r->n) return t;
		char *p = (char *)r->b + r->i;
		if (p[0] == '#' || (p[0] == '/' && p[1] == '/')) { while (r->i < r->n && r->b[r->i] != '\n') r->i++; continue; }
		if (p[0] == '<') { while (r->i < r->n && r->b[r->i] != '>') r->i++; r->i++; continue; }
		break;
	}
	char *p = (char *)r->b + r->i;
	if (*p == '{') { r->i++; t.k = T_OB; return t; }
	if (*p == '}') { r->i++; t.k = T_CB; return t; }
	if (*p == '"') {
		size_t j = ++r->i;
		while (r->i < r->n && r->b[r->i] != '"') r->i++;
		snprintf(t.s, sizeof t.s, "%.*s", (int)(r->i - j), r->b + j);
		r->i++; t.k = T_STR; return t;
	}
	char *e;
	t.n = strtod(p, &e);
	if (e != p && (size_t)(e - (char *)r->b) <= r->n && delim(*e)) { r->i = e - (char *)r->b; t.k = T_NUM; return t; }
	size_t j = r->i;
	while (r->i < r->n && !delim(r->b[r->i])) r->i++;
	snprintf(t.s, sizeof t.s, "%.*s", (int)(r->i - j), r->b + j);
	t.k = T_NAME;
	return t;
}

static tok next(xr *r) { if (r->haspk) { r->haspk = 0; return r->pk; } return r->bin ? bnext(r) : tnext(r); }
static double num(xr *r) { tok t = next(r); return t.k == T_NUM ? t.n : 0; }
static int cnt(xr *r, int max) { int n = num(r); return n < 0 ? 0 : n > max ? max : n; }

static void skip(xr *r) { // rest of a block, '{' already eaten
	for (int d = 1; d;) { tok t = next(r); if (!t.k) return; d += t.k == T_OB ? 1 : t.k == T_CB ? -1 : 0; }
}
static void close_(xr *r) { // eat stuff until our '}'
	for (;;) { tok t = next(r); if (!t.k || t.k == T_CB) return; if (t.k == T_OB) skip(r); }
}
// "Type [name] {" -> name, 0 if no '{'
static int open_(xr *r, char *name) {
	name[0] = 0;
	tok t = next(r);
	if (t.k == T_NAME) { snprintf(name, 128, "%s", t.s); t = next(r); }
	return t.k == T_OB;
}

// ---- materials
static brush material(xr *r) {
	brush b = bb_brdef();
	b.r = num(r); b.g = num(r); b.b = num(r); b.a = num(r);
	b.shine = num(r) / 128.0f; // power
	for (int i = 0; i < 6; i++) num(r); // specular, emissive
	char n[128];
	for (;;) {
		tok t = next(r);
		if (!t.k || t.k == T_CB) break;
		if (t.k == T_OB) { skip(r); continue; }
		if (t.k != T_NAME) continue;
		if (!open_(r, n)) continue;
		if (!strcasecmp(t.s, "TextureFilename")) {
			tok s = next(r);
			if (s.k == T_STR && s.s[0]) {
				char p[1024];
				snprintf(p, sizeof p, "%s%s", r->dir, s.s);
				htex tx = bbLoadTexture(p, TF_COLOR | TF_MIP);
				const char *bn = s.s;
				for (const char *q = s.s; *q; q++) if (*q == '/' || *q == '\\') bn = q + 1;
				if (!tx) { snprintf(p, sizeof p, "%s%s", r->dir, bn); tx = bbLoadTexture(p, TF_COLOR | TF_MIP); } // exporter's abs paths
				if (!tx) tx = bbLoadTexture(s.s, TF_COLOR | TF_MIP);
				if (tx) { b.tex[0] = tx; } // brush owns the load ref
				else fprintf(stderr, "x: texture %s missing\n", p);
			}
			close_(r);
		} else skip(r);
	}
	if (b.a < 1) b.blend = BL_ALPHA;
	return b;
}
static void addmat(xr *r, const char *n, brush b) {
	r->mats = realloc(r->mats, sizeof *r->mats * (r->nmat + 1));
	snprintf(r->mats[r->nmat].name, 128, "%s", n);
	r->mats[r->nmat++].br = b;
}
static brush matref(xr *r, const char *n) {
	for (int i = r->nmat - 1; i >= 0; i--) if (!strcmp(r->mats[i].name, n)) { brush b = r->mats[i].br; br_ref(&b); return b; }
	return bb_brdef();
}

// ---- meshes
#define MAXN (1 << 22)
typedef struct { int nv, nf, nn, nuv, nfi, nb, *fc, *fv, *fn, *fm, *vc; float *v, *n, *uv, *col; brush *b; } xm;

static void mesh(xr *r, ent e) {
	xm m = {0};
	m.nv = cnt(r, MAXN);
	m.v = malloc(sizeof(float) * 3 * (m.nv + 1));
	for (int i = 0; i < m.nv * 3; i++) m.v[i] = num(r);
	m.nf = cnt(r, MAXN);
	m.fc = malloc(sizeof(int) * (m.nf + 1));
	int cap = 0, tot = 0;
	for (int f = 0; f < m.nf; f++) {
		int k = m.fc[f] = cnt(r, 64);
		if (tot + k > cap) { cap = (tot + k) * 2; m.fv = realloc(m.fv, sizeof(int) * cap); }
		for (int j = 0; j < k; j++) { int v = num(r); m.fv[tot++] = v >= 0 && v < m.nv ? v : 0; }
	}
	char n[128];
	for (;;) {
		tok t = next(r);
		if (!t.k || t.k == T_CB) break;
		if (t.k == T_OB) { skip(r); continue; }
		if (t.k != T_NAME || !open_(r, n)) continue;
		if (!strcasecmp(t.s, "MeshNormals")) {
			m.nn = cnt(r, MAXN);
			m.n = malloc(sizeof(float) * 3 * (m.nn + 1));
			for (int i = 0; i < m.nn * 3; i++) m.n[i] = num(r);
			int nf = cnt(r, MAXN), o = 0;
			m.fn = malloc(sizeof(int) * (tot + 1));
			for (int f = 0; f < nf; f++) {
				int k = cnt(r, 64);
				for (int j = 0; j < k; j++) { int v = num(r); if (f < m.nf && o < tot) m.fn[o++] = v >= 0 && v < m.nn ? v : 0; }
			}
			for (; o < tot; o++) m.fn[o] = 0;
			close_(r);
		} else if (!strcasecmp(t.s, "MeshTextureCoords")) {
			m.nuv = cnt(r, MAXN);
			m.uv = malloc(sizeof(float) * 2 * (m.nuv + 1));
			for (int i = 0; i < m.nuv * 2; i++) m.uv[i] = num(r);
			close_(r);
		} else if (!strcasecmp(t.s, "MeshVertexColors")) {
			int k = cnt(r, MAXN);
			m.col = malloc(sizeof(float) * 4 * (m.nv + 1));
			for (int i = 0; i < m.nv * 4; i++) m.col[i] = 1;
			for (int i = 0; i < k; i++) {
				int v = num(r); float c[4] = {num(r), num(r), num(r), num(r)};
				if (v >= 0 && v < m.nv) memcpy(m.col + v * 4, c, 16);
			}
			close_(r);
		} else if (!strcasecmp(t.s, "MeshMaterialList")) {
			int nm = cnt(r, 4096);
			m.nfi = cnt(r, MAXN);
			m.fm = malloc(sizeof(int) * (m.nfi + 1));
			for (int i = 0; i < m.nfi; i++) m.fm[i] = num(r);
			for (;;) {
				tok u = next(r);
				if (!u.k || u.k == T_CB) break;
				brush b;
				if (u.k == T_OB) { tok rn = next(r); b = matref(r, rn.s); if (rn.k != T_CB) close_(r); }
				else if (u.k == T_NAME && !strcasecmp(u.s, "Material") && open_(r, n)) b = material(r);
				else { if (u.k == T_NAME && open_(r, n)) skip(r); continue; }
				m.b = realloc(m.b, sizeof *m.b * (m.nb + 1));
				m.b[m.nb++] = b;
			}
			(void)nm;
		} else skip(r);
	}

	// build: one surface per material, corners deduped on (vertex, normal)
	E *x = bb_e(e);
	if (!x->md) { x->kind = EK_MESH; x->md = md_new(); }
	int ns = m.nb ? m.nb : 1;
	int *key = malloc(sizeof(int) * 2 * (tot + 1) * 2), hs = (tot + 1) * 2; // open addressing: (vi*nn+ni) -> local idx
	for (int s = 0; s < ns; s++) {
		surf *sf = NULL;
		for (int i = 0; i < hs * 2; i++) key[i] = -1;
		int o = 0;
		for (int f = 0; f < m.nf; f++) {
			int k = m.fc[f], mi = m.fm && m.nfi ? m.fm[f < m.nfi ? f : m.nfi - 1] : 0;
			if (mi < 0 || mi >= ns) mi = 0;
			if (mi != s || k < 3) { o += k; continue; }
			if (!sf) sf = md_addsurf(bb_md(e), m.nb ? m.b[s] : bb_brdef());
			int ix[64];
			for (int j = 0; j < k; j++) {
				int vi = m.fv[o + j], ni = m.fn ? m.fn[o + j] : 0, kk = vi * (m.nn ? m.nn : 1) + ni;
				unsigned h = (unsigned)kk * 2654435761u % hs;
				while (key[h*2] >= 0 && key[h*2] != kk) h = (h + 1) % hs;
				if (key[h*2] < 0) {
					float *p = m.v + vi * 3;
					int v = bbAddVertex(sf, p[0], p[1], p[2], 0, 0, 0);
					if (m.uv && vi < m.nuv) { bbVertexTexCoords(sf, v, m.uv[vi*2], m.uv[vi*2+1], 0, 0); bbVertexTexCoords(sf, v, m.uv[vi*2], m.uv[vi*2+1], 0, 1); }
					if (m.n) { float *q = m.n + ni * 3; bbVertexNormal(sf, v, q[0], q[1], q[2]); }
					if (m.col) { float *c = m.col + vi * 4; bbVertexColor(sf, v, c[0]*255, c[1]*255, c[2]*255, c[3]); }
					key[h*2] = kk; key[h*2+1] = v;
				}
				ix[j] = key[h*2+1];
			}
			for (int j = 2; j < k; j++) bbAddTriangle(sf, ix[0], ix[j-1], ix[j]);
			o += k;
		}
	}
	for (int s = 0; s < m.nb; s++) br_unref(&m.b[s]); // surfaces took their own refs
	free(key); free(m.v); free(m.fc); free(m.fv); free(m.fn); free(m.fm); free(m.n); free(m.uv); free(m.col); free(m.b);
	if (!m.n) bbUpdateNormals(e);
}

static void block(xr *r, const char *type, ent par);
static void frame(xr *r, ent e) {
	for (;;) {
		tok t = next(r);
		if (!t.k || t.k == T_CB) return;
		if (t.k == T_OB) { skip(r); continue; }
		if (t.k == T_NAME) block(r, t.s, e);
	}
}
static void block(xr *r, const char *type, ent par) {
	char n[128];
	if (!strcasecmp(type, "template")) { if (open_(r, n)) skip(r); return; }
	if (!open_(r, n)) return;
	if (!strcasecmp(type, "Frame")) {
		ent e = bb_new(EK_PIVOT, par);
		bbNameEntity(e, n);
		frame(r, e);
	} else if (!strcasecmp(type, "FrameTransformMatrix")) {
		float a[16];
		for (int i = 0; i < 16; i++) a[i] = num(r);
		E *x = bb_e(par);
		m3 m = {{a[0], a[1], a[2]}, {a[4], a[5], a[6]}, {a[8], a[9], a[10]}}; // rows = axes
		x->pos = v3n(a[12], a[13], a[14]);
		x->scl = v3n(v3len(m.i), v3len(m.j), v3len(m.k));
		x->rot = m3ortho(m);
		bb_dirty(par);
		close_(r);
	} else if (!strcasecmp(type, "Mesh")) {
		ent e = bb_new(EK_MESH, par); // own child, so frame matrices still apply
		bbNameEntity(e, n);
		bb_e(e)->md = md_new();
		mesh(r, e);
	} else if (!strcasecmp(type, "Material")) {
		addmat(r, n, material(r));
	} else skip(r);
}

// mszip: "CK" + raw deflate per block. single block only (sinfl has no dict support)
static unsigned char *unzip(unsigned char *b, size_t n, size_t *on) {
	if (n < 28) return NULL;
	uint32_t full; memcpy(&full, b + 16, 4);
	uint16_t un, cn; memcpy(&un, b + 20, 2); memcpy(&cn, b + 22, 2);
	if (memcmp(b + 24, "CK", 2) || 26 + (size_t)cn - 2 > n) return NULL;
	if ((uint32_t)un + 16 < full) { fprintf(stderr, "x: multi-block mszip not supported\n"); return NULL; }
	int dn = 0;
	unsigned char *d = DecompressData(b + 26, cn - 2, &dn);
	if (!d) return NULL;
	unsigned char *o = malloc(16 + dn);
	memcpy(o, b, 16); memcpy(o + 8, "bin ", 4); memcpy(o + 16, d, dn);
	MemFree(d);
	*on = 16 + dn;
	return o;
}

ent x_load(const char *p, ent par) {
	const char *rp = bbPath(p);
	if (!rp) return 0;
	int n = 0;
	unsigned char *b = LoadFileData(rp, &n);
	if (!b || n < 16 || memcmp(b, "xof ", 4)) { UnloadFileData(b); return 0; }
	xr r = {.b = b, .n = n, .i = 16};
	if (!memcmp(b + 8, "bzip", 4)) {
		size_t on;
		unsigned char *u = unzip(b, n, &on);
		UnloadFileData(b);
		if (!u) { fprintf(stderr, "x: can't unpack %s\n", p); return 0; }
		r.b = u; r.n = on;
	}
	r.bin = !memcmp(r.b + 8, "bin ", 4);
	r.f64 = !memcmp(r.b + 12, "0064", 4);
	snprintf(r.dir, sizeof r.dir, "%s", p);
	char *sl = NULL;
	for (char *q = r.dir; *q; q++) if (*q == '\\' || *q == '/') sl = q;
	if (sl) sl[1] = 0; else r.dir[0] = 0;

	ent root = bb_new(EK_PIVOT, par);
	for (;;) {
		tok t = next(&r);
		if (!t.k) break;
		if (t.k == T_NAME) block(&r, t.s, root);
		else if (t.k == T_OB) skip(&r);
	}
	for (int i = 0; i < r.nmat; i++) br_unref(&r.mats[i].br);
	free(r.mats);
	if (r.b != b) free(r.b); else UnloadFileData(b);
	return root;
}
