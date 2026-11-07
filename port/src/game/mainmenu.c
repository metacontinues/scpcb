// mainmenu.c: Menu.bb's top level + UpdateMainMenu (options tabs are in options.c)
#include "g.h"

#define M MenuScale
#define R(v) iround(v)

static int fit(int img) { resize_image(img, R(image_width(img) * M), R(image_height(img) * M)); return img; }

void InitMenu(void) {
	MenuBack = fit(LoadImage_Strict("GFX\\menu\\back.jpg"));
	MenuText = fit(LoadImage_Strict("GFX\\menu\\scptext.jpg"));
	Menu173 = fit(LoadImage_Strict("GFX\\menu\\173back.jpg"));
	MenuWhite = LoadImage_Strict("GFX\\menu\\menuwhite.jpg");
	MenuBlack = LoadImage_Strict("GFX\\menu\\menublack.jpg");
	mask_image(MenuBlack, 255, 255, 0);
	QuickLoadIcon = fit(LoadImage_Strict("GFX\\menu\\QuickLoading.png"));
	for (int i = 0; i < 4; i++) {
		ArrowIMG[i] = LoadImage_Strict("GFX\\menu\\arrow.png");
		rotate_image(ArrowIMG[i], 90 * i);
		handle_image(ArrowIMG[i], 0, 0);
	}
	MenuBlinkTimer[0] = MenuBlinkTimer[1] = 1;
	IntroEnabled = GetINIInt(OptionFile, "options", "intro enabled", 0);
	sset(&SavePath, "Saves\\");
	QuickLoadPercent = -1;
	LoadSaveGames();
}

static const char *menustrs[] = {
	"DON'T BLINK", 0, "DON'T BLINK", "DON'T BLINK", "Secure. Contain. Protect.", "Secure. Contain. Protect.",
	"You want happy endings? Fuck you.", "You want happy endings? Fuck you.", "You want happy endings? Fuck you.",
	"Sometimes we would have had time to scream.", "Sometimes we would have had time to scream.", "Sometimes we would have had time to scream.",
	"NIL", "NO", "black white black white black white gray", "Stone does not care", "9341", "It controls the doors",
	"e8m106]af173o+079m895w914", "NIL", "It has taken over everything", "The spiral is growing",
	"\"Some kind of gestalt effect due to massive reality damage.\"",
}; // indexed by Rand(0, 22); case 1 isn't listed in blitz, so MenuStr stays put
static const char *seeds[] = {"NIL", "NO", "d9341", "5CP_I73", "DONTBLINK", "CRUNCH", "die", "HTAED", "rustledjim", "larry", "JORGE", "dirtymetal", "whatpumpkin"};

static void background(void) {
	set_color(0, 0, 0);
	draw_rect(0, 0, GraphicWidth, GraphicHeight, 1);
	show_pointer();
	draw_image(MenuBack, 0, 0, 0);
	if (MilliSecs2() % MenuBlinkTimer[0] >= irand(1, MenuBlinkDuration[0]))
		draw_image(Menu173, GraphicWidth - image_width(Menu173), GraphicHeight - image_height(Menu173), 0);
	if (irand(1, 300) == 1) { MenuBlinkTimer[0] = irand(4000, 8000); MenuBlinkDuration[0] = irand(200, 500); }
	AASetFont(Font1);
	MenuBlinkTimer[1] = R(MenuBlinkTimer[1] - FPSfactor);
	if (MenuBlinkTimer[1] < MenuBlinkDuration[1]) {
		set_color(50, 50, 50);
		int dy = irand(-5, 5), dx = irand(-5, 5); // blitz evaluates the y arg first here
		AAText(MenuStrX + dx, MenuStrY + dy, MenuStr, 1, 0);
		if (MenuBlinkTimer[1] < 0) {
			MenuBlinkTimer[1] = irand(700, 800);
			MenuBlinkDuration[1] = irand(10, 35);
			MenuStrX = R(irand(700, 1000) * M);
			MenuStrY = R(irand(100, 600) * M);
			const char *s = menustrs[irand(0, 22)];
			if (s) sset(&MenuStr, (str)s);
		}
	}
	AASetFont(Font2);
	draw_image(MenuText, GraphicWidth / 2 - image_width(MenuText) / 2, R(GraphicHeight - 20 * M - image_height(MenuText)), 0);
	if (GraphicWidth > 1240 * M)
		DrawTiledImageRect(MenuWhite, 0, 5, 512, 7 * M, R(985.0f * M), R(407.0f * M), R(GraphicWidth - 1240 * M + 300), R(7 * M));
}

static void maintab(void) {
	for (int i = 0; i <= 3; i++) {
		int x = R(159 * M), y = R((286 + 100 * i) * M), w = R(400 * M), h = R(70 * M);
		int t = MouseHit1 && MouseOn(x, y, w, h);
		const char *txt = "";
		switch (i) {
		case 0:
			txt = "NEW GAME";
			sset(&RandomSeed, "");
			if (!t) break;
			if (irand(1, 15) == 1) sset(&RandomSeed, (str)seeds[irand(1, 13) - 1]);
			else {
				// blitz reuses the outer loop's i here, so the menu loop ends early after a click. kept
				int n = irand(4, 8);
				for (i = 1; i <= n; i++) sset(&RandomSeed, cat(RandomSeed, irand(1, 3) == 1 ? s_str(irand(0, 9)) : s_chr(irand(97, 122))));
			}
			MainMenuTab = 1;
			break;
		case 1: txt = "LOAD GAME"; if (t) { LoadSaveGames(); MainMenuTab = 2; } break;
		case 2: txt = "OPTIONS"; if (t) MainMenuTab = 3; break;
		case 3: txt = "QUIT"; if (t) End(); break;
		}
		DrawButton(x, y, w, h, txt, 1, 0, 1);
	}
}

static void title(const char *s) {
	int x = R(159 * M), y = R(286 * M), w = R(400 * M), h = R(70 * M);
	set_color(255, 255, 255);
	AASetFont(Font2);
	AAText(x + w / 2, y + h / 2, s, 1, 1);
}

static str clean(str s) {
	s = s_left(s, 15);
	for (const char *c = ":./\\<>|?\"*"; *c; c++) s = s_replace(s, (char[]){*c, 0}, "");
	return s;
}

static void newgame(int x, int y) {
	title("NEW GAME");
	x = R(160 * M); y = R(286 * M + 70 * M + 20 * M);
	int w = R(580 * M), h = R(330 * M);
	DrawFrame(x, y, w, h, 0, 0);
	AASetFont(Font1);
	AAText(R(x + 20 * M), R(y + 20 * M), "Name:", 0, 0);
	sset(&CurrSave, clean(InputBox(R(x + 150 * M), R(y + 15 * M), R(200 * M), R(30 * M), CurrSave, 1)));
	set_color(255, 255, 255);
	if (!s_len(SelectedMap)) {
		AAText(R(x + 20 * M), R(y + 60 * M), "Map seed:", 0, 0);
		sset(&RandomSeed, s_left(InputBox(R(x + 150 * M), R(y + 55 * M), R(200 * M), R(30 * M), RandomSeed, 3), 15));
	} else {
		AAText(R(x + 20 * M), R(y + 60 * M), "Selected map:", 0, 0);
		set_color(255, 255, 255); draw_rect(R(x + 150 * M), R(y + 55 * M), R(200 * M), R(30 * M), 1);
		set_color(0, 0, 0); draw_rect(R(x + 150 * M + 2), R(y + 55 * M + 2), R(200 * M - 4), R(30 * M - 4), 1);
		set_color(255, 0, 0);
		AAText(R(x + 150 * M + 100 * M), R(y + 55 * M + 15 * M), s_len(SelectedMap) > 15 ? cat(s_left(SelectedMap, 14), "...") : SelectedMap, 1, 1);
		if (DrawButton(R(x + 370 * M), R(y + 55 * M), R(120 * M), R(30 * M), "Deselect", 0, 0, 1)) sset(&SelectedMap, "");
	}
	AAText(R(x + 20 * M), R(y + 110 * M), "Enable intro sequence:", 0, 0);
	IntroEnabled = DrawTick(R(x + 280 * M), R(y + 110 * M), IntroEnabled, 0);
	AAText(R(x + 20 * M), R(y + 150 * M), "Difficulty:", 0, 0);
	for (int i = SAFE; i <= CUSTOM; i++) {
		if (DrawTick(R(x + 20 * M), R(y + (180 + 30 * i) * M), SelectedDifficulty == difficulties[i], 0)) SelectedDifficulty = difficulties[i];
		set_color(difficulties[i]->r, difficulties[i]->g, difficulties[i]->b);
		AAText(R(x + 60 * M), R(y + (180 + 30 * i) * M), difficulties[i]->name, 0, 0);
	}
	set_color(255, 255, 255);
	DrawFrame(R(x + 150 * M), R(y + 155 * M), R(410 * M), R(150 * M), 0, 0);
	Difficulty *d = SelectedDifficulty;
	if (d->customizable) {
		d->permaDeath = DrawTick(R(x + 160 * M), R(y + 165 * M), d->permaDeath, 0);
		AAText(R(x + 200 * M), R(y + 165 * M), "Permadeath", 0, 0);
		d->saveType = DrawTick(R(x + 160 * M), R(y + 195 * M), d->saveType == SAVEANYWHERE && !d->permaDeath, d->permaDeath) ? SAVEANYWHERE : SAVEONSCREENS;
		AAText(R(x + 200 * M), R(y + 195 * M), "Save anywhere", 0, 0);
		d->aggressiveNPCs = DrawTick(R(x + 160 * M), R(y + 225 * M), d->aggressiveNPCs, 0);
		AAText(R(x + 200 * M), R(y + 225 * M), "Aggressive NPCs", 0, 0);
		set_color(255, 255, 255);
		draw_image(ArrowIMG[1], R(x + 155 * M), R(y + 251 * M), 0);
		if (MouseHit1 && image_rect_overlap(ArrowIMG[1], R(x + 155 * M), R(y + 251 * M), ScaledMouseX(), ScaledMouseY(), 0, 0)) {
			d->otherFactors = d->otherFactors < HARD ? d->otherFactors + 1 : EASY;
			PlaySound_Strict(ButtonSFX);
		}
		set_color(255, 255, 255);
		static const char *of[] = {"Other difficulty factors: Easy", "Other difficulty factors: Normal", "Other difficulty factors: Hard"};
		if (d->otherFactors >= EASY && d->otherFactors <= HARD) AAText(R(x + 200 * M), R(y + 255 * M), of[d->otherFactors], 0, 0);
	} else RowText(d->description, R(x + 160 * M), R(y + 160 * M), R((410 - 20) * M), 200, 0, 1);

	if (DrawButton(x, R(y + h + 20 * M), R(160 * M), R(70 * M), "Load map", 0, 0, 1)) { MainMenuTab = 4; LoadSavedMaps(); }
	AASetFont(Font2);
	if (DrawButton(R(x + 420 * M), R(y + h + 20 * M), R(160 * M), R(70 * M), "START", 0, 0, 1)) {
		if (!s_len(CurrSave)) sset(&CurrSave, "untitled");
		if (!s_len(RandomSeed)) sset(&RandomSeed, s_str(abs(milli_secs())));
		seed_rnd(GenerateSeedNumber(RandomSeed));
		int same = 0;
		for (int i = 1; i <= SaveGameAmount; i++) if (s_eq(SaveGames[i - 1], CurrSave)) same++;
		if (same > 0) sset(&CurrSave, cat(CurrSave, " (", s_str(same + 1), ")"));
		if (StartGame("new")) {
			MainMenuOpen = 0;
			flush_keys(); flush_mouse();
			PutINIValue(OptionFile, "options", "intro enabled", s_str(IntroEnabled));
		}
	}
}

// page arrows + "Page n/m" for the load game and load map lists
static void pager(int x, int y, int w, int amount, int locked) {
	int pages = (int)ceilf(amount / 6.0f);
	AASetFont(Font2);
	if (CurrLoadGamePage < pages - 1 && !locked) {
		if (DrawButton(R(x + 530 * M), R(y + 510 * M), R(50 * M), R(55 * M), ">", 1, 0, 1)) CurrLoadGamePage++;
	} else {
		DrawFrame(R(x + 530 * M), R(y + 510 * M), R(50 * M), R(55 * M), 0, 0);
		set_color(100, 100, 100); AAText(R(x + 555 * M), R(y + 537.5f * M), ">", 1, 1);
	}
	if (CurrLoadGamePage > 0 && !locked) {
		if (DrawButton(x, R(y + 510 * M), R(50 * M), R(55 * M), "<", 1, 0, 1)) CurrLoadGamePage--;
	} else {
		DrawFrame(x, R(y + 510 * M), R(50 * M), R(55 * M), 0, 0);
		set_color(100, 100, 100); AAText(R(x + 25 * M), R(y + 537.5f * M), "<", 1, 1);
	}
	DrawFrame(R(x + 50 * M), R(y + 510 * M), R(w - 100 * M), R(55 * M), 0, 0);
	AAText(R(x + w / 2.0f), R(y + 536 * M), cat("Page ", s_str((int)Max(CurrLoadGamePage + 1, 1)), "/", s_str((int)Max(pages, 1))), 1, 1);
	AASetFont(Font1);
	if (CurrLoadGamePage > pages - 1) CurrLoadGamePage--;
}

static int oldsave(int i) { return !s_eq(SaveGameVersion[i], CompatibleNumber) && !s_eq(SaveGameVersion[i], "1.3.10"); }

static void loadgame(int x, int y, int h) {
	DrawFrame(x, R(y + h + 20 * M), R(580 * M), R(510 * M), 0, 0);
	title("LOAD GAME");
	x = R(160 * M); y = R(286 * M + 70 * M + 20 * M);
	int w = R(580 * M);
	pager(x, y, w, SaveGameAmount, s_len(SaveMSG) != 0);
	if (SaveGameAmount == 0) { AAText(R(x + 20 * M), R(y + 20 * M), "No saved games.", 0, 0); return; }
	x = R(x + 20 * M); y = R(y + 20 * M);
	for (int i = 1 + 6 * CurrLoadGamePage; i <= 6 + 6 * CurrLoadGamePage && i <= SaveGameAmount; i++) {
		int k = i - 1;
		DrawFrame(x, y, R(540 * M), R(70 * M), 0, 0);
		if (oldsave(k)) set_color(255, 0, 0); else set_color(255, 255, 255);
		AAText(R(x + 20 * M), R(y + 10 * M), SaveGames[k], 0, 0);
		AAText(R(x + 20 * M), R(y + 28 * M), SaveGameTime[k], 0, 0);
		AAText(R(x + 120 * M), R(y + 28 * M), SaveGameDate[k], 0, 0);
		AAText(R(x + 20 * M), R(y + 46 * M), SaveGameVersion[k], 0, 0);
		if (!s_len(SaveMSG)) {
			if (oldsave(k)) {
				DrawFrame(R(x + 280 * M), R(y + 20 * M), R(100 * M), R(30 * M), 0, 0);
				set_color(255, 0, 0); AAText(R(x + 330 * M), R(y + 34 * M), "Load", 1, 1);
			} else if (DrawButton(R(x + 280 * M), R(y + 20 * M), R(100 * M), R(30 * M), "Load", 0, 0, 1)) {
				sset(&CurrSave, SaveGames[k]);
				if (StartGame("load")) MainMenuOpen = 0;
			}
			if (DrawButton(R(x + 400 * M), R(y + 20 * M), R(100 * M), R(30 * M), "Delete", 0, 0, 1)) { sset(&SaveMSG, SaveGames[k]); break; }
		} else {
			DrawFrame(R(x + 280 * M), R(y + 20 * M), R(100 * M), R(30 * M), 0, 0);
			if (oldsave(k)) set_color(255, 0, 0); else set_color(100, 100, 100);
			AAText(R(x + 330 * M), R(y + 34 * M), "Load", 1, 1);
			DrawFrame(R(x + 400 * M), R(y + 20 * M), R(100 * M), R(30 * M), 0, 0);
			set_color(100, 100, 100); AAText(R(x + 450 * M), R(y + 34 * M), "Delete", 1, 1);
		}
		y = R(y + 80 * M);
	}
	if (!s_len(SaveMSG)) return;
	x = R(740 * M); y = R(376 * M);
	DrawFrame(x, y, R(420 * M), R(200 * M), 0, 0);
	RowText("Are you sure you want to delete this save?", R(x + 20 * M), R(y + 15 * M), R(400 * M), R(200 * M), 0, 1);
	if (DrawButton(R(x + 50 * M), R(y + 150 * M), R(100 * M), R(30 * M), "Yes", 0, 0, 1)) {
		delete_file(cat(current_dir(), SavePath, SaveMSG, "\\save.txt"));
		delete_dir(cat(current_dir(), SavePath, SaveMSG));
		sset(&SaveMSG, "");
		LoadSaveGames();
	}
	if (DrawButton(R(x + 250 * M), R(y + 150 * M), R(100 * M), R(30 * M), "No", 0, 0, 1)) sset(&SaveMSG, "");
}

static void loadmap(int x, int y, int h) {
	DrawFrame(x, R(y + h + 20 * M), R(580 * M), R(510 * M), 0, 0);
	title("LOAD MAP");
	x = R(160 * M); y = R(286 * M + 70 * M + 20 * M);
	int w = R(580 * M);
	float tx = x + w, ty = y, tw = 400 * M, th = 150 * M;
	pager(x, y, w, SavedMapsAmount, 0);
	if (!SavedMaps || !s_len(SavedMaps[0])) { AAText(R(x + 20 * M), R(y + 20 * M), "No saved maps. Use the Map Creator to create new maps.", 0, 0); return; }
	x = R(x + 20 * M); y = R(y + 20 * M);
	for (int i = 1 + 6 * CurrLoadGamePage; i <= 6 + 6 * CurrLoadGamePage && i <= SavedMapsAmount; i++) {
		DrawFrame(x, y, R(540 * M), R(70 * M), 0, 0);
		AAText(R(x + 20 * M), R(y + 10 * M), SavedMaps[i - 1], 0, 0);
		AAText(R(x + 20 * M), R(y + 37 * M), SavedMapsAuthor[i - 1], 0, 0);
		if (DrawButton(R(x + 400 * M), R(y + 20 * M), R(100 * M), R(30 * M), "Load", 0, 0, 1)) { sset(&SelectedMap, SavedMaps[i - 1]); MainMenuTab = 1; }
		if (MouseOn(R(x + 400 * M), R(y + 20 * M), R(100 * M), R(30 * M))) DrawMapCreatorTooltip(R(tx), R(ty), R(tw), R(th), SavedMaps[i - 1]);
		y = R(y + 80 * M);
	}
}

void UpdateMainMenu(void) {
	background();
	if (!MouseDown1) OnSliderID = 0;
	if (MainMenuTab == 0) maintab();
	else {
		int x = R(159 * M), y = R(286 * M), w = R(400 * M), h = R(70 * M);
		DrawFrame(x, y, w, h, 0, 0);
		if (DrawButton(R(x + w + 20 * M), y, R(580 * M - w - 20 * M), h, "BACK", 0, 0, 1)) {
			switch (MainMenuTab) {
			case 1: PutINIValue(OptionFile, "options", "intro enabled", s_str(IntroEnabled)); MainMenuTab = 0; break;
			case 2: CurrLoadGamePage = 0; MainMenuTab = 0; break;
			case 3: case 5: case 6: case 7:
				SaveOptionsINI();
				UserTrackCheck = UserTrackCheck2 = 0;
				anti_alias(Opt_AntiAlias);
				MainMenuTab = 0;
				break;
			case 4: MainMenuTab = 1; CurrLoadGamePage = 0; MouseHit1 = 0; break;
			default: MainMenuTab = 0;
			}
		}
		switch (MainMenuTab) {
		case 1: newgame(x, y); break;
		case 2: loadgame(x, y, h); break;
		case 3: case 5: case 6: case 7:
			title("OPTIONS");
			UpdateOptionsTab(R(160 * M), R(y + h + 20 * M), R(580 * M));
			break;
		case 4: loadmap(x, y, h); break;
		}
	}
	set_color(255, 255, 255);
	AASetFont(ConsoleFont);
	AAText(20, GraphicHeight - 30, cat("v", VersionNumber), 0, 0);
	if (Fullscreen) draw_image(CursorIMG, ScaledMouseX(), ScaledMouseY(), 0);
	AASetFont(Font1);
}
