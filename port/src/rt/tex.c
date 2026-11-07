// tex.c: textures + their flags
#include <stdlib.h>
#include <string.h>
#include "in.h"
#include "rlgl.h"

static tex *ts;
static int nts, cts;

tex *rt_t(htex t) {
	if (!t) return NULL;
	if (t < 0 || t >= nts || !ts[t].used) die("bad texture handle %d", t);
	return &ts[t];
}
static int pot(int n) { int p = 1; while (p < n) p <<= 1; return p; }

static htex upload(Image im, int flags, const char *name) {
	if (!nts) nts = 1;
	htex h = 0;
	for (int i = 1; i < nts; i++) if (!ts[i].used) { h = i; break; }
	if (!h) { grow(ts, nts, cts); h = nts++; }
	tex *t = &ts[h];
	*t = (tex){.used = 1, .rc = 1, .w = im.width, .h = im.height, .flags = flags,
	           .blend = TB_MUL, .su = 1, .sv = 1};
	snprintf(t->name, sizeof t->name, "%s", name);
	// es2: npot can't mip or repeat, so resize to pow2
	int pw = pot(im.width), ph = pot(im.height);
	if (pw != im.width || ph != im.height) ImageResize(&im, pw, ph);
	if (flags & TF_MIP) ImageMipmaps(&im);
	t->t = LoadTextureFromImage(im);
	SetTextureFilter(t->t, flags & TF_MIP ? TEXTURE_FILTER_TRILINEAR : TEXTURE_FILTER_BILINEAR);
	rlTextureParameters(t->t.id, RL_TEXTURE_WRAP_S, flags & TF_CLAMPU ? RL_TEXTURE_WRAP_CLAMP : RL_TEXTURE_WRAP_REPEAT);
	rlTextureParameters(t->t.id, RL_TEXTURE_WRAP_T, flags & TF_CLAMPV ? RL_TEXTURE_WRAP_CLAMP : RL_TEXTURE_WRAP_REPEAT);
	UnloadImage(im);
	return h;
}

static Image texload(const char *p, int flags) {
	const char *r = res_path(p);
	if (!r || DirectoryExists(r)) return (Image){0};
	Image im = rt_loadimg(r);
	if (!im.data) return im;
	int hasa = im.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 || im.format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
	ImageFormat(&im, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	unsigned char *px = im.data;
	for (int i = 0; i < im.width * im.height; i++, px += 4) {
		if (flags & TF_ALPHA) { if (!hasa) px[3] = (px[0] + px[1] + px[2]) / 3; }
		else px[3] = 255;
		if (flags & TF_MASK && !px[0] && !px[1] && !px[2]) px[3] = 0;
	}
	return im;
}
htex load_texture(const char *p, int flags) {
	Image im = texload(p, flags);
	return im.data ? upload(im, (flags ? flags : TF_COLOR) | TF_MIP, p) : 0; // always mip
}
// frames are fw x fh cells, left to right then down. each frame is its own gl texture
htex load_anim_texture(const char *p, int flags, int fw, int fh, int first, int count) {
	Image im = texload(p, flags);
	if (!im.data || fw <= 0 || fh <= 0 || count < 1) { UnloadImage(im); return 0; }
	int per = im.width / fw, f = flags ? flags : TF_COLOR;
	if (per < 1) { UnloadImage(im); return 0; }
	htex h[count];
	for (int i = 0; i < count; i++) {
		int k = first + i;
		h[i] = upload(ImageFromImage(im, (Rectangle){k % per * fw, k / per * fh, fw, fh}), f | TF_MIP, p);
	}
	UnloadImage(im);
	tex *t = rt_t(h[0]);
	t->nf = count - 1;
	t->fr = malloc(sizeof(htex) * count);
	memcpy(t->fr, h + 1, sizeof(htex) * (count - 1));
	return h[0];
}
htex rt_texframe(htex h, int f) {
	if (!h || f <= 0) return h;
	tex *t = rt_t(h);
	return f <= t->nf ? t->fr[f - 1] : h;
}

htex create_texture(int w, int h, int flags, int frames) {
	(void)frames;
	return upload(GenImageColor(w, h, BLACK), flags, "");
}

void texture_fill(htex h, int r, int g, int b, int a) {
	tex *t = rt_t(h);
	int n = t->t.width * t->t.height;
	unsigned char *px = malloc(n * 4);
	for (int i = 0; i < n; i++) px[i*4] = r, px[i*4+1] = g, px[i*4+2] = b, px[i*4+3] = a;
	UpdateTexture(t->t, px);
	free(px);
	rt_g2dtouch();
}

void tex_ref(htex h) { if (h) rt_t(h)->rc++; }
void tex_unref(htex h) {
	if (!h) return;
	tex *t = rt_t(h);
	if (--t->rc > 0) return;
	if (t->hasrt) UnloadRenderTexture(t->rt); else UnloadTexture(t->t);
	t->used = 0;
	htex *fr = t->fr; int nf = t->nf; t->fr = NULL; t->nf = 0;
	for (int i = 0; i < nf; i++) tex_unref(fr[i]);
	free(fr);
}
// clear_world: whatever's left goes, refs or not
void rt_texclear(void) {
	for (int i = 1; i < nts; i++) {
		tex *t = &ts[i];
		if (!t->used) continue;
		if (t->hasrt) UnloadRenderTexture(t->rt); else UnloadTexture(t->t);
		free(t->fr); t->fr = NULL; t->nf = 0; t->used = 0;
	}
}
// handle drop; brushes may still hold it.
void free_texture(htex h) { tex_unref(h); }

void texture_blend(htex h, int b) { if (h) rt_t(h)->blend = b; }
void texture_coords(htex h, int s) { if (h) rt_t(h)->coords = s; }
void scale_texture(htex h, float u, float v) { tex *t = rt_t(h); t->su = u; t->sv = v; }
void position_texture(htex h, float u, float v) { tex *t = rt_t(h); t->u0 = u; t->v0 = v; }
int texture_width(htex h) { return rt_t(h)->w; }
int texture_height(htex h) { return rt_t(h)->h; }
const char *texture_name(htex h) { return h ? rt_t(h)->name : ""; }
void texture_lod_bias(float b) { (void)b; }

// images come from assetconv's qoi copies (it decodes png/jpg/bmp/dds, incl. the jpgs the game hides as .dc/.pt)
Image rt_loadimg(const char *path) {
	const char *q = rt_media(path, "qoi");
	return q ? LoadImage(q) : (Image){0};
}
