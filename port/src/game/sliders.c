// sliders.c: Menu.bb's SlideBar, Slider3/4/5/7, Button, DrawScrollBar, DrawButton2
#include "g.h"

#define M MenuScale

float SlideBar(int x, int y, int w, float value) {
	int mx = ScaledMouseX(), my = ScaledMouseY();
	if (MouseDown1 && OnSliderID == 0 && mx >= x && mx <= x + w + 14 && my >= y && my <= y + 20)
		value = Min(Max((mx - x) * 100 / w, 0), 100); // int division, like blitz
	bbColor(255, 255, 255);
	bbRect(x, y, w + 14, 20, 0);
	bbDrawImage(BlinkMeterIMG, bbInt(x + w * value / 100.0f + 3), y + 3, 0);
	bbColor(170, 170, 170);
	AAText(bbInt(x - 50 * M), bbInt(y + 4 * M), "LOW", 0, 0);
	AAText(bbInt(x + w + 38 * M), bbInt(y + 4 * M), "HIGH", 0, 0);
	return value;
}

// the 4 SliderN fns in blitz are the same thing w/ n stops: stop k sits at w*k/(n-1) + 10k/(n-1)
// (Slider3 used int width/2 where the others use floats, so it can be off by half a pixel)
int SliderN(int x, int y, int w, int value, int id, int n, const char **vals) {
	int mx = ScaledMouseX(), my = ScaledMouseY();
	int over = mx >= x && mx <= x + w + 14 && my >= y - 8 && my <= y + 10;
	if (MouseDown1 && over) OnSliderID = id;
	bbColor(200, 200, 200);
	bbRect(x, y, w + 14, 10, 1);
	for (int k = 0; k < n; k++) bbRect(bbInt(x + w * (float)k / (n - 1) + 10.0f * k / (n - 1)), y - 8, 4, 14, 1);
	if (id == OnSliderID) {
		if (mx <= x + 8) value = 0;
		else if (mx >= x + w) value = n - 1;
		else for (int k = 1; k < n - 1; k++) {
			float p = x + w * (float)k / (n - 1);
			if (mx >= p && mx <= p + 8) { value = k; break; }
		}
		bbColor(0, 255, 0);
		bbRect(x, y, w + 14, 10, 1);
	} else if (over) { bbColor(0, 200, 0); bbRect(x, y, w + 14, 10, 0); }
	int v = value < 0 ? 0 : value >= n ? n - 1 : value;
	bbDrawImage(BlinkMeterIMG, bbInt(x + w * (float)v / (n - 1) + 6.0f * v / (n - 1)), y - 8, 0);
	bbColor(170, 170, 170);
	AAText(bbInt(x + w * (float)v / (n - 1) + 2 + 10.0f * v / (n - 1)), bbInt(y + 10 + M), vals[v], 1, 0);
	return value;
}

int Button(int x, int y, int w, int h, const char *txt, int disabled) {
	int pushed = 0;
	bbColor(50, 50, 50);
	if (!disabled && bbMouseX() > x && bbMouseX() < x + w && bbMouseY() > y && bbMouseY() < y + h) {
		if (MouseDown1) { pushed = 1; bbColor(30, 30, 30); } else bbColor(60, 60, 60);
	}
	int o = bbInt(1 * M);
	bbRect(x, y, w, h, 1);
	if (pushed) {
		bbColor(133, 130, 125); bbRect(x + o, y + o, w - o, h - o, 0);
		bbColor(10, 10, 10); bbRect(x, y, w, h, 0);
		bbColor(250, 250, 250);
		bbLine(x, y + h - o, x + w - o, y + h - o);
		bbLine(x + w - o, y, x + w - o, y + h - o);
	} else {
		bbColor(133, 130, 125); bbRect(x, y, w - o, h - o, 0);
		bbColor(250, 250, 250); bbRect(x, y, w, h, 0);
		bbColor(10, 10, 10);
		bbLine(x, y + h - 1, x + w - 1, y + h - 1);
		bbLine(x + w - 1, y, x + w - 1, y + h - 1);
	}
	if (disabled) bbColor(70, 70, 70); else bbColor(255, 255, 255);
	bbText(x + w / 2, y + h / 2 - o, txt, 1, 1);
	bbColor(0, 0, 0);
	if (pushed && MouseHit1) { PlaySound_Strict(ButtonSFX); return 1; }
	return 0;
}

float DrawScrollBar(int x, int y, int w, int h, int bx, int by, int bw, int bh, float bar, int dir) {
	int sx = bbMouseXSpeed(), sy = bbMouseYSpeed();
	bbColor(0, 0, 0);
	Button(bx, by, bw, bh, "", 0);
	bbColor(250, 250, 250);
	for (int k = -1; k <= 1; k++) {
		if (dir == 0 && h > 10) bbRect(bbInt(bx + bw / 2 + 3 * k * M), bbInt(by + 5 * M), bbInt(2 * M), bh - 10, 1);
		if (dir != 0 && w > 10) bbRect(bbInt(bx + 4 * M), bbInt(by + bh / 2 + 3 * k * M), bbInt(bw - 10 * M), bbInt(2 * M), 1);
	}
	int in = bbMouseX() > bx && bbMouseX() < bx + bw && bbMouseY() > by && bbMouseY() < by + bh;
	if (in) OnBar = 1; else if (!MouseDown1) OnBar = 0;
	if (MouseDown1 && OnBar) return dir == 0 ? Min(Max(bar + sx / (float)(w - bw), 0), 1) : Min(Max(bar + sy / (float)(h - bh), 0), 1);
	return bar;
}

int DrawButton2(int x, int y, int w, int h, const char *txt, int bigfont) {
	int clicked = 0;
	DrawFrame(x, y, w, h, 0, 0);
	int hit = bbMouseHit(1);
	if (MouseOn(x, y, w, h)) {
		bbColor(30, 30, 30);
		if (hit) { clicked = 1; PlaySound_Strict(ButtonSFX); }
		bbRect(x + 4, y + 4, w - 8, h - 8, 1);
	} else bbColor(0, 0, 0);
	bbColor(255, 255, 255);
	bbSetFont(bigfont ? Font2 : Font1);
	bbText(x + w / 2, y + h / 2, txt, 1, 1);
	return clicked;
}
