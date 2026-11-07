// achv.c: Achievements.bb
#include "g.h"

#define AF "Data\\achievementstrings.ini"
static int fit(int img) { return ResizeImage2(img, iround(image_width(img) * GraphicHeight / 768.0f), iround(image_height(img) * GraphicHeight / 768.0f)); }

void InitAchievements(void) {
	AchvMSGenabled = GetINIInt("options.ini", "options", "achievement popup enabled", 0);
	for (int i = 0; i < MAXACHIEVEMENTS; i++) {
		int loc = GetINISectionLocation(AF, cat("s", s_str(i)));
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
		int loc = GetINISectionLocation(AF, cat("s", s_str(id)));
		CreateAchievementMsg(id, GetINIString2(AF, loc, "string1", ""));
	}
}

AchievementMsg *CreateAchievementMsg(int id, const char *txt) {
	AchievementMsg *a = lnew(AchievementMsg);
	a->achvID = id; a->txt = s_dup(txt); a->msgx = 0; a->msgtime = FPSfactor2;
	a->msgID = CurrAchvMSGID++;
	return a;
}

// the popups sliding in from the bottom right
void UpdateAchievementMsg(void) {
	float sc = GraphicHeight / 768.0f;
	int w = iround(264 * sc), h = iround(84 * sc);
	each(AchievementMsg, a) {
		if (a->msgtime == 0) { ldel(a); continue; }
		int x = iround(GraphicWidth + a->msgx), y = GraphicHeight - h;
		each(AchievementMsg, a2) if (a2 != a && a2->msgID > a->msgID) y -= h;
		DrawFrame(x, y, w, h, 0, 0);
		set_color(0, 0, 0); draw_rect(iround(x + 10 * sc), iround(y + 10 * sc), iround(64 * sc), iround(64 * sc), 1);
		draw_image(AchvIMG[a->achvID], iround(x + 10 * sc), iround(y + 10 * sc), 0);
		set_color(50, 50, 50); draw_rect(iround(x + 10 * sc), iround(y + 10 * sc), iround(64 * sc), iround(64 * sc), 0);
		set_color(255, 255, 255);
		AASetFont(Font1);
		RowText(cat("Achievement Unlocked - ", a->txt), iround(x + 84 * sc), iround(y + 10 * sc), iround(w - 94 * sc), iround(y - 20 * sc), 0, 1);
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
