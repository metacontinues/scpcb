// fill1.c: FillRoom's per-room cases (MapSystem.bb's big Select, same order as
// port/'s fillroom1.c). cams, emitters, decals, sprites + waypoints come w/
// their systems; port/ has them
#include "fill.h"
#include "raymath.h"

// room860
static void f_room860(F *f) {
  int d = -1;
  int it = -1;
  fmesh(f, 2, "gfx/map/forest/door_frame");
  fopos(f, 2, 184 * S, 0, 0);
  foscale(f, 2, 45 * S, 45 * S, 80 * S);
  fmesh(f, 3, "gfx/map/forest/door");
  fopos(f, 3, 112 * S, 0, 0.05f);
  fcoll(f, 3);
  foscale(f, 3, 46 * S, 45 * S, 46 * S);
  fcopy(f, 4, 3);
  fopos(f, 4, 256 * S, 0, -0.05f);
  forot(f, 4, 0, 180, 0);
  foscale(f, 4, 46 * S, 45 * S, 46 * S);
  fdoor(f, 928 * S, 0, 640 * S, 0, 0, 0, 0, "ABCD");
  d = fdoor(f, 928 * S, 0, -640 * S, 0, 1, 0, 0, "ABCD");
  D(d)->autoclose = 0;
  fdoor(f, 416 * S, 0, -640 * S, 0, 0, 0, 1, "");
  fdoor(f, 416 * S, 0, 640 * S, 0, 0, 0, 1, "");
  // the forest itself comes w/ 860's event
  it = fitem(f, "Document SCP-860-1", "paper", 672 * S, 176 * S, 335 * S);
  fiyaw(f, it, 0 + 10);
  it = fitem(f, "Document SCP-860", "paper", 1152 * S, 176 * S, -384 * S);
  fiyaw(f, it, 0 + 170);
}

// lockroom
static void f_lockroom(F *f) {
  int d = -1;
  int d2 = -1;
  d = fdoor(f, -736 * S, 0, -104 * S, 0, 1, 0, 0, "");
  D(d)->timer = 70 * 5;
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, -288 * S, 0.7f, -640 * S);
  fnobtn(f, d, 1);
  d2 = fdoor(f, 104 * S, 0, 736 * S, 270, 1, 0, 0, "");
  D(d2)->timer = 70 * 5;
  D(d2)->autoclose = 0;
  D(d2)->open = 0;
  fbtn(f, d2, 0, 640 * S, 0.7f, 288 * S);
  fbyaw(f, d2, 0, 90);
  fnobtn(f, d2, 1);
  D(d)->link = d2;
  D(d2)->link = d;
}

// lockroom2
static void f_lockroom2(F *f) { // only decals (blood) + cams
  (void)f;
}

// gatea
static void f_gatea(F *f) {
  int d2 = -1;
  RD(2) = fdoor(f, -4064 * S, -1280 * S, 3952 * S, 0, 0, 0, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  d2 = fdoor(f, 0, 0, -1024 * S, 0, 0, 0, 0, "");
  D(d2)->autoclose = 0;
  D(d2)->open = 0;
  D(d2)->locked = 1;
  d2 = fdoor(f, -1440 * S, -480 * S, 2328 * S, 0, 0, 0, 2, "");
  // the A2 ending (console only) has these open + locked
  D(d2)->autoclose = 0;
  D(d2)->open = 0;
  D(d2)->locked = 0;
  fbtn(f, d2, 0, -1320 * S, KEEP, 2288 * S);
  fbtn(f, d2, 1, -1584 * S, KEEP, 2488 * S);
  fbyaw(f, d2, 1, 90);
  d2 = fdoor(f, -1440 * S, -480 * S, 4352 * S, 0, 0, 0, 2, "");
  // the A2 ending (console only) has these open + locked
  D(d2)->autoclose = 0;
  D(d2)->open = 0;
  D(d2)->locked = 0;
  fbtn(f, d2, 0, -1320 * S, KEEP, 4384 * S);
  fbyaw(f, d2, 0, 180);
  fbtn(f, d2, 1, -1584 * S, KEEP, 4232 * S);
  fbyaw(f, d2, 1, 90);
  // exit1's Objects[1], [2] get shared w/ this room by its event
  int ge = fnamed(f, "gateaentrance");
  if (ge >= 0) { // the elevator down from here, + where it lets out, in gate
                 // a's space
    RD(1) = fdoor(f, 1544 * S, 0, -64 * S, 90, 0, 3, 0, "");
    D(RD(1))->autoclose = 0;
    D(RD(1))->open = 0;
    fbtn(f, RD(1), 0, 1584 * S, KEEP, 80 * S);
    fbtn(f, RD(1), 1, 1456 * S, KEEP, -208 * S);
    Vector3 p = floc(f, 1848 * S, 240 * S, -64 * S);
    f->w->rooms[ge].obj[1] =
        (RoomObj){.on = 1, .m = MatrixTranslate(p.x, p.y, p.z)};
  }
  fpiv(f, 3);
  fopos(f, 3, 1216 * S, 0, 2112 * S);
  fpiv(f, 4);
  fopos(f, 4, 0, 96 * S, 6400 * S);
  fpiv(f, 5);
  fopos(f, 5, 1784 * S, 2124 * S, 4512 * S);
  fpiv(f, 6);
  fopos(f, 6, -5048 * S, 1912 * S, 4656 * S);
  fpiv(f, 7);
  fopos(f, 7, 1824 * S, 224 * S, 7056 * S);
  fpiv(f, 8);
  fopos(f, 8, -1824 * S, 224 * S, 7056 * S);
  fpiv(f, 9);
  fopos(f, 9, 2624 * S, 992 * S, 6157 * S);
  fpiv(f, 11);
  fopos(f, 11, -4064 * S, -1248 * S, -1696 * S);
  fmesh(f, 13, "gfx/map/gateawall1");
  fopos(f, 13, -4308 * S, -1045 * S, 544 * S);
  fcoll(f, 13);
  fmesh(f, 14, "gfx/map/gateawall2");
  fopos(f, 14, -3820 * S, -1045 * S, 544 * S);
  fcoll(f, 14);
  fpiv(f, 15);
  fopos(f, 15, -3568 * S, -1089 * S, 4944 * S);
  fmesh(f, 16, "gfx/map/gatea_hitbox1");
  fcoll(f, 16);
  fhide(f, 16);
}

// gateaentrance
static void f_gateaentrance(F *f) {

  RD(0) = fdoor(f, 744 * S, 0, 512 * S, 90, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbtn(f, RD(0), 1, 688 * S, KEEP, 368 * S);
  fbtn(f, RD(0), 0, 784 * S, KEEP, 656 * S);
  fpiv(f, 0);
  fopos(f, 0, 1048 * S, 0, 512 * S);
  RD(1) = fdoor(f, 0, 0, -360 * S, 0, 0, 1, 5, "");
  D(RD(1))->kind = 1;
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  fbtn(f, RD(1), 1, 416 * S, KEEP, -576 * S);
  fbyaw(f, RD(1), 1, 0 - 90);
  fbtn(f, RD(1), 0, 0, 20, 0);
}

// exit1
static void f_exit1(F *f) {
  int d = -1;
  fpiv(f, 0);
  fopos(f, 0, 4356 * S, 9767 * S, 2588 * S);
  RD(4) = fdoor(f, 0, 0, -320 * S, 0, 0, 1, 5, "");
  D(RD(4))->kind = 1;
  D(RD(4))->autoclose = 0;
  D(RD(4))->open = 0;
  fbtn(f, RD(4), 1, 352 * S, 0.7f, -528 * S);
  fbyaw(f, RD(4), 1, 0 - 90);
  fbtn(f, RD(4), 0, 0, 7, 0);
  fpiv(f, 3);
  fopos(f, 3, -7680 * S, 10992 * S, -27048 * S);
  fpiv(f, 4);
  fopos(f, 4, 5203.36f * S, 12128 * S, -1739.19f * S);
  fpiv(f, 5);
  fopos(f, 5, 4363.02f * S, 10536 * S, 2766.16f * S);
  fpiv(f, 6);
  fopos(f, 6, 5192 * S, 12192 * S, -1760 * S);
  fpiv(f, 7);
  fopos(f, 7, 5192 * S, 12192 * S, -4352 * S);
  RD(0) = fdoor(f, 720 * S, 0, 1432 * S, 0, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbmove(f, RD(0), 0, 22 * S);
  fbmove(f, RD(0), 1, 22 * S);
  fpiv(f, 8);
  fopos(f, 8, 720 * S, 0, 1744 * S);
  RD(1) = fdoor(f, -5424 * S, 10784 * S, -1380 * S, 0, 0, 3, 0, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  fbmove(f, RD(1), 0, 22 * S);
  fbmove(f, RD(1), 1, 22 * S);
  fpiv(f, 9);
  fopos(f, 9, -5424 * S, 10784 * S, -1068 * S);
  RD(2) = fdoor(f, 4352 * S, 10784 * S, -492 * S, 0, 0, 0, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 0;
  RD(3) = fdoor(f, 4352 * S, 10784 * S, 500 * S, 0, 0, 0, 0, "");
  D(RD(3))->autoclose = 0;
  D(RD(3))->open = 0;
  fpiv(f, 10);
  fopos(f, 10, 4352 * S, 10778 * S, 1344 * S);
  fpiv(f, 11);
  fopos(f, 11, 2816 * S, 11024 * S, -2816 * S);
  RD(5) = fdoor(f, 3248 * S, 9856 * S, 6400 * S, 0, 0, 0, 0, "ABCD");
  D(RD(5))->autoclose = 0;
  D(RD(5))->open = 0;
  d = fdoor(f, 3072 * S, 9856 * S, 5800 * S, 90, 0, 0, 3, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fpiv(f, 14);
  fopos(f, 14, 3536 * S, 10256 * S, 5512 * S);
  fpiv(f, 15);
  fopos(f, 15, 3536 * S, 10256 * S, 5824 * S);
  fpiv(f, 16);
  fopos(f, 16, 3856 * S, 10256 * S, 5512 * S);
  fpiv(f, 17);
  fopos(f, 17, 3856 * S, 10256 * S, 5824 * S);
  fpiv(f, 18);
  fopos(f, 18, 3250 * S, 9896 * S, 6623 * S);
  fpiv(f, 19);
  fopos(f, 19, 3808 * S, 12320 * S, -13568 * S);
}

// roompj
static void f_roompj(F *f) {
  int d = -1;
  int it = -1;
  it = fitem(f, "Document SCP-372", "paper", 800 * S, 176 * S, 1108 * S);
  fiyaw(f, it, 0);
  it = fitem(f, "Radio Transceiver", "radio", 800 * S, 112 * S, 944 * S);
  fistate(f, it, 80.0f);
  fmesh(f, 3, "gfx/map/372_hb");
  fcoll(f, 3);
  fhide(f, 3);
  d = fdoor(f, 0, 0, -368 * S, 0, 1, 1, 2, "");
  D(d)->autoclose = 0;
  fbtn(f, d, 0, -496 * S, 0.7f, -272 * S);
  fbturn(f, d, 0, 90);
}

// room079
static void f_room079(F *f) {
  int d = -1;
  d = fdoor(f, 0, -448 * S, 1136 * S, 0, 0, 1, 4, "");
  D(d)->kind = 1;
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 1, 224 * S, -250 * S, 918 * S);
  fbtn(f, d, 0, -240 * S, -250 * S, 1366 * S);
  RD(0) = fdoor(f, 1456 * S, -448 * S, 976 * S, 0, 0, 1, 3, "");
  D(RD(0))->kind = 1;
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  fbtn(f, RD(0), 1, 1760 * S, -250 * S, 1236 * S);
  fbturn(f, RD(0), 0, -90 - 90);
  fbtn(f, RD(0), 0, 1760 * S, -240 * S, 740 * S);
  fbturn(f, RD(0), 1, 0); // the original: 90 - 90
  fdoor(f, 1144 * S, -448 * S, 704 * S, 90, 0, 0, -1, "");
  fmesh(f, 0, "gfx/map/079");
  foscale(f, 0, 1.3f, 1.3f, 1.3f);
  fopos(f, 0, 1856 * S, -560 * S, -672 * S);
  foturn(f, 0, 0, 180, 0);
  fpiv(f, 2);
  fopos(f, 2, 1184 * S, -448 * S, 1792 * S);
}

// checkpoint1
static void f_checkpoint1(F *f) {
  RD(0) = fdoor(f, 48 * S, 0, -128 * S, 0, 0, 0, 3, "");
  fbtn(f, RD(0), 0, -152 * S, KEEP, -352 * S);
  fbtn(f, RD(0), 1, -152 * S, KEEP, 96 * S);
  RD(1) = fdoor(f, -352 * S, 0, -128 * S, 0, 0, 0, 3, "");
  D(RD(1))->link = RD(0);
  D(RD(0))->link = RD(1);
  fpiv(f, 0);
  fopos(f, 0, 720 * S, 120 * S, 333 * S);
  D(RD(0))->timer = 70 * 5;
  D(RD(1))->timer = 70 * 5;
  fmesh(f, 2, "gfx/map/monitor_checkpoint");
  foscale(f, 2, 2, 2, 2);
  fopos(f, 2, -152 * S, 384 * S, 124 * S);
  forot(f, 2, 0, 180, 0);
  fmesh(f, 3, "gfx/map/monitor_checkpoint");
  foscale(f, 3, 2, 2, 2);
  fopos(f, 3, -152 * S, 384 * S, -380 * S);
  forot(f, 3, 0, 0, 0);
  if (!fcell(f, 0, -1)) { // nothing past it: seal it off
    fdoor(f, 0, 0, -4, 0, 0, 0, 0, "GEAR");
  }
}

// checkpoint2
static void f_checkpoint2(F *f) {

  RD(0) = fdoor(f, -48 * S, 0, 128 * S, 0, 0, 0, 5, "");
  fbtn(f, RD(0), 0, 152 * S, KEEP, -96 * S);
  fbtn(f, RD(0), 1, 152 * S, KEEP, 352 * S);
  RD(1) = fdoor(f, 352 * S, 0, 128 * S, 0, 0, 0, 5, "");
  D(RD(1))->link = RD(0);
  D(RD(0))->link = RD(1);
  fpiv(f, 0);
  fopos(f, 0, -720 * S, 120 * S, 464 * S);
  fmesh(f, 2, "gfx/map/monitor_checkpoint");
  foscale(f, 2, 2, 2, 2);
  fopos(f, 2, 152 * S, 384 * S, 380 * S);
  forot(f, 2, 0, 180, 0);
  fmesh(f, 3, "gfx/map/monitor_checkpoint");
  foscale(f, 3, 2, 2, 2);
  fopos(f, 3, 152 * S, 384 * S, -124 * S);
  forot(f, 3, 0, 0, 0);
  D(RD(0))->timer = 70 * 5;
  D(RD(1))->timer = 70 * 5;
  if (!fcell(f, 0, -1)) { // nothing past it: seal it off
    fdoor(f, 0, 0, -4, 0, 0, 0, 0, "GEAR");
  }
}

// room2pit
static void f_room2pit(F *f) {
  // + 6 emitters (steam), w/ particles
  fpiv(f, 6);
  fopos(f, 6, 640 * S, 8 * S, -896 * S);
  fpiv(f, 7);
  fopos(f, 7, -864 * S, -400 * S, -632 * S);
}

// room2testroom2
static void f_room2testroom2(F *f) {
  int d = -1;
  int it = -1;
  fpiv(f, 0);
  fopos(f, 0, -640 * S, 0.5f, -912 * S);
  fpiv(f, 1);
  fopos(f, 1, -669 * S, 0.5f, -16 * S);
  RD(0) = fdoor(f, -240 * S, 0, 640 * S, 90, 0, 0, 1, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  d = fdoor(f, -512 * S, 0, 384 * S, 0, 0, 0, 0, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fitem(f, "Level 2 Key Card", "key2", -914 * S, 137 * S, 61 * S);
  it = fitem(f, "S-NAV 300 Navigator", "nav", -312 * S, 264 * S, 176 * S);
  fistate(f, it, 20);
}

// room3tunnel
static void f_room3tunnel(F *f) {

  fpiv(f, 0);
  fopos(f, 0, -190 * S, 4 * S, 190 * S);
}

// room2toilets
static void f_room2toilets(F *f) {

  fpiv(f, 0);
  fopos(f, 0, 1040 * S, 192 * S, 0);
  fpiv(f, 1);
  fopos(f, 1, 1530 * S, 0.5f, 512 * S);
  fpiv(f, 2);
  fopos(f, 2, 1535 * S, 150 * S, 512 * S);
}

// room2storage
static void f_room2storage(F *f) {
  int i = 0;
  int it = -1;
  RD(0) = fdoor(f, -1288 * S, 0, 0, 270, 0, 0, 0, "");
  RD(1) = fdoor(f, -760 * S, 0, 0, 270, 0, 0, 0, "");
  RD(2) = fdoor(f, -264 * S, 0, 0, 270, 0, 0, 0, "");
  RD(3) = fdoor(f, 264 * S, 0, 0, 270, 0, 0, 0, "");
  RD(4) = fdoor(f, 760 * S, 0, 0, 270, 0, 0, 0, "");
  RD(5) = fdoor(f, 1288 * S, 0, 0, 270, 0, 0, 0, "");
  for (i = 0; i <= 5; i += 1) {
    fbmove(f, RD(i), 0, -8);
    fbmove(f, RD(i), 1, -8);
    D(RD(i))->autoclose = 0;
    D(RD(i))->open = 0;
  }
  it = fitem(f, "Document SCP-939", "paper", 352 * S, 176 * S, 256 * S);
  fiyaw(f, it, 0 + 4);
  fitem(f, "9V Battery", "bat", 352 * S, 112 * S, 448 * S);
  fitem(f, "Empty Cup", "emptycup", -672 * S, 240 * S, 288 * S);
  fitem(f, "Level 1 Key Card", "key1", -672 * S, 240 * S, 224 * S);
}

// room2sroom
static void f_room2sroom(F *f) {
  int d = -1;
  int it = -1;
  d = fdoor(f, 1440 * S, 224 * S, 32 * S, 90, 0, 0, 4, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fitem(f, "Some SCP-420-J", "420", 1776 * S, 400 * S, 427 * S);
  fitem(f, "Some SCP-420-J", "420", 1808 * S, 400 * S, 435 * S);
  it = fitem(f, "Level 5 Key Card", "key5", 2232 * S, 392 * S, 387 * S);
  fiyaw(f, it, 0);
  fitem(f, "Nuclear Device Document", "paper", 2248 * S, 440 * S, 372 * S);
  fitem(f, "Radio Transceiver", "radio", 2240 * S, 320 * S, 128 * S);
}

// room2shaft
static void f_room2shaft(F *f) {
  int d = -1;
  int it = -1;
  d = fdoor(f, 1552 * S, 0, 552 * S, 0, 0, 0, 0, "");
  fbtn(f, d, 0, KEEP, KEEP, 518 * S);
  fbtn(f, d, 1, KEEP, KEEP, 575 * S);
  D(d)->autoclose = 0;
  D(d)->open = 0;
  d = fdoor(f, 256 * S, 0, 744 * S, 90, 0, 0, 2, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fitem(f, "Level 3 Key Card", "key3", 1119 * S, 233 * S, 494 * S);
  it = fitem(f, "First Aid Kit", "firstaid", 1035 * S, 145 * S, 56 * S);
  fiyaw(f, it, 90);
  fitem(f, "9V Battery", "bat", 1930 * S, 97 * S, 256 * S);
  fitem(f, "9V Battery", "bat", 1061 * S, 161 * S, 494 * S);
  fitem(f, "ReVision Eyedrops", "eyedrops", 1930 * S, 225 * S, 128 * S);
  fpiv(f, 0);
  fopos(f, 0, 1560 * S, 0, 250 * S);
  fpiv(f, 1);
  fopos(f, 1, 1344 * S, -752 * S, -384 * S);
  fmesh(f, 2, "gfx/map/button");
  foscale(f, 2, .03f, .03f, .03f);
  fopos(f, 2, 1181 * S, 180 * S, -512 * S);
  forot(f, 2, 0, 270, 0);
}

// room2poffices
static void f_room2poffices(F *f) {
  int d = -1;
  int it = -1;
  d = fdoor(f, 240 * S, 0, 448 * S, 90, 0, 0, 0, fcode(f->w->accesscode));
  fbtn(f, d, 0, 248 * S, KEEP, KEEP);
  fbtn(f, d, 1, 232 * S, KEEP, KEEP);
  D(d)->autoclose = 0;
  D(d)->open = 0;
  d = fdoor(f, -496 * S, 0, 0, 90, 0, 0, 0, "ABCD");
  fbtn(f, d, 0, -488 * S, KEEP, KEEP);
  fbtn(f, d, 1, -504 * S, KEEP, KEEP);
  D(d)->autoclose = 0;
  D(d)->open = 0;
  D(d)->locked = 1;
  d = fdoor(f, 240 * S, 0, -576 * S, 90, 0, 0, 0, "7816");
  fbtn(f, d, 0, 248 * S, KEEP, KEEP);
  fbtn(f, d, 1, 232 * S, KEEP, KEEP);
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fitem(f, "Mysterious Note", "paper", 736 * S, 224 * S, 544 * S);
  it = fitem(f, "Ballistic Vest", "vest", 608 * S, 112 * S, 32 * S);
  fiyaw(f, it, 90);
  fitem(f, "Incident Report SCP-106-0204", "paper", 704 * S, 183 * S, -576 * S);
  fitem(f, "Journal Page", "paper", 912 * S, 176 * S, -160 * S);
  it = fitem(f, "First Aid Kit", "firstaid", 912 * S, 112 * S, -336 * S);
  fiyaw(f, it, 90);
}

// room2poffices2
static void f_room2poffices2(F *f) {
  int d = -1;
  d = fdoor(f, 240 * S, 0, 48 * S, 270, 0, 0, 3, "");
  fbtn(f, d, 0, 224 * S, KEEP, 176 * S);
  fbtn(f, d, 1, 256 * S, KEEP, KEEP);
  D(d)->autoclose = 0;
  D(d)->open = 0;
  RD(0) = fdoor(f, -432 * S, 0, 0, 90, 0, 0, 0, "1234");
  fbtn(f, RD(0), 0, -416 * S, KEEP, 176 * S);
  fnobtn(f, RD(0), 1);
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 0;
  D(RD(0))->locked = 1;
  fpiv(f, 0);
  fopos(f, 0, -808 * S, 1, -72 * S);
  fitem(f, "Dr. L's Burnt Note", "paper", -688 * S, 1, -16 * S);
  fitem(f, "Dr L's Burnt Note", "paper", -808 * S, 1, -72 * S);
  fitem(f, "The Modular Site Project", "paper", 622 * S, 125 * S, -73 * S);
}

// room2elevator
static void f_room2elevator(F *f) {

  fpiv(f, 0);
  fopos(f, 0, 888 * S, 240 * S, 0);
  fpiv(f, 1);
  fopos(f, 1, 1024 * S - 0.01f, 120 * S, 0);
  RD(0) = fdoor(f, 448 * S, 0, 0, 90, 0, 3, 0, "");
  fbtn(f, RD(0), 1, 416 * S, KEEP, -208 * S);
  fbtn(f, RD(0), 0, 480 * S, KEEP, 184 * S);
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  D(RD(0))->locked = 1;
}

// room2cafeteria
static void f_room2cafeteria(F *f) {
  int it = -1;
  fpiv(f, 0);
  fopos(f, 0, 1847 * S, -240 * S, -321 * S);
  fpiv(f, 1);
  fopos(f, 1, 1780 * S, -248 * S, -276 * S);
  it = fitem(f, "cup", "cup", -508 * S, -187 * S, 284 * S);
  finame(f, it, "Cup of Orange Juice");
  it = fitem(f, "cup", "cup", 1412 * S, -187 * S, -716 * S);
  finame(f, it, "Cup of Coffee");
  fitem(f, "Empty Cup", "emptycup", -540 * S, -187 * S, 124 * S);
  fitem(f, "Quarter", "25ct", -447 * S, -334 * S, 36 * S);
  fitem(f, "Quarter", "25ct", 1409 * S, -334 * S, -732 * S);
}

// room2nuke
static void f_room2nuke(F *f) {
  int d = -1;
  int i = 0;
  int it = -1;
  int n = 0;
  d = fdoor(f, 576 * S, 0, 152 * S, 90, 0, 0, 5, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, 602 * S, KEEP, 20 * S);
  fbtn(f, d, 1, 550 * S, KEEP, 20 * S);
  D(d)->one = 1;
  d = fdoor(f, -544 * S, 1504 * S, 738 * S, 90, 0, 0, 5, "");
  D(d)->autoclose = 0;
  D(d)->open = 0;
  fbtn(f, d, 0, KEEP, KEEP, 608 * S);
  fbtn(f, d, 1, KEEP, KEEP, 608 * S);
  RD(0) = fdoor(f, 1192 * S, 0, 0, 90, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fpiv(f, 4);
  fopos(f, 4, 1496 * S, 240 * S, 0);
  RD(1) = fdoor(f, 680 * S, 1504 * S, 0, 90, 0, 3, 0, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  fpiv(f, 5);
  fopos(f, 5, 984 * S, 1744 * S, 0);
  for (n = 0; n <= 1; n += 1) {
    fmesh(f, n * 2, "gfx/map/leverbase");
    fmesh(f, n * 2 + 1, "gfx/map/leverhandle");
    f->r->lever[n] = n * 2 + 1;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n * 2 + i, 0.04f, 0.04f, 0.04f);
      fopos(f, n * 2 + i, -975 * S, 1712 * S, -(502 - 132 * n) * S);
    }
    forot(f, n * 2, 0, -90 - 180, 0);
    forot(f, n * 2 + 1, 10, -90 - 180 - 180, 0);
  }
  fitem(f, "Nuclear Device Document", "paper", -768 * S, 1684 * S, -768 * S);
  it = fitem(f, "Ballistic Vest", "vest", -944 * S, 1652 * S, -656 * S);
  fiyaw(f, it, -90);
  fpiv(f, 6);
  fopos(f, 6, 1110 * S, 36 * S, -208 * S);
}

// room2tunnel
static void f_room2tunnel(F *f) {
  int d = -1;
  int temp = 0;
  fpiv(f, 0);
  fopos(f, 0, 2640 * S, -2496 * S, 400 * S);
  fpiv(f, 1);
  fopos(f, 1, -4336 * S, -2496 * S, -2512 * S);
  fpiv(f, 2);
  forot(f, 2, 0, 180, 0);
  fopos(f, 2, 552 * S, 240 * S, 656 * S);
  fpiv(f, 4);
  fopos(f, 4, -552 * S, 240 * S, -656 * S);
  RD(0) = fdoor(f, 264 * S, 0, 656 * S, 90, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbtn(f, RD(0), 1, 224 * S, 0.7f, 480 * S);
  fbtn(f, RD(0), 0, 304 * S, 0.7f, 832 * S);
  RD(2) = fdoor(f, -264 * S, 0, -656 * S, 90, 1, 3, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 1;
  fbtn(f, RD(2), 0, -224 * S, 0.7f, -480 * S);
  fbtn(f, RD(2), 1, -304 * S, 0.7f, -832 * S);
  temp = f->w->accesscode * 3 % 10000;
  if (temp < 1000) {
    temp = temp + 1000;
  }
  d = fdoor(f, 0, 0, 0, 0, 0, 1, 0, fcode(temp));
  fbtn(f, d, 0, 224 * S, 0.7f, -384 * S);
  fbyaw(f, d, 0, -90);
  fbtn(f, d, 1, -224 * S, 0.7f, 384 * S);
  fbyaw(f, d, 1, 90);
  fitem(f, "Scorched Note", "paper", 64 * S, 144 * S, -384 * S);
}

// 008
static void f_008(F *f) {
  int d = -1;
  int d2 = -1;
  int it = -1;
  fpiv(f, 0);
  fopos(f, 0, 292 * S, 130 * S, 516 * S);
  fmesh(f, 1, "gfx/map/008_2");
  foscale(f, 1, S, S, S);
  fopos(f, 1, 292 * S, 151 * S, 576 * S);
  forot(f, 1, 89, 0, 0);
  f->r->lever[0] = 1;
  fpiv(f, 3);
  fopos(f, 3, -445 * S, 120 * S, 544 * S);
  fpiv(f, 4);
  fopos(f, 4, 67 * S, 120 * S, 464 * S);
  d = fdoor(f, 296 * S, 0, -672 * S, 180, 1, 0, 4, "");
  D(d)->autoclose = 0;
  fbtn(f, d, 1, 164 * S, KEEP, KEEP);
  fnobtn(f, d, 0);
  D(d)->one = 1;
  RD(0) = d;
  d2 = fdoor(f, 296 * S, 0, -144 * S, 0, 0, 0, 0, "");
  D(d2)->autoclose = 0;
  fbtn(f, d2, 0, 432 * S, KEEP, -480 * S);
  fbyaw(f, d2, 0, -90);
  fbtn(f, d2, 1, 164 * S, KEEP, -128 * S);
  D(d2)->one = 1;
  RD(1) = d2;
  D(d)->link = d2;
  D(d2)->link = d;
  d = fdoor(f, -384 * S, 0, -672 * S, 0, 0, 0, 4, "");
  D(d)->autoclose = 0;
  D(d)->locked = 1;
  RD(2) = d;
  it = fitem(f, "Hazmat Suit", "hazmatsuit", -76 * S, 0.5f, -396 * S);
  fiyaw(f, it, 90);
  fitem(f, "Document SCP-008", "paper", -245 * S, 192 * S, 368 * S);
  fpiv(f, 6);
  fopos(f, 6, 160 * S, 672 * S, -384 * S);
  fpiv(f, 7);
  fopos(f, 7, 0, 672 * S, 352 * S);
}

// room035
static void f_room035(F *f) {
  int d = -1;
  int d2 = -1;
  int i = 0;
  int n = 0;
  d = fdoor(f, -296 * S, 0, -672 * S, 180, 1, 0, 5, "");
  D(d)->autoclose = 0;
  D(d)->locked = 1;
  RD(0) = d;
  fbtn(f, d, 1, -164 * S, KEEP, KEEP);
  fnobtn(f, d, 0);
  D(d)->one = 1;
  d2 = fdoor(f, -296 * S, 0, -144 * S, 0, 0, 0, 0, "");
  D(d2)->autoclose = 0;
  D(d2)->locked = 1;
  RD(1) = d2;
  fbtn(f, d2, 0, -432 * S, KEEP, -480 * S);
  fbyaw(f, d2, 0, 90);
  fnobtn(f, d2, 1);
  D(d2)->one = 1;
  RD(2) = fdoor(f, 384 * S, 0, -672 * S, 180, 0, 0, 5, "");
  D(RD(2))->autoclose = 0;
  RD(3) = fdoor(f, 768 * S, 0, 512 * S, 90, 0, 0, 0, "5731");
  D(RD(3))->autoclose = 0;
  D(d)->link = d2;
  D(d2)->link = d;
  for (i = 0; i <= 1; i += 1) {
    fmesh(f, i * 2, "gfx/map/leverbase");
    fmesh(f, i * 2 + 1, "gfx/map/leverhandle");
    f->r->lever[i] = i * 2 + 1;
    for (n = 0; n <= 1; n += 1) {
      foscale(f, i * 2 + n, 0.04f, 0.04f, 0.04f);
      fopos(f, i * 2 + n, 210 * S, 224 * S, -(208 - i * 76) * S);
    }
    forot(f, i * 2, 0, -90 - 180, 0);
    forot(f, i * 2 + 1, -80, -90, 0);
  }
  fpiv(f, 3);
  fopos(f, 3, 456 * S, 0.5f, 400 * S);
  fpiv(f, 4);
  fopos(f, 4, -576 * S, 0.5f, 640 * S);
  fpiv(f, 7);
  fopos(f, 7, -720 * S, 0.5f, 880 * S);
  fpiv(f, 8);
  fopos(f, 8, 176 * S, 0.5f, -144 * S);
  fitem(f, "SCP-035 Addendum", "paper", 248 * S, 220 * S, 576 * S);
  fitem(f, "Radio Transceiver", "radio", -544 * S, 0.5f, 704 * S);
  fitem(f, "SCP-500-01", "scp500", 1168 * S, 224 * S, 576 * S);
  fitem(f, "Metal Panel", "scp148", -360 * S, 0.5f, 644 * S);
  fitem(f, "Document SCP-035", "paper", 1168 * S, 104 * S, 608 * S);
}

// room513
static void f_room513(F *f) {
  int d = -1;
  d = fdoor(f, -704 * S, 0, 304 * S, 0, 0, 0, 2, "");
  D(d)->autoclose = 0;
  fbtn(f, d, 0, KEEP, KEEP, 288 * S);
  fbtn(f, d, 1, KEEP, KEEP, 320 * S);
  fitem(f, "SCP-513", "scp513", -32 * S, 196 * S, 688 * S);
  fitem(f, "Blood-stained Note", "paper", 736 * S, 1, 48 * S);
  fitem(f, "Document SCP-513", "paper", -480 * S, 104 * S, -176 * S);
}

// room966
static void f_room966(F *f) {
  int it = -1;
  fdoor(f, -400 * S, 0, 0, -90, 0, 0, 3, "");
  fdoor(f, 0, 0, -480 * S, 180, 0, 0, 3, "");
  fpiv(f, 0);
  fopos(f, 0, 0, 0.5f, 512 * S);
  fpiv(f, 1);
  fopos(f, 1, 64 * S, 0.5f, -640 * S);
  fpiv(f, 2);
  fopos(f, 2, 0, 0.5f, 0);
  fpiv(f, 3);
  fopos(f, 3, 320 * S, 0.5f, 704 * S);
  it = fitem(f, "Night Vision Goggles", "nvgoggles", 320 * S, 0.5f, 704 * S);
  fistate(f, it, 300);
}

// room3storage
static void f_room3storage(F *f) {
  int d = -1;
  int i = 0;
  int it = -1;
  int n = 0;
  float x = 0;
  float z = 0;
  fpiv(f, 0);
  fopos(f, 0, 0, 240 * S, 752 * S);
  fpiv(f, 1);
  fopos(f, 1, 5840 * S, -5392 * S, 1360 * S);
  fpiv(f, 2);
  fopos(f, 2, 608 * S, 240 * S, -624 * S);
  fpiv(f, 3);
  fopos(f, 3, -456 * S, -5392 * S, -1136 * S);
  fpiv(f, 4);
  fopos(f, 4, 2128 * S, -5550 * S, 2048 * S);
  fpiv(f, 5);
  fopos(f, 5, 2128 * S, -5550 * S, -1136 * S);
  fpiv(f, 6);
  fopos(f, 6, 3824 * S, -5550 * S, -1168 * S);
  fpiv(f, 7);
  fopos(f, 7, 3760 * S, -5550 * S, 2048 * S);
  fpiv(f, 8);
  fopos(f, 8, 4848 * S, -5550 * S, 112 * S);
  fpiv(f, 9);
  fopos(f, 9, 592 * S, -5550 * S, 6352 * S);
  fpiv(f, 10);
  fopos(f, 10, 2928 * S, -5550 * S, 6352 * S);
  fpiv(f, 11);
  fopos(f, 11, 2928 * S, -5550 * S, 5200 * S);
  fpiv(f, 12);
  fopos(f, 12, 592 * S, -5550 * S, 5200 * S);
  fpiv(f, 13);
  fopos(f, 13, 1136 * S, -5550 * S, 2944 * S);
  fpiv(f, 14);
  fopos(f, 14, 1104 * S, -5550 * S, 1184 * S);
  fpiv(f, 15);
  fopos(f, 15, -464 * S, -5550 * S, 1216 * S);
  fpiv(f, 16);
  fopos(f, 16, -432 * S, -5550 * S, 2976 * S);
  fmesh(f, 20, "gfx/map/room3storage_hb");
  fcoll(f, 20);
  fhide(f, 20);
  RD(0) = fdoor(f, 0, 0, 448 * S, 0, 1, 3, 0, "");
  D(RD(0))->autoclose = 0;
  D(RD(0))->open = 1;
  fbtn(f, RD(0), 1, -160 * S, 0.7f, 480 * S);
  fbtn(f, RD(0), 0, 160 * S, 0.7f, 416 * S);
  RD(1) = fdoor(f, 5840 * S, -5632 * S, 1048 * S, 0, 0, 3, 0, "");
  D(RD(1))->autoclose = 0;
  D(RD(1))->open = 0;
  fbtn(f, RD(1), 0, 6000 * S, KEEP, 1008 * S);
  fbtn(f, RD(1), 1, 5680 * S, KEEP, 1088 * S);
  RD(2) = fdoor(f, 608 * S, 0, -312 * S, 0, 1, 3, 0, "");
  D(RD(2))->autoclose = 0;
  D(RD(2))->open = 1;
  fbtn(f, RD(2), 1, 448 * S, 0.7f, -272 * S);
  fbtn(f, RD(2), 0, 768 * S, 0.7f, -352 * S);
  RD(3) = fdoor(f, -456 * S, -5632 * S, -824 * S, 0, 0, 3, 0, "");
  D(RD(3))->autoclose = 0;
  D(RD(3))->open = 0;
  fbtn(f, RD(3), 0, -280 * S, KEEP, -864 * S);
  fbtn(f, RD(3), 1, -632 * S, KEEP, -784 * S);
  {
    int sel1 = irnd(1, 3);
    if (sel1 == 1) {
      x = 2312;
      z = -952;
    } else if (sel1 == 2) {
      x = 3032;
      z = 1288;
    } else if (sel1 == 3) {
      x = 2824;
      z = 2808;
    }
  }
  fitem(f, "Black Severed Hand", "hand2", x * S, -5596 * S + 1, z * S);
  it = fitem(f, "Night Vision Goggles", "nvgoggles", 1936 * S, -5496 * S,
             -944 * S);
  fistate(f, it, 450);
  for (n = 10; n <= 11; n += 1) {
    fmesh(f, n * 2, "gfx/map/leverbase");
    fmesh(f, n * 2 + 1, "gfx/map/leverhandle");
    f->r->lever[n - 10] = n * 2 + 1;
    for (i = 0; i <= 1; i += 1) {
      foscale(f, n * 2 + i, 0.04f, 0.04f, 0.04f);
      if (n == 10) {
        fopos(f, n * 2 + i, 3101 * S, -5461 * S, 6568 * S);
      } else {
        fopos(f, n * 2 + i, 1209 * S, -5461 * S, 3164 * S);
      }
    }
    forot(f, n * 2, 0, 0, 0);
    forot(f, n * 2 + 1, -10, 0 - 180, 0);
  }
  RD(4) = fdoor(f, 56 * S, -5632 * S, 6344 * S, 90, 0, 2, 0, "");
  D(RD(4))->autoclose = 0;
  D(RD(4))->open = 0;
  for (i = 0; i <= 1; i += 1) {
    fnobtn(f, RD(4), i);
  }
  d = fdoor(f, 1157 * S, -5632 * S, 660 * S, 0, 0, 2, 0, "");
  D(d)->locked = 1;
  D(d)->open = 0;
  D(d)->autoclose = 0;
  for (i = 0; i <= 1; i += 1) {
    fnobtn(f, d, i);
  }
  d = fdoor(f, 234 * S, -5632 * S, 5239 * S, 90, 0, 2, 0, "");
  D(d)->locked = 1;
  D(d)->open = 0;
  D(d)->autoclose = 0;
  for (i = 0; i <= 1; i += 1) {
    fnobtn(f, d, i);
  }
  d = fdoor(f, 3446 * S, -5632 * S, 6369 * S, 90, 0, 2, 0, "");
  D(d)->locked = 1;
  D(d)->open = 0;
  D(d)->autoclose = 0;
  for (i = 0; i <= 1; i += 1) {
    fnobtn(f, d, i);
  }
}

const Fill fills1[] = {
    {"room860", f_room860},
    {"lockroom", f_lockroom},
    {"lockroom2", f_lockroom2},
    {"gatea", f_gatea},
    {"gateaentrance", f_gateaentrance},
    {"exit1", f_exit1},
    {"roompj", f_roompj},
    {"room079", f_room079},
    {"checkpoint1", f_checkpoint1},
    {"checkpoint2", f_checkpoint2},
    {"room2pit", f_room2pit},
    {"room2testroom2", f_room2testroom2},
    {"room3tunnel", f_room3tunnel},
    {"room2toilets", f_room2toilets},
    {"room2storage", f_room2storage},
    {"room2sroom", f_room2sroom},
    {"room2shaft", f_room2shaft},
    {"room2poffices", f_room2poffices},
    {"room2poffices2", f_room2poffices2},
    {"room2elevator", f_room2elevator},
    {"room2cafeteria", f_room2cafeteria},
    {"room2nuke", f_room2nuke},
    {"room2tunnel", f_room2tunnel},
    {"008", f_008},
    {"room035", f_room035},
    {"room513", f_room513},
    {"room966", f_room966},
    {"room3storage", f_room3storage},
    {0},
};
