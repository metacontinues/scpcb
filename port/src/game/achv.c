// achv.c: Achievements.bb
#include "g.h"

#define AF "Data\\achievementstrings.ini"
static int fit(int img) { return ResizeImage2(img, iround(image_width(img) * GraphicHeight / 768.0f), iround(image_height(img) * GraphicHeight / 768.0f)); }

void InitAchievements(void) {
	AchvMSGenabled = GetINIInt("options.ini", "options", "achievement popup enabled", 0);
	for (int i = 0; i < MAXACHIEVEMENTS; i++) {
		int loc = GetINISectionLocation(AF, cat("s", s_str(i)));
		sset(&AchievementStrings[i], GetINIString2(AF, loc, "string1", ""));
		sset(&AchievementDescs[i], GetINIString2(AF, loc, "AchvDesc", ""));
		AchvIMG[i] = fit(LoadImage_Strict(cat("GFX\\menu\\achievements\\", GetINIString2(AF, loc, "image", ""), ".jpg")));
	}
	AchvLocked = fit(LoadImage_Strict("GFX\\menu\\achievements\\achvlocked.jpg"));
}

static void amfree(AchievementMsg *a) { free(a->txt); }
TLIST_DTOR(AchievementMsg, amfree);

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
	a->achvID = id; sset(&a->txt, s_dup(txt)); a->msgx = 0; a->msgtime = FPSfactor2;
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

// the pause menu's achievement grid: 4 a row, 64px tiles
void DrawAchvIMG(int x, int y, int no) {
	float sc = GraphicHeight / 768.0f;
	int ax = iround(x + no % 4 * iround(76 * sc)), s = iround(64 * sc);
	set_color(0, 0, 0); draw_rect(ax, y, s, s, 1);
	draw_image(Achievements[no] ? AchvIMG[no] : AchvLocked, ax, y, 0);
	set_color(50, 50, 50); draw_rect(ax, y, s, s, 0);
}
void AchievementTooltip(int no) {
	float M = MenuScale;
	AASetFont(Font3); int w = AAStringWidth(AchievementStrings[no]);
	AASetFont(Font1); w = iround(Max(w, AAStringWidth(AchievementDescs[no])) + 20 * M);
	int h = iround(38 * GraphicHeight / 768.0f), x = iround(ScaledMouseX() + 20 * M), y = iround(ScaledMouseY() + 20 * M);
	set_color(25, 25, 25); draw_rect(x, y, w, h, 1);
	set_color(150, 150, 150); draw_rect(x, y, w, h, 0);
	AASetFont(Font3); AAText(x + w / 2, iround(ScaledMouseY() + 35 * M), AchievementStrings[no], 1, 1);
	AASetFont(Font1); AAText(x + w / 2, iround(ScaledMouseY() + 55 * M), AchievementDescs[no], 1, 1);
}
