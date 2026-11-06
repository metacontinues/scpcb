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
