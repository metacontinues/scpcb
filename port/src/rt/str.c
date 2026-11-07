// str.c: string helpers. results are temporaries from a frame arena (see tmp_reset), stored strings are heap copies (sset)
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

// ---- the temp arena: chunks, all dropped at once by tmp_reset
typedef struct chunk { struct chunk *next; size_t used, cap; char data[]; } chunk;
static chunk *head;
char *tmp_alloc(size_t n) {
	n = (n + 7) & ~(size_t)7;
	if (!head || head->used + n > head->cap) {
		size_t cap = n > 1 << 16 ? n : 1 << 16;
		chunk *c = malloc(sizeof *c + cap);
		c->next = head; c->used = 0; c->cap = cap;
		head = c;
	}
	char *p = head->data + head->used;
	head->used += n;
	return p;
}
void tmp_reset(void) {
	if (!head) return;
#ifdef TMP_POISON // debug: stale temps read as garbage instead of quietly still working
	for (chunk *c = head; c; c = c->next) memset(c->data, 0x5a, c->used);
#endif
	for (chunk *c = head->next, *nx; c; c = nx) { nx = c->next; free(c); }
	head->next = NULL; head->used = 0;
}
void sset(str *d, const char *s) { char *n = s ? strdup(s) : NULL; free(*d); *d = n; }

#define Z(s) ((s) ? (s) : "")
static str mk(const char *s, size_t n) {
	str r = tmp_alloc(n + 1);
	memcpy(r, s, n); r[n] = 0;
	return r;
}
static int cl(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

str s_dup(const char *s) { return mk(Z(s), strlen(Z(s))); }
str rt_cat(int n, const char **v) {
	size_t t = 0, l[n];
	for (int i = 0; i < n; i++) t += l[i] = strlen(Z(v[i]));
	str r = tmp_alloc(t + 1), p = r;
	for (int i = 0; i < n; i++) { memcpy(p, Z(v[i]), l[i]); p += l[i]; }
	*p = 0;
	return r;
}
int s_eq(const char *a, const char *b) { return !strcmp(Z(a), Z(b)); }
int s_cmp(const char *a, const char *b) { return strcmp(Z(a), Z(b)); }

int s_len(const char *s) { return strlen(Z(s)); }
str s_left(const char *s, int n) { s = Z(s); return mk(s, cl(n, 0, strlen(s))); }
str s_right(const char *s, int n) { s = Z(s); int l = strlen(s); n = cl(n, 0, l); return mk(s + l - n, n); }
str s_mid(const char *s, int at, int n) {
	s = Z(s); int l = strlen(s);
	at = cl(at, 0, l);
	if (n < 0 || n > l - at) n = l - at;
	return mk(s + at, n);
}
int s_find(const char *s, const char *sub, int from) {
	s = Z(s); int l = strlen(s);
	if (from < 0) from = 0;
	if (from > l) return -1;
	const char *p = strstr(s + from, Z(sub));
	return p ? p - s : -1;
}
int s_has(const char *s, const char *sub) { return strstr(Z(s), Z(sub)) != NULL; }
static str mapc(const char *s, int up) {
	str r = s_dup(s);
	for (char *c = r; *c; c++)
		if (up && *c >= 'a' && *c <= 'z') *c -= 32;
		else if (!up && *c >= 'A' && *c <= 'Z') *c += 32;
	return r;
}
str s_upper(const char *s) { return mapc(s, 1); }
str s_lower(const char *s) { return mapc(s, 0); }
str s_trim(const char *s) {
	s = Z(s);
	while (*s && (unsigned char)*s <= ' ') s++;
	size_t n = strlen(s);
	while (n && (unsigned char)s[n-1] <= ' ') n--;
	return mk(s, n);
}
static str pad(const char *s, int n, int left) {
	s = Z(s); if (n < 0) n = 0;
	int l = strlen(s);
	str r = tmp_alloc(n + 1);
	memset(r, ' ', n); r[n] = 0;
	if (left) memcpy(r, s, l < n ? l : n);
	else if (l >= n) memcpy(r, s + l - n, n);
	else memcpy(r + n - l, s, l);
	return r;
}
str s_lset(const char *s, int n) { return pad(s, n, 1); }
str s_rset(const char *s, int n) { return pad(s, n, 0); }
str s_replace(const char *s, const char *from, const char *to) {
	s = Z(s); from = Z(from); to = Z(to);
	size_t fl = strlen(from), tl = strlen(to), k = 0;
	if (!fl) return s_dup(s);
	for (const char *p = s; (p = strstr(p, from)); p += fl) k++;
	str r = tmp_alloc(strlen(s) + k * tl - k * fl + 1), o = r;
	for (const char *p; (p = strstr(s, from)); s = p + fl) {
		memcpy(o, s, p - s); o += p - s;
		memcpy(o, to, tl); o += tl;
	}
	strcpy(o, s);
	return r;
}
str s_repeat(const char *s, int n) {
	s = Z(s); size_t l = strlen(s); if (n < 0) n = 0;
	str r = tmp_alloc(l * n + 1);
	for (int i = 0; i < n; i++) memcpy(r + i * l, s, l);
	r[l * n] = 0;
	return r;
}
// chr(0) can't live in a c string, so it comes back "". the ini code that used it is rewritten
str s_chr(int c) { char b = c; return mk(&b, (b != 0)); }
int s_asc(const char *s) { return s && *s ? (unsigned char)*s : -1; }
str s_hex(int v) { return sf("%08X", (unsigned)v); }
str s_bin(int v) {
	char b[33];
	for (int i = 0; i < 32; i++) b[i] = (unsigned)v >> (31 - i) & 1 ? '1' : '0';
	return mk(b, 32);
}

str rt_itos(int v) { return sf("%d", v); }
str rt_ftos(float v) { return sf("%g", v); }
int rt_stoi(const char *s) { return atoi(Z(s)); }
float rt_stof(const char *s) { return atof(Z(s)); }
int rt_ftoi(float f) { return (int)lrintf(f); }

str sf(const char *fmt, ...) {
	va_list a, b;
	va_start(a, fmt); va_copy(b, a);
	int n = vsnprintf(NULL, 0, fmt, a);
	str r = tmp_alloc(n + 1);
	vsnprintf(r, n + 1, fmt, b);
	va_end(a); va_end(b);
	return r;
}
