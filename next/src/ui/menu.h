// menu.h: the main menu (new game / load game / options / quit) + the pause
// menu. both draw inside Begin/EndDrawing
#pragma once
#include "play/difficulty.h"
#include "ui/settings.h"

typedef enum {
  MENU_NONE,
  MENU_START,
  MENU_QUIT,
  MENU_RESUME,
  MENU_TOMENU
} MenuAct;

typedef struct {
  char name[16], seed[16];
  int sel;      // which of difficulties[]
  Difficulty d; // its copy, custom's edited
} NewGame;

void menu_init(int t,
               int ot); // tab: 0 main, 1 new game, 2 load game, 3 options
MenuAct menu_main(NewGame *ng, Settings *s); // options are saved on "BACK"
// the pause menu over a paused game; seed + difficulty for the info lines
MenuAct menu_pause(const NewGame *ng, Settings *s, int died);
