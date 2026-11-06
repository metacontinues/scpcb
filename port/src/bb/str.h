// str.h: blitz strings. gc'd (bdwgc), immutable by convention, NULL reads as "".
// literals pass straight in; results are fresh gc strings, never free them.
// gc ptrs must live in globals, the stack, or gc memory (bbNew/GC_MALLOC), not in malloc'd memory.
#pragma once
#include <gc.h>

typedef char *str;

str bb_cat(int n, const char **v);
#define cat(...) bb_cat(sizeof((const char *[]){__VA_ARGS__}) / sizeof(char *), (const char *[]){__VA_ARGS__})
int bbSeq(const char *a, const char *b); // blitz a = b (case sensitive)
int bbScmp(const char *a, const char *b);
str bbSdup(const char *s);

int bbLen(const char *s);
str bbLeft(const char *s, int n);
str bbRight(const char *s, int n);
str bbMid(const char *s, int at, int n); // n < 0 = rest
int bbInstr(const char *s, const char *sub, int at);
str bbUpper(const char *s);
str bbLower(const char *s);
str bbTrim(const char *s);
str bbLSet(const char *s, int n);
str bbRSet(const char *s, int n);
str bbReplace(const char *s, const char *from, const char *to);
str bbString(const char *s, int n);
str bbChr(int c);
int bbAsc(const char *s);
str bbHex(int v);
str bbBin(int v);

str bb_itos(int v);
str bb_ftos(float v);
int bb_stoi(const char *s);
float bb_stof(const char *s);
int bb_ftoi(float f); // rounds to nearest like blitz's fistp, also what `i% = f#` does
// Str(x), Int(x), Float(x) on whatever x is
#define bbStr(x) _Generic((x), char *: bbSdup, const char *: bbSdup, float: bb_ftos, double: bb_ftos, default: bb_itos)(x)
#define bbInt(x) _Generic((x), char *: bb_stoi, const char *: bb_stoi, float: bb_ftoi, double: bb_ftoi, default: bb_id)(x)
#define bbFloat(x) _Generic((x), char *: bb_stof, const char *: bb_stof, default: bb_tof)(x)
static inline int bb_id(int x) { return x; }
static inline float bb_tof(double x) { return (float)x; }

str sf(const char *fmt, ...) __attribute__((format(printf, 1, 2))); // printf into a gc string
