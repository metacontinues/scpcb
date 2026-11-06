// misctest: rng + file io. no window.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/bb/bb.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); puts(""); } else printf("ok   " #c "\n"); } while (0)

int main(void) {
	// c++ requires minstd_rand (a=48271) seeded w/ 1 to give 399268537 as its 10000th value
	bbSeedRnd(1);
	for (int i = 0; i < 10000; i++) bbRnd(0, 1);
	CHECK(bbRndSeed() == 399268537, "minstd 10000th = %d", bbRndSeed());
	bbSeedRnd(0); CHECK(bbRndSeed() == 1, "seed 0 -> 1");
	bbSeedRnd(42);
	int lo = 99, hi = -99; float flo = 9, fhi = -9;
	for (int i = 0; i < 100000; i++) {
		int r = bbRand(-3, 3); if (r < lo) lo = r; if (r > hi) hi = r;
		float f = bbRnd(0, 1); if (f < flo) flo = f; if (f > fhi) fhi = f;
	}
	CHECK(lo == -3 && hi == 3, "rand range %d..%d", lo, hi);
	CHECK(flo > 0 && fhi < 1, "rnd open range %f..%f", flo, fhi);
	CHECK(bbRand(5, 1) >= 1, "swapped args");

	char dir[256]; snprintf(dir, sizeof dir, "%s/bbmisc", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
	char p[300]; snprintf(p, sizeof p, "%s\\Test.DAT", dir);
	bbCreateDir(dir);
	CHECK(bbFileType(dir) == 2, "dir");
	FILE *f = bbWriteFile(p);
	CHECK(f != NULL, "write file via backslash path");
	bbWriteInt(f, -123456); bbWriteFloat(f, 3.25f); bbWriteString(f, "hello"); bbWriteByte(f, 200); bbWriteShort(f, 0xbeef); bbWriteLine(f, "line one");
	bbCloseFile(f);
	snprintf(p, sizeof p, "%s/test.dat", dir); // other case
	CHECK(bbFileType(p) == 1 && bbFileSize(p) == 4 + 4 + 9 + 1 + 2 + 10, "type/size %d", bbFileSize(p));
	f = bbReadFile(p);
	int i = bbReadInt(f); float fl = bbReadFloat(f); char *s = bbReadString(f); int b = bbReadByte(f), sh = bbReadShort(f); char *l = bbReadLine(f);
	CHECK(i == -123456 && fl == 3.25f && !strcmp(s, "hello") && b == 200 && sh == 0xbeef && !strcmp(l, "line one"), "roundtrip");
	CHECK(bbEof(f), "eof");
	CHECK(!*bbReadLine(f), "readline at eof is empty");
	bbCloseFile(f);
	CHECK(bbDeleteFile(p) && !bbFileType(p), "delete");
	CHECK(strlen(bbCurrentDate()) == 11 && strlen(bbCurrentTime()) == 8, "date %s time %s", bbCurrentDate(), bbCurrentTime());
	// CopyEntity w/ children used to link copies into their parent twice (cycle in the child list)
	ent root = bbCreatePivot(0), a = bbCreatePivot(root);
	bbCreatePivot(a); bbCreatePivot(a);
	ent c = bbCopyEntity(root, 0), c2 = bbCopyEntity(a, root);
	CHECK(bbCountChildren(c) == 1 && bbCountChildren(bbGetChild(c, 1)) == 2, "copy keeps the tree");
	CHECK(bbCountChildren(root) == 2 && bbCountChildren(c2) == 2 && bbGetParent(c2) == root, "copy into a parent: %d", bbCountChildren(root));
	bbScaleEntity(c, 2, 2, 2, 0); // walks the copied tree, hung on the cycle
	printf("%s (%d failed)\n", fails ? "FAILED" : "all good", fails);
	return fails != 0;
}
