// strtest: blitz string cmds + Type lists + gc. no window.
#include <stdio.h>
#include <string.h>
#include "../src/rt/rt.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } } while (0)
#define SEQ(a, b) CHECK(s_eq(a, b), "%s: got '%s' want '%s'", #a, a, b)

typedef struct Thing { lnode n; int v; str name; } Thing;
static TLIST(Thing);

int main(void) {
	SEQ(cat("a", NULL, s_str(5), "-", s_str(1.5f)), "a5-1.5");
	SEQ(s_left("hello", 2), "he"); SEQ(s_left("hi", 9), "hi"); SEQ(s_left("hi", -1), "");
	SEQ(s_right("hello", 3), "llo"); SEQ(s_mid("hello", 2, 3), "ell"); SEQ(s_mid("hello", 4, -1), "lo");
	SEQ(s_mid("hello", 9, 1), "");
	CHECK(s_instr("a=b=c", "=", 1) == 2 && s_instr("a=b=c", "=", 3) == 4 && s_instr("abc", "x", 1) == 0, "instr");
	SEQ(s_trim("  x y \t\r"), "x y"); SEQ(s_upper("aBc1"), "ABC1"); SEQ(s_lower("aBc1"), "abc1");
	SEQ(s_replace("a.b.c", ".", "::"), "a::b::c"); SEQ(s_replace("aaa", "aa", "b"), "ba");
	SEQ(s_lset("ab", 4), "ab  "); SEQ(s_rset("ab", 4), "  ab"); SEQ(s_rset("abcdef", 3), "def");
	SEQ(s_repeat("ab", 3), "ababab"); SEQ(s_chr(65), "A"); CHECK(s_asc("A") == 65 && s_asc("") == -1, "asc");
	SEQ(s_hex(255), "000000FF"); SEQ(s_hex(-1), "FFFFFFFF");
	CHECK(s_len(NULL) == 0 && s_eq(NULL, ""), "null is empty");
	SEQ(s_str(1.0f), "1.0"); SEQ(s_str(0.1f), "0.1"); SEQ(s_str(-2.25f), "-2.25"); SEQ(s_str(1.0f / 3), "0.333333");
	SEQ(s_str(123456.7f), "123457.0"); SEQ(s_str(1e9f), "1.00000e+009"); SEQ(s_str(-42), "-42");
	CHECK(iround("  12abc") == 12 && iround(2.5f) == 2 && iround(3.5f) == 4 && iround(-1.6f) == -2 && iround(7) == 7, "int");
	CHECK(to_float("1.5") == 1.5f && to_float(3) == 3.0f, "float");

	for (int i = 0; i < 5; i++) { Thing *t = lnew(Thing); t->v = i; t->name = cat("t", s_str(i)); }
	each(Thing, t) if (t->v % 2) ldel(t);
	int s = 0, k = 0; each(Thing, t) { s = s * 10 + t->v; k++; }
	CHECK(s == 24 && k == 3, "each/delete: %d %d", s, k);
	Thing *f = lfirst(Thing), *l = llast(Thing);
	linsert_before(l, f); // 4 0 2
	s = 0; each(Thing, t) s = s * 10 + t->v;
	CHECK(s == 402 && lfirst(Thing) == l && lafter(l) == f && !lbefore(l), "insert before: %d", s);
	linsert_after(l, llast(Thing)); // 0 2 4
	s = 0; each(Thing, t) s = s * 10 + t->v;
	CHECK(s == 24 && llast(Thing) == l, "insert after: %d", s);
	SEQ(lafter(lfirst(Thing))->name, "t2");
	int h = lhandle(l);
	CHECK(h && lhandle(l) == h && lobject(Thing, h) == l && !lobject(Thing, 0) && !lobject(Thing, 999), "handles");
	ldel(l);
	CHECK(!lobject(Thing, h), "deleted handle is null");
	ldel_each(Thing);
	CHECK(!lfirst(Thing) && !llast(Thing), "delete each");

	k = 0; each(Thing, t) k++;
	CHECK(k == 0, "left %d", k);

	// churn a lot of garbage, heap should stay small
	str keep = "";
	for (int i = 0; i < 2000000; i++) { str t = cat("garbage ", s_str(i)); if (i % 500000 == 0) keep = cat(keep, t, ";"); }
	GC_gcollect();
	SEQ(keep, "garbage 0;garbage 500000;garbage 1000000;garbage 1500000;");
	CHECK(GC_get_heap_size() < 16 << 20, "heap %zu", GC_get_heap_size());
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
