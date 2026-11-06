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

static Image texload(const char *p, int flags) {
	const char *r = bbPath(p);
	if (!r || DirectoryExists(r)) return (Image){0};
	Image im = bb_loadimg(r);
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
htex bbLoadTexture(const char *p, int flags) {
	Image im = texload(p, flags);
	return im.data ? upload(im, (flags ? flags : TF_COLOR) | TF_MIP, p) : 0; // always mip, looks like blitz
}
// frames are fw x fh cells, left to right then down, like blitz. each frame is its own gl texture
htex bbLoadAnimTexture(const char *p, int flags, int fw, int fh, int first, int count) {
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
	tex *t = bb_t(h[0]);
	t->nf = count - 1;
	t->fr = malloc(sizeof(htex) * count);
	memcpy(t->fr, h + 1, sizeof(htex) * (count - 1));
	return h[0];
}
htex bb_texframe(htex h, int f) {
	if (!h || f <= 0) return h;
	tex *t = bb_t(h);
	return f <= t->nf ? t->fr[f - 1] : h;
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
	bb_g2dtouch();
}

void tex_ref(htex h) { if (h) bb_t(h)->rc++; }
void tex_unref(htex h) {
	if (!h) return;
	tex *t = bb_t(h);
	if (--t->rc > 0) return;
	if (t->hasrt) UnloadRenderTexture(t->rt); else UnloadTexture(t->t);
	t->used = 0;
	htex *fr = t->fr; int nf = t->nf; t->fr = NULL; t->nf = 0;
	for (int i = 0; i < nf; i++) tex_unref(fr[i]);
	free(fr);
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
void bbTextureLodBias(float b) { (void)b; }

// ---- DXT1/3/5 -> rgba (level 0 only). es2 can't be counted on for s3tc, and the texture code wants pixels
static void c565(unsigned v, unsigned char *o) { o[0] = (v >> 11 & 31) * 255 / 31; o[1] = (v >> 5 & 63) * 255 / 63; o[2] = (v & 31) * 255 / 31; o[3] = 255; }
static Image undxt(Image im) {
	int f = im.format, w = im.width, h = im.height, bs = f == PIXELFORMAT_COMPRESSED_DXT1_RGB || f == PIXELFORMAT_COMPRESSED_DXT1_RGBA ? 8 : 16;
	const unsigned char *src = im.data;
	unsigned char *out = calloc(w * h, 4);
	for (int by = 0; by < (h + 3) / 4; by++) for (int bx = 0; bx < (w + 3) / 4; bx++, src += bs) {
		const unsigned char *a = bs == 16 ? src : NULL, *c = src + (bs == 16 ? 8 : 0);
		unsigned c0 = c[0] | c[1] << 8, c1 = c[2] | c[3] << 8, bits = c[4] | c[5] << 8 | c[6] << 16 | (unsigned)c[7] << 24;
		unsigned char pal[4][4];
		c565(c0, pal[0]); c565(c1, pal[1]);
		int four = c0 > c1 || bs == 16;
		for (int k = 0; k < 4; k++) {
			pal[2][k] = four ? (2 * pal[0][k] + pal[1][k]) / 3 : (pal[0][k] + pal[1][k]) / 2;
			pal[3][k] = four ? (pal[0][k] + 2 * pal[1][k]) / 3 : 0;
		}
		if (!four) pal[3][3] = f == PIXELFORMAT_COMPRESSED_DXT1_RGBA ? 0 : 255;
		unsigned char al[8] = {0};
		if (f == PIXELFORMAT_COMPRESSED_DXT5_RGBA) {
			al[0] = a[0]; al[1] = a[1];
			for (int k = 2; k < 8; k++) al[k] = a[0] > a[1] ? ((8 - k) * a[0] + (k - 1) * a[1]) / 7 : k < 6 ? ((6 - k) * a[0] + (k - 1) * a[1]) / 5 : k == 6 ? 0 : 255;
		}
		unsigned long long ab = 0;
		if (a) for (int k = 7; k >= 2; k--) ab = ab << 8 | a[k];
		for (int py = 0; py < 4; py++) for (int px = 0; px < 4; px++) {
			int x = bx * 4 + px, y = by * 4 + py, i = py * 4 + px;
			if (x >= w || y >= h) continue;
			unsigned char *o = out + (y * w + x) * 4;
			memcpy(o, pal[bits >> (2 * i) & 3], 4);
			if (f == PIXELFORMAT_COMPRESSED_DXT3_RGBA) o[3] = (a[i / 2] >> (4 * (i & 1)) & 15) * 17;
			else if (f == PIXELFORMAT_COMPRESSED_DXT5_RGBA) o[3] = al[ab >> (3 * i) & 7];
		}
	}
	UnloadImage(im);
	return (Image){out, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}

// raylib picks the decoder by extension, but the game hides jpgs as .dc/.pt, so sniff the magic
Image bb_loadimg(const char *path) {
	int n = 0;
	unsigned char *d = LoadFileData(path, &n);
	if (!d) return (Image){0};
	const char *ext = n > 3 && d[0] == 0x89 && d[1] == 'P' ? ".png" : n > 2 && d[0] == 0xff && d[1] == 0xd8 ? ".jpg"
		: n > 2 && d[0] == 'B' && d[1] == 'M' ? ".bmp" : n > 3 && !memcmp(d, "DDS ", 4) ? ".dds" : GetFileExtension(path);
	Image im = LoadImageFromMemory(ext, d, n);
	UnloadFileData(d);
	if (!im.data) fprintf(stderr, "bb: can't decode image %s\n", path);
	else if (im.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB && im.format <= PIXELFORMAT_COMPRESSED_DXT5_RGBA) im = undxt(im);
	return im;
}
