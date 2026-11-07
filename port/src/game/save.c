// save.c: Save.bb (only the save/map lists so far)
#include "g.h"

// owned string lists: grow() keeps the new slots NULL, drop() frees the strings + the list
static str *grow(str *l, int old, int cap) { l = realloc(l, sizeof(str) * cap); memset(l + old, 0, sizeof(str) * (cap - old)); return l; }
static void drop(str **l, int n) { if (!*l) return; for (int i = 0; i < n; i++) free((*l)[i]); free(*l); *l = NULL; }
static int issave(const char *f) {
	return file_type(cat(SavePath, "\\", f)) == 2 && !s_eq(f, ".") && !s_eq(f, "..") && file_type(cat(SavePath, f, "\\save.txt")) > 0;
}

void LoadSaveGames(void) {
	drop(&SaveGames, SaveGameAmount); drop(&SaveGameTime, SaveGameAmount); drop(&SaveGameDate, SaveGameAmount); drop(&SaveGameVersion, SaveGameAmount);
	SaveGameAmount = 0;
	if (file_type(SavePath) == 1) RuntimeError(cat("Can't create dir \"", SavePath, "\""));
	if (file_type(SavePath) == 0) create_dir(SavePath);
	void *d = read_dir(SavePath);
	int cap = 16; str *names = grow(NULL, 0, cap);
	for (const char *f; *(f = next_file(d));) {
		if (!issave(f)) continue;
		if (SaveGameAmount + 1 >= cap) { names = grow(names, cap, cap * 2); cap *= 2; }
		sset(&names[SaveGameAmount++], f);
	}
	close_dir(d);
	SaveGames = names;
	SaveGameTime = grow(NULL, 0, SaveGameAmount + 1); SaveGameDate = grow(NULL, 0, SaveGameAmount + 1); SaveGameVersion = grow(NULL, 0, SaveGameAmount + 1);
	for (int i = 0; i < SaveGameAmount; i++) {
		FILE *f = read_file(cat(SavePath, SaveGames[i], "\\save.txt"));
		if (!f) continue;
		sset(&SaveGameTime[i], read_string(f));
		sset(&SaveGameDate[i], read_string(f));
		read_int(f); // skip to the CompatibleVersion number
		for (int j = 0; j < 6; j++) read_float(f);
		read_string(f); read_float(f); read_float(f);
		sset(&SaveGameVersion[i], read_string(f));
		close_file(f);
	}
}

void LoadSavedMaps(void) {
	drop(&SavedMaps, SavedMapsAmount); drop(&SavedMapsAuthor, SavedMapsAmount);
	SavedMapsAmount = 0;
	int cap = 16; str *m = grow(NULL, 0, cap), *a = grow(NULL, 0, cap);
	void *d = read_dir("Map Creator\\Maps");
	for (const char *f; *(f = next_file(d));) {
		if (file_type(cat(current_dir(), "Map Creator\\Maps\\", f)) != 1) continue;
		if (!s_eq(s_right(f, 6), "cbmap2") && !s_eq(s_right(f, 5), "cbmap")) continue;
		if (SavedMapsAmount + 1 >= cap) { m = grow(m, cap, cap * 2); a = grow(a, cap, cap * 2); cap *= 2; }
		int i = SavedMapsAmount++;
		sset(&m[i], f);
		sset(&a[i], "[Unknown]");
		if (s_eq(s_right(f, 6), "cbmap2")) {
			FILE *h = read_file(cat("Map Creator\\Maps\\", f));
			if (h) { sset(&a[i], read_line(h)); close_file(h); }
		}
	}
	close_dir(d);
	SavedMaps = m; SavedMapsAuthor = a;
}
