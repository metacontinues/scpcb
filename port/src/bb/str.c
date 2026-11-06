// str.c: blitz string commands on gc'd char*
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

__attribute__((constructor)) static void gcinit(void) { GC_INIT(); }

#define Z(s) ((s) ? (s) : "")
static str mk(const char *s, size_t n) {
	str r = GC_MALLOC_ATOMIC(n + 1);
	memcpy(r, s, n); r[n] = 0;
	return r;
}
static int cl(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

str bbSdup(const char *s) { return mk(Z(s), strlen(Z(s))); }
str bb_cat(int n, const char **v) {
	size_t t = 0, l[n];
	for (int i = 0; i < n; i++) t += l[i] = strlen(Z(v[i]));
	str r = GC_MALLOC_ATOMIC(t + 1), p = r;
	for (int i = 0; i < n; i++) { memcpy(p, Z(v[i]), l[i]); p += l[i]; }
	*p = 0;
	return r;
}
int bbSeq(const char *a, const char *b) { return !strcmp(Z(a), Z(b)); }
int bbScmp(const char *a, const char *b) { return strcmp(Z(a), Z(b)); }

int bbLen(const char *s) { return strlen(Z(s)); }
str bbLeft(const char *s, int n) { s = Z(s); return mk(s, cl(n, 0, strlen(s))); }
str bbRight(const char *s, int n) { s = Z(s); int l = strlen(s); n = cl(n, 0, l); return mk(s + l - n, n); }
str bbMid(const char *s, int at, int n) {
	s = Z(s); int l = strlen(s);
	at = cl(at - 1, 0, l);
	if (n < 0 || n > l - at) n = l - at;
	return mk(s + at, n);
}
int bbInstr(const char *s, const char *sub, int at) {
	s = Z(s); int l = strlen(s);
	if (at < 1) at = 1;
	if (at > l + 1) return 0;
	const char *p = strstr(s + at - 1, Z(sub));
	return p ? p - s + 1 : 0;
}
static str mapc(const char *s, int up) {
	str r = bbSdup(s);
	for (char *c = r; *c; c++)
		if (up && *c >= 'a' && *c <= 'z') *c -= 32;
		else if (!up && *c >= 'A' && *c <= 'Z') *c += 32;
	return r;
}
str bbUpper(const char *s) { return mapc(s, 1); }
str bbLower(const char *s) { return mapc(s, 0); }
str bbTrim(const char *s) {
	s = Z(s);
	while (*s && (unsigned char)*s <= ' ') s++;
	size_t n = strlen(s);
	while (n && (unsigned char)s[n-1] <= ' ') n--;
	return mk(s, n);
}
static str pad(const char *s, int n, int left) {
	s = Z(s); if (n < 0) n = 0;
	int l = strlen(s);
	str r = GC_MALLOC_ATOMIC(n + 1);
	memset(r, ' ', n); r[n] = 0;
	if (left) memcpy(r, s, l < n ? l : n);
	else if (l >= n) memcpy(r, s + l - n, n);
	else memcpy(r + n - l, s, l);
	return r;
}
str bbLSet(const char *s, int n) { return pad(s, n, 1); }
str bbRSet(const char *s, int n) { return pad(s, n, 0); }
str bbReplace(const char *s, const char *from, const char *to) {
	s = Z(s); from = Z(from); to = Z(to);
	size_t fl = strlen(from), tl = strlen(to), k = 0;
	if (!fl) return bbSdup(s);
	for (const char *p = s; (p = strstr(p, from)); p += fl) k++;
	str r = GC_MALLOC_ATOMIC(strlen(s) + k * tl - k * fl + 1), o = r;
	for (const char *p; (p = strstr(s, from)); s = p + fl) {
		memcpy(o, s, p - s); o += p - s;
		memcpy(o, to, tl); o += tl;
	}
	strcpy(o, s);
	return r;
}
str bbString(const char *s, int n) {
	s = Z(s); size_t l = strlen(s); if (n < 0) n = 0;
	str r = GC_MALLOC_ATOMIC(l * n + 1);
	for (int i = 0; i < n; i++) memcpy(r + i * l, s, l);
	r[l * n] = 0;
	return r;
}
// chr(0) can't live in a c string, so it comes back "". the ini code that used it is rewritten
str bbChr(int c) { char b = c; return mk(&b, (b != 0)); }
int bbAsc(const char *s) { return s && *s ? (unsigned char)*s : -1; }
str bbHex(int v) { return sf("%08X", (unsigned)v); }
str bbBin(int v) {
	char b[33];
	for (int i = 0; i < 32; i++) b[i] = (unsigned)v >> (31 - i) & 1 ? '1' : '0';
	return mk(b, 32);
}

str bb_itos(int v) { return sf("%d", v); }
// blitz's ftoa: 6 significant digits, fixed in [1e-4, 1e8) w/ at least one decimal, else
// msvc style 3-digit exponent. from memory of blitz3d's source, unverified
str bb_ftos(float v) {
	if (isnan(v)) return bbSdup("NaN");
	if (isinf(v)) return bbSdup(v > 0 ? "Infinity" : "-Infinity");
	if (v == 0) return bbSdup("0.0");
	int e = (int)floor(log10(fabs(v)));
	char b[64];
	if (e < -4 || e >= 8) {
		snprintf(b, sizeof b, "%.5e", v);
		char *x = strchr(b, 'e'), sg = x[1];
		int ev = atoi(x + 2);
		snprintf(x, b + sizeof b - x, "e%c%03d", sg, ev);
		return bbSdup(b);
	}
	double d = v;
	if (e > 5) { double p = pow(10, e - 5); d = round(d / p) * p; }
	snprintf(b, sizeof b, "%.*f", e < 5 ? 5 - e : 0, d);
	if (!strchr(b, '.')) strcat(b, ".0");
	char *dp = strchr(b, '.'), *t = b + strlen(b) - 1;
	while (t > dp + 1 && *t == '0') *t-- = 0;
	return bbSdup(b);
}
int bb_stoi(const char *s) { return atoi(Z(s)); }
float bb_stof(const char *s) { return atof(Z(s)); }
int bb_ftoi(float f) { return (int)lrintf(f); }

str sf(const char *fmt, ...) {
	va_list a, b;
	va_start(a, fmt); va_copy(b, a);
	int n = vsnprintf(NULL, 0, fmt, a);
	str r = GC_MALLOC_ATOMIC(n + 1);
	vsnprintf(r, n + 1, fmt, b);
	va_end(a); va_end(b);
	return r;
}
