// menu.c: Menu.bb. launcher, loading screens, widgets (main menu itself not yet)
#include "g.h"

TLIST(LoadingScreens);

void UpdateLauncher(void) {
	MenuScale = 1;
	Graphics3DExt(LauncherWidth, LauncherHeight, 0, 2);
	bbSetBuffer(bbBackBuffer());
	RealGraphicWidth = GraphicWidth; RealGraphicHeight = GraphicHeight;
	Font1 = LoadFont_Strict("GFX\\font\\cour\\Courier New.ttf", 18, 0, 0, 0);
	bbSetFont(Font1);
	MenuWhite = LoadImage_Strict("GFX\\menu\\menuwhite.jpg");
	MenuBlack = LoadImage_Strict("GFX\\menu\\menublack.jpg");
	bbMaskImage(MenuBlack, 255, 255, 0);
	LauncherIMG = LoadImage_Strict("GFX\\menu\\launcher.jpg");
	ButtonSFX = LoadSound_Strict("SFX\\Interact\\Button.ogg");
	for (int i = 0; i < 4; i++) {
		ArrowIMG[i] = LoadImage_Strict("GFX\\menu\\arrow.png");
		bbRotateImage(ArrowIMG[i], 90 * i);
		bbHandleImage(ArrowIMG[i], 0, 0);
	}
	CollectGfxModes();
	BlinkMeterIMG = LoadImage_Strict("GFX\\blinkmeter.jpg");
	// TODO CheckForUpdates (Update.bb talks http over blitz tcp streams)
	bbAppTitle("SCP - Containment Breach Launcher");

	for (;;) {
		bbColor(0, 0, 0);
		bbRect(0, 0, LauncherWidth, LauncherHeight, 1);
		MouseHit1 = bbMouseHit(1);
		bbColor(255, 255, 255);
		bbDrawImage(LauncherIMG, 0, 0, 0);
		bbText(20, 240 - 65, "Resolution: ", 0, 0);

		int x = 40, y = 270 - 65;
		for (int i = 0; i < GFXModes; i++) {
			bbColor(0, 0, 0);
			if (SelectedGFXMode == i) bbRect(x - 1, y - 1, 100, 20, 0);
			bbText(x, y, cat(bbStr(GfxModeWidths[i]), "x", bbStr(GfxModeHeights[i])), 0, 0);
			if (MouseOn(x - 1, y - 1, 100, 20)) {
				bbColor(100, 100, 100);
				bbRect(x - 1, y - 1, 100, 20, 0);
				if (MouseHit1) SelectedGFXMode = i;
			}
			y += 20;
			if (y >= 250 - 65 + (LauncherHeight - 80 - 260)) { y = 270 - 65; x += 100; }
		}

		bbColor(255, 255, 255);
		x = 30; y = 369;
		bbRect(x - 10, y, 340, 95, 1);
		bbText(x - 10, y - 25, "Graphics:", 0, 0);
		y += 10;
		for (int i = 1; i <= bbCountGfxDrivers(); i++) {
			bbColor(0, 0, 0);
			if (SelectedGFXDriver == i) bbRect(x - 1, y - 1, 290, 20, 0);
			LimitText(bbGfxDriverName(i), x, y, 290, 0);
			if (MouseOn(x - 1, y - 1, 290, 20)) {
				bbColor(100, 100, 100);
				bbRect(x - 1, y - 1, 290, 20, 0);
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

		if (BorderlessWindowed) { bbColor(255, 0, 0); Fullscreen = 0; } else bbColor(255, 255, 255);
		bbText(lx, 262 - 55 + 5 - 8, "Fullscreen", 0, 0);
		bbColor(255, 255, 255);
		bbText(lx, 262 - 55 + 35 - 8, "Borderless", 0, 0);
		bbText(lx, 262 - 55 + 35 + 12, "windowed mode", 0, 0);
		if (BorderlessWindowed || !Fullscreen) { bbColor(255, 0, 0); Bit16Mode = 0; } else bbColor(255, 255, 255);
		bbText(lx, 262 - 55 + 65 + 8, "16 Bit", 0, 0);
		bbColor(255, 255, 255);
		bbText(lx, 262 - 55 + 95 + 8, "Use launcher", 0, 0);

		str res = cat("Current Resolution: ", bbStr(GfxModeWidths[SelectedGFXMode]), "x", bbStr(GfxModeHeights[SelectedGFXMode]));
		int vw = bbDesktopWidth(), vh = bbDesktopHeight(); // G_viewport_width/height
		if (!BorderlessWindowed) bbText(40 + 260 + 15, 262 - 55 + 140, cat(res, ",", Fullscreen ? bbStr(16 + 16 * !Bit16Mode) : "32"), 0, 0);
		else {
			bbText(40 + 260 + 15, 262 - 55 + 140, cat(res, ",32"), 0, 0);
			if (GfxModeWidths[SelectedGFXMode] != vw) {
				bbText(40 + 260 + 65, 262 - 55 + 160, GfxModeWidths[SelectedGFXMode] < vw ? "(upscaled to" : "(downscaled to", 0, 0);
				bbText(40 + 260 + 65, 262 - 55 + 180, cat(bbStr(vw), "x", bbStr(vh), ",32)"), 0, 0);
			}
		}

		UpdateCheckEnabled = DrawTick(LauncherWidth - 275, LauncherHeight - 50, UpdateCheckEnabled, 0);
		bbColor(255, 255, 255);
		bbText(LauncherWidth - 250, LauncherHeight - 70, "Check for", 0, 0);
		bbText(LauncherWidth - 250, LauncherHeight - 50, "updates on", 0, 0);
		bbText(LauncherWidth - 250, LauncherHeight - 30, "launch", 0, 0);

		if (DrawButton(LauncherWidth - 30 - 90, LauncherHeight - 50 - 55, 100, 30, "LAUNCH", 0, 0, 0)) {
			GraphicWidth = GfxModeWidths[SelectedGFXMode];
			GraphicHeight = GfxModeHeights[SelectedGFXMode];
			RealGraphicWidth = GraphicWidth; RealGraphicHeight = GraphicHeight;
			break;
		}
		if (DrawButton(LauncherWidth - 30 - 90, LauncherHeight - 50, 100, 30, "EXIT", 0, 0, 0)) End();
		bbFlip(1);
	}

#define TF(v) ((v) ? "true" : "false")
	PutINIValue(OptionFile, "options", "width", bbStr(GfxModeWidths[SelectedGFXMode]));
	PutINIValue(OptionFile, "options", "height", bbStr(GfxModeHeights[SelectedGFXMode]));
	PutINIValue(OptionFile, "options", "fullscreen", TF(Fullscreen));
	PutINIValue(OptionFile, "launcher", "launcher enabled", TF(LauncherEnabled));
	PutINIValue(OptionFile, "options", "borderless windowed", TF(BorderlessWindowed));
	PutINIValue(OptionFile, "options", "16bit", TF(Bit16Mode));
	PutINIValue(OptionFile, "options", "gfx driver", bbStr(SelectedGFXDriver));
	PutINIValue(OptionFile, "options", "check for updates", TF(UpdateCheckEnabled));
}

void DrawTiledImageRect(int img, int sx, int sy, float sw, float sh, int x, int y, int w, int h) {
	for (int x2 = x; x2 < x + w; x2 += bbInt(sw))
		for (int y2 = y; y2 < y + h; y2 += bbInt(sh)) {
			if (x2 + sw > x + w) sw -= Max(x2 + sw - (x + w), 1);
			if (y2 + sh > y + h) sh -= Max(y2 + sh - (y + h), 1);
			bbDrawImageRect(img, x2, y2, sx, sy, bbInt(sw), bbInt(sh), 0);
		}
}

static int align(const char *s, int neg, int mid, int pos) {
	str l = bbLower(s);
	return bbSeq(l, "left") || bbSeq(l, "top") || bbSeq(l, "up") ? neg
		: bbSeq(l, "middle") || bbSeq(l, "center") ? mid
		: bbSeq(l, "right") || bbSeq(l, "bottom") || bbSeq(l, "down") ? pos : 0;
}

void InitLoadingScreens(const char *file) {
	FILE *f = bbOpenFile(file);
	while (f && !bbEof(f)) {
		str t = bbTrim(bbReadLine(f));
		if (!bbSeq(bbLeft(t, 1), "[")) continue;
		t = bbMid(t, 2, bbLen(t) - 2);
		LoadingScreens *ls = bbNew(LoadingScreens);
		ls->ID = ++LoadingScreenAmount;
		ls->title = t;
		ls->imgpath = GetINIString(file, t, "image path", "");
		for (int i = 0; i < 5; i++) {
			ls->txt[i] = GetINIString(file, t, cat("text", bbStr(i + 1)), "");
			if (*ls->txt[i]) ls->txtamount++;
		}
		ls->disablebackground = GetINIInt(file, t, "disablebackground", 0);
		// unknown values leave the field alone in blitz; it's 0 then anyway
		ls->alignx = align(GetINIString(file, t, "align x", ""), -1, 0, 1);
		ls->aligny = align(GetINIString(file, t, "align y", ""), -1, 0, 1);
	}
	if (f) bbCloseFile(f);
}
