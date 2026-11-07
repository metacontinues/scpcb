// save.c: Save.bb (only the save/map lists so far)
#include "g.h"

static str *strs(int n) { return GC_MALLOC(sizeof(str) * (n + 1)); }
static int issave(const char *f) {
	return file_type(cat(SavePath, "\\", f)) == 2 && !s_eq(f, ".") && !s_eq(f, "..") && file_type(cat(SavePath, f, "\\save.txt")) > 0;
}

void LoadSaveGames(void) {
	SaveGameAmount = 0;
	if (file_type(SavePath) == 1) RuntimeError(cat("Can't create dir \"", SavePath, "\""));
	if (file_type(SavePath) == 0) create_dir(SavePath);
	// blitz walked the dir twice (count, then fill); one pass into a big enough array is the same
	void *d = read_dir(SavePath);
	int cap = 16; str *names = strs(cap);
	for (const char *f; *(f = next_file(d));) {
		if (!issave(f)) continue;
		if (SaveGameAmount + 1 >= cap) { str *n = strs(cap *= 2); memcpy(n, names, sizeof(str) * SaveGameAmount); names = n; }
		names[SaveGameAmount++] = s_dup(f);
	}
	close_dir(d);
	SaveGames = names;
	SaveGameTime = strs(SaveGameAmount + 1); SaveGameDate = strs(SaveGameAmount + 1); SaveGameVersion = strs(SaveGameAmount + 1);
	for (int i = 0; i < SaveGameAmount; i++) {
		FILE *f = read_file(cat(SavePath, SaveGames[i], "\\save.txt"));
		if (!f) continue;
		SaveGameTime[i] = read_string(f);
		SaveGameDate[i] = read_string(f);
		read_int(f); // skip to the CompatibleVersion number
		for (int j = 0; j < 6; j++) read_float(f);
		read_string(f); read_float(f); read_float(f);
		SaveGameVersion[i] = read_string(f);
		close_file(f);
	}
}

void LoadSavedMaps(void) {
	SavedMapsAmount = 0;
	int cap = 16; str *m = strs(cap), *a = strs(cap);
	void *d = read_dir("Map Creator\\Maps");
	for (const char *f; *(f = next_file(d));) {
		if (file_type(cat(current_dir(), "Map Creator\\Maps\\", f)) != 1) continue;
		if (!s_eq(s_right(f, 6), "cbmap2") && !s_eq(s_right(f, 5), "cbmap")) continue;
		if (SavedMapsAmount + 1 >= cap) {
			str *m2 = strs(cap * 2), *a2 = strs(cap * 2);
			memcpy(m2, m, sizeof(str) * cap); memcpy(a2, a, sizeof(str) * cap);
			m = m2; a = a2; cap *= 2;
		}
		int i = SavedMapsAmount++;
		m[i] = s_dup(f);
		a[i] = "[Unknown]";
		if (s_eq(s_right(f, 6), "cbmap2")) {
			FILE *h = read_file(cat("Map Creator\\Maps\\", f));
			if (h) { a[i] = read_line(h); close_file(h); }
		}
	}
	close_dir(d);
	SavedMaps = m; SavedMapsAuthor = a;
}
