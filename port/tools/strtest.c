// strtest: blitz string cmds + Type lists + gc. no window.
#include <stdio.h>
#include <string.h>
#include "../src/bb/bb.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } } while (0)
#define SEQ(a, b) CHECK(bbSeq(a, b), "%s: got '%s' want '%s'", #a, a, b)

typedef struct Thing { lnode n; int v; str name; } Thing;
static TLIST(Thing);

int main(void) {
	SEQ(cat("a", NULL, bbStr(5), "-", bbStr(1.5f)), "a5-1.5");
	SEQ(bbLeft("hello", 2), "he"); SEQ(bbLeft("hi", 9), "hi"); SEQ(bbLeft("hi", -1), "");
	SEQ(bbRight("hello", 3), "llo"); SEQ(bbMid("hello", 2, 3), "ell"); SEQ(bbMid("hello", 4, -1), "lo");
	SEQ(bbMid("hello", 9, 1), "");
	CHECK(bbInstr("a=b=c", "=", 1) == 2 && bbInstr("a=b=c", "=", 3) == 4 && bbInstr("abc", "x", 1) == 0, "instr");
	SEQ(bbTrim("  x y \t\r"), "x y"); SEQ(bbUpper("aBc1"), "ABC1"); SEQ(bbLower("aBc1"), "abc1");
	SEQ(bbReplace("a.b.c", ".", "::"), "a::b::c"); SEQ(bbReplace("aaa", "aa", "b"), "ba");
	SEQ(bbLSet("ab", 4), "ab  "); SEQ(bbRSet("ab", 4), "  ab"); SEQ(bbRSet("abcdef", 3), "def");
	SEQ(bbString("ab", 3), "ababab"); SEQ(bbChr(65), "A"); CHECK(bbAsc("A") == 65 && bbAsc("") == -1, "asc");
	SEQ(bbHex(255), "000000FF"); SEQ(bbHex(-1), "FFFFFFFF");
	CHECK(bbLen(NULL) == 0 && bbSeq(NULL, ""), "null is empty");
	SEQ(bbStr(1.0f), "1.0"); SEQ(bbStr(0.1f), "0.1"); SEQ(bbStr(-2.25f), "-2.25"); SEQ(bbStr(1.0f / 3), "0.333333");
	SEQ(bbStr(123456.7f), "123457.0"); SEQ(bbStr(1e9f), "1.00000e+009"); SEQ(bbStr(-42), "-42");
	CHECK(bbInt("  12abc") == 12 && bbInt(2.5f) == 2 && bbInt(3.5f) == 4 && bbInt(-1.6f) == -2 && bbInt(7) == 7, "int");
	CHECK(bbFloat("1.5") == 1.5f && bbFloat(3) == 3.0f, "float");

	for (int i = 0; i < 5; i++) { Thing *t = bbNew(Thing); t->v = i; t->name = cat("t", bbStr(i)); }
	each(Thing, t) if (t->v % 2) bbDelete(t);
	int s = 0, k = 0; each(Thing, t) { s = s * 10 + t->v; k++; }
	CHECK(s == 24 && k == 3, "each/delete: %d %d", s, k);
	Thing *f = bbFirst(Thing), *l = bbLast(Thing);
	bbInsertBefore(l, f); // 4 0 2
	s = 0; each(Thing, t) s = s * 10 + t->v;
	CHECK(s == 402 && bbFirst(Thing) == l && bbAfter(l) == f && !bbBefore(l), "insert before: %d", s);
	bbInsertAfter(l, bbLast(Thing)); // 0 2 4
	s = 0; each(Thing, t) s = s * 10 + t->v;
	CHECK(s == 24 && bbLast(Thing) == l, "insert after: %d", s);
	SEQ(bbAfter(bbFirst(Thing))->name, "t2");
	int h = bbHandle(l);
	CHECK(h && bbHandle(l) == h && bbObject(Thing, h) == l && !bbObject(Thing, 0) && !bbObject(Thing, 999), "handles");
	bbDelete(l);
	CHECK(!bbObject(Thing, h), "deleted handle is null");
	bbDeleteEach(Thing);
	CHECK(!bbFirst(Thing) && !bbLast(Thing), "delete each");

	k = 0; each(Thing, t) k++;
	CHECK(k == 0, "left %d", k);

	// churn a lot of garbage, heap should stay small
	str keep = "";
	for (int i = 0; i < 2000000; i++) { str t = cat("garbage ", bbStr(i)); if (i % 500000 == 0) keep = cat(keep, t, ";"); }
	GC_gcollect();
	SEQ(keep, "garbage 0;garbage 500000;garbage 1000000;garbage 1500000;");
	CHECK(GC_get_heap_size() < 16 << 20, "heap %zu", GC_get_heap_size());
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
