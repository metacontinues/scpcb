// widgets.c: Menu.bb's ui bits (frames, buttons, ticks, input boxes, wrapped text)
#include "g.h"

str rInput(const char *s) {
	int v = get_key(), l = s_len(s);
	if (v == 8) { v = 0; if (l > 0) s = s_left(s, l - 1); }
	if (v == 13 || v == 0 || (v > 0 && v < 7) || (v > 26 && v < 32) || v == 9) return s_dup(s);
	return cat(s, s_chr(v));
}

str InputBox(int x, int y, int w, int h, const char *txt, int id) {
	set_color(255, 255, 255);
	DrawTiledImageRect(MenuWhite, x % 256, y % 256, 512, 512, x, y, w, h);
	set_color(0, 0, 0);
	int on = MouseOn(x, y, w, h);
	if (on) {
		set_color(50, 50, 50);
		if (MouseHit1) { SelectedInputBox = id; flush_keys(); }
	}
	draw_rect(x + 2, y + 2, w - 4, h - 4, 1);
	set_color(255, 255, 255);
	if (!on && MouseHit1 && SelectedInputBox == id) SelectedInputBox = 0;
	str t = s_dup(txt);
	if (SelectedInputBox == id) {
		t = rInput(t);
		if (MilliSecs2() % 800 < 400) draw_rect(x + w / 2 + AAStringWidth(t) / 2 + 2, y + h / 2 - 5, 2, 12, 1);
	}
	AAText(x + w / 2, y + h / 2, t, 1, 1);
	return t;
}

void DrawFrame(int x, int y, int w, int h, int xo, int yo) {
	set_color(255, 255, 255);
	DrawTiledImageRect(MenuWhite, xo, y % 256, 512, 512, x, y, w, h);
	DrawTiledImageRect(MenuBlack, yo, y % 256, 512, 512, iround(x + 3 * MenuScale), iround(y + 3 * MenuScale), iround(w - 6 * MenuScale), iround(h - 6 * MenuScale));
}

int DrawButton(int x, int y, int w, int h, const char *txt, int bigfont, int waitForMouseUp, int usingAA) {
	int clicked = 0;
	DrawFrame(x, y, w, h, 0, 0);
	if (MouseOn(x, y, w, h)) {
		set_color(30, 30, 30);
		if ((MouseHit1 && !waitForMouseUp) || (MouseUp1 && waitForMouseUp)) { clicked = 1; PlaySound_Strict(ButtonSFX); }
		draw_rect(x + 4, y + 4, w - 8, h - 8, 1);
	} else set_color(0, 0, 0);
	set_color(255, 255, 255);
	if (usingAA) { AASetFont(bigfont ? Font2 : Font1); AAText(x + w / 2, y + h / 2, txt, 1, 1); }
	else { set_font(bigfont ? Font2 : Font1); draw_text(x + w / 2, y + h / 2, txt, 1, 1); }
	return clicked;
}

int DrawTick(int x, int y, int selected, int locked) {
	int w = iround(20 * MenuScale), h = iround(20 * MenuScale);
	set_color(255, 255, 255);
	DrawTiledImageRect(MenuWhite, x % 256, y % 256, 512, 512, x, y, w, h);
	int hl = MouseOn(x, y, w, h) && !locked;
	if (hl) {
		set_color(50, 50, 50);
		if (MouseHit1) { selected = !selected; PlaySound_Strict(ButtonSFX); }
	} else set_color(0, 0, 0);
	draw_rect(x + 2, y + 2, w - 4, h - 4, 1);
	if (selected) {
		if (hl) set_color(255, 255, 255); else set_color(200, 200, 200);
		DrawTiledImageRect(MenuWhite, x % 256, y % 256, 512, 512, x + 4, y + 4, w - 8, h - 8);
	}
	set_color(255, 255, 255);
	return selected;
}

// word wrap into w x h (h < 1 = 2048), align centers each line
void RowText(const char *a, int x, int y, int w, int h, int align, float leading) {
	if (h < 1) h = 2048;
	int shown = 0, lh = iround(AAStringHeight(a) + leading);
	str A = s_dup(a), b = "";
#define LINE() AAText(align ? x + w / 2 - AAStringWidth(b) / 2 : x, shown * lh + y, b, 0, 0)
	while (s_len(A) > 0) {
		int sp = s_instr(A, " ", 1);
		if (!sp) sp = s_len(A);
		str t = s_left(A, sp), tr = s_trim(t);
		int extra = 0;
		if (AAStringWidth(cat(b, t)) > w && AAStringWidth(cat(b, tr)) <= w) { t = tr; extra = 1; }
		if (AAStringWidth(cat(b, t)) > w) { LINE(); shown++; b = ""; }
		else { b = cat(b, t); A = s_right(A, s_len(A) - (s_len(t) + extra)); }
		if ((shown + 1) * lh > h) break;
	}
	if (*b && shown + 1 <= h) LINE();
#undef LINE
}

int LimitText(const char *txt, int x, int y, int w, int usingAA) {
	if (!txt || !*txt || !w) return 0;
	int tl = usingAA ? AAStringWidth(txt) : string_width(txt), un = tl - w;
	str t = s_dup(txt);
	if (un > 0) t = cat(s_left(txt, Max(s_len(txt) - un / (tl / s_len(txt) ? tl / s_len(txt) : 1) - 4, 1)), "...");
	if (usingAA) AAText(x, y, t, 0, 0); else draw_text(x, y, t, 0, 0);
	return 0;
}
