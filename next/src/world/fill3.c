// fill3.c: FillRoom's per-room cases (MapSystem.bb's big Select, same order as
// port/'s fillroom3.c). cams, emitters, decals, sprites + waypoints come w/
// their systems; port/ has them
#include "world/fill.h"

// room2ccont
static void f_room2ccont(F *f) {
  int d = -1;
  int i = 0;
  int n = 0;
  d = fdoor(f, 64 * S, 0, 368 * S, 180, 0, 0, 2, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fitem(f, "Note from Daniel", "paper", -400 * S, 1040 * S, 115 * S);
  for (n = 0; n <= 2; n += 1) {
    fmesh(f, n * 2, "gfx/map/leverbase");
    fmesh(f, n * 2 + 1, "gfx/map/leverhandle");
    f->r->lever[n] = n * 2 + 1;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n * 2 + i, 0.04f, 0.04f, 0.04f);
      fopos(f, n * 2 + i, -240 * S, 1104 * S, (632 - 64 * n) * S);
    }
    forot(f, n * 2, 0, -90, 0);
    forot(f, n * 2 + 1, 10, -90 - 180, 0);
  }
}

// room106
static void f_room106(F *f) {
  int d = -1;
  int i = 0;
  int n = 0;
  fitem(f, "Level 5 Key Card", "key5", -752 * S, -592 * S, 3026 * S);
  fitem(f, "Dr. Allok's Note", "paper", -416 * S, -576 * S, 2492 * S);
  fitem(f, "Recall Protocol RP-106-N", "paper", 268 * S, -576 * S, 2593 * S);
  d = fdoor(f, -968 * S, -764 * S, 1392 * S, 0, 0, 0, 4, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  d = fdoor(f, 0, 0, -464 * S, 0, 0, 0, 4, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  d = fdoor(f, -624 * S, -1280 * S, 0, 90, 0, 0, 4, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fmesh(f, 6, "gfx/map/room1062");
  foscale(f, 6, S, S, S);
  fcoll(f, 6);
  fopos(f, 6, 784 * S, -980 * S, 720 * S);
  for (n = 0; n <= 2; n += 2) {
    fmesh(f, n, "gfx/map/leverbase");
    fmesh(f, n + 1, "gfx/map/leverhandle");
    f->r->lever[n / 2] = n + 1;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n + i, 0.04f, 0.04f, 0.04f);
      fopos(f, n + i, -(555 - 81 * (float)(n >> 1)) * S, -576 * S, 3040 * S);
    }
    forot(f, n, 0, 0, 0);
    forot(f, n + 1, 10, -180, 0);
  }
  forot(f, 1, 81, -180, 0);
  forot(f, 3, -81, -180, 0);
  fmesh(f, 4, "gfx/map/button");
  foscale(f, 4, .03f, .03f, .03f);
  fopos(f, 4, -146 * S, -576 * S, 3045 * S);
  forot(f, 4, 0, 0, 0);
  fpiv(f, 5);
  foturn(f, 5, 0, 180, 0);
  fopos(f, 5, 1088 * S, 1104 * S, 1888 * S);
  fpiv(f, 9);
  fopos(f, 9, -272 * S, -672 * S, 2736 * S);
  fpiv(f, 10);
  fopos(f, 10, 0, 0, -720 * S);
}

// room1archive
static void f_room1archive(F *f) {
  int chance = 0;
  int temp3 = 0;
  const char *tempstr = NULL, *tempstr2 = NULL;
  char nm[48];
  float x = 0;
  int xtemp = 0;
  float y = 0;
  int ytemp = 0;
  float z = 0;
  int ztemp = 0;
  for (xtemp = 0; xtemp <= 1; xtemp += 1) {
    for (ytemp = 0; ytemp <= 2; ytemp += 1) {
      for (ztemp = 0; ztemp <= 2; ztemp += 1) {
        tempstr = "9V Battery";
        tempstr2 = "bat";
        chance = irnd(-10, 100);
        if (1 == (chance < 0)) {
          break;
        } else if (1 == (chance < 40)) {
          {
            int sel1 = irnd(1, 6);
            if (sel1 == 1) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "1123"), tempstr = nm;
            } else if (sel1 == 2) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "1048"), tempstr = nm;
            } else if (sel1 == 3) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "939"), tempstr = nm;
            } else if (sel1 == 4) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "682"), tempstr = nm;
            } else if (sel1 == 5) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "079"), tempstr = nm;
            } else if (sel1 == 6) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "096"), tempstr = nm;
            } else if (sel1 == 6) {
              snprintf(nm, sizeof nm, "Document SCP-%s", "966"), tempstr = nm;
            }
          }
          tempstr2 = "paper";
        } else if (1 == (((chance >= 40)) & ((chance < 45)))) {
          temp3 = irnd(1, 2);
          snprintf(nm, sizeof nm, "Level %d Key Card", temp3), tempstr = nm;
          tempstr2 = temp3 == 1 ? "key1" : "key2";
        } else if (1 == (((chance >= 45)) & ((chance < 50)))) {
          tempstr = "First Aid Kit";
          tempstr2 = "firstaid";
        } else if (1 == (((chance >= 50)) & ((chance < 60)))) {
          tempstr = "9V Battery";
          tempstr2 = "bat";
        } else if (1 == (((chance >= 60)) & ((chance < 70)))) {
          tempstr = "S-NAV 300 Navigator";
          tempstr2 = "nav";
        } else if (1 == (((chance >= 70)) & ((chance < 85)))) {
          tempstr = "Radio Transceiver";
          tempstr2 = "radio";
        } else if (1 == (((chance >= 85)) & ((chance < 95)))) {
          tempstr = "Clipboard";
          tempstr2 = "clipboard";
        } else if (1 == (((chance >= 95)) & ((chance <= 100)))) {
          temp3 = irnd(1, 3);
          if (temp3 == 1) {
            tempstr = "Playing Card";
          } else if (temp3 == 2) {
            tempstr = "Mastercard";
          } else if (temp3 == 3) {
            tempstr = "Origami";
          }
          tempstr2 = "misc";
        }
        x = (-672 + 864 * xtemp) * S;
        y = (96 + 96 * ytemp) * S;
        z = (480 - 352 * ztemp + frnd(-96, 96)) * S;
        fitem(f, tempstr, tempstr2, x, y, z);
      }
    }
  }
  RD(0) = fdoor(f, 0, 0, -528 * S, 0, 0, 0, 6, "");
}

// room2test1074
static void f_room2test1074(F *f) {
  RD(0) = fdoor(f, 0, 0, 0, 0, 0, 0, 0, "");
  D(RD(0))->locked = 1;
  RD(1) = fdoor(f, 336 * S, 0, 671 * S, 90, 1, 0, 3, "");
  D(RD(1))->autoclose = 0;
  RD(2) = fdoor(f, 336 * S, 0, -800 * S, 90, 1, 0, 3, "");
  D(RD(2))->autoclose = 0;
  RD(3) = fdoor(f, 672 * S, 0, 0, 0, 0, 0, 0, "");
  fitem(f, "Document SCP-1074", "paper", 300 * S, 20 * S, 671 * S);
  fpiv(f, 0);
  fopos(f, 0, 835 * S, 165 * S, 540 * S);
  fpiv(f, 1);
  fopos(f, 1, 835 * S, 10 * S, 300 * S);
  // the painting's surface (NonFreeAble) comes w/ 1074's event
}

// room1123
static void f_room1123(F *f) {
  int d = -1;
  fitem(f, "Document SCP-1123", "paper", 511 * S, 125 * S, -936 * S);
  fitem(f, "SCP-1123", "1123", 832 * S, 166 * S, 784 * S);
  fitem(f, "Leaflet", "paper", -816 * S, 704 * S, 888 * S);
  fitem(f, "Gas Mask", "gasmask", 457 * S, 150 * S, 960 * S);
  d = fdoor(f, 832 * S, 0, 367 * S, 0, 0, 0, 3, "");
  fbtn(f, d, 0, 956 * S, KEEP, 352 * S);
  fbtn(f, d, 1, 713 * S, KEEP, 384 * S);
  D(d)->one = 1;
  d = fdoor(f, 280 * S, 0, -607 * S, 90, 0, 0, 0, "");
  fbtn(f, d, 0, KEEP, KEEP, KEEP);
  fbtn(f, d, 1, KEEP, KEEP, KEEP);
  d = fdoor(f, 280 * S, 512 * S, -607 * S, 90, 0, 0, 0, "");
  fbtn(f, d, 0, KEEP, KEEP, KEEP);
  fnobtn(f, d, 1);
  RD(0) = d;
  fpiv(f, 3);
  fopos(f, 3, 832 * S, 166 * S, 784 * S);
  fpiv(f, 4);
  fopos(f, 4, -648 * S, 592 * S, 692 * S);
  fpiv(f, 5);
  fopos(f, 5, 828 * S, 592 * S, 592 * S);
  fpiv(f, 6);
  fopos(f, 6, -76 * S, 620 * S, 744 * S);
  fpiv(f, 7);
  fopos(f, 7, -640 * S, 620 * S, -864 * S);
  fmesh(f, 8, "gfx/map/forest/door_frame");
  fopos(f, 8, -272 * S, 512 * S, 288 * S);
  forot(f, 8, 0, 90, 0);
  foscale(f, 8, 45 * S, 45 * S, 80 * S);
  fmesh(f, 9, "gfx/map/forest/door");
  fopos(f, 9, -272 * S, 512 * S, (288 - 70) * S);
  forot(f, 9, 0, 10, 0);
  fcoll(f, 9);
  foscale(f, 9, 46 * S, 45 * S, 46 * S);
  fcopy(f, 10, 8);
  fopos(f, 10, -272 * S, 512 * S, 736 * S);
  forot(f, 10, 0, 90, 0);
  foscale(f, 10, 45 * S, 45 * S, 80 * S);
  fcopy(f, 11, 9);
  fopos(f, 11, -272 * S, 512 * S, (736 - 70) * S);
  forot(f, 11, 0, 90, 0);
  fcoll(f, 11);
  foscale(f, 11, 46 * S, 45 * S, 46 * S);
  fcopy(f, 12, 8);
  fopos(f, 12, -592 * S, 512 * S, -704 * S);
  forot(f, 12, 0, 0, 0);
  foscale(f, 12, 45 * S, 45 * S, 80 * S);
  fcopy(f, 13, 9);
  fopos(f, 13, -(592 + 70) * S, 512 * S, -704 * S);
  forot(f, 13, 0, 0, 0);
  fcoll(f, 13);
  foscale(f, 13, 46 * S, 45 * S, 46 * S);
  fmesh(f, 14, "gfx/map/1123_hb");
  fcoll(f, 14);
  fhide(f, 14);
}

// pocketdimension
static void f_pocketdimension(F *f) {
  float angle = 0;
  int i = 0;
  fmesh(f, 8, "gfx/map/pocketdimension3");
  fmesh(f, 9, "gfx/map/pocketdimension4");
  fcopy(f, 10, 9);
  fmesh(f, 11, "gfx/map/pocketdimension5");
  fitem(f, "Burnt Note", "paper", 0, 0.5f, 0 + 3.5f);
  for (i = 8; i <= 11; i += 1) {
    foscale(f, i, S, S, S);
    fcoll(f, i);
    fopos(f, i, 0, 0, 32);
  }
  fmesh(
      f, 30,
      "gfx/map/pocketdimensionterrain"); // not an Objects slot in the original
  foscale(f, 30, S, S, S);
  fcoll(f, 30);
  fopos(f, 30, 0, 2944 * S, 32);
  RD(0) = fdoor(f, 0, 2048 * S, 32 - 1024 * S, 0, 0, 0, 0, "");
  RD(1) = fdoor(f, 0, 2048 * S, 32 + 1024 * S, 180, 0, 0, 0, "");
  foscale(f, 10, S * 1.5f, S * 2, S * 1.5f);
  fopos(f, 11, 0, 0, 64);
  for (i = 1; i <= 8; i += 1) {
    fmesh(f, i - 1, "gfx/map/pocketdimension2");
    foscale(f, i - 1, S, S, S);
    angle = (i - 1) * 45.0f;
    fcoll(f, i - 1);
    forot(f, i - 1, 0, angle - 90, 0);
    fopos(f, i - 1, cosf(angle * DEG2RAD) * 512 * S, 0,
          sinf(angle * DEG2RAD) * 512 * S);
  }
  for (i = 12; i <= 16; i += 1) {
    fpiv(f, i);
    if (i == 12) {
      fopos(f, i, 0, 200 * S, 64);
    } else if (i == 13) {
      fopos(f, i, 390 * S, 200 * S, 64 + 272 * S);
    } else if (i == 14) {
      fopos(f, i, 838 * S, 200 * S, 64 - 551 * S);
    } else if (i == 15) {
      fopos(f, i, -139 * S, 200 * S, 64 + 1201 * S);
    } else if (i == 16) {
      fopos(f, i, -1238 * S, -1664 * S, 64 + 381 * S);
    }
  }
}

// room3z3
static void f_room3z3(F *f) {}

// room2_3, room3_3
static void f_room2_3(F *f) {}

// room1lifts
static void f_room1lifts(F *f) {
  fmesh(f, 0, "gfx/map/button");
  foscale(f, 0, .03f, .03f, .03f);
  fopos(f, 0, 96 * S, 160 * S, 64 * S);
  forot(f, 0, 0, 0, 0);
  fmesh(f, 1, "gfx/map/button");
  foscale(f, 1, .03f, .03f, .03f);
  fopos(f, 1, -96 * S, 160 * S, 64 * S);
  forot(f, 1, 0, 0, 0);
}

// room2servers2
static void f_room2servers2(F *f) {
  int d = -1;
  int it = -1;
  d = fdoor(f, 264 * S, 0, 672 * S, 270, 0, 0, 3, "");
  fbtn(f, d, 0, 224 * S, KEEP, 510 * S);
  fbtn(f, d, 1, 304 * S, KEEP, 840 * S);
  fbturn(f, d, 1, 0);
  fdoor(f, -512 * S, -768 * S, -336 * S, 0, 0, 0, 3, "");
  d = fdoor(f, -509 * S, -768 * S, -1037 * S, 0, 0, 0, 3, "");
  D(d)->locked = 1;
  D(d)->nowp = 1;
  it = fitem(f, "Night Vision Goggles", "nvgoggles", 56.0154f * S, -648 * S,
             749.638f * S);
  fistate(f, it, 200);
  fiyaw(f, it, 0 + irnd(1, 245));
}

// room2gw, room2gw_b
static void f_room2gw(F *f) {
  if (!strcmp(f->r->t->name, "room2gw_b")) {
    fpiv(f, 2);
    fopos(f, 2, -156.825f * S, -37.3458f * S, 121.364f * S);
    fpiv(f, 0);
    fopos(f, 0, 280 * S, 345 * S, -340 * S);
  }
  RD(0) = fdoor(f, 336 * S, 0, -382 * S, 0, 0, 0, 0, "");
  fbtn(f, RD(0), 0, 580.822f * S, KEEP, -606.679f * S);
  fbtn(f, RD(0), 1, 580.822f * S, KEEP, -606.679f * S);
  D(RD(0))->kind = 0;
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  D(RD(0))->locked = 1;
  D(RD(0))->mtf = 0;
  RD(1) = fdoor(f, 336 * S, 0, 462 * S, 180, 0, 0, 0, "");
  fbtn(f, RD(1), 0, 580.822f * S, KEEP, -606.679f * S);
  fbtn(f, RD(1), 1, 580.822f * S, KEEP, -606.679f * S);
  D(RD(1))->kind = 0;
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 1;
  D(RD(1))->locked = 1;
  D(RD(1))->mtf = 0;
  fmesh(f, 3, "gfx/map/room2gw_pipes");
  if (!strcmp(f->r->t->name, "room2gw")) {
    fpiv(f, 0);
    fopos(f, 0, 344 * S, 128 * S, 0);
    if (!f->w->gwbroken &&
        irnd(1, 2) == 1) { // the leaf, knocked off + lying there
      Vector3 k = fleafscale();
      fmesh(f, 1, "gfx/map/door01");
      foscale(f, 1, k.x, k.y, k.z);
      fcoll(f, 1);
      fopos(f, 1, 336 * S + 120 * k.x, 0, 462 * S + 5 * k.z);
      forot(f, 1, 0, 180 + 180, 0);
      f->w->gwbroken = 1;
      D(RD(1))->one = 1;
    }
  }
}

// room3gw
static void f_room3gw(F *f) {
  int d = -1;
  d = fdoor(f, -728 * S, 0, -458 * S, 0, 0, 0, 3, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  D(d)->locked = 0;
  d = fdoor(f, -223 * S, 0, -736 * S, -90, 0, 0, 3, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  D(d)->locked = 0;
  RD(0) = fdoor(f, -459 * S, 0, 339 * S, 90, 0, 0, 0, "");
  fbtn(f, RD(0), 0, 580.822f * S, KEEP, -606.679f * S);
  fbtn(f, RD(0), 1, 580.822f * S, KEEP, -606.679f * S);
  D(RD(0))->kind = 0;
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  D(RD(0))->locked = 1;
  D(RD(0))->mtf = 0;
  RD(1) = fdoor(f, 385 * S, 0, 339 * S, 270, 0, 0, 0, "");
  fbtn(f, RD(1), 0, 580.822f * S, KEEP, -606.679f * S);
  fbtn(f, RD(1), 1, 580.822f * S, KEEP, -606.679f * S);
  D(RD(1))->kind = 0;
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 1;
  D(RD(1))->locked = 1;
  D(RD(1))->mtf = 0;
  D(RD(1))->one = 1;
  fpiv(f, 0);
  fopos(f, 0, -48 * S, 128 * S, 320 * S);
  fmesh(f, 3, "gfx/map/room3gw_pipes");
}

// room1162
static void f_room1162(F *f) {
  fdoor(f, 248 * S, 0, -736 * S, 90, 0, 0, 2, "");
  fpiv(f, 0);
  fopos(f, 0, 1012 * S, 128 * S, -640 * S);
  fitem(f, "Document SCP-1162", "paper", 863.227f * S, 152 * S, -953.231f * S);
}

// room2scps2
static void f_room2scps2(F *f) {
  int it = -1;
  RD(0) = fdoor(f, 288 * S, 0, 576 * S, 90, 0, 0, 3, "");
  D(RD(0))->open = 0;
  D(RD(0))->locked = 1;
  fdoor(f, 777 * S, 0, 671 * S, 90, 0, 0, 4, "");
  fdoor(f, 556 * S, 0, 296 * S, 0, 0, 0, 3, "");
  fpiv(f, 0);
  fopos(f, 0, 576 * S, 160 * S, 632 * S);
  it = fitem(f, "SCP-1499", "scp1499", 600 * S, 176 * S, -228 * S);
  fiyaw(f, it, 0);
  fitem(f, "Document SCP-1499", "paper", 840 * S, 260 * S, 224 * S);
  fitem(f, "Document SCP-500", "paper", 1152 * S, 224 * S, 336 * S);
  fitem(f, "Emily Ross' Badge", "badge", 364 * S, 5 * S, 716 * S);
}

// room3offices
static void f_room3offices(F *f) {
  int d = -1;
  d = fdoor(f, 736 * S, 0, 240 * S, 0, 0, 0, 3, "");
  fbtn(f, d, 0, 892 * S, KEEP, 224 * S);
  fbtn(f, d, 1, 892 * S, KEEP, 255 * S);
  D(d)->one = 1;
  fmesh(f, 0, "gfx/map/room3offices_hb");
  fcoll(f, 0);
  fhide(f, 0);
}

// room2offices4
static void f_room2offices4(F *f) {
  int d = -1;
  d = fdoor(f, -240 * S, 0, 0, 90, 0, 0, 0, "");
  fbtn(f, d, 0, -230 * S, KEEP, KEEP);
  fbtn(f, d, 1, -250 * S, KEEP, KEEP);
  D(d)->open = 0;
  D(d)->autoclose = 0;
  fitem(f, "Sticky Note", "paper", -991 * S, -242 * S, 904 * S);
}

// room2sl
static void f_room2sl(F *f) {
  int d = -1;
  int i = 0;
  float scale = 0;
  scale = S * 4.5f * 0.4f;
  for (i = 0; i <= 14; i += 1) {
    if (i != 7) {
      fmesh(f, i, "gfx/map/monitor");
      foscale(f, i, scale, scale, scale);
      // + their screens (sprites showing cams), w/ cams
    }
  }
  for (i = 0; i <= 2; i += 1) {
    fopos(f, i, -207.94f * S, (648 + (112 * i)) * S, -60.0686f * S);
    forot(f, i, 0, 105 + 0, 0);
  }
  for (i = 3; i <= 5; i += 1) {
    fopos(f, i, -231.489f * S, (648 + (112 * (i - 3))) * S, 95.7443f * S);
    forot(f, i, 0, 90 + 0, 0);
  }
  for (i = 6; i <= 8; i += 2) {
    fopos(f, i, -231.489f * S, (648 + (112 * (i - 6))) * S, 255.744f * S);
    forot(f, i, 0, 90 + 0, 0);
  }
  for (i = 9; i <= 11; i += 1) {
    fopos(f, i, -231.489f * S, (648 + (112 * (i - 9))) * S, 415.744f * S);
    forot(f, i, 0, 90 + 0, 0);
  }
  for (i = 12; i <= 14; i += 1) {
    fopos(f, i, -208.138f * S, (648 + (112 * (i - 12))) * S, 571.583f * S);
    forot(f, i, 0, 75 + 0, 0);
  }
  RD(0) = fdoor(f, 480 * S, 0, -640 * S, 90, 0, 0, 3, "");
  D(RD(0))->autoclose = 0;
  fbtn(f, RD(0), 0, 576 * S, KEEP, -480 * S);
  fbyaw(f, RD(0), 0, 270);
  RD(1) = fdoor(f, 544 * S, 480 * S, 256 * S, 270, 0, 0, 3, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->one = 1;
  d = fdoor(f, 1504 * S, 480 * S, 960 * S, 0, 0, 0, 0, "");
  D(d)->autoclose = 0;
  D(d)->locked = 1;
  fpiv(f, 7);
  fopos(f, 7, 0, 100 * S, -800 * S);
  fpiv(f, 15);
  fopos(f, 15, 700 * S, 700 * S, 256 * S);
  fpiv(f, 16);
  fopos(f, 16, -60 * S, 700 * S, 200 * S);
  fpiv(f, 17);
  fopos(f, 17, -48 * S, 540 * S, 656 * S);
  fmesh(f, 9 * 2, "gfx/map/leverbase");
  fmesh(f, 9 * 2 + 1, "gfx/map/leverhandle");
  f->r->lever[0] = 9 * 2 + 1;
  for (i = 0; i <= 1; i += 1) {
    foscale(f, 9 * 2 + i, 0.04f, 0.04f, 0.04f);
    fopos(f, 9 * 2 + i, -49 * S, 689 * S, 912 * S);
  }
  forot(f, 9 * 2, 0, 0, 0);
  forot(f, 9 * 2 + 1, 10, 0 - 180, 0);
}

// room2_4
static void f_room2_4(F *f) {

  fpiv(f, 6);
  fopos(f, 6, 640 * S, 8 * S, -896 * S);
}

// room3z2
static void f_room3z2(F *f) {
  fmesh(f, 0, "gfx/map/room3z2_hb");
  fcoll(f, 0);
  fhide(f, 0);
}

// lockroom3
static void f_lockroom3(F *f) {
  int d = -1;
  int d2 = -1;
  float scale = 0;
  d = fdoor(f, -736 * S, 0, -104 * S, 0, 1, 0, 0, "");
  D(d)->timer = 70 * 5;
  D(d)->autoclose = 0;
  D(d)->open = 0;
  D(d)->locked = 1;
  fbtn(f, d, 0, -288 * S, 0.7f, -640 * S);
  fnobtn(f, d, 1);
  d2 = fdoor(f, 104 * S, 0, 736 * S, 270, 1, 0, 0, "");
  D(d2)->timer = 70 * 5;
  D(d2)->autoclose = 0;
  D(d2)->open = 0;
  D(d2)->locked = 1;
  fbtn(f, d2, 0, 640 * S, 0.7f, 288 * S);
  fbyaw(f, d2, 0, 90);
  fnobtn(f, d2, 1);
  D(d)->link = d2;
  D(d2)->link = d;
  scale = S * 4.5f * 0.4f;
  fmesh(f, 0, "gfx/map/monitor");
  foscale(f, 0, scale, scale, scale);
  fopos(f, 0, 668 * S, 1.1f, -96 * S);
  forot(f, 0, 0, 90, 0);
  fmesh(f, 1, "gfx/map/monitor");
  foscale(f, 1, scale, scale, scale);
  fopos(f, 1, 96 * S, 1.1f, -668 * S);
}

// medibay
static void f_medibay(F *f) {
  fmesh(f, 0, "gfx/map/medibay_props");
  fcoll(f, 0);
  fpiv(f, 1);
  fopos(f, 1, -762 * S, 0 * S, -346 * S);
  fpiv(f, 2);
  fopos(f, 2, (fox(f, 1) + 126 * S), foy(f, 1), foz(f, 1));
  fitem(f, "First Aid Kit", "firstaid", -506 * S, 192 * S, -322 * S);
  fitem(f, "Syringe", "syringe", -333 * S, 100 * S, 97.3f * S);
  fitem(f, "Syringe", "syringe", -340 * S, 100 * S, 52.3f * S);
  RD(0) = fdoor(f, -264 * S, -0 * S, 640 * S, 90, 0, 0, 3, "");
  fpiv(f, 3);
  fopos(f, 3, -820 * S, 0, -318.399f * S);
}

// room2cpit
static void f_room2cpit(F *f) {
  int d = -1;
  d = fdoor(f, -256 * S, 0, -752 * S, 90, 0, 2, 3, "");
  D(d)->locked = 1;
  D(d)->open = 0;
  D(d)->autoclose = 0;
  D(d)->mtf = 0;
  D(d)->nowp = 1;
  fbtn(f, d, 0, -240 * S, KEEP, KEEP);
  fitem(f, "Dr L's Note", "paper", -160 * S, 32 * S, -353 * S);
}

// dimension1499
static void f_dimension1499(F *f) {

  // the original kept these in Levers[]: [1] the cull hitbox, [0] a pivot.
  // spare slots here
  fmesh(f, 29, "gfx/map/dimension1499/1499object0_cull");
  fcoll(f, 29);
  fhide(f, 29);
  fpiv(f, 30);
  fopos(f, 30, 205 * S, 200 * S, 2287 * S);
  f->r->lever[1] = 29;
  f->r->lever[0] = 30;
}

const Fill fills3[] = {
    {"room2ccont", f_room2ccont},
    {"room106", f_room106},
    {"room1archive", f_room1archive},
    {"room2test1074", f_room2test1074},
    {"room1123", f_room1123},
    {"pocketdimension", f_pocketdimension},
    {"room3z3", f_room3z3},
    {"room2_3", f_room2_3},
    {"room3_3", f_room2_3},
    {"room1lifts", f_room1lifts},
    {"room2servers2", f_room2servers2},
    {"room2gw", f_room2gw},
    {"room2gw_b", f_room2gw},
    {"room3gw", f_room3gw},
    {"room1162", f_room1162},
    {"room2scps2", f_room2scps2},
    {"room3offices", f_room3offices},
    {"room2offices4", f_room2offices4},
    {"room2sl", f_room2sl},
    {"room2_4", f_room2_4},
    {"room3z2", f_room3z2},
    {"lockroom3", f_lockroom3},
    {"medibay", f_medibay},
    {"room2cpit", f_room2cpit},
    {"dimension1499", f_dimension1499},
    {0},
};
