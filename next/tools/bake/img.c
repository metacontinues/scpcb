// img.c: any image the game has -> pow2 qoi w/ the alpha it's meant to have
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "bake.h"

// by content: the game hides jpgs behind odd extensions
static const char *ext(const unsigned char *d, int n) {
	if (n < 4) return NULL;
	if (d[0] == 0x89 && d[1] == 'P') return ".png";
	if (d[0] == 0xff && d[1] == 0xd8) return ".jpg";
	if (d[0] == 'B' && d[1] == 'M') return ".bmp";
	if (!memcmp(d, "DDS ", 4)) return ".dds";
	return NULL;
}

static int pot(int v) { int p = 1; while (p < v) p <<= 1; return p; }

const char *bake_tex(const char *path, int mode) {
	static char id[1024];
	const char *src = ci(path);
	if (!src) return NULL;
	char s[4096], out[4096];
	snprintf(s, sizeof s, "%s", src);
	mkid(s, id, sizeof id - 4);
	if (mode) strcat(id, mode == IM_ALPHA ? ".a" : mode == IM_MASK ? ".m" : ".am");
	snprintf(out, sizeof out, OUT "%s.qoi", id);
	if (!stale(s, out)) return id;

	int n;
	unsigned char *d = LoadFileData(s, &n);
	const char *e = d ? ext(d, n) : NULL;
	Image im = e ? LoadImageFromMemory(e, d, n) : (Image){0};
	UnloadFileData(d);
	if (!im.data) { warn("can't decode %s", s); return NULL; }
	if (im.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) { warn("%s is compressed, skipped", s); UnloadImage(im); return NULL; }

	int hasa = im.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 || im.format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
	if (mode == IM_OPAQUE) ImageFormat(&im, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
	else {
		ImageFormat(&im, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
		for (unsigned char *p = im.data, *end = p + im.width * im.height * 4; p < end; p += 4) {
			if (!(mode & IM_ALPHA)) p[3] = 255;
			else if (!hasa) p[3] = (p[0] + p[1] + p[2]) / 3;
			if (mode & IM_MASK && !p[0] && !p[1] && !p[2]) p[3] = 0;
		}
	}
	// es2: npot can't mip or repeat
	int pw = pot(im.width), ph = pot(im.height);
	if (pw != im.width || ph != im.height) ImageResize(&im, pw, ph);
	mkdirs(out);
	int ok = ExportImage(im, out);
	UnloadImage(im);
	if (!ok) { warn("can't write %s", out); return NULL; }
	return id;
}
