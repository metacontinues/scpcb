// difficulty.c: Difficulty.bb
#include "g.h"

TLIST(Difficulty);

static Difficulty *mk(int i, const char *name, const char *desc, int pd, int agg, int save, int other, int r, int g, int b) {
	Difficulty *d = difficulties[i] = lnew(Difficulty);
	sset(&d->name, (str)name); sset(&d->description, (str)desc);
	d->permaDeath = pd; d->aggressiveNPCs = agg; d->saveType = save; d->otherFactors = other;
	d->r = r; d->g = g; d->b = b;
	return d;
}

void InitDifficulties(void) {
	mk(SAFE, "Safe", "The game can be saved any time. However, as in the case of SCP Objects, a Safe classification does not mean that handling it does not pose a threat.",
		0, 0, SAVEANYWHERE, EASY, 120, 150, 50);
	mk(EUCLID, "Euclid", "In Euclid difficulty, saving is only allowed at specific locations marked by lit up computer screens. "
		"Euclid-class objects are inherently unpredictable, so that reliable containment is not always possible.",
		0, 0, SAVEONSCREENS, NORMAL, 200, 200, 0);
	mk(KETER, "Keter", "Keter-class objects are considered the most dangerous ones in Foundation containment. "
		"The same can be said for this difficulty level: the SCPs are more aggressive, and you have only one life - when you die, the game is over. ",
		1, 1, SAVEONQUIT, HARD, 200, 0, 0);
	mk(CUSTOM, "Custom", "", 0, 1, SAVEANYWHERE, EASY, 255, 255, 255)->customizable = 1;
	SelectedDifficulty = difficulties[SAFE];
}
