// loading.h: the loading screen, one of Loadingscreens/loadingscreens.ini's
// pages (an scp's picture + a few lines about it) under a progress bar
#pragma once

typedef struct {
  char title[64], img[128];
  int ax, ay, nobg; // align: -1 left/top, 0 centre, 1 right/bottom
  char txt[6][512];
  int ntxt;
} Loading;

void loading_pick(Loading *l);                    // a random page
void loading_draw(const Loading *l, int percent); // inside Begin/EndDrawing
