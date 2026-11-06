// ini.c: Main.bb's INI-functions. files are cached by lowercased name like the bank version,
// so PutINIValue doesn't show up in GetINI* until UpdateINIFile (same as blitz)
#include "g.h"

typedef struct INIFile { lnode n; str name, buf; int bankOffset, size; } INIFile;
static TLIST(INIFile);

static str ReadINILine(INIFile *f) {
	int o = f->bankOffset, a;
	while (o < f->size && (f->buf[o] == 13 || f->buf[o] == 10)) o++;
	a = o;
	while (o < f->size && f->buf[o] != 13 && f->buf[o] != 10) o++;
	str r = bbMid(f->buf, a + 1, o - a);
	f->bankOffset = o < f->size ? o + 1 : o;
	return r;
}

static INIFile *find(const char *name) {
	str l = bbLower(name);
	each(INIFile, k) if (bbSeq(k->name, l)) return k;
	return NULL;
}

void UpdateINIFile(const char *name) {
	INIFile *f = find(name);
	if (!f) return;
	f->buf = ""; f->size = 0;
	FILE *h = bbReadFile(f->name);
	if (!h) return;
	int n = bbFileSize(f->name);
	f->buf = GC_MALLOC_ATOMIC(n + 1);
	f->size = fread(f->buf, 1, n, h);
	f->buf[f->size] = 0;
	bbCloseFile(h);
}

static str val(const char *line) { return bbTrim(bbRight(line, bbLen(line) - bbInstr(line, "=", 1))); }
static int iskey(const char *line, const char *par) {
	return bbSeq(bbLower(bbTrim(bbLeft(line, Max(bbInstr(line, "=", 1) - 1, 0)))), bbLower(par));
}

str GetINIString(const char *file, const char *section, const char *par, const char *def) {
	INIFile *f = find(file);
	if (!f) { f = bbNew(INIFile); f->name = bbLower(file); UpdateINIFile(f->name); }
	f->bankOffset = 0;
	str sec = bbLower(section);
	while (f->bankOffset < f->size) {
		str s = ReadINILine(f);
		if (!bbSeq(bbLeft(s, 1), "[")) continue;
		s = bbLower(s);
		if (!bbSeq(bbMid(s, 2, bbLen(s) - 2), sec)) continue;
		str t;
		do {
			t = ReadINILine(f);
			if (iskey(t, par)) return val(t);
		} while (!bbSeq(bbLeft(t, 1), "[") && f->bankOffset < f->size);
		return bbSdup(def);
	}
	return bbSdup(def);
}
static int toint(const char *t) {
	str l = bbLower(t);
	return bbSeq(l, "true") ? 1 : bbSeq(l, "false") ? 0 : bbInt(t);
}
int GetINIInt(const char *file, const char *section, const char *par, int def) {
	return toint(GetINIString(file, section, par, bbStr(def)));
}
float GetINIFloat(const char *file, const char *section, const char *par, float def) {
	return bbFloat(GetINIString(file, section, par, bbStr(def)));
}

// the *2 ones read the file directly, by line number from GetINISectionLocation
str GetINIString2(const char *file, int start, const char *par, const char *def) {
	FILE *f = bbReadFile(file);
	if (!f) return bbSdup(def);
	for (int n = 1; !bbEof(f); n++) {
		bbReadLine(f);
		if (n != start) continue;
		str t;
		do {
			t = bbReadLine(f);
			if (iskey(t, par)) { bbCloseFile(f); return val(t); }
		} while (!bbSeq(bbLeft(t, 1), "[") && !bbEof(f));
		break;
	}
	bbCloseFile(f);
	return bbSdup(def);
}
int GetINIInt2(const char *file, int start, const char *par, const char *def) { return toint(GetINIString2(file, start, par, def)); }

int GetINISectionLocation(const char *file, const char *section) {
	FILE *f = bbReadFile(file);
	if (!f) return 0;
	str sec = bbLower(section);
	for (int n = 1; !bbEof(f); n++) {
		str s = bbReadLine(f);
		if (!bbSeq(bbLeft(s, 1), "[")) continue;
		s = bbLower(s);
		int t = bbInstr(s, sec, 1);
		if (t > 0 && (bbSeq(bbMid(s, t - 1, 1), "[") || bbSeq(bbMid(s, t - 1, 1), "|"))) { bbCloseFile(f); return n; }
	}
	bbCloseFile(f);
	return 0;
}

// blitz built the file as a chr(0) separated string first; a line array does the same job
int PutINIValue(const char *file, const char *section, const char *key, const char *value) {
	str sec = cat("[", bbTrim(section), "]"), usec = bbUpper(sec), k = bbTrim(key), v = bbTrim(value);
	int nl = 0, cl = 0; str *ls = NULL;
	FILE *r = bbReadFile(file);
	if (r) {
		while (!bbEof(r)) { if (nl >= cl) ls = GC_REALLOC(ls, sizeof *ls * (cl = cl ? cl * 2 : 64)); ls[nl++] = bbReadLine(r); }
		bbCloseFile(r);
	}
	FILE *w = bbWriteFile(file);
	if (!w) return 0;
	int wrote = 0, found = 0;
	str cur = "";
#define SECT(s) do { if (bbFilePos(w)) bbWriteLine(w, ""); bbWriteLine(w, s); } while (0)
#define KEY() bbWriteLine(w, cat(k, " = ", v))
	for (int i = 0; i < nl; i++) {
		str t = ls[i];
		if (!*t) continue;
		if (bbSeq(bbLeft(t, 1), "[") && bbSeq(bbRight(t, 1), "]")) {
			if (bbSeq(cur, usec) && !wrote) { KEY(); wrote = 1; }
			SECT(t); cur = bbUpper(t);
			if (bbSeq(cur, usec)) found = 1;
		} else if (bbSeq(bbLeft(t, 1), ":")) bbWriteLine(w, t);
		else if (bbInstr(t, "=", 1)) {
			int e = bbInstr(t, "=", 1);
			if (bbSeq(cur, usec) && bbSeq(bbUpper(bbTrim(bbLeft(t, e - 1))), bbUpper(k))) { if (*v) KEY(); wrote = 1; }
			else bbWriteLine(w, t);
		}
	}
	if (!wrote) { if (!found) SECT(sec); KEY(); }
	bbCloseFile(w);
	return 1;
}
