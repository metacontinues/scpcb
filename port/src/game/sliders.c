// sliders.c: Menu.bb's SlideBar, Slider3/4/5/7, Button, DrawScrollBar, DrawButton2
#include "g.h"

#define M MenuScale

float SlideBar(int x, int y, int w, float value) {
	int mx = ScaledMouseX(), my = ScaledMouseY();
	if (MouseDown1 && OnSliderID == 0 && mx >= x && mx <= x + w + 14 && my >= y && my <= y + 20)
		value = Min(Max((mx - x) * 100 / w, 0), 100); // int division, like blitz
	set_color(255, 255, 255);
	draw_rect(x, y, w + 14, 20, 0);
	draw_image(BlinkMeterIMG, iround(x + w * value / 100.0f + 3), y + 3, 0);
	set_color(170, 170, 170);
	AAText(iround(x - 50 * M), iround(y + 4 * M), "LOW", 0, 0);
	AAText(iround(x + w + 38 * M), iround(y + 4 * M), "HIGH", 0, 0);
	return value;
}

// the 4 SliderN fns in blitz are the same thing w/ n stops: stop k sits at w*k/(n-1) + 10k/(n-1)
// (Slider3 used int width/2 where the others use floats, so it can be off by half a pixel)
int SliderN(int x, int y, int w, int value, int id, int n, const char **vals) {
	int mx = ScaledMouseX(), my = ScaledMouseY();
	int over = mx >= x && mx <= x + w + 14 && my >= y - 8 && my <= y + 10;
	if (MouseDown1 && over) OnSliderID = id;
	set_color(200, 200, 200);
	draw_rect(x, y, w + 14, 10, 1);
	for (int k = 0; k < n; k++) draw_rect(iround(x + w * (float)k / (n - 1) + 10.0f * k / (n - 1)), y - 8, 4, 14, 1);
	if (id == OnSliderID) {
		if (mx <= x + 8) value = 0;
		else if (mx >= x + w) value = n - 1;
		else for (int k = 1; k < n - 1; k++) {
			float p = x + w * (float)k / (n - 1);
			if (mx >= p && mx <= p + 8) { value = k; break; }
		}
		set_color(0, 255, 0);
		draw_rect(x, y, w + 14, 10, 1);
	} else if (over) { set_color(0, 200, 0); draw_rect(x, y, w + 14, 10, 0); }
	int v = value < 0 ? 0 : value >= n ? n - 1 : value;
	draw_image(BlinkMeterIMG, iround(x + w * (float)v / (n - 1) + 6.0f * v / (n - 1)), y - 8, 0);
	set_color(170, 170, 170);
	AAText(iround(x + w * (float)v / (n - 1) + 2 + 10.0f * v / (n - 1)), iround(y + 10 + M), vals[v], 1, 0);
	return value;
}

int Button(int x, int y, int w, int h, const char *txt, int disabled) {
	int pushed = 0;
	set_color(50, 50, 50);
	if (!disabled && mouse_x() > x && mouse_x() < x + w && mouse_y() > y && mouse_y() < y + h) {
		if (MouseDown1) { pushed = 1; set_color(30, 30, 30); } else set_color(60, 60, 60);
	}
	int o = iround(1 * M);
	draw_rect(x, y, w, h, 1);
	if (pushed) {
		set_color(133, 130, 125); draw_rect(x + o, y + o, w - o, h - o, 0);
		set_color(10, 10, 10); draw_rect(x, y, w, h, 0);
		set_color(250, 250, 250);
		draw_line(x, y + h - o, x + w - o, y + h - o);
		draw_line(x + w - o, y, x + w - o, y + h - o);
	} else {
		set_color(133, 130, 125); draw_rect(x, y, w - o, h - o, 0);
		set_color(250, 250, 250); draw_rect(x, y, w, h, 0);
		set_color(10, 10, 10);
		draw_line(x, y + h - 1, x + w - 1, y + h - 1);
		draw_line(x + w - 1, y, x + w - 1, y + h - 1);
	}
	if (disabled) set_color(70, 70, 70); else set_color(255, 255, 255);
	draw_text(x + w / 2, y + h / 2 - o, txt, 1, 1);
	set_color(0, 0, 0);
	if (pushed && MouseHit1) { PlaySound_Strict(ButtonSFX); return 1; }
	return 0;
}

float DrawScrollBar(int x, int y, int w, int h, int bx, int by, int bw, int bh, float bar, int dir) {
	int sx = mouse_x_speed(), sy = mouse_y_speed();
	set_color(0, 0, 0);
	Button(bx, by, bw, bh, "", 0);
	set_color(250, 250, 250);
	for (int k = -1; k <= 1; k++) {
		if (dir == 0 && h > 10) draw_rect(iround(bx + bw / 2 + 3 * k * M), iround(by + 5 * M), iround(2 * M), bh - 10, 1);
		if (dir != 0 && w > 10) draw_rect(iround(bx + 4 * M), iround(by + bh / 2 + 3 * k * M), iround(bw - 10 * M), iround(2 * M), 1);
	}
	int in = mouse_x() > bx && mouse_x() < bx + bw && mouse_y() > by && mouse_y() < by + bh;
	if (in) OnBar = 1; else if (!MouseDown1) OnBar = 0;
	if (MouseDown1 && OnBar) return dir == 0 ? Min(Max(bar + sx / (float)(w - bw), 0), 1) : Min(Max(bar + sy / (float)(h - bh), 0), 1);
	return bar;
}

int DrawButton2(int x, int y, int w, int h, const char *txt, int bigfont) {
	int clicked = 0;
	DrawFrame(x, y, w, h, 0, 0);
	int hit = mouse_hit(1);
	if (MouseOn(x, y, w, h)) {
		set_color(30, 30, 30);
		if (hit) { clicked = 1; PlaySound_Strict(ButtonSFX); }
		draw_rect(x + 4, y + 4, w - 8, h - 8, 1);
	} else set_color(0, 0, 0);
	set_color(255, 255, 255);
	set_font(bigfont ? Font2 : Font1);
	draw_text(x + w / 2, y + h / 2, txt, 1, 1);
	return clicked;
}
