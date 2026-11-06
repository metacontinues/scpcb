// achv.c: Achievements.bb
#include "g.h"

#define AF "Data\\achievementstrings.ini"
static int fit(int img) { return ResizeImage2(img, bbInt(bbImageWidth(img) * GraphicHeight / 768.0f), bbInt(bbImageHeight(img) * GraphicHeight / 768.0f)); }

void InitAchievements(void) {
	AchvMSGenabled = GetINIInt("options.ini", "options", "achievement popup enabled", 0);
	for (int i = 0; i < MAXACHIEVEMENTS; i++) {
		int loc = GetINISectionLocation(AF, cat("s", bbStr(i)));
		AchievementStrings[i] = GetINIString2(AF, loc, "string1", "");
		AchievementDescs[i] = GetINIString2(AF, loc, "AchvDesc", "");
		AchvIMG[i] = fit(LoadImage_Strict(cat("GFX\\menu\\achievements\\", GetINIString2(AF, loc, "image", ""), ".jpg")));
	}
	AchvLocked = fit(LoadImage_Strict("GFX\\menu\\achievements\\achvlocked.jpg"));
}

TLIST(AchievementMsg);

void GiveAchievement(int id, int showMessage) {
	if (Achievements[id] == 1) return;
	Achievements[id] = 1;
	if (AchvMSGenabled && showMessage) {
		int loc = GetINISectionLocation(AF, cat("s", bbStr(id)));
		CreateAchievementMsg(id, GetINIString2(AF, loc, "string1", ""));
	}
}

AchievementMsg *CreateAchievementMsg(int id, const char *txt) {
	AchievementMsg *a = bbNew(AchievementMsg);
	a->achvID = id; a->txt = bbSdup(txt); a->msgx = 0; a->msgtime = FPSfactor2;
	a->msgID = CurrAchvMSGID++;
	return a;
}

// the popups sliding in from the bottom right
void UpdateAchievementMsg(void) {
	float sc = GraphicHeight / 768.0f;
	int w = bbInt(264 * sc), h = bbInt(84 * sc);
	each(AchievementMsg, a) {
		if (a->msgtime == 0) { bbDelete(a); continue; }
		int x = bbInt(GraphicWidth + a->msgx), y = GraphicHeight - h;
		each(AchievementMsg, a2) if (a2 != a && a2->msgID > a->msgID) y -= h;
		DrawFrame(x, y, w, h, 0, 0);
		bbColor(0, 0, 0); bbRect(bbInt(x + 10 * sc), bbInt(y + 10 * sc), bbInt(64 * sc), bbInt(64 * sc), 1);
		bbDrawImage(AchvIMG[a->achvID], bbInt(x + 10 * sc), bbInt(y + 10 * sc), 0);
		bbColor(50, 50, 50); bbRect(bbInt(x + 10 * sc), bbInt(y + 10 * sc), bbInt(64 * sc), bbInt(64 * sc), 0);
		bbColor(255, 255, 255);
		AASetFont(Font1);
		RowText(cat("Achievement Unlocked - ", a->txt), bbInt(x + 84 * sc), bbInt(y + 10 * sc), bbInt(w - 94 * sc), bbInt(y - 20 * sc), 0, 1);
		if (a->msgtime > 0 && a->msgtime < 70 * 7) {
			a->msgtime += FPSfactor2;
			if (a->msgx > -w) a->msgx = Max(a->msgx - 4 * FPSfactor2, -w);
		} else if (a->msgtime >= 70 * 7) a->msgtime = -1;
		else if (a->msgtime == -1) {
			if (a->msgx < 0) a->msgx = Min(a->msgx + 4 * FPSfactor2, 0);
			else a->msgtime = 0;
		}
	}
}
