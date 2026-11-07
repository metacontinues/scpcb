// fs.c: game paths (backslashes, any case) on a case-sensitive fs + binary io
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include "in.h"

// resolve "GFX\map\door01.x" -> "GFX/map/Door01.x". walks components, case-insensitive
const char *res_path(const char *p) {
	static char out[1024];
	char in[1024], *save, *c;
	struct stat st;
	snprintf(in, sizeof in, "%s", p);
	for (c = in; *c; c++) if (*c == '\\') *c = '/';
	if (!stat(in, &st)) { strcpy(out, in); return out; }
	out[0] = 0;
	if (in[0] == '/') strcpy(out, "/");
	for (c = strtok_r(in, "/", &save); c; c = strtok_r(NULL, "/", &save)) {
		if (!strcmp(c, ".")) continue;
		size_t n = strlen(out);
		const char *dir = n ? out : ".";
		DIR *d = opendir(dir);
		if (!d) return NULL;
		struct dirent *de; int ok = 0;
		while ((de = readdir(d))) if (!strcasecmp(de->d_name, c)) { ok = 1; break; }
		if (ok) snprintf(out + n, sizeof out - n, "%s%s", n && out[n-1] != '/' ? "/" : "", de->d_name);
		closedir(d);
		if (!ok) return NULL;
	}
	return out;
}

// media (images, sounds) is read from the qoi/qoa copies tools/assetconv.c writes: <data>/<real path>.qoi|qoa.
// <data> is SCPCB_DATA or port/data, both relative to the game dir like the assets themselves
const char *rt_media(const char *real, const char *ext) {
	static char out[2048], cwd[1024];
	if (!real) return NULL;
	const char *d = getenv("SCPCB_DATA");
	if (real[0] == '/') { // absolute (CurrentDir() + ...): make it relative again
		if (!cwd[0] && !getcwd(cwd, sizeof cwd)) return NULL;
		size_t n = strlen(cwd);
		if (strncmp(real, cwd, n) || real[n] != '/') return NULL;
		real += n + 1;
	}
	snprintf(out, sizeof out, "%s/%s.%s", d ? d : "port/data", real, ext);
	struct stat st;
	if (stat(out, &st)) {
		static int warned;
		if (!warned++) fprintf(stderr, "bb: no converted asset %s (run port/build/assetconv from the game dir)\n", out);
		return NULL;
	}
	return out;
}

FILE *read_file(const char *p) {
	const char *r = res_path(p);
	return r ? fopen(r, "rb") : NULL;
}
// files are little-endian
int read_byte(FILE *f) { int c = fgetc(f); return c == EOF ? 0 : c; }
int read_short(FILE *f) { unsigned char b[2] = {0}; (void)!fread(b, 1, 2, f); return b[0] | b[1] << 8; }
int read_int(FILE *f) {
	unsigned char b[4] = {0};
	(void)!fread(b, 1, 4, f);
	return (int)((unsigned)b[0] | (unsigned)b[1] << 8 | (unsigned)b[2] << 16 | (unsigned)b[3] << 24);
}
float read_float(FILE *f) { int i = read_int(f); float r; memcpy(&r, &i, 4); return r; }
str read_string(FILE *f) {
	int n = read_int(f);
	if (n < 0 || n > 1 << 24) n = 0;
	str s = GC_MALLOC_ATOMIC(n + 1);
	n = fread(s, 1, n, f);
	s[n] = 0;
	return s;
}
str read_line(FILE *f) {
	size_t n = 0, cap = 64; str s = GC_MALLOC_ATOMIC(cap); int c;
	while ((c = fgetc(f)) != EOF && c != '\n') {
		if (n + 1 >= cap) s = GC_REALLOC(s, cap *= 2);
		s[n++] = c;
	}
	if (n && s[n-1] == '\r') n--;
	s[n] = 0;
	return s;
}
