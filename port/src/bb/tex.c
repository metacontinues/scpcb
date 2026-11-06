// tex.c: textures, blitz flag semantics
#include <stdlib.h>
#include <string.h>
#include "in.h"
#include "rlgl.h"

static tex *ts;
static int nts, cts;

tex *bb_t(htex t) {
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
	// es2: npot can't mip or repeat, blitz resized too
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

htex bbLoadTexture(const char *p, int flags) {
	const char *r = bbPath(p);
	if (!r || DirectoryExists(r)) return 0;
	Image im = LoadImage(r);
	if (!im.data) return 0;
	int hasa = im.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 || im.format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
	ImageFormat(&im, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	unsigned char *px = im.data;
	for (int i = 0; i < im.width * im.height; i++, px += 4) {
		if (flags & TF_ALPHA) { if (!hasa) px[3] = (px[0] + px[1] + px[2]) / 3; }
		else px[3] = 255;
		if (flags & TF_MASK && !px[0] && !px[1] && !px[2]) px[3] = 0;
	}
	return upload(im, (flags ? flags : TF_COLOR) | TF_MIP, p); // always mip, looks like blitz
}

htex bbCreateTexture(int w, int h, int flags, int frames) {
	(void)frames;
	return upload(GenImageColor(w, h, BLACK), flags, "");
}

void bbTextureFill(htex h, int r, int g, int b, int a) {
	tex *t = bb_t(h);
	int n = t->t.width * t->t.height;
	unsigned char *px = malloc(n * 4);
	for (int i = 0; i < n; i++) px[i*4] = r, px[i*4+1] = g, px[i*4+2] = b, px[i*4+3] = a;
	UpdateTexture(t->t, px);
	free(px);
}

void tex_ref(htex h) { if (h) bb_t(h)->rc++; }
void tex_unref(htex h) {
	if (!h) return;
	tex *t = bb_t(h);
	if (--t->rc > 0) return;
	UnloadTexture(t->t);
	t->used = 0;
}
// handle drop; brushes may still hold it.
void bbFreeTexture(htex h) { tex_unref(h); }

void bbTextureBlend(htex h, int b) { if (h) bb_t(h)->blend = b; }
void bbTextureCoords(htex h, int s) { if (h) bb_t(h)->coords = s; }
void bbScaleTexture(htex h, float u, float v) { tex *t = bb_t(h); t->su = u; t->sv = v; }
void bbPositionTexture(htex h, float u, float v) { tex *t = bb_t(h); t->u0 = u; t->v0 = v; }
int bbTextureWidth(htex h) { return bb_t(h)->w; }
int bbTextureHeight(htex h) { return bb_t(h)->h; }
const char *bbTextureName(htex h) { return h ? bb_t(h)->name : ""; }
