// g2d.c: 2d. buffers, images, text/rect/line/oval, fonts, copy_rect, pixels.
// image/texture buffers become render textures on first SetBuffer, drawn w/ a y-flipped
// ortho so row 0 stays the top row, which is what the game's uvs expect.
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
int back_buffer(void) { return BACKBUF; }
int front_buffer(void) { return BACKBUF; }
int image_buffer(int i, int frame) { (void)frame; return 2 << 24 | i; }
int texture_buffer(htex t, int frame) { (void)frame; return 3 << 24 | t; }
static int cur = BACKBUF, active = BACKBUF;
int graphics_buffer(void) { return cur; }

static img *gi(int h) { if (h <= 0 || h >= nis || !is[h].used) die("bad image handle %d", h); return &is[h]; }

// give an image/texture a render target, keeping what it showed
static RenderTexture2D *rtof(int b, int *w, int *h) {
	Texture2D *t; RenderTexture2D *rt; int *has;
	if (b >> 24 == 2) { img *m = gi(b & 0xffffff); t = &m->t; rt = &m->rt; has = &m->hasrt; m->stale = 1; }
	else { tex *x = rt_t(b & 0xffffff); t = &x->t; rt = &x->rt; has = &x->hasrt; }
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
	// images are masked at draw time: pull the drawing back + re-key the mask color
	if (was >> 24 == 2 && (was & 0xffffff) < nis && is[was & 0xffffff].used) { img *m = &is[was & 0xffffff]; sync(m); upload(m); }
}
// bumped by anything that can change a texture's pixels; ReadPixel's cache keys on it
static unsigned pxgen;
void rt_g2dtouch(void) { pxgen++; }
static void target(void) {
	rt_frame();
	pxgen++;
	if (active == cur) return;
	g2d_end();
	if (cur == BACKBUF) return;
	int w, h;
	RenderTexture2D *rt = rtof(cur, &w, &h);
	BeginTextureMode(*rt);
	rlSetMatrixProjection(MatrixOrtho(0, w, 0, h, 0, 1)); // y up in memory = rows top-down
	rlDisableBackfaceCulling();
	active = cur;
}
void set_buffer(int b) { cur = b ? b : BACKBUF; }

// ---- state
void set_color(int r, int g, int b) { col = (Color){r, g, b, 255}; }
int color_red(void) { return col.r; }
int color_green(void) { return col.g; }
int color_blue(void) { return col.b; }
void clear_color(int r, int g, int b) { clsc = (Color){r, g, b, 255}; }
void clear_screen(void) { target(); ClearBackground(clsc); }
void set_origin(int x, int y) { ox = x; oy = y; }
void set_viewport(int x, int y, int w, int h) {
	target();
	if (w <= 0 || h <= 0) { EndScissorMode(); return; }
	BeginScissorMode(x, y, w, h);
}

// ---- primitives
void draw_plot(int x, int y) { target(); DrawPixel(x + ox, y + oy, col); }
void draw_line(int x1, int y1, int x2, int y2) {
	target();
	DrawLineV((Vector2){x1 + ox + 0.5f, y1 + oy + 0.5f}, (Vector2){x2 + ox + 0.5f, y2 + oy + 0.5f}, col);
}
void draw_rect(int x, int y, int w, int h, int solid) {
	target();
	if (solid) DrawRectangle(x + ox, y + oy, w, h, col);
	else DrawRectangleLines(x + ox, y + oy, w, h, col);
}
void draw_oval(int x, int y, int w, int h, int solid) {
	target();
	float rx = w * 0.5f, ry = h * 0.5f;
	if (solid) DrawEllipse(x + ox + rx, y + oy + ry, rx, ry, col);
	else DrawEllipseLines(x + ox + rx, y + oy + ry, rx, ry, col);
}

// ---- fonts. names that aren't files ("Blitz", "Tahoma") get courier, like the game mostly wants
int load_font(const char *p, int height, int bold, int italic, int underline) {
	(void)bold; (void)italic; (void)underline;
	const char *r = res_path(p);
	if (!r || DirectoryExists(r)) r = res_path("GFX\\font\\cour\\Courier New.ttf");
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
void free_font(int h) {
	if (h <= 0 || h >= nfs || !fs[h].texture.id) return;
	UnloadFont(fs[h]); fs[h] = (Font){0};
	if (curfont == h) curfont = 0;
}
void set_font(int h) { curfont = h > 0 && h < nfs && fs[h].texture.id ? h : 0; }
static Font font(void) { return curfont ? fs[curfont] : GetFontDefault(); }
static float fsize(void) { return curfont ? fsz[curfont] : 13; }
int font_height(void) { return fsize(); }
int string_width(const char *s) { return MeasureTextEx(font(), s, fsize(), 0).x; }
int string_height(const char *s) { (void)s; return fsize(); }
int font_width(void) { return string_width("W"); }
// text w/ alpha. cx/cy center on x/y
void draw_text_a(int x, int y, const char *s, int cx, int cy, float a) {
	target();
	Vector2 sz = MeasureTextEx(font(), s, fsize(), 0);
	float px = x + ox - (cx ? sz.x / 2 : 0), py = y + oy - (cy ? fsize() / 2 : 0);
	Color c = col; c.a = 255 * fmaxf(0, fminf(1, a));
	DrawTextEx(font(), s, (Vector2){(int)px, (int)py}, fsize(), 0, c);
}
void draw_text(int x, int y, const char *s, int cx, int cy) { draw_text_a(x, y, s, cx, cy, 1); }

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
// masked texture from cpu: image alpha is ignored, the mask color is keyed out
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
int load_image(const char *p) {
	const char *r = res_path(p);
	if (!r || DirectoryExists(r)) return 0;
	Image c = rt_loadimg(r);
	if (!c.data) return 0;
	int h = newimg(c, c.width, c.height, 1);
	upload(&is[h]);
	return h;
}
int load_anim_image(const char *p, int fw, int fh, int first, int count) {
	const char *r = res_path(p);
	if (!r || fw <= 0 || fh <= 0) return 0;
	Image c = rt_loadimg(r);
	if (!c.data) return 0;
	int per = c.width / fw;
	if (per <= 0) { UnloadImage(c); return 0; }
	Image strip = GenImageColor(fw * count, fh, BLACK); // frames side by side
	for (int i = 0; i < count; i++) {
		int f = first + i;
		ImageDrawImagePro(&strip, c, (Rectangle){(f % per) * fw, (f / per) * fh, fw, fh}, (Rectangle){i * fw, 0, fw, fh}, (Vector2){0}, 0, WHITE);
	}
	UnloadImage(c);
	int h = newimg(strip, fw, fh, count);
	upload(&is[h]);
	return h;
}
int create_image(int w, int h, int frames) {
	if (frames < 1) frames = 1;
	int i = newimg(GenImageColor(w * frames, h, BLACK), w, h, frames);
	upload(&is[i]);
	return i;
}
int copy_image(int h) {
	img *m = gi(h); sync(m);
	int n = newimg(ImageCopy(m->cpu), m->fw, m->fh, m->nfr);
	m = gi(h);
	is[n].hx = m->hx; is[n].hy = m->hy; is[n].mask = m->mask;
	upload(&is[n]);
	return n;
}
void free_image(int h) {
	if (!h) return;
	img *m = gi(h);
	if (cur == image_buffer(h, 0)) { g2d_end(); cur = BACKBUF; }
	if (m->hasrt) UnloadRenderTexture(m->rt); else UnloadTexture(m->t);
	UnloadImage(m->cpu);
	m->used = 0;
}
int image_width(int h) { return gi(h)->fw; }
int image_height(int h) { return gi(h)->fh; }
void mask_image(int h, int r, int g, int b) { img *m = gi(h); sync(m); m->mask = (Color){r, g, b, 255}; upload(m); }
void handle_image(int h, float x, float y) { img *m = gi(h); m->hx = x; m->hy = y; }
void mid_handle(int h) { img *m = gi(h); m->hx = m->fw / 2; m->hy = m->fh / 2; }
void auto_mid_handle(int on) { autoMid = on; }
void resize_image(int h, float w, float hh) {
	img *m = gi(h); sync(m);
	if (w < 1 || hh < 1) return;
	ImageResize(&m->cpu, (int)w * m->nfr, (int)hh);
	m->fw = m->w = w; m->fh = m->h = hh;
	upload(m);
}
void scale_image(int h, float sx, float sy) { img *m = gi(h); resize_image(h, m->fw * sx, m->fh * sy); }
void rotate_image(int h, float ang) { // clockwise around the center
	img *m = gi(h); sync(m);
	ImageRotate(&m->cpu, (int)ang);
	m->fw = m->w = m->cpu.width; m->fh = m->h = m->cpu.height; m->nfr = 1;
	upload(m);
}
void draw_image_rect(int h, int x, int y, int rx, int ry, int rw, int rhh, int frame) {
	img *m = gi(h);
	target();
	if (frame < 0 || frame >= m->nfr) frame = 0;
	Rectangle src = {frame * m->fw + rx, ry, rw, rhh};
	DrawTexturePro(m->t, src, (Rectangle){x + ox - m->hx, y + oy - m->hy, rw, rhh}, (Vector2){0}, 0, WHITE);
}
void draw_image(int h, int x, int y, int frame) { img *m = gi(h); draw_image_rect(h, x, y, 0, 0, m->fw, m->fh, frame); }
void draw_block(int h, int x, int y, int frame) { draw_image(h, x, y, frame); } // TODO: unmasked
int image_rect_overlap(int h, int x, int y, int rx, int ry, int rw, int rh) {
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
	Texture2D t = b >> 24 == 2 ? gi(b & 0xffffff)->t : rt_t(b & 0xffffff)->t;
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
	Texture2D t = b >> 24 == 2 ? gi(b & 0xffffff)->t : rt_t(b & 0xffffff)->t;
	Rectangle r = {dx, dy, src.width, src.height};
	if (r.x < 0 || r.y < 0 || r.x + r.width > t.width || r.y + r.height > t.height) { // clip
		Rectangle c = GetCollisionRec(r, (Rectangle){0, 0, t.width, t.height});
		if (c.width <= 0 || c.height <= 0) return;
		ImageCrop(&src, (Rectangle){c.x - r.x, c.y - r.y, c.width, c.height});
		r = c;
	}
	g2d_end();
	pxgen++;
	UpdateTextureRec(t, r, src.data);
	if (b >> 24 == 2) gi(b & 0xffffff)->stale = 1;
}
void copy_rect(int sx, int sy, int w, int h, int dx, int dy, int src, int dst) {
	if (!src) src = cur;
	if (!dst) dst = cur;
	Image g = grab(src);
	ImageCrop(&g, (Rectangle){sx, sy, w, h});
	put(dst, g, dx, dy);
	UnloadImage(g);
}

static Image lk; static int lkb, lkdirty;
void lock_buffer(int b) { if (lkb) return; lk = grab(b ? b : cur); lkb = b ? b : cur; lkdirty = 0; }
void unlock_buffer(int b) {
	(void)b;
	if (!lkb) return;
	if (lkdirty) put(lkb, lk, 0, 0);
	UnloadImage(lk); lkb = 0;
}
// argb ints. outside a lock, reads go through a few cached downloads (one gpu readback
// per buffer until something draws), images just read their cpu copy
static struct { int b; unsigned gen; Image im; } pxc[4];
static int pxn;
static Image *pxsrc(int b) {
	if (lkb == b) return &lk;
	if (b >> 24 == 2) { img *m = gi(b & 0xffffff); if (m->stale) { g2d_end(); rlDrawRenderBatchActive(); } sync(m); return &m->cpu; }
	for (int i = 0; i < 4; i++) if (pxc[i].b == b && pxc[i].gen == pxgen) return &pxc[i].im;
	int i = pxn++ % 4;
	UnloadImage(pxc[i].im);
	pxc[i].im = grab(b); pxc[i].b = b; pxc[i].gen = pxgen;
	return &pxc[i].im;
}
int read_pixel(int x, int y, int b) {
	Image *im = pxsrc(b ? b : cur);
	if (x < 0 || y < 0 || x >= im->width || y >= im->height) return 0;
	unsigned char *p = (unsigned char *)im->data + (y * im->width + x) * 4;
	return p[3] << 24 | p[0] << 16 | p[1] << 8 | p[2];
}
void write_pixel(int x, int y, int argb, int b) {
	b = b ? b : cur;
	int own = lkb != b;
	if (own) lock_buffer(b);
	if (x >= 0 && y >= 0 && x < lk.width && y < lk.height) {
		unsigned char *p = (unsigned char *)lk.data + (y * lk.width + x) * 4;
		p[0] = argb >> 16; p[1] = argb >> 8; p[2] = argb; p[3] = (unsigned)argb >> 24;
		lkdirty = 1;
	}
	if (own) unlock_buffer(b);
}

// ---- window-ish bits
static const int modes[][2] = {{640, 480}, {800, 600}, {1024, 768}, {1280, 720}, {1280, 1024}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1440}};
int count_gfx_modes_3d(void) { return sizeof modes / sizeof *modes; }
int gfx_mode_width(int i) { return i >= 1 && i <= count_gfx_modes_3d() ? modes[i-1][0] : 0; }
int gfx_mode_height(int i) { return i >= 1 && i <= count_gfx_modes_3d() ? modes[i-1][1] : 0; }
// the game used to keep the mouse in by warping it to the center every frame (MouseLook), which xwayland/wayland
// don't allow, so a hidden pointer is a locked one here (glfw pointer lock, relative motion).
// scripted runs (SCPCB_INPUT) only hide it, so they never grab the real mouse
static int locked;
void hide_pointer(void) {
	if (getenv("SCPCB_INPUT")) { HideCursor(); return; }
	if (!locked) { DisableCursor(); locked = 1; }
}
void show_pointer(void) {
	if (locked) { EnableCursor(); locked = 0; }
	ShowCursor();
}
void move_mouse(int x, int y) { SetMousePosition(x, y); }
void app_title(const char *t) { if (IsWindowReady()) SetWindowTitle(t); }
int total_vid_mem(void) { return 1 << 30; }
int avail_vid_mem(void) { return 1 << 30; }
