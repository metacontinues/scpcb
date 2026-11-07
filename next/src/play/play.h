// play.h: one game, from the loading screen until it's quit
#pragma once
#include "ui/menu.h"

// MENU_TOMENU or MENU_QUIT. dev: skip the loading screen. shot: screenshot
// that frame + quit (SCPCB_SHOT)
MenuAct play(Settings *s, const NewGame *ng, unsigned seed, int dev,
             const char *shot, int shotf);
