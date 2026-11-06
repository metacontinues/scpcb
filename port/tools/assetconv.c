// assetconv: the game's images -> qoi, oggs -> qoa, into port/data/<same path>.<ext>.qoi|qoa.
// run from the repo root: port/build/assetconv [jobs]. only redoes files newer than their output.
// the png/jpg/bmp/dds/ogg decoders live here only; the game itself just reads qoi/qoa.
// files are recognised by content, not extension (the game hides jpgs as .dc/.pt and oggs as .cwm)
#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#include "external/stb_image.h"
#include "external/stb_vorbis.c"
#define QOI_IMPLEMENTATION
#include "external/qoi.h"
#define QOA_IMPLEMENTATION
#include "external/qoa.h"

#define OUT "port/data/"
static char **files; static int nf, cf;
enum { IMG, DDS, OGG };

static int kind(const char *p) {
	unsigned char b[4] = {0};
	FILE *f = fopen(p, "rb");
	if (!f) return -1;
	size_t n = fread(b, 1, 4, f);
	fclose(f);
	if (n < 4) return -1;
	if ((b[0] == 0x89 && b[1] == 'P') || (b[0] == 0xff && b[1] == 0xd8) || (b[0] == 'B' && b[1] == 'M')) return IMG;
	if (!memcmp(b, "DDS ", 4)) return DDS;
	if (!memcmp(b, "OggS", 4)) return OGG;
	return -1;
}

static void walk(const char *dir) {
	DIR *d = opendir(dir);
	if (!d) return;
	for (struct dirent *e; (e = readdir(d));) {
		if (e->d_name[0] == '.') continue; // ., .., .git
		char p[4096];
		if (strcmp(dir, ".")) snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
		else snprintf(p, sizeof p, "%s", e->d_name);
		if (!strcmp(dir, ".") && !strcmp(e->d_name, "port")) continue;
		struct stat st;
		if (stat(p, &st)) continue;
		if (S_ISDIR(st.st_mode)) { walk(p); continue; }
		if (!S_ISREG(st.st_mode) || kind(p) < 0) continue;
		if (nf >= cf) files = realloc(files, sizeof *files * (cf = cf ? cf * 2 : 1024));
		files[nf++] = strdup(p);
	}
	closedir(d);
}

static void mkdirs(const char *path) { // parents of path
	char t[4096];
	snprintf(t, sizeof t, "%s", path);
	for (char *c = t + 1; *c; c++) if (*c == '/') { *c = 0; mkdir(t, 0755); *c = '/'; }
}

// ---- dds: dxt1/3/5, top mip only
static void c565(unsigned v, unsigned char *o) { o[0] = (v >> 11 & 31) * 255 / 31; o[1] = (v >> 5 & 63) * 255 / 63; o[2] = (v & 31) * 255 / 31; o[3] = 255; }
static unsigned char *dds(const char *p, int *w, int *h) {
	FILE *f = fopen(p, "rb");
	if (!f) return NULL;
	unsigned char hd[128];
	if (fread(hd, 1, 128, f) != 128) { fclose(f); return NULL; }
	*h = hd[12] | hd[13] << 8 | hd[14] << 16; *w = hd[16] | hd[17] << 8 | hd[18] << 16;
	int fmt = !memcmp(hd + 84, "DXT1", 4) ? 1 : !memcmp(hd + 84, "DXT3", 4) ? 3 : !memcmp(hd + 84, "DXT5", 4) ? 5 : 0;
	int alpha1 = (hd[80] & 1) != 0; // DDPF_ALPHAPIXELS: dxt1 w/ 1-bit alpha
	if (!fmt) { fclose(f); return NULL; }
	int bs = fmt == 1 ? 8 : 16, bw = (*w + 3) / 4, bh = (*h + 3) / 4;
	unsigned char *src = malloc((size_t)bw * bh * bs), *out = calloc((size_t)*w * *h, 4), *s = src;
	if (fread(src, bs, (size_t)bw * bh, f) != (size_t)bw * bh) { fclose(f); free(src); free(out); return NULL; }
	fclose(f);
	for (int by = 0; by < bh; by++) for (int bx = 0; bx < bw; bx++, s += bs) {
		const unsigned char *a = bs == 16 ? s : NULL, *c = s + (bs == 16 ? 8 : 0);
		unsigned c0 = c[0] | c[1] << 8, c1 = c[2] | c[3] << 8, bits = c[4] | c[5] << 8 | c[6] << 16 | (unsigned)c[7] << 24;
		unsigned char pal[4][4];
		c565(c0, pal[0]); c565(c1, pal[1]);
		int four = c0 > c1 || bs == 16;
		for (int k = 0; k < 4; k++) {
			pal[2][k] = four ? (2 * pal[0][k] + pal[1][k]) / 3 : (pal[0][k] + pal[1][k]) / 2;
			pal[3][k] = four ? (pal[0][k] + 2 * pal[1][k]) / 3 : 0;
		}
		if (!four) pal[3][3] = alpha1 ? 0 : 255;
		unsigned char al[8] = {0};
		unsigned long long ab = 0;
		if (fmt == 5) {
			al[0] = a[0]; al[1] = a[1];
			for (int k = 2; k < 8; k++) al[k] = a[0] > a[1] ? ((8 - k) * a[0] + (k - 1) * a[1]) / 7 : k < 6 ? ((6 - k) * a[0] + (k - 1) * a[1]) / 5 : k == 6 ? 0 : 255;
			for (int k = 7; k >= 2; k--) ab = ab << 8 | a[k];
		}
		for (int i = 0; i < 16; i++) {
			int x = bx * 4 + i % 4, y = by * 4 + i / 4;
			if (x >= *w || y >= *h) continue;
			unsigned char *o = out + ((size_t)y * *w + x) * 4;
			memcpy(o, pal[bits >> (2 * i) & 3], 4);
			if (fmt == 3) o[3] = (a[i / 2] >> (4 * (i & 1)) & 15) * 17;
			else if (fmt == 5) o[3] = al[ab >> (3 * i) & 7];
		}
	}
	free(src);
	return out;
}

static int conv(const char *p) {
	int k = kind(p);
	char o[4200];
	snprintf(o, sizeof o, OUT "%s.%s", p, k == OGG ? "qoa" : "qoi");
	struct stat si, so;
	if (!stat(p, &si) && !stat(o, &so) && so.st_mtime >= si.st_mtime) return 0; // up to date
	mkdirs(o);
	if (k == OGG) {
		int ch, rate; short *pcm;
		int n = stb_vorbis_decode_filename(p, &ch, &rate, &pcm);
		if (n <= 0) { fprintf(stderr, "assetconv: can't decode %s\n", p); return 1; }
		qoa_desc d = {.channels = ch, .samplerate = rate, .samples = n};
		int ok = qoa_write(o, pcm, &d) > 0;
		free(pcm);
		return !ok;
	}
	int w, h, n = 4;
	unsigned char *px = NULL;
	if (k == DDS) px = dds(p, &w, &h);
	else {
		if (!stbi_info(p, &w, &h, &n)) n = 4;
		n = n == 1 ? 3 : n == 2 ? 4 : n; // qoi is rgb or rgba: gray widens, alpha presence is kept
		px = stbi_load(p, &w, &h, NULL, n);
	}
	if (!px) { fprintf(stderr, "assetconv: can't decode %s\n", p); return 1; }
	qoi_desc d = {.width = w, .height = h, .channels = n, .colorspace = QOI_SRGB};
	int ok = qoi_write(o, px, &d) > 0;
	free(px);
	return !ok;
}

int main(int argc, char **argv) {
	int jobs = argc > 1 ? atoi(argv[1]) : (int)sysconf(_SC_NPROCESSORS_ONLN);
	if (jobs < 1) jobs = 1;
	if (access("GFX", F_OK)) { fprintf(stderr, "assetconv: run from the repo root\n"); return 1; }
	walk(".");
	for (int j = 0; j < jobs; j++) {
		if (fork() == 0) {
			int bad = 0;
			for (int i = j; i < nf; i += jobs) bad += conv(files[i]);
			_exit(bad ? 1 : 0);
		}
	}
	int bad = 0;
	for (int st; wait(&st) > 0;) bad |= !WIFEXITED(st) || WEXITSTATUS(st);
	printf("assetconv: %d files, %s\n", nf, bad ? "some failed" : "ok");
	return bad;
}
