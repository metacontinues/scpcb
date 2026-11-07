// bake: source assets -> next/data. run from the repo root: next/build/bake [file.rmesh|.b3d|.x ...]
// w/o args it bakes every room + model under GFX. only redoes outputs older than their source.
#define _POSIX_C_SOURCE 200809L // dirent, stat, strcasecmp
#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include "raylib.h"
#include "bake.h"
#ifdef _WIN32
	#include <direct.h>
	#define mkdir(p, m) _mkdir(p)
#endif

int nwarn;
void warn(const char *fmt, ...) {
	va_list a; va_start(a, fmt);
	fprintf(stderr, "warn: "); vfprintf(stderr, fmt, a); fputc('\n', stderr);
	va_end(a);
	nwarn++;
}

void sbraw(sb *b, const void *d, size_t n) {
	if (b->n + n + 1 > b->c) { b->c = (b->n + n + 1) * 2; b->s = realloc(b->s, b->c); }
	memcpy(b->s + b->n, d, n);
	b->n += n; b->s[b->n] = 0;
}
void sbf(sb *b, const char *fmt, ...) {
	va_list a; va_start(a, fmt);
	int n = vsnprintf(NULL, 0, fmt, a);
	va_end(a);
	if (n <= 0) return;
	char *t = malloc(n + 1);
	va_start(a, fmt); vsnprintf(t, n + 1, fmt, a); va_end(a);
	sbraw(b, t, n);
	free(t);
}

const char *ci(const char *path) {
	static char o[4096];
	char p[4096];
	snprintf(p, sizeof p, "%s", path);
	for (char *c = p; *c; c++) if (*c == '\\') *c = '/';
	o[0] = 0;
	for (char *s = p, *e; s && *s; s = e) {
		e = strchr(s, '/');
		if (e) *e++ = 0;
		if (!*s) continue;
		char t[4096];
		snprintf(t, sizeof t, "%s%s%s", o, *o ? "/" : "", s);
		struct stat st;
		if (stat(t, &st)) {
			DIR *d = opendir(*o ? o : ".");
			if (!d) return NULL;
			struct dirent *de;
			while ((de = readdir(d)) && strcasecmp(de->d_name, s));
			if (!de) { closedir(d); return NULL; }
			snprintf(t, sizeof t, "%s%s%s", o, *o ? "/" : "", de->d_name);
			closedir(d);
		}
		snprintf(o, sizeof o, "%s", t);
	}
	return o;
}

void mkid(const char *path, char *o, int n) {
	int k = 0;
	for (const char *c = path; *c && k < n - 1; c++) o[k++] = *c == '\\' ? '/' : *c == ' ' ? '_' : tolower((unsigned char)*c);
	o[k] = 0;
	char *dot = strrchr(o, '.'), *sl = strrchr(o, '/');
	if (dot && (!sl || dot > sl)) *dot = 0;
}

void mkdirs(const char *path) {
	char t[4096];
	snprintf(t, sizeof t, "%s", path);
	for (char *c = t + 1; *c; c++) if (*c == '/') { *c = 0; mkdir(t, 0755); *c = '/'; }
}

int hasci(const char *s, const char *sub) {
	for (size_t n = strlen(sub); *s; s++) if (!strncasecmp(s, sub, n)) return 1;
	return 0;
}

int stale(const char *src, const char *out) {
	struct stat a, b;
	if (stat(out, &b)) return 1;
	return stat(src, &a) || a.st_mtime > b.st_mtime;
}

static int nrooms, nmodels, nfail;
static int ends(const char *p, const char *e) { size_t l = strlen(p), k = strlen(e); return l >= k && !strcasecmp(p + l - k, e); }
static void one(const char *p) {
	int ok;
	if (ends(p, ".rmesh")) { nrooms++; ok = bake_room(p); }
	else if (ends(p, ".b3d") || ends(p, ".x")) { nmodels++; ok = bake_model(p); }
	else { warn("%s: don't know what that is", p); ok = 0; }
	if (!ok) nfail++;
}
static void walk(const char *dir) {
	DIR *d = opendir(dir);
	if (!d) return;
	for (struct dirent *e; (e = readdir(d));) {
		if (e->d_name[0] == '.') continue;
		char p[4096];
		snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
		struct stat st;
		if (stat(p, &st)) continue;
		if (S_ISDIR(st.st_mode)) { walk(p); continue; }
		if (!ends(p, ".rmesh") && !ends(p, ".b3d") && !ends(p, ".x")) continue;
		if (!st.st_size) { warn("%s is empty, skipped", p); continue; }
		one(p);
	}
	closedir(d);
}

int main(int argc, char **argv) {
	SetTraceLogLevel(LOG_WARNING);
	if (!DirectoryExists("GFX") || !DirectoryExists("next")) { fprintf(stderr, "bake: run me from the repo root\n"); return 1; }
	if (argc > 1) for (int i = 1; i < argc; i++) one(argv[i]);
	else walk("GFX");
	printf("bake: %d rooms, %d models, %d failed, %d warnings\n", nrooms, nmodels, nfail, nwarn);
	return nfail != 0;
}
