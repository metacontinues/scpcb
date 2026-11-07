// mapgen.c: MapSystem.bb's CreateMap, SetRoom, GetZone. ported statement by statement:
// the Rand/Rnd call order is what makes a seed give the same facility
#include "g.h"

#define T(x, y) (*MT(x, y))
static int m1(int v) { return v < 1 ? v : 1; } // Min(MapTemp(..), 1)

int GetZone(int y) { return iround(Min(floorf((float)(MapWidth - y) / MapWidth * ZONEAMOUNT), ZONEAMOUNT - 1)); }

int SetRoom(const char *name, int type, int pos, int min_pos, int max_pos) {
	if (max_pos < min_pos) return 0;
	int looped = 0;
	while (s_len(*MR(type, pos))) {
		pos++;
		if (pos > max_pos) {
			if (looped) return 0;
			pos = min_pos + 1; looped = 1;
		}
	}
	*MR(type, pos) = (str)name;
	return 1;
}

static void hallways(void) {
	int x = (int)floorf(MapWidth / 2), y = MapHeight - 2, temp = 0, tempheight = 0;
	for (int i = y; i <= MapHeight - 1; i++) T(x, i) = 1;
	do {
		int width = irand(10, 15);
		if (x > MapWidth * .6f) width = -width;
		else if (x > MapWidth * .4f) x = x - width / 2;
		if (x + width > MapWidth - 3) width = MapWidth - 3 - x;
		else if (x + width < 2) width = -x + 2;
		x = iround(Min(x, x + width));
		width = abs(width);
		for (int i = x; i <= x + width; i++) T(iround(Min(i, MapWidth)), y) = 1;
		int height = irand(3, 4);
		if (y - height < 1) height = y - 1;
		int yhallways = irand(4, 5);
		if (GetZone(y - height) != GetZone(y - height + 1)) height--;
		for (int i = 1; i <= yhallways; i++) {
			int x2 = iround(Max(Min(irand(x, x + width - 1), MapWidth - 2), 2));
			while (T(x2, y - 1) || T(x2 - 1, y - 1) || T(x2 + 1, y - 1)) x2++;
			if (x2 >= x + width) continue;
			if (i == 1) {
				tempheight = height;
				x2 = irand(1, 2) == 1 ? x : x + width;
			} else tempheight = irand(1, height);
			for (int y2 = y - tempheight; y2 <= y; y2++) T(x2, y2) = GetZone(y2) != GetZone(y2 + 1) ? 255 : 1;
			if (tempheight == height) temp = x2;
		}
		x = temp;
		y -= height;
	} while (y >= 2);
}

static int R1[4], R2[4], R2C[4], R3[4], R4[4]; // Room1Amount.. per zone

static void countrooms(void) {
	memset(R1, 0, sizeof R1); memset(R2, 0, sizeof R2); memset(R2C, 0, sizeof R2C); memset(R3, 0, sizeof R3); memset(R4, 0, sizeof R4);
	for (int y = 1; y <= MapHeight - 1; y++) {
		int zone = GetZone(y);
		for (int x = 1; x <= MapWidth - 1; x++) {
			if (T(x, y) <= 0) continue;
			int temp = m1(T(x + 1, y)) + m1(T(x - 1, y)) + m1(T(x, y + 1)) + m1(T(x, y - 1));
			if (T(x, y) < 255) T(x, y) = temp;
			switch (T(x, y)) {
			case 1: R1[zone]++; break;
			case 2:
				if (m1(T(x + 1, y)) + m1(T(x - 1, y)) == 2 || m1(T(x, y + 1)) + m1(T(x, y - 1)) == 2) R2[zone]++;
				else R2C[zone]++;
				break;
			case 3: R3[zone]++; break;
			case 4: R4[zone]++; break;
			}
		}
	}
}

static void morerooms1(void) {
	int zoneamount = 3, x2 = 0, y2 = 0;
	for (int i = 0; i <= 2; i++) {
		int temp = -R1[i] + 5;
		if (temp <= 0) continue;
		int y0 = (MapHeight / zoneamount) * (2 - i) + 1, y1 = iround((MapHeight / zoneamount) * ((2 - i) + 1.0f) - 2);
		for (int y = y0; y <= y1; y++) {
			for (int x = 2; x <= MapWidth - 2; x++) {
				if (T(x, y) == 0 && m1(T(x + 1, y)) + m1(T(x - 1, y)) + m1(T(x, y + 1)) + m1(T(x, y - 1)) == 1) {
					if (T(x + 1, y)) { x2 = x + 1; y2 = y; }
					else if (T(x - 1, y)) { x2 = x - 1; y2 = y; }
					else if (T(x, y + 1)) { x2 = x; y2 = y + 1; }
					else if (T(x, y - 1)) { x2 = x; y2 = y - 1; }
					int placed = 0;
					if (T(x2, y2) > 1 && T(x2, y2) < 4) {
						if (T(x2, y2) == 2) {
							if (m1(T(x2 + 1, y2)) + m1(T(x2 - 1, y2)) == 2 || m1(T(x2, y2 + 1)) + m1(T(x2, y2 - 1)) == 2) { R2[i]--; R3[i]++; placed = 1; }
						} else { R3[i]--; R4[i]++; placed = 1; }
						if (placed) { T(x2, y2)++; T(x, y) = 1; R1[i]++; temp--; }
					}
				}
				if (temp == 0) break;
			}
			if (temp == 0) break;
		}
	}
}

// one of the 4 mirrored "can a ROOM2C go here" checks. (dx, dy) points away from the existing neighbour
static int try2c(int x, int y, int dx, int dy) {
	// side pieces: sx/sy is perpendicular to (dx, dy)
	int px = dy != 0, py = dx != 0; // perpendicular axis
	if (T(x - px, y - py) + T(x + px, y + py) + T(x + 2 * dx, y + 2 * dy) != 0) return 0;
	for (int s = -1; s <= 1; s += 2) {
		int cx = x + dx + s * px, cy = y + dy + s * py; // the new ROOM1 next to the corner
		if (T(cx + s * px, cy + s * py) + T(cx + dx, cy + dy) + T(cx, cy) == 0) {
			T(x, y) = 2; T(x + dx, y + dy) = 2; T(cx, cy) = 1;
			return 1;
		}
	}
	return 0;
}

static void morerooms4and2c(void) {
	int zone = 0, temp2 = 0;
	for (int i = 0; i <= 2; i++) {
		if (i == 2) { zone = 2; temp2 = MapHeight / 3; }
		else if (i == 1) { zone = MapHeight / 3 + 1; temp2 = iround(MapHeight * (2.0f / 3.0f) - 1); }
		else { zone = iround(MapHeight * (2.0f / 3.0f) + 1); temp2 = MapHeight - 2; }

		if (R4[i] < 1) { // at least 1 ROOM4
			int temp = 0;
			for (int y = zone; y <= temp2 && !temp; y++)
				for (int x = 2; x <= MapWidth - 2 && !temp; x++) {
					if (T(x, y) != 3) continue;
					// Select 0: the first neighbour slot whose area is all empty gets a ROOM1
					if ((T(x + 1, y) | T(x + 1, y + 1) | T(x + 1, y - 1) | T(x + 2, y)) == 0) { T(x + 1, y) = 1; temp = 1; }
					else if ((T(x - 1, y) | T(x - 1, y + 1) | T(x - 1, y - 1) | T(x - 2, y)) == 0) { T(x - 1, y) = 1; temp = 1; }
					else if ((T(x, y + 1) | T(x + 1, y + 1) | T(x - 1, y + 1) | T(x, y + 2)) == 0) { T(x, y + 1) = 1; temp = 1; }
					else if ((T(x, y - 1) | T(x + 1, y - 1) | T(x - 1, y - 1) | T(x, y - 2)) == 0) { T(x, y - 1) = 1; temp = 1; }
					if (temp) { T(x, y) = 4; R4[i]++; R3[i]--; R1[i]++; }
				}
		}

		if (R2C[i] < 1) { // at least 1 ROOM2C
			int temp = 0;
			zone++; temp2--;
			for (int y = zone; y <= temp2 && !temp; y++)
				for (int x = 3; x <= MapWidth - 3 && !temp; x++) {
					if (T(x, y) != 1) continue;
					// Select True: only the first neighbour that exists is tried
					if (T(x - 1, y) > 0) temp = try2c(x, y, 1, 0);
					else if (T(x + 1, y) > 0) temp = try2c(x, y, -1, 0);
					else if (T(x, y - 1) > 0) temp = try2c(x, y, 0, 1);
					else if (T(x, y + 1) > 0) temp = try2c(x, y, 0, -1);
					if (temp) { R2C[i]++; R2[i]++; }
				}
		}
	}
}

#define F(k, a) ((int)floorf((k) * (float)(a)))

static void assignrooms(void) {
	int mr = 55 * MapWidth / 20;
	mr = Max(mr, R1[0] + R1[1] + R1[2] + 1); mr = Max(mr, R2[0] + R2[1] + R2[2] + 1);
	mr = Max(mr, R2C[0] + R2C[1] + R2C[2] + 1); mr = Max(mr, R3[0] + R3[1] + R3[2] + 1);
	mr = Max(mr, R4[0] + R4[1] + R4[2] + 1);
	MaxRooms_ = mr;
	free(MapRoom); MapRoom = calloc((ROOM4 + 2) * (mr + 1), sizeof(str)); // points at literals / template names, owns nothing

	// zone 1
	int lo = 1, hi = R1[0] - 1;
	*MR(ROOM1, 0) = "start";
	SetRoom("roompj", ROOM1, F(.1f, R1[0]), lo, hi);
	SetRoom("914", ROOM1, F(.3f, R1[0]), lo, hi);
	SetRoom("room1archive", ROOM1, F(.5f, R1[0]), lo, hi);
	SetRoom("room205", ROOM1, F(.6f, R1[0]), lo, hi);
	*MR(ROOM2C, 0) = "lockroom";
	lo = 1; hi = R2[0] - 1;
	*MR(ROOM2, 0) = "room2closets";
	static const struct { const char *n; float k; } z1[] = {
		{"room2testroom2", .1f}, {"room2scps", .2f}, {"room2storage", .3f}, {"room2gw_b", .4f}, {"room2sl", .5f},
		{"room012", .55f}, {"room2scps2", .6f}, {"room1123", .7f}, {"room2elevator", .85f}};
	for (int i = 0; i < 9; i++) SetRoom(z1[i].n, ROOM2, F(z1[i].k, R2[0]), lo, hi);
	*MR(ROOM3, (int)floorf(frnd(.2f, .8f) * (float)R3[0])) = "room3storage";
	*MR(ROOM2C, F(.5f, R2C[0])) = "room1162";
	*MR(ROOM4, F(.3f, R4[0])) = "room4info";

	// zone 2
	lo = R1[0]; hi = R1[0] + R1[1] - 1;
	static const struct { const char *n; float k; } z2a[] = {{"room079", .15f}, {"room106", .3f}, {"008", .4f}, {"room035", .5f}, {"coffin", .7f}};
	for (int i = 0; i < 5; i++) SetRoom(z2a[i].n, ROOM1, R1[0] + F(z2a[i].k, R1[1]), lo, hi);
	lo = R2[0]; hi = R2[0] + R2[1] - 1;
	*MR(ROOM2, R2[0] + F(.1f, R2[1])) = "room2nuke";
	static const struct { const char *n; float k; } z2b[] = {{"room2tunnel", .25f}, {"room049", .4f}, {"room2shaft", .6f}, {"testroom", .7f}, {"room2servers", .9f}};
	for (int i = 0; i < 5; i++) SetRoom(z2b[i].n, ROOM2, R2[0] + F(z2b[i].k, R2[1]), lo, hi);
	*MR(ROOM3, R3[0] + F(.3f, R3[1])) = "room513";
	*MR(ROOM3, R3[0] + F(.6f, R3[1])) = "room966";
	*MR(ROOM2C, R2C[0] + F(.5f, R2C[1])) = "room2cpit";

	// zone 3
	*MR(ROOM1, R1[0] + R1[1] + R1[2] - 2) = "exit1";
	*MR(ROOM1, R1[0] + R1[1] + R1[2] - 1) = "gateaentrance";
	*MR(ROOM1, R1[0] + R1[1]) = "room1lifts";
	lo = R2[0] + R2[1]; hi = R2[0] + R2[1] + R2[2] - 1;
	*MR(ROOM2, lo + F(.1f, R2[2])) = "room2poffices";
	static const struct { const char *n; float k; } z3[] = {
		{"room2cafeteria", .2f}, {"room2sroom", .3f}, {"room2servers2", .4f}, {"room2offices", .45f}, {"room2offices4", .5f},
		{"room860", .6f}, {"medibay", .7f}, {"room2poffices2", .8f}, {"room2offices2", .9f}};
	for (int i = 0; i < 9; i++) SetRoom(z3[i].n, ROOM2, lo + F(z3[i].k, R2[2]), lo, hi);
	*MR(ROOM2C, R2C[0] + R2C[1]) = "room2ccont";
	*MR(ROOM2C, R2C[0] + R2C[1] + 1) = "lockroom2";
	*MR(ROOM3, R3[0] + R3[1] + F(.3f, R3[2])) = "room3servers";
	*MR(ROOM3, R3[0] + R3[1] + F(.7f, R3[2])) = "room3servers2";
	*MR(ROOM3, R3[0] + R3[1] + F(.5f, R3[2])) = "room3offices";
}

// MapName(x, y) gets the next planned room of that shape, if any
static str nextname(int x, int y, int shape) {
	if (MapRoomID[shape] < MaxRooms_ && !s_len(*MN(x, y)) && s_len(*MR(shape, MapRoomID[shape]))) *MN(x, y) = *MR(shape, MapRoomID[shape]);
	return *MN(x, y);
}
static void turn(Rooms *r, int a) { r->angle = a; turn_entity(r->obj, 0, a, 0, 0); }

static void createrooms(void) {
	for (int y = MapHeight - 1; y >= 1; y--) {
		int zone = y < MapHeight / 3 + 1 ? 3 : y < MapHeight * (2.0f / 3.0f) ? 2 : 1;
		for (int x = 1; x <= MapWidth - 2; x++) {
			float X = x * 8, Z = y * 8;
			if (T(x, y) == 255) { CreateRoom(zone, ROOM2, X, 0, Z, y > MapHeight / 2 ? "checkpoint1" : "checkpoint2"); continue; }
			if (T(x, y) <= 0) continue;
			int temp = m1(T(x + 1, y)) + m1(T(x - 1, y)) + m1(T(x, y + 1)) + m1(T(x, y - 1));
			Rooms *r;
			switch (temp) {
			case 1:
				r = CreateRoom(zone, ROOM1, X, 0, Z, nextname(x, y, ROOM1));
				if (T(x, y + 1)) turn(r, 180);
				else if (T(x - 1, y)) turn(r, 270);
				else if (T(x + 1, y)) turn(r, 90);
				else r->angle = 0;
				MapRoomID[ROOM1]++;
				break;
			case 2:
				if (T(x - 1, y) > 0 && T(x + 1, y) > 0) {
					r = CreateRoom(zone, ROOM2, X, 0, Z, nextname(x, y, ROOM2));
					turn(r, irand(1, 2) == 1 ? 90 : 270);
					MapRoomID[ROOM2]++;
				} else if (T(x, y - 1) > 0 && T(x, y + 1) > 0) {
					r = CreateRoom(zone, ROOM2, X, 0, Z, nextname(x, y, ROOM2));
					turn(r, irand(1, 2) == 1 ? 180 : 0);
					MapRoomID[ROOM2]++;
				} else {
					str n = nextname(x, y, ROOM2C);
					r = CreateRoom(zone, ROOM2C, X, 0, Z, n);
					if (T(x - 1, y) > 0 && T(x, y + 1) > 0) turn(r, 180);
					else if (T(x + 1, y) > 0 && T(x, y + 1) > 0) turn(r, 90);
					else if (T(x - 1, y) > 0 && T(x, y - 1) > 0) turn(r, 270);
					MapRoomID[ROOM2C]++;
				}
				break;
			case 3:
				r = CreateRoom(zone, ROOM3, X, 0, Z, nextname(x, y, ROOM3));
				if (!T(x, y - 1)) turn(r, 180);
				else if (!T(x - 1, y)) turn(r, 90);
				else if (!T(x + 1, y)) { turn_entity(r->obj, 0, -90, 0, 0); r->angle = 270; }
				MapRoomID[ROOM3]++;
				break;
			case 4:
				CreateRoom(zone, ROOM4, X, 0, Z, nextname(x, y, ROOM4));
				MapRoomID[ROOM4]++;
				break;
			}
		}
	}
	CreateRoom(0, ROOM1, (MapWidth - 1) * 8, 500, 8, "gatea"); MapRoomID[ROOM1]++;
	CreateRoom(0, ROOM1, (MapWidth - 1) * 8, 0, (MapHeight - 1) * 8, "pocketdimension"); MapRoomID[ROOM1]++;
	if (IntroEnabled) { CreateRoom(0, ROOM1, 8, 0, (MapHeight - 1) * 8, "173"); MapRoomID[ROOM1]++; }
	CreateRoom(0, ROOM1, 8, 800, 0, "dimension1499"); MapRoomID[ROOM1]++;
	each(Rooms, r) PreventRoomOverlap(r);
}

// which sides of a room shape get the door between it and the next cell: +x (side 0) and +z (side 3)
static int doorx(Rooms *r) {
	int a = r->angle;
	switch (r->RoomTemplate ? r->RoomTemplate->Shape : 0) {
	case ROOM1: return a == 90;
	case ROOM2: return a == 90 || a == 270;
	case ROOM2C: return a == 0 || a == 90;
	case ROOM3: return a == 0 || a == 180 || a == 90;
	default: return 1;
	}
}
static int doorz(Rooms *r) {
	int a = r->angle;
	switch (r->RoomTemplate ? r->RoomTemplate->Shape : 0) {
	case ROOM1: return a == 180;
	case ROOM2: return a == 0 || a == 180;
	case ROOM2C: return a == 180 || a == 90;
	case ROOM3: return a == 180 || a == 90 || a == 270;
	default: return 1;
	}
}

static void createdoors(void) {
	float sp = 8;
	for (int y = MapHeight; y >= 0; y--) {
		int zone = y < I_Zone->Transition[1] - 1 ? 3 : y < I_Zone->Transition[0] - 1 ? 2 : 1;
		for (int x = MapWidth; x >= 0; x--) {
			if (T(x, y) <= 0) continue;
			int big = zone == 2 ? 2 : 0;
			each(Rooms, r) {
				r->angle = iround(WrapAngle(r->angle));
				if (iround(r->x / 8.0f) != x || iround(r->z / 8.0f) != y) continue;
				if (doorx(r) && x + 1 < MapWidth + 1 && T(x + 1, y) > 0)
					r->AdjDoor[0] = CreateDoor(r->zone, x * sp + sp / 2, 0, y * sp, 90, r, iround(Max(irand(-3, 1), 0)), big, 0, "", 0);
				if (doorz(r) && y + 1 < MapHeight + 1 && T(x, y + 1) > 0)
					r->AdjDoor[3] = CreateDoor(r->zone, x * sp, 0, y * sp + sp / 2, 0, r, iround(Max(irand(-3, 1), 0)), big, 0, "", 0);
				break;
			}
		}
	}
}

static void adjacency(void) {
	each(Rooms, r) {
		r->angle = iround(WrapAngle(r->angle));
		for (int k = 0; k < 4; k++) r->Adjacent[k] = NULL;
		each(Rooms, r2) {
			if (r != r2) {
				if (r2->z == r->z) {
					if (r2->x == r->x + 8) { r->Adjacent[0] = r2; if (!r->AdjDoor[0]) r->AdjDoor[0] = r2->AdjDoor[2]; }
					else if (r2->x == r->x - 8) { r->Adjacent[2] = r2; if (!r->AdjDoor[2]) r->AdjDoor[2] = r2->AdjDoor[0]; }
				} else if (r2->x == r->x) {
					if (r2->z == r->z - 8) { r->Adjacent[1] = r2; if (!r->AdjDoor[1]) r->AdjDoor[1] = r2->AdjDoor[3]; }
					else if (r2->z == r->z + 8) { r->Adjacent[3] = r2; if (!r->AdjDoor[3]) r->AdjDoor[3] = r2->AdjDoor[1]; }
				}
			}
			if (r->Adjacent[0] && r->Adjacent[1] && r->Adjacent[2] && r->Adjacent[3]) break;
		}
	}
}

void CreateMap(void) {
	I_Zone->Transition[0] = 13; I_Zone->Transition[1] = 7;
	I_Zone->HasCustomForest = I_Zone->HasCustomMT = 0;
	seed_rnd(GenerateSeedNumber(RandomSeed));
	free(MapName); MapName = calloc((MapWidth + 1) * (MapHeight + 1), sizeof(str)); // same
	memset(MapRoomID, 0, sizeof MapRoomID);
	hallways();
	countrooms();
	morerooms1();
	morerooms4and2c();
	assignrooms();
	createrooms();
	for (int y = 0; y <= MapHeight; y++) for (int x = 0; x <= MapWidth; x++) T(x, y) = m1(T(x, y));
	createdoors();
	adjacency();
}
