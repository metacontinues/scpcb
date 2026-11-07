// misctest: rng + file io. no window.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/rt/rt.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)

int main(void) {
	// c++ requires minstd_rand (a=48271) seeded w/ 1 to give 399268537 as its 10000th value
	seed_rnd(1);
	for (int i = 0; i < 10000; i++) frnd(0, 1);
	CHECK(rnd_seed() == 399268537, "minstd 10000th = %d", rnd_seed());
	seed_rnd(0); CHECK(rnd_seed() == 1, "seed 0 -> 1");
	seed_rnd(42);
	int lo = 99, hi = -99; float flo = 9, fhi = -9;
	for (int i = 0; i < 100000; i++) {
		int r = irand(-3, 3); if (r < lo) lo = r; if (r > hi) hi = r;
		float f = frnd(0, 1); if (f < flo) flo = f; if (f > fhi) fhi = f;
	}
	CHECK(lo == -3 && hi == 3, "rand range %d..%d", lo, hi);
	CHECK(flo > 0 && fhi < 1, "rnd open range %f..%f", flo, fhi);
	CHECK(irand(5, 1) >= 1, "swapped args");

	char dir[256]; snprintf(dir, sizeof dir, "%s/bbmisc", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
	char p[300]; snprintf(p, sizeof p, "%s\\Test.DAT", dir);
	create_dir(dir);
	CHECK(file_type(dir) == 2, "dir");
	FILE *f = write_file(p);
	CHECK(f != NULL, "write file via backslash path");
	write_int(f, -123456); write_float(f, 3.25f); write_string(f, "hello"); write_byte(f, 200); write_short(f, 0xbeef); write_line(f, "line one");
	close_file(f);
	snprintf(p, sizeof p, "%s/test.dat", dir); // other case
	CHECK(file_type(p) == 1 && file_size(p) == 4 + 4 + 9 + 1 + 2 + 10, "type/size %d", file_size(p));
	f = read_file(p);
	int i = read_int(f); float fl = read_float(f); char *s = read_string(f); int b = read_byte(f), sh = read_short(f); char *l = read_line(f);
	CHECK(i == -123456 && fl == 3.25f && !strcmp(s, "hello") && b == 200 && sh == 0xbeef && !strcmp(l, "line one"), "roundtrip");
	CHECK(file_eof(f), "eof");
	CHECK(!*read_line(f), "readline at eof is empty");
	close_file(f);
	CHECK(delete_file(p) && !file_type(p), "delete");
	CHECK(strlen(current_date()) == 11 && strlen(current_time()) == 8, "date %s time %s", current_date(), current_time());
	// CopyEntity w/ children used to link copies into their parent twice (cycle in the child list)
	ent root = create_pivot(0), a = create_pivot(root);
	create_pivot(a); create_pivot(a);
	ent c = copy_entity(root, 0), c2 = copy_entity(a, root);
	CHECK(count_children(c) == 1 && count_children(get_child(c, 1)) == 2, "copy keeps the tree");
	CHECK(count_children(root) == 2 && count_children(c2) == 2 && get_parent(c2) == root, "copy into a parent: %d", count_children(root));
	scale_entity(c, 2, 2, 2, 0); // walks the copied tree, hung on the cycle
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
