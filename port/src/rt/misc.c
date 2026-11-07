// misc.c: rng, file writing, dirs, dates
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "in.h"

// ---- rng: park-miller minimal standard via schrage
static int rs = 0x1234;
static float rnd(void) {
	rs = 48271 * (rs % 44488) - 3399 * (rs / 44488);
	if (rs < 0) rs += 2147483647;
	return (rs & 65535) / 65536.0f + .5f / 65536.0f;
}
float frnd(float lo, float hi) { return rnd() * (hi - lo) + lo; }
int irand(int lo, int hi) {
	if (hi < lo) { int t = lo; lo = hi; hi = t; }
	return (int)(rnd() * (hi - lo + 1)) + lo;
}
void seed_rnd(int seed) { seed &= 0x7fffffff; rs = seed ? seed : 1; }
int rnd_seed(void) { return rs; }

// ---- paths for new files: resolve the dir case-insensitively, keep the name as given
static const char *newpath(const char *p) {
	static char o[1024];
	char d[1024];
	snprintf(d, sizeof d, "%s", p);
	for (char *c = d; *c; c++) if (*c == '\\') *c = '/';
	char *sl = strrchr(d, '/');
	if (!sl) return strcpy(o, d);
	*sl = 0;
	const char *rd = res_path(d[0] ? d : "/");
	snprintf(o, sizeof o, "%s/%s", rd ? rd : d, sl + 1);
	return o;
}
FILE *write_file(const char *p) { const char *r = res_path(p); return fopen(r ? r : newpath(p), "wb"); }
FILE *open_file(const char *p) { const char *r = res_path(p); return r ? fopen(r, "r+b") : NULL; }
void close_file(FILE *f) { if (f) fclose(f); }
int file_eof(FILE *f) { int c = fgetc(f); if (c == EOF) return 1; ungetc(c, f); return 0; }
int file_pos(FILE *f) { return ftell(f); }
int seek_file(FILE *f, int pos) { fseek(f, pos, SEEK_SET); return ftell(f); }
void write_byte(FILE *f, int v) { fputc(v & 255, f); }
void write_short(FILE *f, int v) { fputc(v & 255, f); fputc(v >> 8 & 255, f); }
void write_int(FILE *f, int v) { for (int i = 0; i < 4; i++) fputc((unsigned)v >> (i * 8) & 255, f); }
void write_float(FILE *f, float v) { int i; memcpy(&i, &v, 4); write_int(f, i); }
void write_string(FILE *f, const char *s) { int n = strlen(s); write_int(f, n); fwrite(s, 1, n, f); }
void write_line(FILE *f, const char *s) { fputs(s, f); fputs("\r\n", f); } // crlf, the game's files use it

int file_type(const char *p) { // 0 none, 1 file, 2 dir
	const char *r = res_path(p);
	struct stat st;
	if (!r || stat(r, &st)) return 0;
	return S_ISDIR(st.st_mode) ? 2 : 1;
}
int file_size(const char *p) { const char *r = res_path(p); struct stat st; return r && !stat(r, &st) ? (int)st.st_size : 0; }
int create_dir(const char *p) { return !mkdir(newpath(p), 0755); }
int delete_file(const char *p) { const char *r = res_path(p); return r && !remove(r); }
int copy_file(const char *a, const char *b) {
	FILE *i = read_file(a), *o;
	if (!i) return 0;
	if (!(o = write_file(b))) { fclose(i); return 0; }
	char buf[1 << 14]; size_t n;
	while ((n = fread(buf, 1, sizeof buf, i))) fwrite(buf, 1, n, o);
	fclose(i); fclose(o);
	return 1;
}
const char *current_dir(void) { static char b[1024]; if (!getcwd(b, sizeof b - 1)) b[0] = 0; strcat(b, "/"); return b; }
// read_dir/next_file: yields "." and ".." too, then "" at the end. sorted case-insensitively,
// which is the order ntfs hands them to FindNextFile (save/map lists show in that order)
typedef struct { struct dirent **e; int n, i; } bdir;
static int ci(const struct dirent **a, const struct dirent **b) { return strcasecmp((*a)->d_name, (*b)->d_name); }
void *read_dir(const char *p) {
	const char *r = res_path(p);
	bdir *d = r ? calloc(1, sizeof *d) : NULL;
	if (d && (d->n = scandir(r, &d->e, NULL, ci)) < 0) { free(d); d = NULL; }
	return d;
}
const char *next_file(void *v) { bdir *d = v; return d && d->i < d->n ? d->e[d->i++]->d_name : ""; }
void close_dir(void *v) {
	bdir *d = v;
	if (!d) return;
	for (int i = 0; i < d->n; i++) free(d->e[i]);
	free(d->e); free(d);
}
int delete_dir(const char *p) { const char *r = res_path(p); return r && !rmdir(r); }

const char *current_date(void) { // "05 Nov 2026"
	static char b[32]; time_t t = time(NULL);
	strftime(b, sizeof b, "%d %b %Y", localtime(&t));
	return b;
}
const char *current_time(void) { static char b[16]; time_t t = time(NULL); strftime(b, sizeof b, "%H:%M:%S", localtime(&t)); return b; }
void delay_ms(int ms) { WaitTime(ms / 1000.0); }
