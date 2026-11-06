// misc.c: blitz rng (exact, map gen depends on it), file writing, dirs, dates
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "in.h"

// ---- rng: blitz3d bbmath.cpp, park-miller minimal standard via schrage
static int rs = 0x1234;
static float rnd(void) {
	rs = 48271 * (rs % 44488) - 3399 * (rs / 44488);
	if (rs < 0) rs += 2147483647;
	return (rs & 65535) / 65536.0f + .5f / 65536.0f;
}
float bbRnd(float from, float to) { return rnd() * (to - from) + from; }
int bbRand(int from, int to) {
	if (to < from) { int t = from; from = to; to = t; }
	return (int)(rnd() * (to - from + 1)) + from;
}
void bbSeedRnd(int seed) { seed &= 0x7fffffff; rs = seed ? seed : 1; }
int bbRndSeed(void) { return rs; }

// ---- paths for new files: resolve the dir case-insensitively, keep the name as given
static const char *newpath(const char *p) {
	static char o[1024];
	char d[1024];
	snprintf(d, sizeof d, "%s", p);
	for (char *c = d; *c; c++) if (*c == '\\') *c = '/';
	char *sl = strrchr(d, '/');
	if (!sl) return strcpy(o, d);
	*sl = 0;
	const char *rd = bbPath(d[0] ? d : "/");
	snprintf(o, sizeof o, "%s/%s", rd ? rd : d, sl + 1);
	return o;
}
FILE *bbWriteFile(const char *p) { const char *r = bbPath(p); return fopen(r ? r : newpath(p), "wb"); }
FILE *bbOpenFile(const char *p) { const char *r = bbPath(p); return r ? fopen(r, "r+b") : NULL; }
void bbCloseFile(FILE *f) { if (f) fclose(f); }
int bbEof(FILE *f) { int c = fgetc(f); if (c == EOF) return 1; ungetc(c, f); return 0; }
int bbFilePos(FILE *f) { return ftell(f); }
int bbSeekFile(FILE *f, int pos) { fseek(f, pos, SEEK_SET); return ftell(f); }
void bbWriteByte(FILE *f, int v) { fputc(v & 255, f); }
void bbWriteShort(FILE *f, int v) { fputc(v & 255, f); fputc(v >> 8 & 255, f); }
void bbWriteInt(FILE *f, int v) { for (int i = 0; i < 4; i++) fputc((unsigned)v >> (i * 8) & 255, f); }
void bbWriteFloat(FILE *f, float v) { int i; memcpy(&i, &v, 4); bbWriteInt(f, i); }
void bbWriteString(FILE *f, const char *s) { int n = strlen(s); bbWriteInt(f, n); fwrite(s, 1, n, f); }
void bbWriteLine(FILE *f, const char *s) { fputs(s, f); fputs("\r\n", f); } // blitz writes crlf

int bbFileType(const char *p) { // 0 none, 1 file, 2 dir
	const char *r = bbPath(p);
	struct stat st;
	if (!r || stat(r, &st)) return 0;
	return S_ISDIR(st.st_mode) ? 2 : 1;
}
int bbFileSize(const char *p) { const char *r = bbPath(p); struct stat st; return r && !stat(r, &st) ? (int)st.st_size : 0; }
int bbCreateDir(const char *p) { return !mkdir(newpath(p), 0755); }
int bbDeleteFile(const char *p) { const char *r = bbPath(p); return r && !remove(r); }
int bbCopyFile(const char *a, const char *b) {
	FILE *i = bbReadFile(a), *o;
	if (!i) return 0;
	if (!(o = bbWriteFile(b))) { fclose(i); return 0; }
	char buf[1 << 14]; size_t n;
	while ((n = fread(buf, 1, sizeof buf, i))) fwrite(buf, 1, n, o);
	fclose(i); fclose(o);
	return 1;
}
const char *bbCurrentDir(void) { static char b[1024]; if (!getcwd(b, sizeof b - 1)) b[0] = 0; strcat(b, "/"); return b; }
// ReadDir/NextFile: blitz yields "." and ".." too, then "" at the end
void *bbReadDir(const char *p) { const char *r = bbPath(p); return r ? opendir(r) : NULL; }
const char *bbNextFile(void *d) { struct dirent *e = d ? readdir(d) : NULL; return e ? e->d_name : ""; }
void bbCloseDir(void *d) { if (d) closedir(d); }

const char *bbCurrentDate(void) { // "05 Nov 2026"
	static char b[32]; time_t t = time(NULL);
	strftime(b, sizeof b, "%d %b %Y", localtime(&t));
	return b;
}
const char *bbCurrentTime(void) { static char b[16]; time_t t = time(NULL); strftime(b, sizeof b, "%H:%M:%S", localtime(&t)); return b; }
void bbDelay(int ms) { WaitTime(ms / 1000.0); }
