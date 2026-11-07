// forest.c: MapSystem.bb's SCP-860-1 forest: GenForestGrid, PlaceForest, load_terrain
#include "g.h"

TLIST(Forest);
static int LastForestID;
enum { deviation_chance = 40, branch_chance = 65, branch_max_life = 4, branch_die_chance = 18,
	max_deviation_distance = 3, return_chance = 27, center = 5 };
static int cobble_chance; // never had a value upstream, so paths get no cobbles

#define GN (gridsize * gridsize + 12)
// blitz didn't bounds check in release builds; out of range reads give 0 and writes are dropped here
static int gget(Forest *f, int i) { return i >= 0 && i < GN ? f->grid[i] : 0; }
static void gset(Forest *f, int i, int v) { if (i >= 0 && i < GN) f->grid[i] = v; }
#define CELL(y) ((gridsize - 1 - (y)) * gridsize)

static int fwdx(int dir, int px) { return dir == 1 ? px : px - 1 + dir; }
static int fwdy(int dir, int py) { return dir == 1 ? py + 1 : py; }
static int chance(int c) { return bbRand(0, 100) <= c; }
static int deviates(int px, int dir) { int cd = center - px; return (dir == 0 && cd >= max_deviation_distance) || (dir == 2 && cd <= -max_deviation_distance); }
static int turnif(int px, int dir) { return deviates(px, dir) ? (dir + 2) % 4 : dir; }

void GenForestGrid(Forest *fr) {
	fr->ID = ++LastForestID;
	int door1 = bbRand(3, 7), door2 = bbRand(3, 7);
	for (int i = 0; i < gridsize * gridsize; i++) fr->grid[i] = 0;
	gset(fr, door1, 3);
	gset(fr, (gridsize - 1) * gridsize + door2, 3);

	// the main path, from door2 up
	int px = door2, py = 1, dir = 1;
	gset(fr, CELL(py) + px, 1);
	while (py < gridsize - 4) {
		if (dir == 1) {
			if (chance(deviation_chance)) {
				dir = 2 * bbRand(0, 1);
				dir = turnif(px, dir);
				if (deviates(px, dir)) gset(fr, CELL(py) + px, 1);
				px = fwdx(dir, px); py = fwdy(dir, py);
			}
		} else {
			dir = turnif(px, dir);
			int dev = deviates(px, dir);
			if (dev || chance(return_chance)) dir = 1;
			px = fwdx(dir, px); py = fwdy(dir, py);
			if (dir == 1) { // going forward again: step twice, no 2x2 blobs
				gset(fr, CELL(py) + px, 1);
				px = fwdx(dir, px); py = fwdy(dir, py);
			}
		}
		gset(fr, CELL(py) + px, 1);
	}
	dir = 1;
	while (py < gridsize - 2) { px = fwdx(dir, px); py = fwdy(dir, py); gset(fr, CELL(py) + px, 1); }
	if (px != door1) {
		dir = door1 > px ? 2 : 0;
		while (px != door1) { px = fwdx(dir, px); py = fwdy(dir, py); gset(fr, CELL(py) + px, 1); }
	}

	// dead-end branches, considered every 4 rows
	for (int ny = -3; ny < gridsize - 6;) {
		ny += 4;
		int ty = ny, nx = 0;
		if (!chance(branch_chance)) continue;
		int bt = -1;
		if (chance(cobble_chance)) bt = -2;
		int bp = 2 * bbRand(0, 1), lm = gridsize, rm = 0;
		for (int i = 0; i <= gridsize; i++) if (gget(fr, CELL(ny) + i) == 1) { if (i < lm) lm = i; if (i > rm) rm = i; }
		nx = bp == 0 ? lm - 1 : rm + 1;
		if ((ty != 0 && gget(fr, CELL(ty - 1) + nx) == 1) || gget(fr, CELL(ty + 1) + nx) == 1) break; // blitz Exit leaves the whole loop
		gset(fr, CELL(ty) + nx, bt);
		nx = bp == 0 ? lm - 2 : rm + 2;
		gset(fr, CELL(ty) + nx, bt);
		for (int i = 2; i < branch_max_life;) {
			i++;
			if (chance(branch_die_chance)) break;
			if (bbRand(0, 3) == 0) nx += bp == 0 ? -1 : 1; else ty++;
			int n = CELL(ty - 1) + nx;
			if (n < gridsize - 1 && ty != 0 && gget(fr, n) == 1) break;
			n = CELL(ty + 1) + nx;
			if (n > 0 && gget(fr, n) == 1) break;
			gset(fr, CELL(ty) + nx, bt);
			if (ty >= gridsize - 2) break;
		}
	}
	for (int i = 0; i < gridsize * gridsize; i++) if (fr->grid[i] == -1 || fr->grid[i] == -2) fr->grid[i] = 1;
}

static int red(int argb) { return (argb >> 16) & 255; }
static float alpha1(int argb) { return ((unsigned)argb >> 24) / 255.0f; }

// heightmap -> terrain: mesh w/ t1, a child copy 0.01 above w/ t2 faded in by the mask's alpha
static int load_terrain(int hmap, float yscale, int t1, int t2, int mask) {
	if (!hmap) RuntimeError("Heightmap image does not exist.");
	if (!t1 || !t2 || !mask) RuntimeError("load_terrain error: invalid texture");
	int x = bbImageWidth(hmap) - 1, y = bbImageHeight(hmap) - 1;
	bbScaleTexture(t1, x / 4, y / 4);
	bbScaleTexture(t2, x / 4, y / 4);
	bbScaleTexture(mask, x, y);
	int mesh = bbCreateMesh(0);
	surf *s = bbCreateSurface(mesh, 0);
	for (int ly = 0; ly <= y; ly++) for (int lx = 0; lx <= x; lx++) bbAddVertex(s, lx, 0, ly, 1.0f / lx, 1.0f / ly, 0);
	// (blitz called RenderWorld here, no visible effect)
	for (int ly = 0; ly < y; ly++) for (int lx = 0; lx < x; lx++) {
		int a = lx + (x + 1) * ly;
		bbAddTriangle(s, a, a + x + 1, a + 1);
		bbAddTriangle(s, a + 1, a + x + 1, a + x + 2);
	}
	int mesh2 = bbCopyMesh(mesh, mesh);
	surf *s2 = bbGetSurface(mesh2, 1);
	bbPositionMesh(mesh, -x / 2.0f, 0, -y / 2.0f);
	bbPositionMesh(mesh2, -x / 2.0f, .01f, -y / 2.0f);
	int hb = bbImageBuffer(hmap, 0), mb = bbTextureBuffer(mask, 0), mw = bbTextureWidth(mask), mh = bbTextureHeight(mask);
	for (int lx = 0; lx <= x; lx++) for (int ly = 0; ly <= y; ly++) {
		float mx = Min(lx * (float)mw / bbImageWidth(hmap), mw - 1);
		float my = mh - Min(ly * (float)mh / bbImageHeight(hmap), mh - 1);
		int r = red(bbReadPixel(bbInt(Min(lx, x - 1)), bbInt(y - Min(ly, y - 1)), hb));
		float al = alpha1(bbReadPixel(bbInt(Max(mx - 5, 5)), bbInt(Max(my - 5, 5)), mb));
		al += alpha1(bbReadPixel(bbInt(Min(mx + 5, mw - 5)), bbInt(Min(my + 5, mh - 5)), mb));
		al += alpha1(bbReadPixel(bbInt(Max(mx - 5, 5)), bbInt(Min(my + 5, mh - 5)), mb));
		al += alpha1(bbReadPixel(bbInt(Min(mx + 5, mw - 5)), bbInt(Max(my - 5, 5)), mb));
		al = sqrtf(al * .25f);
		int i = lx + (x + 1) * ly;
		bbVertexCoords(s, i, bbVertexX(s, i), r * yscale, bbVertexZ(s, i));
		bbVertexCoords(s2, i, bbVertexX(s2, i), r * yscale, bbVertexZ(s2, i));
		bbVertexColor(s2, i, 255, 255, 255, al);
		bbVertexTexCoords(s, i, lx, -ly, 0, 0);
		bbVertexTexCoords(s2, i, lx, -ly, 0, 0);
	}
	bbUpdateNormals(mesh);
	bbUpdateNormals(mesh2);
	bbEntityTexture(mesh, t1, 0, 0);
	bbEntityTexture(mesh2, t2, 0, 0);
	bbEntityFX(mesh, 1);
	bbEntityFX(mesh2, 1 + 2 + 32);
	return mesh;
}

// trees/rocks scattered by the heightmap's brightness. every pixel rolls Rand, so this is rng heavy
static void details(Forest *fr, int tile, int hmap, float tempf3) {
	int hb = bbImageBuffer(hmap, 0), width = bbImageWidth(hmap);
	float tempf4 = tempf3 / (float)width;
	for (int lx = 3; lx <= width - 2; lx++) for (int ly = 3; ly <= width - 2; ly++) {
		int cr = red(bbReadPixel(lx, width - ly, hb)); // GetColor + ColorRed
		if (cr <= bbRand(100, 260)) continue;
		int de = 0;
		float px = lx * tempf4 - tempf3 / 2.0f, pz = ly * tempf4 - tempf3 / 2.0f;
		int kind = bbRand(0, 7);
		if (kind <= 5) { // tree
			de = bbCopyEntity(fr->DetailMesh[1], 0);
			float t2 = bbRnd(.25f, .4f);
			for (int i = 0; i <= 3; i++) {
				int d = bbCopyEntity(fr->DetailMesh[4], 0);
				bbRotateEntity(d, 0, 90 * i + bbRnd(-20, 20), 0, 0);
				bbEntityParent(d, de, 1);
				bbEntityFX(d, 1);
			}
			bbScaleEntity(de, t2 * 1.1f, t2, t2 * 1.1f, 1);
			bbPositionEntity(de, px, cr * .03f - bbRnd(3, 3.2f), pz, 1);
			bbRotateEntity(de, bbRnd(-5, 5), bbRnd(360, 0), 0, 1);
		} else if (kind == 6) { // stump
			de = bbCopyEntity(fr->DetailMesh[4], 0);
			float t = bbRnd(.1f, .12f);
			bbScaleEntity(de, t, t, t, 1);
			bbPositionEntity(de, px, cr * .03f - 1.3f, pz, 1);
		} else { // rock
			de = bbCopyEntity(fr->DetailMesh[2], 0);
			bbPositionEntity(de, px, cr * .03f - 1.3f, pz, 1);
			bbEntityFX(de, 1);
			bbRotateEntity(de, 0, bbRnd(360, 0), 0, 1);
		}
		bbEntityFX(de, 1);
		bbEntityParent(de, tile, 1);
	}
}

static int tileshape(Forest *fr, int tx, int ty, int *angle) {
	int *g = fr->grid, c = ty * gridsize + tx;
	int t = 0;
	if (tx + 1 < gridsize) t = gget(fr, c + 1) > 0;
	if (tx - 1 >= 0) t += gget(fr, c - 1) > 0;
	if (ty + 1 < gridsize) t += gget(fr, c + gridsize) > 0;
	if (ty - 1 >= 0) t += gget(fr, c - gridsize) > 0;
	(void)g;
	int up = gget(fr, c + gridsize) > 0, dn = gget(fr, c - gridsize) > 0, l = gget(fr, c - 1) > 0, rt = gget(fr, c + 1) > 0;
	*angle = 0;
	switch (t) {
	case 1: *angle = up ? 180 : l ? 270 : rt ? 90 : 0; return ROOM1;
	case 2:
		if (dn && up) return ROOM2;
		if (rt && l) { *angle = 90; return ROOM2; }
		*angle = l && up ? 180 : rt && dn ? 0 : l && dn ? 270 : 90;
		return ROOM2C;
	case 3: *angle = !gget(fr, c - gridsize) ? 180 : !gget(fr, c - 1) ? 90 : !gget(fr, c + 1) ? 270 : 0; return ROOM3;
	case 4: return ROOM4;
	}
	return 0;
}

void PlaceForest(Forest *fr, float x, float y, float z, Rooms *r) {
	float ts = 12;
	if (fr->Forest_Pivot) { bbFreeEntity(fr->Forest_Pivot); fr->Forest_Pivot = 0; }
	for (int i = 0; i <= 3; i++) if (fr->TileMesh[i]) { bbFreeEntity(fr->TileMesh[i]); fr->TileMesh[i] = 0; }
	for (int i = 0; i <= 4; i++) if (fr->DetailMesh[i]) { bbFreeEntity(fr->DetailMesh[i]); fr->DetailMesh[i] = 0; }
	fr->Forest_Pivot = bbCreatePivot(0);
	bbPositionEntity(fr->Forest_Pivot, x, y, z, 1);
	int hmap[ROOM4 + 1], mask[ROOM4 + 1];
	int ground = LoadTexture_Strict("GFX\\map\\forest\\forestfloor.jpg", 1), path = LoadTexture_Strict("GFX\\map\\forest\\forestpath.jpg", 1);
	static const char *hn[] = {0, "forest1h", "forest2h", "forest2Ch", "forest3h", "forest4h"};
	for (int i = ROOM1; i <= ROOM4; i++) {
		hmap[i] = LoadImage_Strict(cat("GFX\\map\\forest\\", hn[i], ".png"));
		mask[i] = LoadTexture_Strict(cat("GFX\\map\\forest\\", hn[i], "_mask.png"), 1 + 2);
	}
	for (int i = ROOM1; i <= ROOM4; i++) fr->TileMesh[i] = load_terrain(hmap[i], .03f, ground, path, mask[i]);
	static const char *dn[] = {0, "detail\\treetest4", "detail\\rock", "detail\\rock2", "detail\\treetest5", "wall"};
	for (int i = 1; i <= 5; i++) fr->DetailMesh[i] = LoadMesh_Strict(cat("GFX\\map\\forest\\", dn[i], ".b3d"), 0);
	for (int i = ROOM1; i <= ROOM4; i++) bbHideEntity(fr->TileMesh[i]);
	for (int i = 1; i <= 5; i++) bbHideEntity(fr->DetailMesh[i]);
	float tempf3 = bbMeshWidth(fr->TileMesh[ROOM1]), tempf1 = ts / tempf3;
	for (int tx = 1; tx <= gridsize - 1; tx++) for (int ty = 1; ty <= gridsize - 1; ty++) {
		if (gget(fr, ty * gridsize + tx) != 1) continue;
		int angle, type = tileshape(fr, tx, ty, &angle);
		if (!type) continue;
		int tile = bbCopyEntity(fr->TileMesh[type], 0);
		int placed[5] = {0}; // blitz's Local itemPlaced[4] sits inside the loop, so it's fresh per tile
		Items *it = NULL;
		if (ty % 3 == 2 && !placed[ty / 3]) {
			placed[ty / 3] = 1;
			it = CreateItem(cat("Log #", bbStr(ty / 3 + 1)), "paper", 0, .5f, 0, 0, 0, 0, 1, 0);
			bbEntityType(it->collider, HIT_ITEM, 0);
			bbEntityParent(it->collider, tile, 1);
		}
		details(fr, tile, hmap[type], tempf3);
		bbTurnEntity(tile, 0, angle, 0, 0);
		bbPositionEntity(tile, x + tx * ts, y, z + ty * ts, 1);
		bbScaleEntity(tile, tempf1, tempf1, tempf1, 0);
		bbEntityType(tile, HIT_MAP, 0);
		bbEntityFX(tile, 1);
		bbEntityParent(tile, fr->Forest_Pivot, 1);
		bbEntityPickMode(tile, 2, 1);
		if (it) bbEntityParent(it->collider, 0, 1);
		fr->TileEntities[tx + ty * gridsize] = tile;
	}
	for (int i = 0; i <= 1; i++) {
		int ty = (gridsize - 1) * i;
		for (int tx = 1; tx <= gridsize - 1; tx++) {
			if (gget(fr, ty * gridsize + tx) != 3) continue;
			int de = fr->DetailEntities[i] = bbCopyEntity(fr->DetailMesh[5], 0);
			bbScaleEntity(de, RoomScale, RoomScale, RoomScale, 0);
			int door = fr->Door[i] = bbCopyEntity(r->Objects[3], 0);
			bbPositionEntity(door, 72 * RoomScale, 32 * RoomScale, 0, 1);
			bbRotateEntity(door, 0, 180, 0, 0);
			bbScaleEntity(door, 48 * RoomScale, 45 * RoomScale, 48 * RoomScale, 1);
			bbEntityParent(door, de, 1);
			int frame = bbCopyEntity(r->Objects[2], door);
			bbPositionEntity(frame, 0, 32 * RoomScale, 0, 1);
			bbScaleEntity(frame, 48 * RoomScale, 45 * RoomScale, 48 * RoomScale, 1);
			bbEntityParent(frame, de, 1);
			bbEntityType(de, HIT_MAP, 0);
			bbEntityPickMode(de, 2, 1);
			bbPositionEntity(de, x + tx * ts, y, z + ty * ts + ts / 2 - ts * i, 1);
			bbRotateEntity(de, 0, 180 * i, 0, 0);
			bbEntityParent(de, fr->Forest_Pivot, 1);
		}
	}
}
