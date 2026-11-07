// menu.c: Menu.bb. launcher, loading screens, widgets (main menu itself not yet)
#include "g.h"

TLIST(LoadingScreens);

void UpdateLauncher(void) {
	MenuScale = 1;
	Graphics3DExt(LauncherWidth, LauncherHeight, 0, 2);
	set_buffer(back_buffer());
	RealGraphicWidth = GraphicWidth; RealGraphicHeight = GraphicHeight;
	Font1 = LoadFont_Strict("GFX\\font\\cour\\Courier New.ttf", 18, 0, 0, 0);
	set_font(Font1);
	MenuWhite = LoadImage_Strict("GFX\\menu\\menuwhite.jpg");
	MenuBlack = LoadImage_Strict("GFX\\menu\\menublack.jpg");
	mask_image(MenuBlack, 255, 255, 0);
	LauncherIMG = LoadImage_Strict("GFX\\menu\\launcher.jpg");
	ButtonSFX = LoadSound_Strict("SFX\\Interact\\Button.ogg");
	for (int i = 0; i < 4; i++) {
		ArrowIMG[i] = LoadImage_Strict("GFX\\menu\\arrow.png");
		rotate_image(ArrowIMG[i], 90 * i);
		handle_image(ArrowIMG[i], 0, 0);
	}
	CollectGfxModes();
	BlinkMeterIMG = LoadImage_Strict("GFX\\blinkmeter.jpg");
	// TODO CheckForUpdates (Update.bb talks http over blitz tcp streams)
	app_title("SCP - Containment Breach Launcher");

	for (;;) {
		set_color(0, 0, 0);
		draw_rect(0, 0, LauncherWidth, LauncherHeight, 1);
		MouseHit1 = mouse_hit(1);
		set_color(255, 255, 255);
		draw_image(LauncherIMG, 0, 0, 0);
		draw_text(20, 240 - 65, "Resolution: ", 0, 0);

		int x = 40, y = 270 - 65;
		for (int i = 0; i < GFXModes; i++) {
			set_color(0, 0, 0);
			if (SelectedGFXMode == i) draw_rect(x - 1, y - 1, 100, 20, 0);
			draw_text(x, y, cat(s_str(GfxModeWidths[i]), "x", s_str(GfxModeHeights[i])), 0, 0);
			if (MouseOn(x - 1, y - 1, 100, 20)) {
				set_color(100, 100, 100);
				draw_rect(x - 1, y - 1, 100, 20, 0);
				if (MouseHit1) SelectedGFXMode = i;
			}
			y += 20;
			if (y >= 250 - 65 + (LauncherHeight - 80 - 260)) { y = 270 - 65; x += 100; }
		}

		set_color(255, 255, 255);
		x = 30; y = 369;
		draw_rect(x - 10, y, 340, 95, 1);
		draw_text(x - 10, y - 25, "Graphics:", 0, 0);
		y += 10;
		for (int i = 1; i <= count_gfx_drivers(); i++) {
			set_color(0, 0, 0);
			if (SelectedGFXDriver == i) draw_rect(x - 1, y - 1, 290, 20, 0);
			LimitText(gfx_driver_name(i), x, y, 290, 0);
			if (MouseOn(x - 1, y - 1, 290, 20)) {
				set_color(100, 100, 100);
				draw_rect(x - 1, y - 1, 290, 20, 0);
				if (MouseHit1) SelectedGFXDriver = i;
			}
			y += 20;
		}

		int tx = 40 + 430 - 15, lx = 40 + 430 + 15;
		Fullscreen = DrawTick(tx, 260 - 55 + 5 - 8, Fullscreen, BorderlessWindowed);
		BorderlessWindowed = DrawTick(tx, 260 - 55 + 35, BorderlessWindowed, 0);
		int lock = BorderlessWindowed || !Fullscreen;
		Bit16Mode = DrawTick(tx, 260 - 55 + 65 + 8, Bit16Mode, lock);
		LauncherEnabled = DrawTick(tx, 260 - 55 + 95 + 8, LauncherEnabled, 0);

		if (BorderlessWindowed) { set_color(255, 0, 0); Fullscreen = 0; } else set_color(255, 255, 255);
		draw_text(lx, 262 - 55 + 5 - 8, "Fullscreen", 0, 0);
		set_color(255, 255, 255);
		draw_text(lx, 262 - 55 + 35 - 8, "Borderless", 0, 0);
		draw_text(lx, 262 - 55 + 35 + 12, "windowed mode", 0, 0);
		if (BorderlessWindowed || !Fullscreen) { set_color(255, 0, 0); Bit16Mode = 0; } else set_color(255, 255, 255);
		draw_text(lx, 262 - 55 + 65 + 8, "16 Bit", 0, 0);
		set_color(255, 255, 255);
		draw_text(lx, 262 - 55 + 95 + 8, "Use launcher", 0, 0);

		str res = cat("Current Resolution: ", s_str(GfxModeWidths[SelectedGFXMode]), "x", s_str(GfxModeHeights[SelectedGFXMode]));
		int vw = desktop_width(), vh = desktop_height(); // G_viewport_width/height
		if (!BorderlessWindowed) draw_text(40 + 260 + 15, 262 - 55 + 140, cat(res, ",", Fullscreen ? s_str(16 + 16 * !Bit16Mode) : "32"), 0, 0);
		else {
			draw_text(40 + 260 + 15, 262 - 55 + 140, cat(res, ",32"), 0, 0);
			if (GfxModeWidths[SelectedGFXMode] != vw) {
				draw_text(40 + 260 + 65, 262 - 55 + 160, GfxModeWidths[SelectedGFXMode] < vw ? "(upscaled to" : "(downscaled to", 0, 0);
				draw_text(40 + 260 + 65, 262 - 55 + 180, cat(s_str(vw), "x", s_str(vh), ",32)"), 0, 0);
			}
		}

		UpdateCheckEnabled = DrawTick(LauncherWidth - 275, LauncherHeight - 50, UpdateCheckEnabled, 0);
		set_color(255, 255, 255);
		draw_text(LauncherWidth - 250, LauncherHeight - 70, "Check for", 0, 0);
		draw_text(LauncherWidth - 250, LauncherHeight - 50, "updates on", 0, 0);
		draw_text(LauncherWidth - 250, LauncherHeight - 30, "launch", 0, 0);

		if (DrawButton(LauncherWidth - 30 - 90, LauncherHeight - 50 - 55, 100, 30, "LAUNCH", 0, 0, 0)) {
			GraphicWidth = GfxModeWidths[SelectedGFXMode];
			GraphicHeight = GfxModeHeights[SelectedGFXMode];
			RealGraphicWidth = GraphicWidth; RealGraphicHeight = GraphicHeight;
			break;
		}
		if (DrawButton(LauncherWidth - 30 - 90, LauncherHeight - 50, 100, 30, "EXIT", 0, 0, 0)) End();
		flip_frame(1);
	}

#define TF(v) ((v) ? "true" : "false")
	PutINIValue(OptionFile, "options", "width", s_str(GfxModeWidths[SelectedGFXMode]));
	PutINIValue(OptionFile, "options", "height", s_str(GfxModeHeights[SelectedGFXMode]));
	PutINIValue(OptionFile, "options", "fullscreen", TF(Fullscreen));
	PutINIValue(OptionFile, "launcher", "launcher enabled", TF(LauncherEnabled));
	PutINIValue(OptionFile, "options", "borderless windowed", TF(BorderlessWindowed));
	PutINIValue(OptionFile, "options", "16bit", TF(Bit16Mode));
	PutINIValue(OptionFile, "options", "gfx driver", s_str(SelectedGFXDriver));
	PutINIValue(OptionFile, "options", "check for updates", TF(UpdateCheckEnabled));
}

void DrawTiledImageRect(int img, int sx, int sy, float sw, float sh, int x, int y, int w, int h) {
	for (int x2 = x; x2 < x + w; x2 += iround(sw))
		for (int y2 = y; y2 < y + h; y2 += iround(sh)) {
			if (x2 + sw > x + w) sw -= Max(x2 + sw - (x + w), 1);
			if (y2 + sh > y + h) sh -= Max(y2 + sh - (y + h), 1);
			draw_image_rect(img, x2, y2, sx, sy, iround(sw), iround(sh), 0);
		}
}

static int align(const char *s, int neg, int mid, int pos) {
	str l = s_lower(s);
	return s_eq(l, "left") || s_eq(l, "top") || s_eq(l, "up") ? neg
		: s_eq(l, "middle") || s_eq(l, "center") ? mid
		: s_eq(l, "right") || s_eq(l, "bottom") || s_eq(l, "down") ? pos : 0;
}

void InitLoadingScreens(const char *file) {
	FILE *f = open_file(file);
	while (f && !file_eof(f)) {
		str t = s_trim(read_line(f));
		if (!s_eq(s_left(t, 1), "[")) continue;
		t = s_mid(t, 1, s_len(t) - 2);
		LoadingScreens *ls = lnew(LoadingScreens);
		ls->ID = ++LoadingScreenAmount;
		sset(&ls->title, t);
		sset(&ls->imgpath, GetINIString(file, t, "image path", ""));
		for (int i = 0; i < 5; i++) {
			sset(&ls->txt[i], GetINIString(file, t, cat("text", s_str(i + 1)), ""));
			if (*ls->txt[i]) ls->txtamount++;
		}
		ls->disablebackground = GetINIInt(file, t, "disablebackground", 0);
		// unknown values leave the field alone in blitz; it's 0 then anyway
		ls->alignx = align(GetINIString(file, t, "align x", ""), -1, 0, 1);
		ls->aligny = align(GetINIString(file, t, "align y", ""), -1, 0, 1);
	}
	if (f) close_file(f);
}
