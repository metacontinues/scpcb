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
static int chance(int c) { return irand(0, 100) <= c; }
static int deviates(int px, int dir) { int cd = center - px; return (dir == 0 && cd >= max_deviation_distance) || (dir == 2 && cd <= -max_deviation_distance); }
static int turnif(int px, int dir) { return deviates(px, dir) ? (dir + 2) % 4 : dir; }

void GenForestGrid(Forest *fr) {
	fr->ID = ++LastForestID;
	int door1 = irand(3, 7), door2 = irand(3, 7);
	for (int i = 0; i < gridsize * gridsize; i++) fr->grid[i] = 0;
	gset(fr, door1, 3);
	gset(fr, (gridsize - 1) * gridsize + door2, 3);

	// the main path, from door2 up
	int px = door2, py = 1, dir = 1;
	gset(fr, CELL(py) + px, 1);
	while (py < gridsize - 4) {
		if (dir == 1) {
			if (chance(deviation_chance)) {
				dir = 2 * irand(0, 1);
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
		int bp = 2 * irand(0, 1), lm = gridsize, rm = 0;
		for (int i = 0; i <= gridsize; i++) if (gget(fr, CELL(ny) + i) == 1) { if (i < lm) lm = i; if (i > rm) rm = i; }
		nx = bp == 0 ? lm - 1 : rm + 1;
		if ((ty != 0 && gget(fr, CELL(ty - 1) + nx) == 1) || gget(fr, CELL(ty + 1) + nx) == 1) break; // blitz Exit leaves the whole loop
		gset(fr, CELL(ty) + nx, bt);
		nx = bp == 0 ? lm - 2 : rm + 2;
		gset(fr, CELL(ty) + nx, bt);
		for (int i = 2; i < branch_max_life;) {
			i++;
			if (chance(branch_die_chance)) break;
			if (irand(0, 3) == 0) nx += bp == 0 ? -1 : 1; else ty++;
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
	int x = image_width(hmap) - 1, y = image_height(hmap) - 1;
	scale_texture(t1, x / 4, y / 4);
	scale_texture(t2, x / 4, y / 4);
	scale_texture(mask, x, y);
	int mesh = create_mesh(0);
	surf *s = create_surface(mesh, 0);
	for (int ly = 0; ly <= y; ly++) for (int lx = 0; lx <= x; lx++) add_vertex(s, lx, 0, ly, 1.0f / lx, 1.0f / ly, 0);
	// (blitz called RenderWorld here, no visible effect)
	for (int ly = 0; ly < y; ly++) for (int lx = 0; lx < x; lx++) {
		int a = lx + (x + 1) * ly;
		add_triangle(s, a, a + x + 1, a + 1);
		add_triangle(s, a + 1, a + x + 1, a + x + 2);
	}
	int mesh2 = copy_mesh(mesh, mesh);
	surf *s2 = get_surface(mesh2, 1);
	position_mesh(mesh, -x / 2.0f, 0, -y / 2.0f);
	position_mesh(mesh2, -x / 2.0f, .01f, -y / 2.0f);
	int hb = image_buffer(hmap, 0), mb = texture_buffer(mask, 0), mw = texture_width(mask), mh = texture_height(mask);
	for (int lx = 0; lx <= x; lx++) for (int ly = 0; ly <= y; ly++) {
		float mx = Min(lx * (float)mw / image_width(hmap), mw - 1);
		float my = mh - Min(ly * (float)mh / image_height(hmap), mh - 1);
		int r = red(read_pixel(iround(Min(lx, x - 1)), iround(y - Min(ly, y - 1)), hb));
		float al = alpha1(read_pixel(iround(Max(mx - 5, 5)), iround(Max(my - 5, 5)), mb));
		al += alpha1(read_pixel(iround(Min(mx + 5, mw - 5)), iround(Min(my + 5, mh - 5)), mb));
		al += alpha1(read_pixel(iround(Max(mx - 5, 5)), iround(Min(my + 5, mh - 5)), mb));
		al += alpha1(read_pixel(iround(Min(mx + 5, mw - 5)), iround(Max(my - 5, 5)), mb));
		al = sqrtf(al * .25f);
		int i = lx + (x + 1) * ly;
		vertex_coords(s, i, vertex_x(s, i), r * yscale, vertex_z(s, i));
		vertex_coords(s2, i, vertex_x(s2, i), r * yscale, vertex_z(s2, i));
		vertex_color(s2, i, 255, 255, 255, al);
		vertex_tex_coords(s, i, lx, -ly, 0, 0);
		vertex_tex_coords(s2, i, lx, -ly, 0, 0);
	}
	update_normals(mesh);
	update_normals(mesh2);
	entity_texture(mesh, t1, 0, 0);
	entity_texture(mesh2, t2, 0, 0);
	entity_fx(mesh, 1);
	entity_fx(mesh2, 1 + 2 + 32);
	return mesh;
}

// trees/rocks scattered by the heightmap's brightness. every pixel rolls Rand, so this is rng heavy
static void details(Forest *fr, int tile, int hmap, float tempf3) {
	int hb = image_buffer(hmap, 0), width = image_width(hmap);
	float tempf4 = tempf3 / (float)width;
	for (int lx = 3; lx <= width - 2; lx++) for (int ly = 3; ly <= width - 2; ly++) {
		int cr = red(read_pixel(lx, width - ly, hb)); // GetColor + ColorRed
		if (cr <= irand(100, 260)) continue;
		int de = 0;
		float px = lx * tempf4 - tempf3 / 2.0f, pz = ly * tempf4 - tempf3 / 2.0f;
		int kind = irand(0, 7);
		if (kind <= 5) { // tree
			de = copy_entity(fr->DetailMesh[1], 0);
			float t2 = frnd(.25f, .4f);
			for (int i = 0; i <= 3; i++) {
				int d = copy_entity(fr->DetailMesh[4], 0);
				rotate_entity(d, 0, 90 * i + frnd(-20, 20), 0, 0);
				entity_parent(d, de, 1);
				entity_fx(d, 1);
			}
			scale_entity(de, t2 * 1.1f, t2, t2 * 1.1f, 1);
			position_entity(de, px, cr * .03f - frnd(3, 3.2f), pz, 1);
			rotate_entity(de, frnd(-5, 5), frnd(360, 0), 0, 1);
		} else if (kind == 6) { // stump
			de = copy_entity(fr->DetailMesh[4], 0);
			float t = frnd(.1f, .12f);
			scale_entity(de, t, t, t, 1);
			position_entity(de, px, cr * .03f - 1.3f, pz, 1);
		} else { // rock
			de = copy_entity(fr->DetailMesh[2], 0);
			position_entity(de, px, cr * .03f - 1.3f, pz, 1);
			entity_fx(de, 1);
			rotate_entity(de, 0, frnd(360, 0), 0, 1);
		}
		entity_fx(de, 1);
		entity_parent(de, tile, 1);
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
	if (fr->Forest_Pivot) { free_entity(fr->Forest_Pivot); fr->Forest_Pivot = 0; }
	for (int i = 0; i <= 3; i++) if (fr->TileMesh[i]) { free_entity(fr->TileMesh[i]); fr->TileMesh[i] = 0; }
	for (int i = 0; i <= 4; i++) if (fr->DetailMesh[i]) { free_entity(fr->DetailMesh[i]); fr->DetailMesh[i] = 0; }
	fr->Forest_Pivot = create_pivot(0);
	position_entity(fr->Forest_Pivot, x, y, z, 1);
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
	for (int i = ROOM1; i <= ROOM4; i++) hide_entity(fr->TileMesh[i]);
	for (int i = 1; i <= 5; i++) hide_entity(fr->DetailMesh[i]);
	float tempf3 = mesh_width(fr->TileMesh[ROOM1]), tempf1 = ts / tempf3;
	for (int tx = 1; tx <= gridsize - 1; tx++) for (int ty = 1; ty <= gridsize - 1; ty++) {
		if (gget(fr, ty * gridsize + tx) != 1) continue;
		int angle, type = tileshape(fr, tx, ty, &angle);
		if (!type) continue;
		int tile = copy_entity(fr->TileMesh[type], 0);
		int placed[5] = {0}; // blitz's Local itemPlaced[4] sits inside the loop, so it's fresh per tile
		Items *it = NULL;
		if (ty % 3 == 2 && !placed[ty / 3]) {
			placed[ty / 3] = 1;
			it = CreateItem(cat("Log #", s_str(ty / 3 + 1)), "paper", 0, .5f, 0, 0, 0, 0, 1, 0);
			entity_type(it->collider, HIT_ITEM, 0);
			entity_parent(it->collider, tile, 1);
		}
		details(fr, tile, hmap[type], tempf3);
		turn_entity(tile, 0, angle, 0, 0);
		position_entity(tile, x + tx * ts, y, z + ty * ts, 1);
		scale_entity(tile, tempf1, tempf1, tempf1, 0);
		entity_type(tile, HIT_MAP, 0);
		entity_fx(tile, 1);
		entity_parent(tile, fr->Forest_Pivot, 1);
		entity_pick_mode(tile, 2, 1);
		if (it) entity_parent(it->collider, 0, 1);
		fr->TileEntities[tx + ty * gridsize] = tile;
	}
	for (int i = 0; i <= 1; i++) {
		int ty = (gridsize - 1) * i;
		for (int tx = 1; tx <= gridsize - 1; tx++) {
			if (gget(fr, ty * gridsize + tx) != 3) continue;
			int de = fr->DetailEntities[i] = copy_entity(fr->DetailMesh[5], 0);
			scale_entity(de, RoomScale, RoomScale, RoomScale, 0);
			int door = fr->Door[i] = copy_entity(r->Objects[3], 0);
			position_entity(door, 72 * RoomScale, 32 * RoomScale, 0, 1);
			rotate_entity(door, 0, 180, 0, 0);
			scale_entity(door, 48 * RoomScale, 45 * RoomScale, 48 * RoomScale, 1);
			entity_parent(door, de, 1);
			int frame = copy_entity(r->Objects[2], door);
			position_entity(frame, 0, 32 * RoomScale, 0, 1);
			scale_entity(frame, 48 * RoomScale, 45 * RoomScale, 48 * RoomScale, 1);
			entity_parent(frame, de, 1);
			entity_type(de, HIT_MAP, 0);
			entity_pick_mode(de, 2, 1);
			position_entity(de, x + tx * ts, y, z + ty * ts + ts / 2 - ts * i, 1);
			rotate_entity(de, 0, 180 * i, 0, 0);
			entity_parent(de, fr->Forest_Pivot, 1);
		}
	}
}
