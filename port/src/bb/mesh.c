// mesh.c: brushes, surfaces, mesh data + ops
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "in.h"
#include "rlgl.h"
#include "config.h" // MAX_MESH_VERTEX_BUFFERS

// ---- brushes
static brush *bs; static char *bused; static int nbs, cbs;

brush bb_brdef(void) { return (brush){.r = 1, .g = 1, .b = 1, .a = 1}; }
void br_ref(brush *b) { for (int i = 0; i < BB_NTEX; i++) tex_ref(b->tex[i]); }
void br_unref(brush *b) { for (int i = 0; i < BB_NTEX; i++) tex_unref(b->tex[i]); }
static void br_settex(brush *b, htex t, int i) {
	if (i < 0 || i >= BB_NTEX) return;
	tex_ref(t); tex_unref(b->tex[i]); b->tex[i] = t;
}
static void br_assign(brush *d, brush s) { br_ref(&s); br_unref(d); *d = s; }

brush *bb_brp(hbrush h) {
	if (h <= 0 || h >= nbs || !bused[h]) die("bad brush handle %d", h);
	return &bs[h];
}
static hbrush br_new(brush b) {
	if (!nbs) nbs = 1;
	hbrush h = 0;
	for (int i = 1; i < nbs; i++) if (!bused[i]) { h = i; break; }
	if (!h) { int c = cbs; grow(bs, nbs, cbs); if (c != cbs) { bused = realloc(bused, cbs); } h = nbs++; }
	br_ref(&b);
	bs[h] = b; bused[h] = 1;
	return h;
}
hbrush bbCreateBrush(float r, float g, float b) {
	brush x = bb_brdef();
	x.r = r/255; x.g = g/255; x.b = b/255;
	return br_new(x);
}
void bbFreeBrush(hbrush h) { if (!h) return; br_unref(bb_brp(h)); bused[h] = 0; }
void bbBrushColor(hbrush h, float r, float g, float b) { brush *x = bb_brp(h); x->r = r/255; x->g = g/255; x->b = b/255; }
void bbBrushAlpha(hbrush h, float a) { bb_brp(h)->a = a; }
void bbBrushFX(hbrush h, int fx) { bb_brp(h)->fx = fx; }
void bbBrushBlend(hbrush h, int bl) { bb_brp(h)->blend = bl; }
void bbBrushTexture(hbrush h, htex t, int fr, int i) { (void)fr; br_settex(bb_brp(h), t, i); }
htex bbGetBrushTexture(hbrush h, int i) { return i >= 0 && i < BB_NTEX ? bb_brp(h)->tex[i] : 0; }
hbrush bbGetSurfaceBrush(surf *s) { return br_new(s->br); }
hbrush bbGetEntityBrush(ent e) { return br_new(bb_e(e)->br); }

void bbEntityColor(ent e, float r, float g, float b) { E *x = bb_e(e); x->br.r = r/255; x->br.g = g/255; x->br.b = b/255; }
void bbEntityAlpha(ent e, float a) { bb_e(e)->br.a = a; }
void bbEntityShininess(ent e, float s) { bb_e(e)->br.shine = s; }
void bbEntityFX(ent e, int fx) { bb_e(e)->br.fx = fx; }
void bbEntityBlend(ent e, int bl) { bb_e(e)->br.blend = bl; }
void bbEntityTexture(ent e, htex t, int fr, int i) { (void)fr; E *x = bb_e(e); br_settex(&x->br, t, i); x->hasbr = 1; }
void bbEntityOrder(ent e, int o) { bb_e(e)->order = o; }
void bbPaintEntity(ent e, hbrush h) { E *x = bb_e(e); br_assign(&x->br, *bb_brp(h)); x->hasbr = 1; }

// ---- mesh data
mdat *md_new(void) { mdat *m = calloc(1, sizeof *m); m->rc = 1; m->bdirty = 1; return m; }
mdat *bb_md(ent e) { E *x = bb_e(e); if (!x->md) die("entity %d is not a mesh", e); return x->md; }

surf *md_addsurf(mdat *m, brush br) {
	surf *s = calloc(1, sizeof *s);
	br_ref(&br); s->br = br; s->dirty = 1, s->ver++;
	m->s = realloc(m->s, sizeof *m->s * (m->ns + 1));
	m->s[m->ns++] = s;
	return s;
}
static void gpu_free(surf *s) {
	if (!s->gpu.vboId) return;
	rlUnloadVertexArray(s->gpu.vaoId);
	for (int i = 0; i < MAX_MESH_VERTEX_BUFFERS; i++) rlUnloadVertexBuffer(s->gpu.vboId[i]);
	RL_FREE(s->gpu.vboId);
	s->gpu = (Mesh){0};
}
static void surf_free(surf *s) {
	gpu_free(s); br_unref(&s->br);
	free(s->pos); free(s->nrm); free(s->uv0); free(s->uv1); free(s->col); free(s->idx);
	free(s);
}
#define DUP(p, n) ((p) ? memcpy(malloc(n), (p), (n)) : NULL)
static surf *surf_copy(surf *s) {
	surf *d = calloc(1, sizeof *d);
	*d = *s; d->gpu = (Mesh){0}; d->dirty = 1; br_ref(&d->br);
	d->cv = d->nv; d->ct = d->nt;
	d->pos = DUP(s->pos, s->nv * 12); d->nrm = DUP(s->nrm, s->nv * 12);
	d->uv0 = DUP(s->uv0, s->nv * 8); d->uv1 = DUP(s->uv1, s->nv * 8);
	d->col = DUP(s->col, s->nv * 4); d->idx = DUP(s->idx, s->nt * 6);
	return d;
}
mdat *md_copy(mdat *m) {
	mdat *d = md_new();
	for (int i = 0; i < m->ns; i++) {
		d->s = realloc(d->s, sizeof *d->s * (d->ns + 1));
		d->s[d->ns++] = surf_copy(m->s[i]);
	}
	return d;
}
void md_unref(mdat *m) {
	if (--m->rc > 0) return;
	for (int i = 0; i < m->ns; i++) surf_free(m->s[i]);
	free(m->s); free(m);
}
void md_bounds(mdat *m) {
	int d = m->bdirty; // render calls this before upload clears s->dirty
	for (int i = 0; i < m->ns; i++) d |= m->s[i]->dirty;
	if (!d) return;
	v3 lo = {FLT_MAX, FLT_MAX, FLT_MAX}, hi = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	for (int i = 0; i < m->ns; i++) for (int j = 0; j < m->s[i]->nv; j++) {
		float *p = m->s[i]->pos + j*3;
		lo = v3n(fminf(lo.x, p[0]), fminf(lo.y, p[1]), fminf(lo.z, p[2]));
		hi = v3n(fmaxf(hi.x, p[0]), fmaxf(hi.y, p[1]), fmaxf(hi.z, p[2]));
	}
	if (lo.x > hi.x) lo = hi = v3n(0, 0, 0);
	m->lo = lo; m->hi = hi; m->bdirty = 0;
}
static void touch(mdat *m) { m->bdirty = 1; for (int i = 0; i < m->ns; i++) m->s[i]->dirty = 1, m->s[i]->ver++; }

void surf_upload(surf *s) {
	if (!s->dirty) return;
	gpu_free(s);
	s->dirty = 0;
	if (!s->nv || !s->nt) return;
	if (s->nv > 65535) fprintf(stderr, "bb: surface has %d verts (>65535), es2 can't index it\n", s->nv);
	s->gpu = (Mesh){.vertexCount = s->nv, .triangleCount = s->nt, .vertices = s->pos, .normals = s->nrm,
	                .texcoords = s->uv0, .texcoords2 = s->uv1, .colors = s->col, .indices = s->idx};
	UploadMesh(&s->gpu, false);
}

// ---- entity-level mesh api
ent bbCreateMesh(ent par) { ent e = bb_new(EK_MESH, par); bb_e(e)->md = md_new(); return e; }
ent bbCopyMesh(ent e, ent par) {
	ent n = bbCreateMesh(par);
	md_unref(bb_e(n)->md);
	bb_e(n)->md = md_copy(bb_md(e));
	return n;
}
int bbCountSurfaces(ent e) { return bb_md(e)->ns; }
surf *bbGetSurface(ent e, int n) { mdat *m = bb_md(e); return n >= 1 && n <= m->ns ? m->s[n-1] : NULL; }
surf *bbCreateSurface(ent e, hbrush b) {
	mdat *m = bb_md(e);
	m->bdirty = 1;
	return md_addsurf(m, b ? *bb_brp(b) : bb_brdef());
}
void bbPaintSurface(surf *s, hbrush b) { br_assign(&s->br, *bb_brp(b)); }
void bbPaintMesh(ent e, hbrush b) { mdat *m = bb_md(e); for (int i = 0; i < m->ns; i++) bbPaintSurface(m->s[i], b); }

// blitz merges into a dst surface with an equal brush
static int breq(brush *a, brush *b) { return !memcmp(a, b, sizeof *a); }
static void surf_append(surf *d, surf *s) {
	int base = d->nv;
	for (int i = 0; i < s->nv; i++) {
		float *p = s->pos + i*3;
		int v = bbAddVertex(d, p[0], p[1], p[2], 0, 0, 0);
		memcpy(d->nrm + v*3, s->nrm + i*3, 12); memcpy(d->uv0 + v*2, s->uv0 + i*2, 8);
		memcpy(d->uv1 + v*2, s->uv1 + i*2, 8); memcpy(d->col + v*4, s->col + i*4, 4);
	}
	for (int i = 0; i < s->nt; i++)
		bbAddTriangle(d, base + s->idx[i*3], base + s->idx[i*3+1], base + s->idx[i*3+2]);
}
void bbAddMesh(ent se, ent de) {
	mdat *s = bb_md(se), *d = bb_md(de);
	for (int i = 0; i < s->ns; i++) {
		surf *t = NULL;
		for (int j = 0; j < d->ns && !t; j++) if (breq(&d->s[j]->br, &s->s[i]->br)) t = d->s[j];
		if (!t) t = md_addsurf(d, s->s[i]->br);
		surf_append(t, s->s[i]);
	}
	d->bdirty = 1;
}
void bbFlipMesh(ent e) {
	mdat *m = bb_md(e);
	for (int i = 0; i < m->ns; i++) {
		surf *s = m->s[i];
		for (int t = 0; t < s->nt; t++) { unsigned short x = s->idx[t*3+1]; s->idx[t*3+1] = s->idx[t*3+2]; s->idx[t*3+2] = x; }
		for (int v = 0; v < s->nv*3; v++) s->nrm[v] = -s->nrm[v];
	}
	touch(m);
}
static void xform(ent e, tf t) {
	mdat *m = bb_md(e);
	m3 nm = m3t(m3inv(t.m));
	for (int i = 0; i < m->ns; i++) for (int v = 0; v < m->s[i]->nv; v++) {
		float *p = m->s[i]->pos + v*3, *n = m->s[i]->nrm + v*3;
		v3 a = tfp(t, v3n(p[0], p[1], p[2])), b = v3norm(m3v(nm, v3n(n[0], n[1], n[2])));
		p[0] = a.x; p[1] = a.y; p[2] = a.z; n[0] = b.x; n[1] = b.y; n[2] = b.z;
	}
	touch(m);
}
void bbScaleMesh(ent e, float x, float y, float z) { xform(e, (tf){m3scale(v3n(x, y, z)), {0,0,0}}); }
void bbPositionMesh(ent e, float x, float y, float z) { xform(e, (tf){m3id(), {x, y, z}}); }
void bbRotateMesh(ent e, float p, float y, float r) { xform(e, (tf){m3rot(p*DTOR, y*DTOR, r*DTOR), {0,0,0}}); }
void bbFitMesh(ent e, float x, float y, float z, float w, float h, float d, int uni) {
	mdat *m = bb_md(e); md_bounds(m);
	v3 sz = v3sub(m->hi, m->lo);
	float sx = sz.x ? w/sz.x : 1, sy = sz.y ? h/sz.y : 1, sz2 = sz.z ? d/sz.z : 1;
	if (uni) sx = sy = sz2 = fminf(sx, fminf(sy, sz2));
	tf t = {m3scale(v3n(sx, sy, sz2)), {0,0,0}};
	t.v = v3sub(v3n(x, y, z), m3v(t.m, m->lo));
	xform(e, t);
}
void bbUpdateNormals(ent e) {
	mdat *m = bb_md(e);
	for (int i = 0; i < m->ns; i++) {
		surf *s = m->s[i];
		memset(s->nrm, 0, s->nv * 12);
		for (int t = 0; t < s->nt; t++) {
			unsigned short *ix = s->idx + t*3;
			float *a = s->pos + ix[0]*3, *b = s->pos + ix[1]*3, *c = s->pos + ix[2]*3;
			v3 n = v3cross(v3n(b[0]-a[0], b[1]-a[1], b[2]-a[2]), v3n(c[0]-a[0], c[1]-a[1], c[2]-a[2]));
			for (int k = 0; k < 3; k++) { float *o = s->nrm + ix[k]*3; o[0] += n.x; o[1] += n.y; o[2] += n.z; }
		}
		for (int v = 0; v < s->nv; v++) {
			float *o = s->nrm + v*3; v3 n = v3norm(v3n(o[0], o[1], o[2]));
			o[0] = n.x; o[1] = n.y; o[2] = n.z;
		}
	}
	touch(m);
}
static float ext(ent e, int ax) {
	mdat *m = bb_md(e); md_bounds(m);
	return ax == 0 ? m->hi.x - m->lo.x : ax == 1 ? m->hi.y - m->lo.y : m->hi.z - m->lo.z;
}
float bbMeshWidth(ent e) { return ext(e, 0); }
float bbMeshHeight(ent e) { return ext(e, 1); }
float bbMeshDepth(ent e) { return ext(e, 2); }

// ---- surface api
void bbClearSurface(surf *s, int v, int t) { if (v) s->nv = 0; if (t) s->nt = 0; s->dirty = 1, s->ver++; }
int bbAddVertex(surf *s, float x, float y, float z, float u, float v, float w) {
	(void)w;
	if (s->nv >= s->cv) {
		s->cv = s->cv ? s->cv * 2 : 64;
		s->pos = realloc(s->pos, s->cv * 12); s->nrm = realloc(s->nrm, s->cv * 12);
		s->uv0 = realloc(s->uv0, s->cv * 8); s->uv1 = realloc(s->uv1, s->cv * 8);
		s->col = realloc(s->col, s->cv * 4);
	}
	int i = s->nv++;
	float *p = s->pos + i*3, *n = s->nrm + i*3;
	p[0] = x; p[1] = y; p[2] = z; n[0] = n[1] = n[2] = 0; // blitz default: no normal
	s->uv0[i*2] = s->uv1[i*2] = u; s->uv0[i*2+1] = s->uv1[i*2+1] = v;
	memset(s->col + i*4, 255, 4);
	s->dirty = 1, s->ver++;
	return i;
}
int bbAddTriangle(surf *s, int a, int b, int c) {
	if (s->nt >= s->ct) { s->ct = s->ct ? s->ct * 2 : 64; s->idx = realloc(s->idx, s->ct * 6); }
	unsigned short *ix = s->idx + s->nt*3;
	ix[0] = a; ix[1] = b; ix[2] = c;
	s->dirty = 1, s->ver++;
	return s->nt++;
}
void bbVertexCoords(surf *s, int i, float x, float y, float z) { float *p = s->pos + i*3; p[0] = x; p[1] = y; p[2] = z; s->dirty = 1, s->ver++; }
void bbVertexNormal(surf *s, int i, float x, float y, float z) { float *p = s->nrm + i*3; p[0] = x; p[1] = y; p[2] = z; s->dirty = 1, s->ver++; }
void bbVertexColor(surf *s, int i, float r, float g, float b, float a) {
	unsigned char *c = s->col + i*4;
	c[0] = r; c[1] = g; c[2] = b; c[3] = a * 255; s->dirty = 1, s->ver++;
}
void bbVertexTexCoords(surf *s, int i, float u, float v, float w, int set) {
	(void)w;
	float *t = (set ? s->uv1 : s->uv0) + i*2;
	t[0] = u; t[1] = v; s->dirty = 1, s->ver++;
}
int bbCountVertices(surf *s) { return s->nv; }
int bbCountTriangles(surf *s) { return s->nt; }
float bbVertexX(surf *s, int i) { return s->pos[i*3]; }
float bbVertexY(surf *s, int i) { return s->pos[i*3+1]; }
float bbVertexZ(surf *s, int i) { return s->pos[i*3+2]; }
int bbTriangleVertex(surf *s, int t, int c) { return s->idx[t*3+c]; }
