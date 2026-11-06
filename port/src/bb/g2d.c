// g2d.c: blitz 2d. buffers, images, Text/Rect/Line/Oval, fonts, CopyRect, pixels.
// image/texture buffers become render textures on first SetBuffer, drawn w/ a y-flipped
// ortho so row 0 stays the top row, which is what blitz uvs expect.
#include <stdlib.h>
#include <string.h>
#include "in.h"
#include "rlgl.h"
#define RAYMATH_STATIC_INLINE
#include "raymath.h"

typedef struct {
	int used, w, h, fw, fh, nfr;
	float hx, hy;
	Texture2D t; RenderTexture2D rt; int hasrt;
	Image cpu; int stale; // cpu = unmasked source; stale once drawn into on the gpu
	Color mask;
} img;
static img *is; static int nis, cis;
static Font *fs; static int nfs, cfs; static int *fsz;

static Color col = {255, 255, 255, 255}, clsc = {0, 0, 0, 255};
static int ox, oy, autoMid;
static int curfont;

// ---- buffers: 1 = back, 2<<24|i image, 3<<24|i texture
#define BACKBUF 1
int bbBackBuffer(void) { return BACKBUF; }
int bbFrontBuffer(void) { return BACKBUF; }
int bbImageBuffer(int i, int frame) { (void)frame; return 2 << 24 | i; }
int bbTextureBuffer(htex t, int frame) { (void)frame; return 3 << 24 | t; }
static int cur = BACKBUF, active = BACKBUF;
int bbGraphicsBuffer(void) { return cur; }

static img *gi(int h) { if (h <= 0 || h >= nis || !is[h].used) die("bad image handle %d", h); return &is[h]; }

// give an image/texture a render target, keeping what it showed
static RenderTexture2D *rtof(int b, int *w, int *h) {
	Texture2D *t; RenderTexture2D *rt; int *has;
	if (b >> 24 == 2) { img *m = gi(b & 0xffffff); t = &m->t; rt = &m->rt; has = &m->hasrt; m->stale = 1; }
	else { tex *x = bb_t(b & 0xffffff); t = &x->t; rt = &x->rt; has = &x->hasrt; }
	*w = t->width; *h = t->height;
	if (*has) return rt;
	*rt = LoadRenderTexture(*w, *h);
	BeginTextureMode(*rt);
	rlSetMatrixProjection(MatrixOrtho(0, *w, 0, *h, 0, 1));
	rlDisableBackfaceCulling();
	ClearBackground(BLANK);
	DrawTexture(*t, 0, 0, WHITE);
	EndTextureMode();
	rlEnableBackfaceCulling();
	SetTextureFilter(rt->texture, b >> 24 == 2 ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR);
	rlTextureParameters(rt->texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
	rlTextureParameters(rt->texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
	UnloadTexture(*t);
	*t = rt->texture; *has = 1;
	return rt;
}
static void sync(img *m);
static void upload(img *m);
void g2d_end(void) {
	if (active == BACKBUF) return;
	EndTextureMode();
	rlEnableBackfaceCulling();
	int was = active;
	active = BACKBUF;
	// blitz masks images at draw time: pull the drawing back + re-key the mask color
	if (was >> 24 == 2 && (was & 0xffffff) < nis && is[was & 0xffffff].used) { img *m = &is[was & 0xffffff]; sync(m); upload(m); }
}
static void target(void) {
	bb_frame();
	if (active == cur) return;
	g2d_end();
	if (cur == BACKBUF) return;
	int w, h;
	RenderTexture2D *rt = rtof(cur, &w, &h);
	BeginTextureMode(*rt);
	rlSetMatrixProjection(MatrixOrtho(0, w, 0, h, 0, 1)); // y up in memory = blitz rows top-down
	rlDisableBackfaceCulling();
	active = cur;
}
void bbSetBuffer(int b) { cur = b ? b : BACKBUF; }

// ---- state
void bbColor(int r, int g, int b) { col = (Color){r, g, b, 255}; }
int bbColorRed(void) { return col.r; }
int bbColorGreen(void) { return col.g; }
int bbColorBlue(void) { return col.b; }
void bbClsColor(int r, int g, int b) { clsc = (Color){r, g, b, 255}; }
void bbCls(void) { target(); ClearBackground(clsc); }
void bbOrigin(int x, int y) { ox = x; oy = y; }
void bbViewport(int x, int y, int w, int h) {
	target();
	if (w <= 0 || h <= 0) { EndScissorMode(); return; }
	BeginScissorMode(x, y, w, h);
}

// ---- primitives
void bbPlot(int x, int y) { target(); DrawPixel(x + ox, y + oy, col); }
void bbLine(int x1, int y1, int x2, int y2) {
	target();
	DrawLineV((Vector2){x1 + ox + 0.5f, y1 + oy + 0.5f}, (Vector2){x2 + ox + 0.5f, y2 + oy + 0.5f}, col);
}
void bbRect(int x, int y, int w, int h, int solid) {
	target();
	if (solid) DrawRectangle(x + ox, y + oy, w, h, col);
	else DrawRectangleLines(x + ox, y + oy, w, h, col);
}
void bbOval(int x, int y, int w, int h, int solid) {
	target();
	float rx = w * 0.5f, ry = h * 0.5f;
	if (solid) DrawEllipse(x + ox + rx, y + oy + ry, rx, ry, col);
	else DrawEllipseLines(x + ox + rx, y + oy + ry, rx, ry, col);
}

// ---- fonts. names that aren't files ("Blitz", "Tahoma") get courier, like the game mostly wants
int bbLoadFont(const char *p, int height, int bold, int italic, int underline) {
	(void)bold; (void)italic; (void)underline;
	const char *r = bbPath(p);
	if (!r || DirectoryExists(r)) r = bbPath("GFX\\font\\cour\\Courier New.ttf");
	if (!r) return 0;
	if (height < 1) height = 1;
	Font f = LoadFontEx(r, height, NULL, 0);
	if (!f.texture.id) return 0;
	SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
	if (!nfs) { nfs = 1; cfs = 0; }
	int h = 0;
	for (int i = 1; i < nfs; i++) if (!fs[i].texture.id) { h = i; break; }
	if (!h) { int c = cfs; grow(fs, nfs, cfs); if (c != cfs) fsz = realloc(fsz, sizeof *fsz * cfs); h = nfs++; }
	fs[h] = f; fsz[h] = height;
	return h;
}
void bbFreeFont(int h) {
	if (h <= 0 || h >= nfs || !fs[h].texture.id) return;
	UnloadFont(fs[h]); fs[h] = (Font){0};
	if (curfont == h) curfont = 0;
}
void bbSetFont(int h) { curfont = h > 0 && h < nfs && fs[h].texture.id ? h : 0; }
static Font font(void) { return curfont ? fs[curfont] : GetFontDefault(); }
static float fsize(void) { return curfont ? fsz[curfont] : 13; }
int bbFontHeight(void) { return fsize(); }
int bbStringWidth(const char *s) { return MeasureTextEx(font(), s, fsize(), 0).x; }
int bbStringHeight(const char *s) { (void)s; return fsize(); }
int bbFontWidth(void) { return bbStringWidth("W"); }
// Text + the alpha AAText adds. cx/cy center on x/y like blitz
void bbTextA(int x, int y, const char *s, int cx, int cy, float a) {
	target();
	Vector2 sz = MeasureTextEx(font(), s, fsize(), 0);
	float px = x + ox - (cx ? sz.x / 2 : 0), py = y + oy - (cy ? fsize() / 2 : 0);
	Color c = col; c.a = 255 * fmaxf(0, fminf(1, a));
	DrawTextEx(font(), s, (Vector2){(int)px, (int)py}, fsize(), 0, c);
}
void bbText(int x, int y, const char *s, int cx, int cy) { bbTextA(x, y, s, cx, cy, 1); }

// ---- images
static int newimg(Image cpu, int fw, int fh, int nfr) {
	if (!nis) nis = 1;
	int h = 0;
	for (int i = 1; i < nis; i++) if (!is[i].used) { h = i; break; }
	if (!h) { grow(is, nis, cis); h = nis++; }
	ImageFormat(&cpu, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	is[h] = (img){.used = 1, .w = fw, .h = fh, .fw = fw, .fh = fh, .nfr = nfr, .cpu = cpu, .mask = {0, 0, 0, 255}};
	if (autoMid) { is[h].hx = fw / 2; is[h].hy = fh / 2; }
	return h;
}
// masked texture from cpu: blitz ignores image alpha, keys out the mask color
static void upload(img *m) {
	Image c = ImageCopy(m->cpu);
	unsigned char *p = c.data;
	for (int i = 0; i < c.width * c.height; i++, p += 4)
		p[3] = p[0] == m->mask.r && p[1] == m->mask.g && p[2] == m->mask.b ? 0 : 255;
	if (m->hasrt) { UnloadRenderTexture(m->rt); m->hasrt = 0; }
	else if (m->t.id) UnloadTexture(m->t);
	m->t = LoadTextureFromImage(c);
	SetTextureFilter(m->t, TEXTURE_FILTER_POINT);
	UnloadImage(c);
	m->stale = 0;
}
static void sync(img *m) { // pull gpu drawing back into cpu
	if (!m->stale) return;
	Image g = LoadImageFromTexture(m->t);
	ImageFormat(&g, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadImage(m->cpu); m->cpu = g; m->stale = 0;
}
int bbLoadImage(const char *p) {
	const char *r = bbPath(p);
	if (!r || DirectoryExists(r)) return 0;
	Image c = bb_loadimg(r);
	if (!c.data) return 0;
	int h = newimg(c, c.width, c.height, 1);
	upload(&is[h]);
	return h;
}
int bbLoadAnimImage(const char *p, int fw, int fh, int first, int count) {
	const char *r = bbPath(p);
	if (!r || fw <= 0 || fh <= 0) return 0;
	Image c = bb_loadimg(r);
	if (!c.data) return 0;
	int per = c.width / fw;
	if (per <= 0) { UnloadImage(c); return 0; }
	Image strip = GenImageColor(fw * count, fh, BLACK); // frames side by side
	for (int i = 0; i < count; i++) {
		int f = first + i;
		ImageDraw(&strip, c, (Rectangle){(f % per) * fw, (f / per) * fh, fw, fh}, (Rectangle){i * fw, 0, fw, fh}, WHITE);
	}
	UnloadImage(c);
	int h = newimg(strip, fw, fh, count);
	upload(&is[h]);
	return h;
}
int bbCreateImage(int w, int h, int frames) {
	if (frames < 1) frames = 1;
	int i = newimg(GenImageColor(w * frames, h, BLACK), w, h, frames);
	upload(&is[i]);
	return i;
}
int bbCopyImage(int h) {
	img *m = gi(h); sync(m);
	int n = newimg(ImageCopy(m->cpu), m->fw, m->fh, m->nfr);
	m = gi(h);
	is[n].hx = m->hx; is[n].hy = m->hy; is[n].mask = m->mask;
	upload(&is[n]);
	return n;
}
void bbFreeImage(int h) {
	if (!h) return;
	img *m = gi(h);
	if (cur == bbImageBuffer(h, 0)) { g2d_end(); cur = BACKBUF; }
	if (m->hasrt) UnloadRenderTexture(m->rt); else UnloadTexture(m->t);
	UnloadImage(m->cpu);
	m->used = 0;
}
int bbImageWidth(int h) { return gi(h)->fw; }
int bbImageHeight(int h) { return gi(h)->fh; }
void bbMaskImage(int h, int r, int g, int b) { img *m = gi(h); sync(m); m->mask = (Color){r, g, b, 255}; upload(m); }
void bbHandleImage(int h, float x, float y) { img *m = gi(h); m->hx = x; m->hy = y; }
void bbMidHandle(int h) { img *m = gi(h); m->hx = m->fw / 2; m->hy = m->fh / 2; }
void bbAutoMidHandle(int on) { autoMid = on; }
void bbResizeImage(int h, float w, float hh) {
	img *m = gi(h); sync(m);
	if (w < 1 || hh < 1) return;
	ImageResize(&m->cpu, (int)w * m->nfr, (int)hh);
	m->fw = m->w = w; m->fh = m->h = hh;
	upload(m);
}
void bbScaleImage(int h, float sx, float sy) { img *m = gi(h); bbResizeImage(h, m->fw * sx, m->fh * sy); }
void bbRotateImage(int h, float ang) { // blitz rotates clockwise around the center
	img *m = gi(h); sync(m);
	ImageRotate(&m->cpu, (int)ang);
	m->fw = m->w = m->cpu.width; m->fh = m->h = m->cpu.height; m->nfr = 1;
	upload(m);
}
void bbDrawImageRect(int h, int x, int y, int rx, int ry, int rw, int rhh, int frame) {
	img *m = gi(h);
	target();
	if (frame < 0 || frame >= m->nfr) frame = 0;
	Rectangle src = {frame * m->fw + rx, ry, rw, rhh};
	DrawTexturePro(m->t, src, (Rectangle){x + ox - m->hx, y + oy - m->hy, rw, rhh}, (Vector2){0}, 0, WHITE);
}
void bbDrawImage(int h, int x, int y, int frame) { img *m = gi(h); bbDrawImageRect(h, x, y, 0, 0, m->fw, m->fh, frame); }
void bbDrawBlock(int h, int x, int y, int frame) { bbDrawImage(h, x, y, frame); } // TODO: unmasked
int bbImageRectOverlap(int h, int x, int y, int rx, int ry, int rw, int rh) {
	img *m = gi(h);
	float ix = x - m->hx, iy = y - m->hy;
	return ix < rx + rw && ix + m->fw > rx && iy < ry + rh && iy + m->fh > ry;
}

// ---- CopyRect + pixels (cpu paths; fine for scpcb's few uses)
static Image grab(int b) { // whole buffer as a top-down rgba image
	g2d_end(); rlDrawRenderBatchActive(); // pending draws first
	if (b == BACKBUF) {
		int w = GetRenderWidth(), h = GetRenderHeight();
		return (Image){rlReadScreenPixels(w, h), w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	}
	Texture2D t = b >> 24 == 2 ? gi(b & 0xffffff)->t : bb_t(b & 0xffffff)->t;
	Image g = LoadImageFromTexture(t);
	ImageFormat(&g, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	return g;
}
static void put(int b, Image src, int dx, int dy) { // blit into an image/texture buffer at dx,dy
	if (b == BACKBUF) {
		Texture2D t = LoadTextureFromImage(src);
		target(); DrawTexture(t, dx, dy, WHITE); rlDrawRenderBatchActive();
		UnloadTexture(t);
		return;
	}
	Texture2D t = b >> 24 == 2 ? gi(b & 0xffffff)->t : bb_t(b & 0xffffff)->t;
	Rectangle r = {dx, dy, src.width, src.height};
	if (r.x < 0 || r.y < 0 || r.x + r.width > t.width || r.y + r.height > t.height) { // clip
		Rectangle c = GetCollisionRec(r, (Rectangle){0, 0, t.width, t.height});
		if (c.width <= 0 || c.height <= 0) return;
		ImageCrop(&src, (Rectangle){c.x - r.x, c.y - r.y, c.width, c.height});
		r = c;
	}
	g2d_end();
	UpdateTextureRec(t, r, src.data);
	if (b >> 24 == 2) gi(b & 0xffffff)->stale = 1;
}
void bbCopyRect(int sx, int sy, int w, int h, int dx, int dy, int src, int dst) {
	if (!src) src = cur;
	if (!dst) dst = cur;
	Image g = grab(src);
	ImageCrop(&g, (Rectangle){sx, sy, w, h});
	put(dst, g, dx, dy);
	UnloadImage(g);
}

static Image lk; static int lkb, lkdirty;
void bbLockBuffer(int b) { if (lkb) return; lk = grab(b ? b : cur); lkb = b ? b : cur; lkdirty = 0; }
void bbUnlockBuffer(int b) {
	(void)b;
	if (!lkb) return;
	if (lkdirty) put(lkb, lk, 0, 0);
	UnloadImage(lk); lkb = 0;
}
// argb ints like blitz
int bbReadPixel(int x, int y, int b) {
	b = b ? b : cur;
	int own = lkb != b;
	if (own) bbLockBuffer(b);
	int r = 0;
	if (x >= 0 && y >= 0 && x < lk.width && y < lk.height) {
		unsigned char *p = (unsigned char *)lk.data + (y * lk.width + x) * 4;
		r = p[3] << 24 | p[0] << 16 | p[1] << 8 | p[2];
	}
	if (own) bbUnlockBuffer(b);
	return r;
}
void bbWritePixel(int x, int y, int argb, int b) {
	b = b ? b : cur;
	int own = lkb != b;
	if (own) bbLockBuffer(b);
	if (x >= 0 && y >= 0 && x < lk.width && y < lk.height) {
		unsigned char *p = (unsigned char *)lk.data + (y * lk.width + x) * 4;
		p[0] = argb >> 16; p[1] = argb >> 8; p[2] = argb; p[3] = (unsigned)argb >> 24;
		lkdirty = 1;
	}
	if (own) bbUnlockBuffer(b);
}

// ---- window-ish bits
static const int modes[][2] = {{640, 480}, {800, 600}, {1024, 768}, {1280, 720}, {1280, 1024}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1440}};
int bbCountGfxModes3D(void) { return sizeof modes / sizeof *modes; }
int bbGfxModeWidth(int i) { return i >= 1 && i <= bbCountGfxModes3D() ? modes[i-1][0] : 0; }
int bbGfxModeHeight(int i) { return i >= 1 && i <= bbCountGfxModes3D() ? modes[i-1][1] : 0; }
void bbHidePointer(void) { HideCursor(); }
void bbShowPointer(void) { ShowCursor(); }
void bbMoveMouse(int x, int y) { SetMousePosition(x, y); }
void bbAppTitle(const char *t) { if (IsWindowReady()) SetWindowTitle(t); }
int bbTotalVidMem(void) { return 1 << 30; }
int bbAvailVidMem(void) { return 1 << 30; }
