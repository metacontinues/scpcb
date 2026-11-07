// fill2.c: FillRoom's per-room cases (MapSystem.bb's big Select, same order as
// port/'s fillroom2.c). cams, emitters, decals, sprites + waypoints come w/
// their systems; port/ has them
#include "world/fill.h"

// room049
static void f_room049(F *f) {
  int d = -1;
  int i = 0;
  int n = 0;
  fpiv(f, 0);
  fopos(f, 0, 640 * S, 240 * S, 656 * S);
  fpiv(f, 1);
  fopos(f, 1, 3211 * S, -3280 * S, 1824 * S);
  fpiv(f, 2);
  fopos(f, 2, -672 * S, 240 * S, -93 * S);
  fpiv(f, 3);
  fopos(f, 3, -2766 * S, -3280 * S, -1277 * S);
  fpiv(f, 4);
  fopos(f, 4, 528 * S, -3440 * S, 96 * S);
  fpiv(f, 5);
  fopos(f, 5, 64 * S, -3440 * S, -1000 * S);
  for (n = 0; n <= 1; n += 1) {
    fmesh(f, n * 2 + 6, "gfx/map/leverbase");
    fmesh(f, n * 2 + 7, "gfx/map/leverhandle");
    f->r->lever[n] = n * 2 + 7;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n * 2 + 6 + i, 0.03f, 0.03f, 0.03f);
      if (n == 0) {
        fopos(f, n * 2 + 6 + i, 852 * S, -3374 * S, -854 * S);
      } else if (n == 1) {
        fopos(f, n * 2 + 6 + i, -834 * S, -3400 * S, 1093 * S);
      }
    }
    forot(f, n * 2 + 6, 0, 180 + 90 * (!n), 0);
    forot(f, n * 2 + 7, 81 - 92 * n, 90 * (!n), 0);
  }
  RD(0) = fdoor(f, 330 * S, 0, 656 * S, 90, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbtn(f, RD(0), 1, 288 * S, 0.7f, 512 * S);
  fbtn(f, RD(0), 0, 368 * S, 0.7f, 840 * S);
  RD(1) = fdoor(f, 2898 * S, -3520 * S, 1824 * S, 90, 0, 3, 0, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  fbtn(f, RD(1), 1, 2881 * S, KEEP, 1663 * S);
  fbtn(f, RD(1), 0, 2936 * S, KEEP, 2009 * S);
  RD(2) = fdoor(f, -672 * S, 0, -408 * S, 0, 1, 3, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 1;
  fbtn(f, RD(2), 0, -487 * S, 0.7f, -447 * S);
  fbtn(f, RD(2), 1, -857 * S, 0.7f, -369 * S);
  RD(3) = fdoor(f, -2766 * S, -3520 * S, -1592 * S, 0, 0, 3, 0, "");
  D(RD(3))->autoclose = 0;
  D(RD(3))->open = 0;
  fbtn(f, RD(3), 0, -2581 * S, KEEP, -1631 * S);
  fbtn(f, RD(3), 1, -2951 * S, KEEP, -1553 * S);
  RD(4) = fdoor(f, 272 * S, -3552 * S, 104 * S, 90, 0, 0, 0, "");
  D(RD(4))->autoclose = 0;
  D(RD(4))->open = 1;
  D(RD(4))->locked = 1;
  RD(5) = fdoor(f, 264 * S, -3520 * S, -1824 * S, 90, 0, 0, 0, "");
  D(RD(5))->autoclose = 0;
  D(RD(5))->open = 1;
  D(RD(5))->locked = 1;
  RD(6) = fdoor(f, -264 * S, -3520 * S, 1824 * S, 90, 0, 0, 0, "");
  D(RD(6))->autoclose = 0;
  D(RD(6))->open = 1;
  D(RD(6))->locked = 1;
  fdoor(f, 0, 0, 0, 0, 0, 2, -2, "");
  fitem(f, "Document SCP-049", "paper", -608 * S, -3332 * S, 876 * S);
  fitem(f, "Level 4 Key Card", "key4", -512 * S, -3412 * S, 864 * S);
  fitem(f, "First Aid Kit", "firstaid", 385 * S, -3412 * S, 271 * S);
  d = fdoor(f, -272 * S, -3552 * S, 98 * S, 90, 1, 1, 0, "");
  D(d)->autoclose = 0;
  D(d)->open = 1;
  D(d)->mtf = 0;
  D(d)->locked = 1;
  for (i = 0; i <= 1; i += 1) {
    fnobtn(f, d, i);
  }
  d = fdoor(f, -2990 * S, -3520 * S, -1824 * S, 90, 0, 2, 0, "");
  D(d)->locked = 1;
  D(d)->nowp = 1;
  d = fdoor(f, -896 * S, 0, -640 * S, 90, 0, 2, 0, "");
  D(d)->locked = 1;
  D(d)->nowp = 1;
  fpiv(f, 10);
  fopos(f, 10, -832 * S, -3484 * S, 1572 * S);
  fpiv(f, 11);
  fopos(f, 11, 2642 * S, -3516 * S, 1822 * S);
  fpiv(f, 12);
  fopos(f, 12, -2666 * S, -3516 * S, -1792 * S);
}

// room2_2
static void f_room2_2(F *f) {
  fmesh(f, 0, "gfx/map/fan");
  foscale(f, 0, S, S, S);
  fopos(f, 0, -248 * S, 528 * S, 0);
}

// room012
static void f_room012(F *f) {
  int d = -1;
  int i = 0;
  d = fdoor(f, 264 * S, 0, 672 * S, 270, 0, 0, 3, "");
  fbtn(f, d, 0, 224 * S, KEEP, 540 * S);
  fbtn(f, d, 1, 304 * S, KEEP, 840 * S);
  fbturn(f, d, 1, 0);
  RD(0) = fdoor(f, -512 * S, -768 * S, -336 * S, 0, 0, 0, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  D(RD(0))->locked = 1;
  fbtn(f, RD(0), 0, 176 * S, -512 * S, -364 * S);
  fnobtn(f, RD(0), 1);
  fmesh(f, 0, "gfx/map/leverbase");
  fmesh(f, 1, "gfx/map/leverhandle");
  f->r->lever[0] = 1;
  for (i = 0; i <= 1; i += 1) {
    foscale(f, i, 0.04f, 0.04f, 0.04f);
    fopos(f, i, 240 * S, -512 * S, -364 * S);
  }
  forot(f, 1, 10, -180, 0);
  fmesh(f, 2, "gfx/map/room012_2");
  foscale(f, 2, S, S, S);
  fopos(f, 2, -360 * S, -130 * S, 456 * S);
  fmesh(f, 4, "gfx/map/room012_3");
  foscale(f, 4, S, S, S);
  fopos(f, 4, -360 * S, -130 * S, 456 * S);
  fitem(f, "Document SCP-012", "paper", -56 * S, -576 * S, -408 * S);
  fitem(f, "Severed Hand", "hand", -784 * S, -576 * S + 0.3f, 640 * S);
}

// tunnel2
static void f_tunnel2(F *f) {

  fpiv(f, 0);
  fopos(f, 0, 0, 544 * S, 512 * S);
  fpiv(f, 1);
  fopos(f, 1, 0, 544 * S, -512 * S);
}

// room2pipes
static void f_room2pipes(F *f) {

  fpiv(f, 0);
  fopos(f, 0, 368 * S, 0, 0);
  fpiv(f, 1);
  fopos(f, 1, -368 * S, 0, 0);
  fpiv(f, 2);
  fopos(f, 2, 224 * S - 0.005f, 192 * S, 0);
  fpiv(f, 3);
  fopos(f, 3, -224 * S + 0.005f, 192 * S, 0);
}

// room3pit
static void f_room3pit(F *f) {
  fpiv(f, 0);
  fopos(f, 0, 704 * S, 112 * S, -416 * S);
}

// room2servers
static void f_room2servers(F *f) {
  int d = -1;
  int i = 0;
  int n = 0;
  d = fdoor(f, 0, 0, 0, 0, 0, 2, 0, "");
  D(d)->locked = 1;
  RD(0) = fdoor(f, -208 * S, 0, -736 * S, 90, 1, 0, 0, "");
  D(RD(0))->autoclose = 0;
  RD(1) = fdoor(f, -208 * S, 0, 736 * S, 90, 1, 0, 0, "");
  D(RD(1))->autoclose = 0;
  RD(2) = fdoor(f, -672 * S, 0, -1024 * S, 0, 0, 0, 0, "GEAR");
  D(RD(2))->autoclose = 0;
  D(RD(2))->nowp = 1;
  fnobtn(f, RD(2), 0);
  fnobtn(f, RD(2), 1);
  for (n = 0; n <= 2; n += 1) {
    fmesh(f, n * 2, "gfx/map/leverbase");
    fmesh(f, n * 2 + 1, "gfx/map/leverhandle");
    f->r->lever[n] = n * 2 + 1;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n * 2 + i, 0.03f, 0.03f, 0.03f);
      if (n == 0) {
        fopos(f, n * 2 + i, -1260 * S, 234 * S, 750 * S);
      } else if (n == 1) {
        fopos(f, n * 2 + i, -920 * S, 164 * S, 898 * S);
      } else if (n == 2) {
        fopos(f, n * 2 + i, -837 * S, 152 * S, 886 * S);
      }
    }
    forot(f, n * 2 + 1, 81, -180, 0);
  }
  forot(f, 2 + 1, -81, -180, 0);
  forot(f, 4 + 1, -81, -180, 0);
  fpiv(f, 6);
  fopos(f, 6, -320 * S, 0.5f, 0);
  fpiv(f, 7);
  fopos(f, 7, -1328 * S, 0.5f, 528 * S);
  fpiv(f, 8);
  fopos(f, 8, -1376 * S, 0.5f, 32 * S);
  fpiv(f, 9);
  fopos(f, 9, -848 * S, 0.5f, 576 * S);
}

// room3servers
static void f_room3servers(F *f) {
  int it = -1;
  fitem(f, "9V Battery", "bat", -132 * S, -368 * S, -648 * S);
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", -76 * S, -368 * S, -648 * S);
  }
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", -196 * S, -368 * S, -648 * S);
  }
  it = fitem(f, "S-NAV 300 Navigator", "nav", 124 * S, -368 * S, -648 * S);
  fistate(f, it, 20);
  fpiv(f, 0);
  fopos(f, 0, 736 * S, -512 * S, -400 * S);
  fpiv(f, 1);
  fopos(f, 1, -552 * S, -512 * S, -528 * S);
  fpiv(f, 2);
  fopos(f, 2, 736 * S, -512 * S, 272 * S);
  fmesh(f, 3, "gfx/npcs/duck_low_res");
  foscale(f, 3, 0.07f, 0.07f, 0.07f);
  fopos(f, 3, 928 * S, -640 * S, 704 * S);
}

// room3servers2
static void f_room3servers2(F *f) {
  int it = -1;
  fpiv(f, 0);
  fopos(f, 0, -504 * S, -512 * S, 271 * S);
  fpiv(f, 1);
  fopos(f, 1, 628 * S, -512 * S, 271 * S);
  fpiv(f, 2);
  fopos(f, 2, -532 * S, -512 * S, -877 * S);
  it = fitem(f, "Document SCP-970", "paper", 960 * S, -448 * S, 251 * S);
  fiyaw(f, it, 0);
  fitem(f, "Gas Mask", "gasmask", 954 * S, -504 * S, 235 * S);
}

// testroom
static void f_testroom(F *f) {
  int xtemp = 0;
  int ztemp = 0;
  for (xtemp = 0; xtemp <= 1; xtemp += 1) {
    for (ztemp = -1; ztemp <= 1; ztemp += 1) {
      fpiv(f, xtemp * 3 + (ztemp + 1));
      fopos(f, xtemp * 3 + (ztemp + 1), (-236 + 280 * xtemp) * S, -700 * S,
            384 * ztemp * S);
    }
  }
  fpiv(f, 6);
  fopos(f, 6, 754 * S, -1248 * S, 0);
  fdoor(f, 720 * S, 0, 0, 0, 0, 2, -1, "");
  fdoor(f, -624 * S, -1280 * S, 0, 90, 1, 0, 0, "");
  fitem(f, "Document SCP-682", "paper", 656 * S, -1200 * S, -16 * S);
}

// room2closets
static void f_room2closets(F *f) {
  int d = -1;
  int it = -1;
  fitem(f, "Document SCP-1048", "paper", 736 * S, 176 * S, 736 * S);
  fitem(f, "Gas Mask", "gasmask", 736 * S, 176 * S, 544 * S);
  fitem(f, "9V Battery", "bat", 736 * S, 176 * S, -448 * S);
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", 730 * S, 176 * S, -496 * S);
  }
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", 740 * S, 176 * S, -560 * S);
  }
  fitem(f, "Level 1 Key Card", "key1", 736 * S, 240 * S, 752 * S);
  fitem(f, "Clipboard", "clipboard", 736 * S, 224 * S, -480 * S);
  it = fitem(f, "Incident Report SCP-1048-A", "paper", 736 * S, 224 * S,
             -480 * S);
  fihide(f, it); // the original hid it (meant for the clipboard?)
  fpiv(f, 0);
  fopos(f, 0, -1120 * S, -256 * S, 896 * S);
  fpiv(f, 1);
  fopos(f, 1, -1232 * S, -256 * S, -160 * S);
  d = fdoor(f, -240 * S, 0, 0, 90, 0, 0, 0, "");
  fbtn(f, d, 0, -230 * S, KEEP, KEEP);
  fbtn(f, d, 1, -250 * S, KEEP, KEEP);
  D(d)->open = 0;
  D(d)->autoclose = 0;
}

// room2offices
static void f_room2offices(F *f) {
  int it = -1;
  fitem(f, "Document SCP-106", "paper", 404 * S, 145 * S, 559 * S);
  fitem(f, "Level 2 Key Card", "key2", -156 * S, 151 * S, 72 * S);
  it = fitem(f, "S-NAV 300 Navigator", "nav", 305 * S, 153 * S, 944 * S);
  fistate(f, it, 20);
  fitem(f, "Notification", "paper", -137 * S, 153 * S, 464 * S);
  // + a waypoint pair, w/ waypoints
}

// room2offices2
static void f_room2offices2(F *f) {
  int it = -1;
  int temp = 0;
  fitem(f, "Level 1 Key Card", "key1", -368 * S, -48 * S, 80 * S);
  fitem(f, "Document SCP-895", "paper", -800 * S, -48 * S, 368 * S);
  if (irnd(1, 2) == 1) {
    fitem(f, "Document SCP-860", "paper", -800 * S, -48 * S, -464 * S);
  } else {
    fitem(f, "SCP-093 Recovered Materials", "paper", -800 * S, -48 * S,
          -464 * S);
  }
  it = fitem(f, "S-NAV 300 Navigator", "nav", -336 * S, -48 * S, -480 * S);
  fistate(f, it, 28);
  fmesh(f, 0, "gfx/npcs/duck_low_res");
  foscale(f, 0, 0.07f, 0.07f, 0.07f);
  fpiv(f, 1);
  fopos(f, 1, -808 * S, -72 * S, -40 * S);
  fpiv(f, 2);
  fopos(f, 2, -488 * S, 160 * S, 700 * S);
  fpiv(f, 3);
  fopos(f, 3, -488 * S, 160 * S, -668 * S);
  fpiv(f, 4);
  fopos(f, 4, -572 * S, 350 * S, -4 * S);
  temp = irnd(1, 4);
  fopos(f, 0, fox(f, temp), foy(f, temp), foz(f, temp));
}

// room2offices3
static void f_room2offices3(F *f) {
  int i = 0;
  if (irnd(1, 2) == 1) {
    fitem(f, "Mobile Task Forces", "paper", 744 * S, 240 * S, 944 * S);
  } else {
    fitem(f, "Security Clearance Levels", "paper", 680 * S, 240 * S, 944 * S);
  }
  fitem(f, "Object Classes", "paper", 160 * S, 240 * S, 568 * S);
  fitem(f, "Document", "paper", -1440 * S, 624 * S, 152 * S);
  fitem(f, "Radio Transceiver", "radio", -1184 * S, 480 * S, -800 * S);
  {
    int to_ = irnd(0, 1);
    for (i = 0; i <= to_; i += 1) {
      fitem(f, "ReVision Eyedrops", "eyedrops", -1529 * S, 563 * S,
            -572 * S + i * 0.05f);
    }
  }
  fitem(f, "9V Battery", "bat", -1545 * S, 603 * S, -372 * S);
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", -1540 * S, 603 * S, -340 * S);
  }
  if (irnd(1, 2) == 1) {
    fitem(f, "9V Battery", "bat", -1529 * S, 603 * S, -308 * S);
  }
  RD(0) = fdoor(f, -1056 * S, 384 * S, 290 * S, 90, 1, 0, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbtn(f, RD(0), 0, KEEP, KEEP, 161 * S);
  fbtn(f, RD(0), 1, KEEP, KEEP, 161 * S);
}

// start
static void f_start(F *f) {
  int d = -1;
  RD(1) = fdoor(f, 4000 * S, 384 * S, 1696 * S, 90, 1, 1, 0, "");
  D(RD(1))->locked = 0;
  D(RD(1))->autoclose = 0;
  D(RD(1))->kind = 1;
  D(RD(1))->open = 1;
  fnobtn(f, RD(1), 0);
  fnobtn(f, RD(1), 1);
  D(RD(1))->mtf = 0;
  RD(2) = fdoor(f, 2704 * S, 384 * S, 624 * S, 90, 0, 0, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  fnobtn(f, RD(2), 0);
  fnobtn(f, RD(2), 1);
  D(RD(2))->mtf = 0;
  d = fdoor(f, 1392 * S, 384 * S, 64 * S, 90, 1, 0, 0, "");
  D(d)->autoclose = 0;
  D(d)->mtf = 0;
  D(d)->locked = 1;
  d = fdoor(f, -640 * S, 384 * S, 64 * S, 90, 0, 0, 0, "");
  D(d)->locked = 1;
  D(d)->autoclose = 0;
  d = fdoor(f, 1280 * S, 384 * S, 312 * S, 180, 1, 0, 0, "");
  D(d)->locked = 1;
  D(d)->autoclose = 0;
  fbtn(f, d, 0, 1120 * S, KEEP, 328 * S);
  fbtn(f, d, 1, 1120 * S, KEEP, 296 * S);
  D(d)->one = 1;
  D(d)->mtf = 0;
  d = fdoor(f, 0, 0, 1184 * S, 0, 0, 0, 0, "");
  D(d)->locked = 1;
  fmesh(f, 0, "gfx/map/introdesk");
  foscale(f, 0, S, S, S);
  fopos(f, 0, 272 * S, 0, 400 * S);
  fmesh(f, 1, "gfx/map/introdrawer");
  foscale(f, 1, S, S, S);
  fopos(f, 1, 448 * S, 0, 192 * S);
  fpiv(f, 2);
  fopos(f, 2, 0 + 40 * S, 460 * S, 0 + 1072 * S);
  fpiv(f, 3);
  fopos(f, 3, 0 - 80 * S, 100 * S, 0 + 526 * S);
  fpiv(f, 4);
  fopos(f, 4, 0 - 128 * S, 100 * S, 0 + 320 * S);
  fpiv(f, 5);
  fopos(f, 5, 0 + 660 * S, 100 * S, 0 + 526 * S);
  fpiv(f, 6);
  fopos(f, 6, 0 + 700 * S, 100 * S, 0 + 320 * S);
  fpiv(f, 7);
  fopos(f, 7, 0 + 1472 * S, 100 * S, 0 + 912 * S);
}

// room2scps
static void f_room2scps(F *f) {
  int d = -1;
  d = fdoor(f, 264 * S, 0, 0, 90, 1, 0, 3, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, 320 * S, KEEP, KEEP);
  fbtn(f, d, 1, 224 * S, KEEP, KEEP);
  d = fdoor(f, -264 * S, 0, 0, 270, 1, 0, 3, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, -320 * S, KEEP, KEEP);
  fbtn(f, d, 1, -224 * S, KEEP, KEEP);
  RD(1) = fdoor(f, -560 * S, 0, -272 * S, 0, 1, 0, 3, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  RD(2) = fdoor(f, 560 * S, 0, -272 * S, 180, 1, 0, 3, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  RD(3) = fdoor(f, 560 * S, 0, 272 * S, 180, 1, 0, 3, "");
  D(RD(3))->autoclose = 0;
  D(RD(3))->open = 0;
  RD(4) = fdoor(f, -560 * S, 0, 272 * S, 0, 1, 0, 3, "");
  D(RD(4))->autoclose = 0;
  D(RD(4))->open = 0;
  fitem(f, "SCP-714", "scp714", -552 * S, 220 * S, -760 * S);
  fitem(f, "SCP-1025", "scp1025", 552 * S, 224 * S, -758 * S);
  fitem(f, "SCP-860", "scp860", 568 * S, 178 * S, 760 * S);
  fitem(f, "Document SCP-714", "paper", -728 * S, 288 * S, -360 * S);
  fitem(f, "Document SCP-427", "paper", -608 * S, 66 * S, 636 * S);
}

// room205
static void f_room205(F *f) {
  RD(1) = fdoor(f, 128 * S, 0, 640 * S, 90, 1, 0, 3, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  RD(0) = fdoor(f, -1392 * S, -128 * S, -384 * S, 0, 1, 0, 3, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  fnobtn(f, RD(0), 0);
  fnobtn(f, RD(0), 1);
  fpiv(f, 0);
  fopos(f, 0, -1536 * S, 730 * S, 192 * S);
  forot(f, 0, 0, -90, 0);
}

// endroom
static void f_endroom(F *f) {

  RD(0) = fdoor(f, 0, 0, 1136 * S, 0, 0, 1, 6, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  fnobtn(f, RD(0), 0);
  fnobtn(f, RD(0), 1);
}

// endroomc
static void f_endroomc(F *f) {
  int d = -1;
  d = fdoor(f, 1024 * S, 0, 0, 0, 0, 2, 0, "");
  D(d)->open = 0;
  D(d)->autoclose = 0;
  D(d)->locked = 1;
}

// coffin
static void f_coffin(F *f) {
  int d = -1;
  int i = 0;
  int it = -1;
  d = fdoor(f, 0, 0, -448 * S, 0, 0, 1, 2, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, -384 * S, 0.7f, -280 * S);
  fmesh(f, 2, "gfx/map/leverbase");
  fmesh(f, 3, "gfx/map/leverhandle");
  f->r->lever[0] = 3;
  for (i = 0; i <= 1; i += 1) {
    foscale(f, 2 + i, 0.04f, 0.04f, 0.04f);
    fopos(f, 2 + i, -800 * S, 180 * S, -336 * S);
  }
  forot(f, 2, 0, 180, 0);
  forot(f, 3, 10, 0, 0);
  fpiv(f, 0);
  fopos(f, 0, 0, -1320 * S, 2304 * S);
  fitem(f, "Document SCP-895", "paper", -688 * S, 133 * S, -304 * S);
  fitem(f, "Level 3 Key Card", "key3", 240 * S, -1456 * S, 2064 * S);
  it = fitem(f, "Night Vision Goggles", "nvgoggles", 280 * S, -1456 * S,
             2164 * S);
  fistate(f, it, 400);
  fpiv(f, 1);
  fopos(f, 1, 96 * S, -1532 * S, 2016 * S);
}

// room2tesla, room2tesla_lcz, room2tesla_hcz
static void f_room2tesla(F *f) {
  fpiv(f, 0);
  fopos(f, 0, -114 * S, 0, 0);
  fpiv(f, 1);
  fopos(f, 1, 114 * S, 0, 0);
  fpiv(f, 2);
  fopos(f, 2, 0, 0, 0);
  // + a waypoint pair, w/ waypoints
  fpiv(f, 5);
  fopos(f, 5, 0, 0, -800 * S);
  fpiv(f, 6);
  fopos(f, 6, 0, 0, 800 * S);
  fmesh(f, 7, "gfx/map/room2tesla_caution");
}

// room2doors
static void f_room2doors(F *f) {
  int d = -1;
  int d2 = -1;
  d = fdoor(f, 0, 0, 528 * S, 0, 1, 0, 0, "");
  D(d)->autoclose = 0;
  fbtn(f, d, 0, -832 * S, 0.7f, 160 * S);
  fbtn(f, d, 1, 160 * S, 0.7f, 536 * S);
  d2 = fdoor(f, 0, 0, -528 * S, 180, 1, 0, 0, "");
  D(d2)->autoclose = 0;
  fnobtn(f, d2, 0);
  fbtn(f, d2, 1, 160 * S, 0.7f, -536 * S);
  fpiv(f, 0);
  fopos(f, 0, -832 * S, 0.5f, 0);
  D(d2)->link = d;
  D(d)->link = d2;
  D(d)->open = 0;
  D(d2)->open = 1;
}

// 914
static void f_914(F *f) {
  int d = -1;
  int i = 0;
  int it = -1;
  RD(2) = fdoor(f, 0, 0, -368 * S, 0, 0, 1, 2, "");
  D(RD(2))->kind = 1;
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  fbtn(f, RD(2), 0, -496 * S, 0.7f, -272 * S);
  fbturn(f, RD(2), 0, 90);
  fmesh(f, 0, "gfx/map/914key");
  fmesh(f, 1, "gfx/map/914knob");
  for (i = 0; i <= 1; i += 1) {
    foscale(f, i, S, S, S);
  }
  fopos(f, 0, 0, 190 * S, 374 * S);
  fopos(f, 1, 0, 230 * S, 374 * S);
  d = fdoor(f, -624 * S, 0, 528 * S, 180, 1, 0, 0, "");
  D(d)->one = 1;
  fnobtn(f, d, 0);
  fnobtn(f, d, 1);
  D(d)->kind = 4;
  RD(0) = d;
  D(d)->autoclose = 0;
  d = fdoor(f, 816 * S, 0, 528 * S, 180, 1, 0, 0, "");
  D(d)->one = 1;
  fnobtn(f, d, 0);
  fnobtn(f, d, 1);
  D(d)->kind = 4;
  RD(1) = d;
  D(d)->autoclose = 0;
  fpiv(f, 2);
  fpiv(f, 3);
  fopos(f, 2, -712 * S, 0.5f, 640 * S);
  fopos(f, 3, 728 * S, 0.5f, 640 * S);
  fitem(f, "Addendum: 5/14 Test Log", "paper", 954 * S, 228 * S, 127 * S);
  it = fitem(f, "First Aid Kit", "firstaid", 960 * S, 112 * S, -40 * S);
  fiyaw(f, it, 90);
  fitem(f, "Dr. L's Note", "paper", -928 * S, 160 * S, -160 * S);
}

// 173
static void f_173(F *f) {
  int d = -1;
  int xtemp = 0;
  int ztemp = 0;
  fpiv(f, 0);
  fopos(f, 0, 0 + 40 * S, 460 * S, 0 + 1072 * S);
  fpiv(f, 1);
  fopos(f, 1, 0 - 80 * S, 100 * S, 0 + 526 * S);
  fpiv(f, 2);
  fopos(f, 2, 0 - 128 * S, 100 * S, 0 + 320 * S);
  fpiv(f, 3);
  fopos(f, 3, 0 + 660 * S, 100 * S, 0 + 526 * S);
  fpiv(f, 4);
  fopos(f, 4, 0 + 700 * S, 100 * S, 0 + 320 * S);
  fpiv(f, 5);
  fopos(f, 5, 0 + 1472 * S, 100 * S, 0 + 912 * S);
  RD(1) = fdoor(f, 0 + 288 * S, 0, 0 + 384 * S, 90, 0, 1, 0, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->kind = 1;
  D(RD(1))->open = 0;
  fnobtn(f, RD(1), 0);
  fnobtn(f, RD(1), 1);
  RD(2) = fdoor(f, -1008 * S, 0, -688 * S, 90, 1, 0, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  D(RD(2))->locked = 1;
  fnobtn(f, RD(2), 0);
  fnobtn(f, RD(2), 1);
  RD(3) = fdoor(f, -2320 * S, 0, -1248 * S, 90, 1, 0, 0, "");
  D(RD(3))->autoclose = 0;
  D(RD(3))->open = 1;
  D(RD(3))->locked = 1;
  RD(4) = fdoor(f, -4352 * S, 0, -1248 * S, 90, 1, 0, 0, "");
  D(RD(4))->autoclose = 0;
  D(RD(4))->open = 1;
  D(RD(4))->locked = 1;
  RD(7) = fdoor(f, -3712 * S, -385 * S, -128 * S, 0, 1, 0, 0, "");
  D(RD(7))->autoclose = 0;
  D(RD(7))->open = 1;
  d = fdoor(f, -3712 * S, -385 * S, -2336 * S, 0, 0, 0, 0, "");
  D(d)->locked = 1;
  D(d)->nowp = 1;
  d = fdoor(f, -6864 * S, 0, -1248 * S, 90, 1, 0, 0, "");
  D(d)->autoclose = 0;
  D(d)->locked = 1;
  d = fdoor(f, -5856 * S, 0, -1504 * S, 0, 0, 0, 0, "");
  D(d)->locked = 1;
  D(d)->nowp = 1;
  d = fdoor(f, -2432 * S, 0, -1000 * S, 0, 0, 0, 0, "");
  fbtn(f, d, 0, -2592 * S, KEEP, -1016 * S);
  fbtn(f, d, 1, -2592 * S, KEEP, -984 * S);
  D(d)->locked = 1;
  D(d)->nowp = 1;
  for (ztemp = 0; ztemp <= 1; ztemp += 1) {
    d = fdoor(f, -5760 * S, 0, (320 + 896 * ztemp) * S, 0, 0, 0, 0, "");
    D(d)->locked = 1;
    D(d)->nowp = 1;
    d = fdoor(f, -8288 * S, 0, (320 + 896 * ztemp) * S, 0, 0, 0, 0, "");
    D(d)->locked = 1;
    if (ztemp == 0) {
      D(d)->open = 1;
    } else {
      D(d)->nowp = 1;
    }
    for (xtemp = 0; xtemp <= 2; xtemp += 1) {
      d = fdoor(f, -(7424 - 512 * xtemp) * S, 0, (1008 - 480 * ztemp) * S,
                180 * (!ztemp), 0, 0, 0, "");
      D(d)->locked = 1;
      D(d)->one = 1;
      fnobtn(f, d, 0);
      fnobtn(f, d, 1);
      D(d)->nowp = 1;
    }
    for (xtemp = 0; xtemp <= 4; xtemp += 1) {
      d = fdoor(f, -(5120 - 512 * xtemp) * S, 0, (1008 - 480 * ztemp) * S,
                180 * (!ztemp), 0, 0, 0, "");
      D(d)->locked = 1;
      D(d)->one = 1;
      fnobtn(f, d, 0);
      fnobtn(f, d, 1);
      D(d)->nowp = 1;
      if (((xtemp == 2) & (ztemp == 1))) {
        RD(6) = d;
      }
    }
  }
  fitem(f, "Class D Orientation Leaflet", "paper", -(2914 + 1024) * S, 170 * S,
        40 * S);
  fmesh(f, 9, "gfx/map/173_2");
  fcoll(f, 9);
  fmesh(f, 10, "gfx/map/intro_labels");
}

const Fill fills2[] = {
    {"room049", f_room049},
    {"room2_2", f_room2_2},
    {"room012", f_room012},
    {"tunnel2", f_tunnel2},
    {"room2pipes", f_room2pipes},
    {"room3pit", f_room3pit},
    {"room2servers", f_room2servers},
    {"room3servers", f_room3servers},
    {"room3servers2", f_room3servers2},
    {"testroom", f_testroom},
    {"room2closets", f_room2closets},
    {"room2offices", f_room2offices},
    {"room2offices2", f_room2offices2},
    {"room2offices3", f_room2offices3},
    {"start", f_start},
    {"room2scps", f_room2scps},
    {"room205", f_room205},
    {"endroom", f_endroom},
    {"endroomc", f_endroomc},
    {"coffin", f_coffin},
    {"room2tesla", f_room2tesla},
    {"room2tesla_lcz", f_room2tesla},
    {"room2tesla_hcz", f_room2tesla},
    {"room2doors", f_room2doors},
    {"914", f_914},
    {"173", f_173},
    {0},
};
