// rend.c: cameras, lights, sprites, RenderWorld
#include <stdlib.h>
#include <string.h>
#include "in.h"
#include "rlgl.h"
#define RAYMATH_STATIC_INLINE
#include "raymath.h"
#include "shd.h"

float rt_amb[3] = {0.5f, 0.5f, 0.5f};
static Shader sh;
static Material mat;
static int U_bcol, U_fx, U_fxf, U_amb, U_nl, U_lpos, U_ldir, U_lcol, U_tm[4], U_tc, U_nt, U_tb, U_tmask, U_fogc, U_fogr;
static int ntris;
static mdat *quad; // sprite geometry
static v3 proj;

#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_ALPHA 0x0302
#define GL_DST_COLOR 0x0306
#define GL_FUNC_ADD 0x8006

void rend_init(void) {
	sh = LoadShaderFromMemory(shd_vs, shd_fs);
	if (!sh.id || sh.id == rlGetShaderIdDefault()) die("shader failed");
	sh.locs[SHADER_LOC_MAP_ROUGHNESS] = GetShaderLocation(sh, "texture3");
	int *u[] = {&U_bcol, &U_fx, &U_amb, &U_nl, &U_lpos, &U_ldir, &U_lcol, &U_tc, &U_nt, &U_tb, &U_tmask, &U_fogc, &U_fogr};
	const char *n[] = {"bcol", "fx", "amb", "nl", "lpos", "ldir", "lcol", "tc", "nt", "tb", "tmask", "fogc", "fogr"};
	for (int i = 0; i < 13; i++) *u[i] = GetShaderLocation(sh, n[i]);
	U_fxf = GetShaderLocation(sh, "nofog");
	for (int i = 0; i < 4; i++) U_tm[i] = GetShaderLocation(sh, TextFormat("tm%d", i));
	mat = LoadMaterialDefault();
	mat.shader = sh;
	quad = md_new();
	surf *s = md_addsurf(quad, rt_brdef());
	add_vertex(s, -1, 1, 0, 0, 0, 0); add_vertex(s, 1, 1, 0, 1, 0, 0);
	add_vertex(s, 1, -1, 0, 1, 1, 0); add_vertex(s, -1, -1, 0, 0, 1, 0);
	add_triangle(s, 0, 1, 2); add_triangle(s, 0, 2, 3);
}
void rend_quit(void) { UnloadShader(sh); }

// ---- cameras
ent create_camera(ent par) {
	ent e = rt_new(EK_CAM, par);
	cam *c = calloc(1, sizeof *c);
	*c = (cam){.n = 1, .f = 1000, .zoom = 1, .clsc = 1, .clsz = 1, .proj = 1, .cls = {0,0,0,255}, .fog = {0,0,0,255}, .fogf = 1000};
	rt_e(e)->cam = c;
	return e;
}
static cam *rt_c(ent e) { E *x = rt_e(e); if (!x->cam) die("entity %d is not a camera", e); return x->cam; }
void camera_range(ent e, float n, float f) { cam *c = rt_c(e); c->n = n; c->f = f; }
void camera_zoom(ent e, float z) { rt_c(e)->zoom = z; }
void camera_viewport(ent e, int x, int y, int w, int h) { cam *c = rt_c(e); c->vx = x; c->vy = y; c->vw = w; c->vh = h; }
void camera_cls_color(ent e, int r, int g, int b) { rt_c(e)->cls = (Color){r, g, b, 255}; }
void camera_cls_mode(ent e, int col, int z) { cam *c = rt_c(e); c->clsc = col; c->clsz = z; }
void camera_proj_mode(ent e, int m) { rt_c(e)->proj = m; }
void camera_fog_mode(ent e, int m) { rt_c(e)->fogm = m; }
void camera_fog_range(ent e, float n, float f) { cam *c = rt_c(e); c->fogn = n; c->fogf = f; }
void camera_fog_color(ent e, int r, int g, int b) { rt_c(e)->fog = (Color){r, g, b, 255}; }

static void vp(cam *c, int *x, int *y, int *w, int *h) {
	*x = c->vx; *y = c->vy; *w = c->vw; *h = c->vh;
	if (!*w || !*h) { *x = *y = 0; *w = GetScreenWidth(); *h = GetScreenHeight(); }
}
static tf viewtf(ent e) { tf w = rt_wt(e); return tfinv((tf){m3ortho(w.m), w.v}); }

void camera_project(ent e, float x, float y, float z) {
	cam *c = rt_c(e);
	int vx, vy, vw, vh; vp(c, &vx, &vy, &vw, &vh);
	v3 p = tfp(viewtf(e), v3n(x, y, z));
	if (p.z <= c->n) { proj = v3n(0, 0, 0); return; }
	float a = (float)vw / vh;
	proj = v3n(vx + vw * (0.5f + 0.5f * p.x * c->zoom / p.z), vy + vh * (0.5f - 0.5f * p.y * c->zoom * a / p.z), p.z);
}
ent camera_pick(ent e, float sx, float sy) {
	cam *c = rt_c(e);
	int vx, vy, vw, vh; vp(c, &vx, &vy, &vw, &vh);
	float a = (float)vw / vh, nx = (sx - vx) / vw * 2 - 1, ny = 1 - (sy - vy) / vh * 2;
	tf w = rt_wt(e);
	v3 d = m3v(m3ortho(w.m), v3mul(v3n(nx / c->zoom, ny / (c->zoom * a), 1), c->f));
	return rt_pick(w.v, d, 0, 0, 0, 0);
}
float projected_x(void) { return proj.x; }
float projected_y(void) { return proj.y; }
float projected_z(void) { return proj.z; }

// view-space sphere vs frustum
static int vis(cam *c, float a, tf v, v3 ctr, float r) {
	v3 p = tfp(v, ctr);
	if (p.z < c->n - r || p.z > c->f + r) return 0;
	if (c->proj == 2) return fabsf(p.x) - r <= c->n / c->zoom && fabsf(p.y) - r <= c->n / (c->zoom * a);
	float zx = c->zoom, zy = c->zoom * a;
	if ((-fabsf(p.x) * zx + p.z) / sqrtf(zx*zx + 1) < -r) return 0;
	if ((-fabsf(p.y) * zy + p.z) / sqrtf(zy*zy + 1) < -r) return 0;
	return 1;
}
static int bsphere(ent e, v3 *ctr, float *r) {
	E *x = rt_e(e);
	if (x->kind == EK_SPRITE) { tf w = rt_wt(e); *ctr = w.v; *r = fmaxf(x->sx, x->sy) * 1.5f * fmaxf(v3len(w.m.i), v3len(w.m.j)); return 1; }
	if (!x->md) return 0;
	md_bounds(x->md);
	tf w = rt_wt(e);
	*ctr = tfp(w, v3mul(v3add(x->md->lo, x->md->hi), 0.5f));
	*r = v3len(v3sub(x->md->hi, x->md->lo)) * 0.5f * fmaxf(v3len(w.m.i), fmaxf(v3len(w.m.j), v3len(w.m.k)));
	return 1;
}
int entity_in_view(ent e, ent c) {
	v3 ct; float r;
	if (!bsphere(e, &ct, &r)) return 0;
	int x, y, w, h; vp(rt_c(c), &x, &y, &w, &h);
	return vis(rt_c(c), (float)w / h, viewtf(c), ct, r);
}

// ---- lights
ent create_light(int t, ent par) {
	ent e = rt_new(EK_LIGHT, par);
	E *x = rt_e(e);
	x->ltype = t ? t : 1; x->lr = x->lg = x->lb = 1; x->lrange = 1000; x->lin = 0; x->lout = 90;
	return e;
}
void light_color(ent e, float r, float g, float b) { E *x = rt_e(e); x->lr = r/255; x->lg = g/255; x->lb = b/255; }
void light_range(ent e, float r) { rt_e(e)->lrange = r; }
void light_cone_angles(ent e, float i, float o) { E *x = rt_e(e); x->lin = i; x->lout = o; }
void ambient_light(float r, float g, float b) { rt_amb[0] = r/255; rt_amb[1] = g/255; rt_amb[2] = b/255; }

// ---- sprites
ent create_sprite(ent par) {
	ent e = rt_new(EK_SPRITE, par);
	E *x = rt_e(e);
	x->sx = x->sy = 1; x->view = 1;
	x->br.fx = FX_FULLBRIGHT; x->br.blend = BL_ALPHA;
	return e;
}
ent load_sprite(const char *p, int flags, ent par) {
	ent e = create_sprite(par);
	htex t = load_texture(p, flags ? flags : TF_COLOR);
	if (t) { entity_texture(e, t, 0, 0); free_texture(t); }
	return e;
}
void scale_sprite(ent e, float x, float y) { E *s = rt_e(e); s->sx = x; s->sy = y; }
void handle_sprite(ent e, float x, float y) { E *s = rt_e(e); s->hx = x; s->hy = y; }
void rotate_sprite(ent e, float a) { rt_e(e)->sang = a; }
void sprite_view_mode(ent e, int m) { rt_e(e)->view = m; }

static tf spritetf(E *x, ent e, m3 crot) {
	tf w = rt_wt(e);
	m3 r = crot;
	if (x->view == 2) r = m3ortho(w.m);
	else if (x->view >= 3) { // upright: yaw toward cam only
		v3 k = crot.k; k.y = 0; k = v3norm(k);
		r = (m3){v3cross(v3n(0, 1, 0), k), {0, 1, 0}, k};
	}
	r = m3mul(r, m3roll(x->sang * DTOR));
	v3 sc = v3n(x->sx * v3len(w.m.i), x->sy * v3len(w.m.j), 1);
	tf t = {m3mul(r, m3scale(sc)), w.v};
	t.v = v3sub(t.v, m3v(t.m, v3n(x->hx, x->hy, 0)));
	return t;
}

// ---- render
typedef struct { ent e; int si; float d; int ord, alpha; } item;
static item *its; static int nits, cits;

static Matrix mk(const float *c) {
	return (Matrix){c[0], c[4], c[8], c[12], c[1], c[5], c[9], c[13], c[2], c[6], c[10], c[14], c[3], c[7], c[11], c[15]};
}
static Matrix mtf(tf t) { float c[16]; tf16(t, c); return mk(c); }

static brush combine(brush s, brush e) {
	brush b = s;
	b.r *= e.r; b.g *= e.g; b.b *= e.b; b.a *= e.a;
	b.fx |= e.fx;
	if (e.blend) b.blend = e.blend;
	for (int i = 0; i < BB_NTEX; i++) if (e.tex[i]) b.tex[i] = e.tex[i];
	return b;
}
static int isalpha_br(brush b) {
	if (b.a < 1 || b.blend == BL_MUL || b.blend == BL_ADD || b.fx & FX_VALPHA) return 1;
	for (int i = 0; i < BB_NTEX; i++) if (b.tex[i] && rt_t(b.tex[i])->flags & TF_ALPHA) return 1;
	return 0;
}

static void setblend(int bl) {
	if (bl == BL_ADD) { rlSetBlendFactors(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD); rlSetBlendMode(RL_BLEND_CUSTOM); }
	else if (bl == BL_MUL) { rlSetBlendFactors(GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD); rlSetBlendMode(RL_BLEND_CUSTOM); }
	else rlSetBlendMode(RL_BLEND_ALPHA);
}

static void drawsurf(surf *s, brush b, tf t, cam *c) {
	if (b.a <= 0) return;
	surf_upload(s);
	if (!s->gpu.vertexCount) return;
	float bc[4] = {b.r, b.g, b.b, b.a};
	float fx[4] = {b.fx & FX_FULLBRIGHT ? 1 : 0, b.fx & FX_VCOLOR ? 1 : 0, b.fx & FX_VALPHA ? 1 : 0, b.fx & FX_NOFOG ? 1 : 0};
	float tc[4] = {0}, tb[4] = {0}, tm[4] = {0};
	int nt = 0;
	for (int i = 0; i < BB_NTEX; i++) mat.maps[i].texture = (Texture2D){0};
	for (int i = 0; i < BB_NTEX; i++) {
		tex *x = rt_t(b.tex[i]);
		if (!x) continue;
		mat.maps[nt].texture = x->t;
		tc[nt] = x->coords; tb[nt] = x->blend; tm[nt] = x->flags & TF_MASK ? 1 : 0;
		float m[4] = {x->su ? x->su : 1, x->sv ? x->sv : 1, x->u0, x->v0};
		SetShaderValue(sh, U_tm[nt], m, SHADER_UNIFORM_VEC4);
		nt++;
	}
	SetShaderValue(sh, U_bcol, bc, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_fx, fx, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_fxf, &fx[3], SHADER_UNIFORM_FLOAT);
	SetShaderValue(sh, U_tc, tc, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_tb, tb, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_tmask, tm, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_nt, &nt, SHADER_UNIFORM_INT);
	if (b.fx & FX_2SIDED) rlDisableBackfaceCulling(); else rlEnableBackfaceCulling();
	(void)c;
	DrawMesh(s->gpu, mat, mtf(t));
	ntris += s->nt;
}

// one queue item per surface (si = -1: sprite), so mixed meshes sort like blitz
static void drawit(ent e, int si, cam *c, m3 crot) {
	E *x = rt_e(e);
	if (si < 0) {
		brush b = x->br;
		b.fx |= FX_2SIDED;
		setblend(b.blend);
		drawsurf(quad->s[0], b, spritetf(x, e, crot), c);
		return;
	}
	brush b = combine(x->md->s[si]->br, x->br);
	setblend(b.blend);
	drawsurf(x->md->s[si], b, rt_wt(e), c);
}

static int itcmp(const void *a, const void *b) {
	const item *p = a, *q = b;
	if (p->ord != q->ord) return q->ord - p->ord; // bigger order first
	if (p->alpha != q->alpha) return p->alpha - q->alpha;
	if (p->alpha) return p->d < q->d ? 1 : p->d > q->d ? -1 : 0; // alpha: far to near
	return p->d < q->d ? -1 : p->d > q->d ? 1 : 0; // opaque: near to far
}

static void lights(void) {
	float lp[32] = {0}, ld[32] = {0}, lc[32] = {0};
	int n = 0;
	for (int e = 1; e < rt_nents && n < 8; e++) {
		E *x = &rt_ents[e];
		if (!x->used || x->kind != EK_LIGHT || entity_hidden(e)) continue;
		tf w = rt_wt(e); v3 k = v3norm(w.m.k);
		lp[n*4] = w.v.x; lp[n*4+1] = w.v.y; lp[n*4+2] = w.v.z; lp[n*4+3] = x->ltype;
		ld[n*4] = k.x; ld[n*4+1] = k.y; ld[n*4+2] = k.z; ld[n*4+3] = x->lrange;
		lc[n*4] = x->lr; lc[n*4+1] = x->lg; lc[n*4+2] = x->lb; lc[n*4+3] = cosf(x->lout * 0.5f * DTOR);
		n++;
	}
	SetShaderValue(sh, U_nl, &n, SHADER_UNIFORM_INT);
	SetShaderValueV(sh, U_lpos, lp, SHADER_UNIFORM_VEC4, 8);
	SetShaderValueV(sh, U_ldir, ld, SHADER_UNIFORM_VEC4, 8);
	SetShaderValueV(sh, U_lcol, lc, SHADER_UNIFORM_VEC4, 8);
	SetShaderValue(sh, U_amb, rt_amb, SHADER_UNIFORM_VEC3);
}


static void rendcam(ent ce) {
	cam *c = rt_c(ce);
	if (!c->proj) return;
	int x, y, w, h; vp(c, &x, &y, &w, &h);
	float a = (float)w / h, n = c->n, f = c->f;
	float k = (float)GetRenderWidth() / GetScreenWidth(); // hidpi framebuffer scale
	int fx = x*k, fy = (GetScreenHeight() - y - h)*k, fw = w*k, fh = h*k;
	rlViewport(fx, fy, fw, fh);
	rlEnableScissorTest(); rlScissor(fx, fy, fw, fh);
	if (c->clsc || c->clsz) {
		rlClearColor(c->cls.r, c->cls.g, c->cls.b, 255);
		rlClearScreenBuffers(); // TODO: honor color-only/z-only cls
	}
	float P[16] = {c->zoom,0,0,0, 0,c->zoom*a,0,0, 0,0,(f+n)/(f-n),1, 0,0,-2*f*n/(f-n),0};
	if (c->proj == 2) { // blitz ortho (gxscene setOrthoProj): x*zoom/near across, depth linear near..far
		float O[16] = {c->zoom/n,0,0,0, 0,c->zoom*a/n,0,0, 0,0,2/(f-n),0, 0,0,-(f+n)/(f-n),1};
		memcpy(P, O, sizeof P);
	}
	tf v = viewtf(ce);
	rlSetMatrixProjection(mk(P));
	rlSetMatrixModelview(mtf(v));
	float fc[4] = {c->fog.r/255.f, c->fog.g/255.f, c->fog.b/255.f, 1}, fr[3] = {c->fogn, c->fogf, c->fogm ? 1 : 0};
	SetShaderValue(sh, U_fogc, fc, SHADER_UNIFORM_VEC4);
	SetShaderValue(sh, U_fogr, fr, SHADER_UNIFORM_VEC3);
	lights();
	m3 crot = m3ortho(rt_wt(ce).m);
	nits = 0;
	for (int e = 1; e < rt_nents; e++) {
		E *p = &rt_ents[e];
		if (!p->used || (p->kind != EK_MESH && p->kind != EK_SPRITE)) continue;
		if (entity_hidden(e)) continue;
		v3 ct; float r;
		if (!bsphere(e, &ct, &r) || !vis(c, a, v, ct, r)) continue;
		p = &rt_ents[e];
		float d = tfp(v, ct).z;
		if (p->kind == EK_SPRITE) { grow(its, nits, cits); its[nits++] = (item){e, -1, d, p->order, 1}; continue; }
		for (int i = 0; i < p->md->ns; i++) {
			grow(its, nits, cits);
			its[nits++] = (item){e, i, d, p->order, isalpha_br(combine(p->md->s[i]->br, p->br))};
		}
	}
	qsort(its, nits, sizeof *its, itcmp);
	rlEnableDepthTest();
	rlSetCullFace(RL_CULL_FACE_FRONT); // blitz front faces are cw
	for (int i = 0; i < nits; i++) {
		if (its[i].ord) rlDisableDepthTest(); else rlEnableDepthTest();
		if (its[i].alpha) rlDisableDepthMask(); else rlEnableDepthMask();
		drawit(its[i].e, its[i].si, c, crot);
	}
	rlEnableDepthMask(); rlDisableDepthTest();
	rlSetCullFace(RL_CULL_FACE_BACK); rlEnableBackfaceCulling();
	rlSetBlendMode(RL_BLEND_ALPHA);
	rlDisableScissorTest();
}

void render_world(float tween) {
	(void)tween;
	rt_frame();
	g2d_end(); // TODO: blitz renders into the current buffer; scpcb only uses the backbuffer
	rlDrawRenderBatchActive();
	ntris = 0;
	for (int e = 1; e < rt_nents; e++)
		if (rt_ents[e].used && rt_ents[e].kind == EK_CAM && !entity_hidden(e)) rendcam(e);
	int W = GetScreenWidth(), H = GetScreenHeight();
	rlViewport(0, 0, GetRenderWidth(), GetRenderHeight());
	rlSetMatrixProjection(MatrixOrtho(0, W, H, 0, 0, 1));
	rlSetMatrixModelview(MatrixIdentity());
}
int tris_rendered(void) { return ntris; }
