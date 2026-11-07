// ini.c: ini files. files are cached by lowercased name, so PutINIValue doesn't show up in GetINI* until UpdateINIFile
#include "g.h"

typedef struct INIFile { lnode n; str name, buf; int bankOffset, size; } INIFile;
static void inifree(INIFile *f) { free(f->name); free(f->buf); }
static TLIST_DTOR(INIFile, inifree);

static str ReadINILine(INIFile *f) {
	int o = f->bankOffset, a;
	while (o < f->size && (f->buf[o] == 13 || f->buf[o] == 10)) o++;
	a = o;
	while (o < f->size && f->buf[o] != 13 && f->buf[o] != 10) o++;
	str r = s_mid(f->buf, a, o - a);
	f->bankOffset = o < f->size ? o + 1 : o;
	return r;
}

static INIFile *find(const char *name) {
	str l = s_lower(name);
	each(INIFile, k) if (s_eq(k->name, l)) return k;
	return NULL;
}

void UpdateINIFile(const char *name) {
	INIFile *f = find(name);
	if (!f) return;
	free(f->buf); f->buf = NULL; f->size = 0;
	FILE *h = read_file(f->name);
	if (!h) return;
	int n = file_size(f->name);
	f->buf = malloc(n + 1);
	f->size = fread(f->buf, 1, n, h);
	f->buf[f->size] = 0;
	close_file(h);
}

static str val(const char *line) { return s_trim(s_right(line, s_len(line) - (s_find(line, "=", 0) + 1))); }
static int iskey(const char *line, const char *par) {
	return s_eq(s_lower(s_trim(s_left(line, Max(s_find(line, "=", 0), 0)))), s_lower(par));
}

str GetINIString(const char *file, const char *section, const char *par, const char *def) {
	INIFile *f = find(file);
	if (!f) { f = lnew(INIFile); sset(&f->name, s_lower(file)); UpdateINIFile(f->name); }
	f->bankOffset = 0;
	str sec = s_lower(section);
	while (f->bankOffset < f->size) {
		str s = ReadINILine(f);
		if (!s_eq(s_left(s, 1), "[")) continue;
		s = s_lower(s);
		if (!s_eq(s_mid(s, 1, s_len(s) - 2), sec)) continue;
		str t;
		do {
			t = ReadINILine(f);
			if (iskey(t, par)) return val(t);
		} while (!s_eq(s_left(t, 1), "[") && f->bankOffset < f->size);
		return s_dup(def);
	}
	return s_dup(def);
}
static int toint(const char *t) {
	str l = s_lower(t);
	return s_eq(l, "true") ? 1 : s_eq(l, "false") ? 0 : iround(t);
}
int GetINIInt(const char *file, const char *section, const char *par, int def) {
	return toint(GetINIString(file, section, par, s_str(def)));
}
float GetINIFloat(const char *file, const char *section, const char *par, float def) {
	return to_float(GetINIString(file, section, par, s_str(def)));
}

// the *2 ones read the file directly, by line number from GetINISectionLocation
str GetINIString2(const char *file, int start, const char *par, const char *def) {
	FILE *f = read_file(file);
	if (!f) return s_dup(def);
	for (int n = 1; !file_eof(f); n++) {
		read_line(f);
		if (n != start) continue;
		str t;
		do {
			t = read_line(f);
			if (iskey(t, par)) { close_file(f); return val(t); }
		} while (!s_eq(s_left(t, 1), "[") && !file_eof(f));
		break;
	}
	close_file(f);
	return s_dup(def);
}
int GetINIInt2(const char *file, int start, const char *par, const char *def) { return toint(GetINIString2(file, start, par, def)); }

int GetINISectionLocation(const char *file, const char *section) {
	FILE *f = read_file(file);
	if (!f) return 0;
	str sec = s_lower(section);
	for (int n = 1; !file_eof(f); n++) {
		str s = read_line(f);
		if (!s_eq(s_left(s, 1), "[")) continue;
		s = s_lower(s);
		int t = s_find(s, sec, 0);
		if (t > 0 && (s[t - 1] == '[' || s[t - 1] == '|')) { close_file(f); return n; }
	}
	close_file(f);
	return 0;
}

int PutINIValue(const char *file, const char *section, const char *key, const char *value) {
	str sec = cat("[", s_trim(section), "]"), usec = s_upper(sec), k = s_trim(key), v = s_trim(value);
	int nl = 0, cl = 0; str *ls = NULL;
	FILE *r = read_file(file);
	if (r) {
		while (!file_eof(r)) { if (nl >= cl) ls = realloc(ls, sizeof *ls * (cl = cl ? cl * 2 : 64)); ls[nl++] = read_line(r); }
		close_file(r);
	}
	FILE *w = write_file(file);
	if (!w) return 0;
	int wrote = 0, found = 0;
	str cur = "";
#define SECT(s) do { if (file_pos(w)) write_line(w, ""); write_line(w, s); } while (0)
#define KEY() write_line(w, cat(k, " = ", v))
	for (int i = 0; i < nl; i++) {
		str t = ls[i];
		if (!*t) continue;
		if (s_eq(s_left(t, 1), "[") && s_eq(s_right(t, 1), "]")) {
			if (s_eq(cur, usec) && !wrote) { KEY(); wrote = 1; }
			SECT(t); cur = s_upper(t);
			if (s_eq(cur, usec)) found = 1;
		} else if (s_eq(s_left(t, 1), ":")) write_line(w, t);
		else if (s_has(t, "=")) {
			int e = s_find(t, "=", 0);
			if (s_eq(cur, usec) && s_eq(s_upper(s_trim(s_left(t, e))), s_upper(k))) { if (*v) KEY(); wrote = 1; }
			else write_line(w, t);
		}
	}
	if (!wrote) { if (!found) SECT(sec); KEY(); }
	free(ls);
	close_file(w);
	return 1;
}
