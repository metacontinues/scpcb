// save.c: Save.bb (only the save/map lists so far)
#include "g.h"

static str *strs(int n) { return GC_MALLOC(sizeof(str) * (n + 1)); }
static int issave(const char *f) {
	return bbFileType(cat(SavePath, "\\", f)) == 2 && !bbSeq(f, ".") && !bbSeq(f, "..") && bbFileType(cat(SavePath, f, "\\save.txt")) > 0;
}

void LoadSaveGames(void) {
	SaveGameAmount = 0;
	if (bbFileType(SavePath) == 1) RuntimeError(cat("Can't create dir \"", SavePath, "\""));
	if (bbFileType(SavePath) == 0) bbCreateDir(SavePath);
	// blitz walked the dir twice (count, then fill); one pass into a big enough array is the same
	void *d = bbReadDir(SavePath);
	int cap = 16; str *names = strs(cap);
	for (const char *f; *(f = bbNextFile(d));) {
		if (!issave(f)) continue;
		if (SaveGameAmount + 1 >= cap) { str *n = strs(cap *= 2); memcpy(n, names, sizeof(str) * SaveGameAmount); names = n; }
		names[SaveGameAmount++] = bbSdup(f);
	}
	bbCloseDir(d);
	SaveGames = names;
	SaveGameTime = strs(SaveGameAmount + 1); SaveGameDate = strs(SaveGameAmount + 1); SaveGameVersion = strs(SaveGameAmount + 1);
	for (int i = 0; i < SaveGameAmount; i++) {
		FILE *f = bbReadFile(cat(SavePath, SaveGames[i], "\\save.txt"));
		if (!f) continue;
		SaveGameTime[i] = bbReadString(f);
		SaveGameDate[i] = bbReadString(f);
		bbReadInt(f); // skip to the CompatibleVersion number
		for (int j = 0; j < 6; j++) bbReadFloat(f);
		bbReadString(f); bbReadFloat(f); bbReadFloat(f);
		SaveGameVersion[i] = bbReadString(f);
		bbCloseFile(f);
	}
}

void LoadSavedMaps(void) {
	SavedMapsAmount = 0;
	int cap = 16; str *m = strs(cap), *a = strs(cap);
	void *d = bbReadDir("Map Creator\\Maps");
	for (const char *f; *(f = bbNextFile(d));) {
		if (bbFileType(cat(bbCurrentDir(), "Map Creator\\Maps\\", f)) != 1) continue;
		if (!bbSeq(bbRight(f, 6), "cbmap2") && !bbSeq(bbRight(f, 5), "cbmap")) continue;
		if (SavedMapsAmount + 1 >= cap) {
			str *m2 = strs(cap * 2), *a2 = strs(cap * 2);
			memcpy(m2, m, sizeof(str) * cap); memcpy(a2, a, sizeof(str) * cap);
			m = m2; a = a2; cap *= 2;
		}
		int i = SavedMapsAmount++;
		m[i] = bbSdup(f);
		a[i] = "[Unknown]";
		if (bbSeq(bbRight(f, 6), "cbmap2")) {
			FILE *h = bbReadFile(cat("Map Creator\\Maps\\", f));
			if (h) { a[i] = bbReadLine(h); bbCloseFile(h); }
		}
	}
	bbCloseDir(d);
	SavedMaps = m; SavedMapsAuthor = a;
}
