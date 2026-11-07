// str.h: strings. NULL reads as "". results (cat, sf, s_*) are temporaries from a frame arena: valid until the next
// tmp_reset (the main loop calls it once a frame), so never keep one. anything kept (fields, globals, tables) is
// an owned heap copy, set w/ sset(&field, value), which frees the old one.
#pragma once
#include <stddef.h>

typedef char *str;
char *tmp_alloc(size_t n);
void tmp_reset(void);
void sset(str *dst, const char *s); // dst = a heap copy of s (NULL stays NULL)

str rt_cat(int n, const char **v);
#define cat(...) rt_cat(sizeof((const char *[]){__VA_ARGS__}) / sizeof(char *), (const char *[]){__VA_ARGS__})
int s_eq(const char *a, const char *b); // case sensitive
int s_cmp(const char *a, const char *b);
str s_dup(const char *s);

int s_len(const char *s);
str s_left(const char *s, int n);
str s_right(const char *s, int n);
str s_mid(const char *s, int at, int n); // at is 0-based, n < 0 = rest
int s_find(const char *s, const char *sub, int from); // 0-based index, -1 if missing
int s_has(const char *s, const char *sub);
str s_upper(const char *s);
str s_lower(const char *s);
str s_trim(const char *s);
str s_lset(const char *s, int n);
str s_rset(const char *s, int n);
str s_replace(const char *s, const char *from, const char *to);
str s_repeat(const char *s, int n);
str s_chr(int c);
int s_asc(const char *s);
str s_hex(int v);
str s_bin(int v);

str rt_itos(int v);
str rt_ftos(float v);
int rt_stoi(const char *s);
float rt_stof(const char *s);
int rt_ftoi(float f); // rounds to nearest
// to string / int (rounded) / float, on whatever x is
#define s_str(x) _Generic((x), char *: s_dup, const char *: s_dup, float: rt_ftos, double: rt_ftos, default: rt_itos)(x)
#define iround(x) _Generic((x), char *: rt_stoi, const char *: rt_stoi, float: rt_ftoi, double: rt_ftoi, default: rt_id)(x)
#define to_float(x) _Generic((x), char *: rt_stof, const char *: rt_stof, default: rt_tof)(x)
static inline int rt_id(int x) { return x; }
static inline float rt_tof(double x) { return (float)x; }

str sf(const char *fmt, ...) __attribute__((format(printf, 1, 2))); // printf into a temp
