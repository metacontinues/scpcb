// str.h: blitz strings. gc'd (bdwgc), immutable by convention, NULL reads as "".
// literals pass straight in; results are fresh gc strings, never free them.
// gc ptrs must live in globals, the stack, or gc memory (lnew/GC_MALLOC), not in malloc'd memory.
#pragma once
#include <gc.h>

typedef char *str;

str rt_cat(int n, const char **v);
#define cat(...) rt_cat(sizeof((const char *[]){__VA_ARGS__}) / sizeof(char *), (const char *[]){__VA_ARGS__})
int s_eq(const char *a, const char *b); // blitz a = b (case sensitive)
int s_cmp(const char *a, const char *b);
str s_dup(const char *s);

int s_len(const char *s);
str s_left(const char *s, int n);
str s_right(const char *s, int n);
str s_mid(const char *s, int at, int n); // n < 0 = rest
int s_instr(const char *s, const char *sub, int at);
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
int rt_ftoi(float f); // rounds to nearest like blitz's fistp, also what `i% = f#` does
// Str(x), Int(x), Float(x) on whatever x is
#define s_str(x) _Generic((x), char *: s_dup, const char *: s_dup, float: rt_ftos, double: rt_ftos, default: rt_itos)(x)
#define iround(x) _Generic((x), char *: rt_stoi, const char *: rt_stoi, float: rt_ftoi, double: rt_ftoi, default: rt_id)(x)
#define to_float(x) _Generic((x), char *: rt_stof, const char *: rt_stof, default: rt_tof)(x)
static inline int rt_id(int x) { return x; }
static inline float rt_tof(double x) { return (float)x; }

str sf(const char *fmt, ...) __attribute__((format(printf, 1, 2))); // printf into a gc string
