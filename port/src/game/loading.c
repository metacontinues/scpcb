// loading.c: Menu.bb's DrawLoading
#include "g.h"

static const char *cwm[] = {
	"A very fine radio might prove to be useful.", "ThIS PLaCE WiLL BUrN", "You cannot control it.",
	"eof9nsd3jue4iwe1fgj", "YOU NEED TO TRUST IT",
	"Look my friend in the eye when you address him, isn't that the way of the gentleman?",
	"???____??_???__????n?", "Jorge has been expecting you.", "Jorge has been expecting you.", "???????????",
	"Make her a member of the midnight crew.", "oncluded that coming here was a mistake. We have to turn back.",
	"This alloy contains the essence of my life.",
};

static void drawcwm(LoadingScreens *ls, int percent, int shortloading, int first) {
	if (!shortloading && first) {
		if (percent == 0) PlaySound_Strict(LoadTempSound("SFX\\SCP\\990\\cwm1.cwm"));
		else if (percent == 100) PlaySound_Strict(LoadTempSound("SFX\\SCP\\990\\cwm2.cwm"));
	}
	AASetFont(Font2);
	str s = "";
	for (int i = 0, n = bbRand(2, 9); i <= n; i++) s = cat(s, bbChr(bbRand(48, 122)));
	AAText(GraphicWidth / 2, GraphicHeight / 2 + 80, s, 1, 1);
	if (percent == 0) {
		if (bbRand(1, 5) == 1) ls->txt[0] = bbRand(1, 2) == 1 ? cat("It will happen on ", bbCurrentDate(), ".") : bbSdup(bbCurrentTime());
		else ls->txt[0] = (str)cwm[bbRand(1, 13) - 1];
	}
	// only the last Replace survives, the loop just burns rng like the original
	s = ls->txt[0];
	for (int i = 0, n = bbRand(10, 15); i <= n; i++) {
		int a = bbRand(1, bbLen(s) - 1), c = bbRand(130, 250);
		s = bbReplace(ls->txt[0], bbMid(ls->txt[0], a, 1), bbChr(c));
	}
	AASetFont(Font1);
	RowText(s, GraphicWidth / 2 - 200, GraphicHeight / 2 + 120, 400, 300, 1, 1);
}

void DrawLoading(int percent, int shortloading) {
	if (percent == 0) {
		LoadingScreenText = 0;
		int t = bbRand(1, LoadingScreenAmount);
		each(LoadingScreens, ls) if (ls->ID == t) {
			if (!ls->img) ls->img = LoadImage_Strict(cat("Loadingscreens\\", ls->imgpath));
			SelectedLoadingScreen = ls;
			break;
		}
	}
	LoadingScreens *ls = SelectedLoadingScreen;
	int first = 1;
	do {
		bbClsColor(0, 0, 0);
		bbCls();
		if (percent > 20) UpdateMusic();
		if (!shortloading && percent > (100.0f / ls->txtamount) * (LoadingScreenText + 1)) LoadingScreenText++;
		if (!ls->disablebackground)
			bbDrawImage(LoadingBack, GraphicWidth / 2 - bbImageWidth(LoadingBack) / 2, GraphicHeight / 2 - bbImageHeight(LoadingBack) / 2, 0);
		int iw = bbImageWidth(ls->img), ih = bbImageHeight(ls->img);
		int x = ls->alignx == 0 ? GraphicWidth / 2 - iw / 2 : ls->alignx == 1 ? GraphicWidth - iw : 0;
		int y = ls->aligny == 0 ? GraphicHeight / 2 - ih / 2 : ls->aligny == 1 ? GraphicHeight - ih : 0;
		bbDrawImage(ls->img, x, y, 0);

		int w = 300, h = 20;
		x = GraphicWidth / 2 - w / 2; y = GraphicHeight / 2 + 30 - 100;
		bbRect(x, y, w + 4, h, 0);
		for (int i = 1; i <= bbInt((w - 2) * (percent / 100.0f) / 10); i++) bbDrawImage(BlinkMeterIMG, x + 3 + 10 * (i - 1), y + 3, 0);

		if (bbSeq(ls->title, "CWM")) drawcwm(ls, percent, shortloading, first);
		else for (int pass = 0; pass < 2; pass++) { // shadow, then text
			int o = !pass;
			if (o) bbColor(0, 0, 0); else bbColor(255, 255, 255);
			AASetFont(Font2);
			AAText(GraphicWidth / 2 + o, GraphicHeight / 2 + 80 + o, ls->title, 1, 1);
			AASetFont(Font1);
			RowText(ls->txt[LoadingScreenText], GraphicWidth / 2 - 200 + o, GraphicHeight / 2 + 120 + o, 400, 300, 1, 1);
		}

		str lt = cat("LOADING - ", bbStr(percent), " %");
		bbColor(0, 0, 0);
		AAText(GraphicWidth / 2 + 1, GraphicHeight / 2 - 100 + 1, lt, 1, 1);
		bbColor(255, 255, 255);
		AAText(GraphicWidth / 2, GraphicHeight / 2 - 100, lt, 1, 1);
		if (percent == 100) {
			if (first && !bbSeq(ls->title, "CWM")) PlaySound_Strict(LoadTempSound("SFX\\Horror\\Horror8.ogg"));
			AAText(GraphicWidth / 2, GraphicHeight - 50, "PRESS ANY KEY TO CONTINUE", 1, 1);
		} else { bbFlushKeys(); bbFlushMouse(); }
		// TODO borderless upscale + ScreenGamma passes (need fresize_*)
		bbFlip(0);
		first = 0;
		if (percent != 100) break;
	} while (!bbGetKey() && !bbMouseHit(1));
}
