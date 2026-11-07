// fillroom3.c: FillRoom's per-room cases room2ccont .. dimension1499 (MapSystem.bb's big Select, same order)
// drafted w/ a throwaway translator, then reviewed by hand against the .bb.

#include "g.h"

#define U __attribute__((unused))

// room2ccont
static void f_room2ccont(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x + 64.0f * RoomScale, 0.0f, r->z + 368.0f * RoomScale, 180, r, 0, 0, 2, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	it = CreateItem("Note from Daniel", "paper", r->x - 400.0f * RoomScale, 1040.0f * RoomScale, r->z + 115.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	for (n = 0; n <= 2; n += 1) {
		r->Objects[n * 2] = copy_entity(LeverBaseOBJ, 0);
		r->Objects[n * 2 + 1] = copy_entity(LeverOBJ, 0);
		r->Levers[n] = r->Objects[n * 2 + 1];
		for (i = 0; i <= 1; i += 1) {
			scale_entity(r->Objects[n * 2 + i], 0.04f, 0.04f, 0.04f, 0);
			position_entity(r->Objects[n * 2 + i], r->x - 240.0f * RoomScale, r->y + 1104.0f * RoomScale, r->z + (632.0f - 64.0f * n) * RoomScale, 1);
			entity_parent(r->Objects[n * 2 + i], r->obj, 1);
		}
		rotate_entity(r->Objects[n * 2], 0, -90, 0, 0);
		rotate_entity(r->Objects[n * 2 + 1], 10, -90 - 180, 0, 0);
		entity_pick_mode(r->Objects[n * 2 + 1], 1, 0);
		entity_radius(r->Objects[n * 2 + 1], 0.1f, 0.1f);
	}
	sc = CreateSecurityCam(r->x - 265.0f * RoomScale, r->y + 1280.0f * RoomScale, r->z + 105.0f * RoomScale, r, 0);
	sc->angle = 45;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
}

// room106
static void f_room106(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	U SecurityCams * sc = NULL;
	it = CreateItem("Level 5 Key Card", "key5", r->x - 752.0f * RoomScale, r->y - 592 * RoomScale, r->z + 3026.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Dr. Allok's Note", "paper", r->x - 416.0f * RoomScale, r->y - 576 * RoomScale, r->z + 2492.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Recall Protocol RP-106-N", "paper", r->x + 268.0f * RoomScale, r->y - 576 * RoomScale, r->z + 2593.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	d = CreateDoor(r->zone, r->x - 968.0f * RoomScale, -764.0f * RoomScale, r->z + 1392.0f * RoomScale, 0, r, 0, 0, 4, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	d = CreateDoor(r->zone, r->x, 0, r->z - 464.0f * RoomScale, 0, r, 0, 0, 4, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	d = CreateDoor(r->zone, r->x - 624.0f * RoomScale, -1280.0f * RoomScale, r->z, 90, r, 0, 0, 4, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	r->Objects[6] = LoadMesh_Strict("GFX\\map\\room1062.b3d", 0);
	scale_entity(r->Objects[6], RoomScale, RoomScale, RoomScale, 0);
	entity_type(r->Objects[6], HIT_MAP, 0);
	entity_pick_mode(r->Objects[6], 3, 1);
	position_entity(r->Objects[6], r->x + 784.0f * RoomScale, -980.0f * RoomScale, r->z + 720.0f * RoomScale, 1);
	entity_parent(r->Objects[6], r->obj, 1);
	for (n = 0; n <= 2; n += 2) {
		r->Objects[n] = copy_entity(LeverBaseOBJ, 0);
		r->Objects[n + 1] = copy_entity(LeverOBJ, 0);
		r->Levers[n / 2] = r->Objects[n + 1];
		for (i = 0; i <= 1; i += 1) {
			scale_entity(r->Objects[n + i], 0.04f, 0.04f, 0.04f, 0);
			position_entity(r->Objects[n + i], r->x - (555.0f - 81.0f * (n / 2)) * RoomScale, r->y - 576.0f * RoomScale, r->z + 3040.0f * RoomScale, 1);
			entity_parent(r->Objects[n + i], r->obj, 1);
		}
		rotate_entity(r->Objects[n], 0, 0, 0, 0);
		rotate_entity(r->Objects[n + 1], 10, -180, 0, 0);
		entity_pick_mode(r->Objects[n + 1], 1, 0);
		entity_radius(r->Objects[n + 1], 0.1f, 0.1f);
	}
	rotate_entity(r->Objects[1], 81, -180, 0, 0);
	rotate_entity(r->Objects[3], -81, -180, 0, 0);
	r->Objects[4] = CreateButton(r->x - 146.0f * RoomScale, r->y - 576.0f * RoomScale, r->z + 3045.0f * RoomScale, 0, 0, 0);
	entity_parent(r->Objects[4], r->obj, 1);
	sc = CreateSecurityCam(r->x + 768.0f * RoomScale, r->y + 1392.0f * RoomScale, r->z + 1696.0f * RoomScale, r, 1);
	sc->angle = 45 + 90 + 180;
	sc->turn = 20;
	turn_entity(sc->CameraObj, 45, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	r->Objects[7] = sc->CameraObj;
	r->Objects[8] = sc->obj;
	position_entity(sc->ScrObj, r->x - 272.0f * RoomScale, -544.0f * RoomScale, r->z + 3020.0f * RoomScale, 0);
	turn_entity(sc->ScrObj, 0, -10, 0, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
	sc->CoffinEffect = 0;
	r->Objects[5] = create_pivot(0);
	turn_entity(r->Objects[5], 0, 180, 0, 0);
	position_entity(r->Objects[5], r->x + 1088.0f * RoomScale, 1104.0f * RoomScale, r->z + 1888.0f * RoomScale, 0);
	entity_parent(r->Objects[5], r->obj, 1);
	r->Objects[9] = create_pivot(r->obj);
	position_entity(r->Objects[9], r->x - 272 * RoomScale, r->y - 672.0f * RoomScale, r->z + 2736.0f * RoomScale, 1);
	r->Objects[10] = create_pivot(r->obj);
	position_entity(r->Objects[10], r->x, r->y, r->z - 720.0f * RoomScale, 1);
}

// room1archive
static void f_room1archive(Rooms *r) {
	U int chance = 0;
	U Items * clipboard = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	U int temp3 = 0;
	U str tempstr = NULL;
	U str tempstr2 = NULL;
	U float x = 0;
	U int xtemp = 0;
	U float y = 0;
	U int ytemp = 0;
	U float z = 0;
	U int ztemp = 0;
	for (xtemp = 0; xtemp <= 1; xtemp += 1) {
		for (ytemp = 0; ytemp <= 2; ytemp += 1) {
			for (ztemp = 0; ztemp <= 2; ztemp += 1) {
				tempstr = (str)"9V Battery";
				tempstr2 = (str)"bat";
				chance = irand(-10, 100);
				if (1 == (chance < 0)) {
					break;
				} else if (1 == (chance < 40)) {
					tempstr = (str)"Document SCP-";
					{ int sel1 = irand(1, 6);
					if (sel1 == 1) {
						tempstr = cat(tempstr, "1123");
					} else if (sel1 == 2) {
						tempstr = cat(tempstr, "1048");
					} else if (sel1 == 3) {
						tempstr = cat(tempstr, "939");
					} else if (sel1 == 4) {
						tempstr = cat(tempstr, "682");
					} else if (sel1 == 5) {
						tempstr = cat(tempstr, "079");
					} else if (sel1 == 6) {
						tempstr = cat(tempstr, "096");
					} else if (sel1 == 6) {
						tempstr = cat(tempstr, "966");
					} }
					tempstr2 = (str)"paper";
				} else if (1 == (((chance >= 40)) & ((chance < 45)))) {
					temp3 = irand(1, 2);
					tempstr = cat(cat("Level ", s_str(temp3)), " Key Card");
					tempstr2 = cat("key", s_str(temp3));
				} else if (1 == (((chance >= 45)) & ((chance < 50)))) {
					tempstr = (str)"First Aid Kit";
					tempstr2 = (str)"firstaid";
				} else if (1 == (((chance >= 50)) & ((chance < 60)))) {
					tempstr = (str)"9V Battery";
					tempstr2 = (str)"bat";
				} else if (1 == (((chance >= 60)) & ((chance < 70)))) {
					tempstr = (str)"S-NAV 300 Navigator";
					tempstr2 = (str)"nav";
				} else if (1 == (((chance >= 70)) & ((chance < 85)))) {
					tempstr = (str)"Radio Transceiver";
					tempstr2 = (str)"radio";
				} else if (1 == (((chance >= 85)) & ((chance < 95)))) {
					tempstr = (str)"Clipboard";
					tempstr2 = (str)"clipboard";
				} else if (1 == (((chance >= 95)) & ((chance <= 100)))) {
					temp3 = irand(1, 3);
					if (temp3 == 1) {
						tempstr = (str)"Playing Card";
					} else if (temp3 == 2) {
						tempstr = (str)"Mastercard";
					} else if (temp3 == 3) {
						tempstr = (str)"Origami";
					}
					tempstr2 = (str)"misc";
				}
				x = (-672.0f + 864.0f * xtemp) * RoomScale;
				y = (96.0f + 96.0f * ytemp) * RoomScale;
				z = (480.0f - 352.0f * ztemp + frnd(-96.0f, 96.0f)) * RoomScale;
				it = CreateItem(tempstr, tempstr2, r->x + x, y, r->z + z, 0, 0, 0, 1, 0);
				entity_parent(it->collider, r->obj, 1);
			}
		}
	}
	r->RoomDoors[0] = CreateDoor(r->zone, r->x, r->y, r->z - 528.0f * RoomScale, 0, r, 0, 0, 6, "", 0);
	sc = CreateSecurityCam(r->x - 256.0f * RoomScale, r->y + 384.0f * RoomScale, r->z + 640.0f * RoomScale, r, 0);
	sc->angle = 180;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
}

// room2test1074
static void f_room2test1074(Rooms *r) {
	U int b = 0;
	U Items * it = NULL;
	U int msh = 0;
	U surf * sf = NULL;
	U int t = 0;
	U int tempint = 0;
	U str texname = NULL;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x, r->y, r->z, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[0]->locked = 1;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 336.0f * RoomScale, r->y, r->z + 671.0f * RoomScale, 90, r, 1, 0, 3, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x + 336.0f * RoomScale, r->y, r->z - 800.0f * RoomScale, 90, r, 1, 0, 3, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[3] = CreateDoor(r->zone, r->x + 672.0f * RoomScale, r->y, r->z, 0, r, 0, 0, 0, "", 0);
	r->Textures[0] = load_texture("GFX\\map\\1074tex0.jpg", 1);
	r->Textures[1] = load_texture("GFX\\map\\1074tex1.jpg", 1);
	texture_blend(r->Textures[0], 5);
	texture_blend(r->Textures[1], 5);
	it = CreateItem("Document SCP-1074", "paper", r->x + 300.0f * RoomScale, r->y + 20.0f * RoomScale, r->z + 671.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 835.0f * RoomScale, r->y + 165.0f * RoomScale, r->z + 540.0f * RoomScale, 1);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = create_pivot(0);
	position_entity(r->Objects[1], r->x + 835.0f * RoomScale, r->y + 10.0f * RoomScale, r->z + 300.0f * RoomScale, 1);
	entity_parent(r->Objects[1], r->obj, 1);
	msh = get_child(r->obj, 2);
	r->NonFreeAble[0] = get_surface(msh, 1);
	{ int to_ = count_surfaces(msh);
	for (tempint = 1; tempint <= to_; tempint += 1) {
		sf = get_surface(msh, tempint);
		b = get_surface_brush(sf);
		t = get_brush_texture(b, 1);
		texname = StripPath(texture_name(t));
		if (s_eq(s_lower(texname), "1074tex1.jpg")) {
			r->NonFreeAble[0] = sf;
			free_texture(t);
			free_brush(b);
			break;
		}
		if (!s_eq(texname, "")) {
			free_texture(t);
		}
		free_brush(b);
	} }
}

// room1123
static void f_room1123(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	it = CreateItem("Document SCP-1123", "paper", r->x + 511.0f * RoomScale, r->y + 125.0f * RoomScale, r->z - 936.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("SCP-1123", "1123", r->x + 832.0f * RoomScale, r->y + 166.0f * RoomScale, r->z + 784.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Leaflet", "paper", r->x - 816.0f * RoomScale, r->y + 704.0f * RoomScale, r->z + 888.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Gas Mask", "gasmask", r->x + 457.0f * RoomScale, r->y + 150.0f * RoomScale, r->z + 960.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	d = CreateDoor(r->zone, r->x + 832.0f * RoomScale, 0.0f, r->z + 367.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	position_entity(d->buttons[0], r->x + 956.0f * RoomScale, entity_y(d->buttons[0], 1), r->z + 352.0f * RoomScale, 1);
	position_entity(d->buttons[1], r->x + 713.0f * RoomScale, entity_y(d->buttons[1], 1), r->z + 384.0f * RoomScale, 1);
	free_entity(d->obj2);
	d->obj2 = 0;
	d = CreateDoor(r->zone, r->x + 280.0f * RoomScale, 0.0f, r->z - 607.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	position_entity(d->buttons[0], entity_x(d->buttons[0], 1), entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	position_entity(d->buttons[1], entity_x(d->buttons[1], 1), entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d = CreateDoor(r->zone, r->x + 280.0f * RoomScale, 512.0f * RoomScale, r->z - 607.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	position_entity(d->buttons[0], entity_x(d->buttons[0], 1), entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	free_entity(d->buttons[1]);
	d->buttons[1] = 0;
	r->RoomDoors[0] = d;
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x + 832.0f * RoomScale, r->y + 166.0f * RoomScale, r->z + 784.0f * RoomScale, 1);
	r->Objects[4] = create_pivot(r->obj);
	position_entity(r->Objects[4], r->x - 648.0f * RoomScale, r->y + 592.0f * RoomScale, r->z + 692.0f * RoomScale, 1);
	r->Objects[5] = create_pivot(r->obj);
	position_entity(r->Objects[5], r->x + 828.0f * RoomScale, r->y + 592.0f * RoomScale, r->z + 592.0f * RoomScale, 1);
	r->Objects[6] = create_pivot(r->obj);
	position_entity(r->Objects[6], r->x - 76.0f * RoomScale, r->y + 620.0f * RoomScale, r->z + 744.0f * RoomScale, 1);
	r->Objects[7] = create_pivot(r->obj);
	position_entity(r->Objects[7], r->x - 640.0f * RoomScale, r->y + 620.0f * RoomScale, r->z - 864.0f * RoomScale, 1);
	r->Objects[8] = LoadMesh_Strict("GFX\\map\\forest\\door_frame.b3d", 0);
	position_entity(r->Objects[8], r->x - 272.0f * RoomScale, 512.0f * RoomScale, r->z + 288.0f * RoomScale, 1);
	rotate_entity(r->Objects[8], 0, 90, 0, 1);
	scale_entity(r->Objects[8], 45.0f * RoomScale, 45.0f * RoomScale, 80.0f * RoomScale, 1);
	entity_parent(r->Objects[8], r->obj, 1);
	r->Objects[9] = LoadMesh_Strict("GFX\\map\\forest\\door.b3d", 0);
	position_entity(r->Objects[9], r->x - 272.0f * RoomScale, 512.0f * RoomScale, r->z + (288.0f - 70) * RoomScale, 1);
	rotate_entity(r->Objects[9], 0, 10, 0, 1);
	entity_type(r->Objects[9], HIT_MAP, 0);
	scale_entity(r->Objects[9], 46.0f * RoomScale, 45.0f * RoomScale, 46.0f * RoomScale, 1);
	entity_parent(r->Objects[9], r->obj, 1);
	r->Objects[10] = copy_entity(r->Objects[8], 0);
	position_entity(r->Objects[10], r->x - 272.0f * RoomScale, 512.0f * RoomScale, r->z + 736.0f * RoomScale, 1);
	rotate_entity(r->Objects[10], 0, 90, 0, 1);
	scale_entity(r->Objects[10], 45.0f * RoomScale, 45.0f * RoomScale, 80.0f * RoomScale, 1);
	entity_parent(r->Objects[10], r->obj, 1);
	r->Objects[11] = copy_entity(r->Objects[9], 0);
	position_entity(r->Objects[11], r->x - 272.0f * RoomScale, 512.0f * RoomScale, r->z + (736.0f - 70) * RoomScale, 1);
	rotate_entity(r->Objects[11], 0, 90, 0, 1);
	entity_type(r->Objects[11], HIT_MAP, 0);
	scale_entity(r->Objects[11], 46.0f * RoomScale, 45.0f * RoomScale, 46.0f * RoomScale, 1);
	entity_parent(r->Objects[11], r->obj, 1);
	r->Objects[12] = copy_entity(r->Objects[8], 0);
	position_entity(r->Objects[12], r->x - 592.0f * RoomScale, 512.0f * RoomScale, r->z - 704.0f * RoomScale, 1);
	rotate_entity(r->Objects[12], 0, 0, 0, 1);
	scale_entity(r->Objects[12], 45.0f * RoomScale, 45.0f * RoomScale, 80.0f * RoomScale, 1);
	entity_parent(r->Objects[12], r->obj, 1);
	r->Objects[13] = copy_entity(r->Objects[9], 0);
	position_entity(r->Objects[13], r->x - (592.0f + 70.0f) * RoomScale, 512.0f * RoomScale, r->z - 704.0f * RoomScale, 1);
	rotate_entity(r->Objects[13], 0, 0, 0, 1);
	entity_type(r->Objects[13], HIT_MAP, 0);
	scale_entity(r->Objects[13], 46.0f * RoomScale, 45.0f * RoomScale, 46.0f * RoomScale, 1);
	entity_parent(r->Objects[13], r->obj, 1);
	r->Objects[14] = LoadMesh_Strict("GFX\\map\\1123_hb.b3d", r->obj);
	entity_pick_mode(r->Objects[14], 2, 1);
	entity_type(r->Objects[14], HIT_MAP, 0);
	entity_alpha(r->Objects[14], 0.0f);
}

// pocketdimension
static void f_pocketdimension(Rooms *r) {
	U int OldManEyes = 0;
	U float angle = 0;
	U Decals * de = NULL;
	U int entity = 0;
	U int hallway = 0;
	U int i = 0;
	U int n = 0;
	U int t = 0;
	U int terrain = 0;
	hallway = LoadMesh_Strict("GFX\\map\\pocketdimension2.b3d", 0);
	r->Objects[8] = LoadMesh_Strict("GFX\\map\\pocketdimension3.b3d", 0);
	r->Objects[9] = LoadMesh_Strict("GFX\\map\\pocketdimension4.b3d", 0);
	r->Objects[10] = copy_entity(r->Objects[9], 0);
	r->Objects[11] = LoadMesh_Strict("GFX\\map\\pocketdimension5.b3d", 0);
	terrain = LoadMesh_Strict("GFX\\map\\pocketdimensionterrain.b3d", 0);
	scale_entity(terrain, RoomScale, RoomScale, RoomScale, 1);
	position_entity(terrain, 0, 2944, 0, 1);
	CreateItem("Burnt Note", "paper", entity_x(r->obj, 0), 0.5f, entity_z(r->obj, 0) + 3.5f, 0, 0, 0, 1, 0);
	for (n = 0; n <= -1; n += 1) {
		if (n == 0) {
			entity = hallway;
		} else if (n == 1) {
			entity = r->Objects[8];
		} else if (n == 2) {
			entity = r->Objects[9];
		} else if (n == 3) {
			entity = r->Objects[10];
		} else if (n == 4) {
			entity = r->Objects[11];
		}
	}
	for (i = 8; i <= 11; i += 1) {
		scale_entity(r->Objects[i], RoomScale, RoomScale, RoomScale, 0);
		entity_type(r->Objects[i], HIT_MAP, 0);
		entity_pick_mode(r->Objects[i], 2, 1);
		position_entity(r->Objects[i], r->x, r->y, r->z + 32.0f, 1);
	}
	scale_entity(terrain, RoomScale, RoomScale, RoomScale, 0);
	entity_type(terrain, HIT_MAP, 0);
	entity_pick_mode(terrain, 3, 1);
	position_entity(terrain, r->x, r->y + 2944.0f * RoomScale, r->z + 32.0f, 1);
	r->RoomDoors[0] = CreateDoor(0, r->x, 2048 * RoomScale, r->z + 32.0f - 1024 * RoomScale, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[1] = CreateDoor(0, r->x, 2048 * RoomScale, r->z + 32.0f + 1024 * RoomScale, 180, r, 0, 0, 0, "", 0);
	de = CreateDecal(18, r->x - (1536 * RoomScale), 0.02f, r->z + 608 * RoomScale + 32.0f, 90, 0, 0);
	entity_parent(de->obj, r->obj, 1);
	de->Size = frnd(0.8f, 0.8f);
	de->blendmode = 2;
	de->fx = 1 + 8;
	scale_sprite(de->obj, de->Size, de->Size);
	entity_fx(de->obj, 1 + 8);
	entity_blend(de->obj, 2);
	scale_entity(r->Objects[10], RoomScale * 1.5f, RoomScale * 2.0f, RoomScale * 1.5f, 1);
	position_entity(r->Objects[11], r->x, r->y, r->z + 64.0f, 1);
	for (i = 1; i <= 8; i += 1) {
		r->Objects[i - 1] = copy_entity(hallway, 0);
		scale_entity(r->Objects[i - 1], RoomScale, RoomScale, RoomScale, 0);
		angle = (i - 1) * (360.0f / 8.0f);
		entity_type(r->Objects[i - 1], HIT_MAP, 0);
		entity_pick_mode(r->Objects[i - 1], 2, 1);
		rotate_entity(r->Objects[i - 1], 0, angle - 90, 0, 0);
		position_entity(r->Objects[i - 1], r->x + cosf((angle) * DTOR) * (512.0f * RoomScale), 0.0f, r->z + sinf((angle) * DTOR) * (512.0f * RoomScale), 0);
		entity_parent(r->Objects[i - 1], r->obj, 1);
		if (i < 6) {
			de = CreateDecal(i + 7, r->x + cosf((angle) * DTOR) * (512.0f * RoomScale) * 3.0f, 0.02f, r->z + sinf((angle) * DTOR) * (512.0f * RoomScale) * 3.0f, 90, angle - 90, 0);
			de->Size = frnd(0.5f, 0.5f);
			de->blendmode = 2;
			de->fx = 1 + 8;
			scale_sprite(de->obj, de->Size, de->Size);
			entity_fx(de->obj, 1 + 8);
			entity_blend(de->obj, 2);
		}
	}
	for (i = 12; i <= 16; i += 1) {
		r->Objects[i] = create_pivot(r->Objects[11]);
		if (i == 12) {
			position_entity(r->Objects[i], r->x, r->y + 200 * RoomScale, r->z + 64.0f, 1);
		} else if (i == 13) {
			position_entity(r->Objects[i], r->x + 390 * RoomScale, r->y + 200 * RoomScale, r->z + 64.0f + 272 * RoomScale, 1);
		} else if (i == 14) {
			position_entity(r->Objects[i], r->x + 838 * RoomScale, r->y + 200 * RoomScale, r->z + 64.0f - 551 * RoomScale, 1);
		} else if (i == 15) {
			position_entity(r->Objects[i], r->x - 139 * RoomScale, r->y + 200 * RoomScale, r->z + 64.0f + 1201 * RoomScale, 1);
		} else if (i == 16) {
			position_entity(r->Objects[i], r->x - 1238 * RoomScale, r->y - 1664 * RoomScale, r->z + 64.0f + 381 * RoomScale, 1);
		}
	}
	OldManEyes = LoadTexture_Strict("GFX\\npcs\\oldmaneyes.jpg", 1);
	r->Objects[17] = create_sprite(0);
	scale_sprite(r->Objects[17], 0.03f, 0.03f);
	entity_texture(r->Objects[17], OldManEyes, 0, 0);
	entity_blend(r->Objects[17], 3);
	entity_fx(r->Objects[17], 1 + 8);
	sprite_view_mode(r->Objects[17], 2);
	r->Objects[18] = LoadTexture_Strict("GFX\\npcs\\pdplane.png", 1 + 2);
	r->Objects[19] = LoadTexture_Strict("GFX\\npcs\\pdplaneeye.png", 1 + 2);
	r->Objects[20] = create_sprite(0);
	scale_sprite(r->Objects[20], 8.0f, 8.0f);
	entity_texture(r->Objects[20], r->Objects[18], 0, 0);
	entity_order(r->Objects[20], 100);
	entity_blend(r->Objects[20], 2);
	entity_fx(r->Objects[20], 1 + 8);
	sprite_view_mode(r->Objects[20], 2);
	free_texture(t);
	free_entity(hallway);
}

// room3z3
static void f_room3z3(Rooms *r) {
	U SecurityCams * sc = NULL;
	sc = CreateSecurityCam(r->x - 320.0f * RoomScale, r->y + 384.0f * RoomScale, r->z + 512.25f * RoomScale, r, 0);
	sc->angle = 225;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
}

// room2_3, room3_3
static void f_room2_3(Rooms *r) {
	U WayPoints * w = NULL;
	w = CreateWaypoint(r->x, r->y + 66.0f * RoomScale, r->z, NULL, r);
}

// room1lifts
static void f_room1lifts(Rooms *r) {
	U SecurityCams * sc = NULL;
	U WayPoints * w = NULL;
	r->Objects[0] = CreateButton(r->x + 96.0f * RoomScale, r->y + 160.0f * RoomScale, r->z + 64.0f * RoomScale, 0, 0, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = CreateButton(r->x - 96.0f * RoomScale, r->y + 160.0f * RoomScale, r->z + 64.0f * RoomScale, 0, 0, 0);
	entity_parent(r->Objects[1], r->obj, 1);
	sc = CreateSecurityCam(r->x + 384.0f * RoomScale, r->y + (448 - 64) * RoomScale, r->z - 960.0f * RoomScale, r, 1);
	sc->angle = 45;
	sc->turn = 45;
	sc->room = r;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	w = CreateWaypoint(r->x, r->y + 66.0f * RoomScale, r->z, NULL, r);
}

// room2servers2
static void f_room2servers2(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	d = CreateDoor(r->zone, r->x + 264.0f * RoomScale, 0.0f, r->z + 672.0f * RoomScale, 270, r, 0, 0, 3, "", 0);
	position_entity(d->buttons[0], r->x + 224.0f * RoomScale, entity_y(d->buttons[0], 1), r->z + 510.0f * RoomScale, 1);
	position_entity(d->buttons[1], r->x + 304.0f * RoomScale, entity_y(d->buttons[1], 1), r->z + 840.0f * RoomScale, 1);
	turn_entity(d->buttons[1], 0, 0, 0, 1);
	d = CreateDoor(r->zone, r->x - 512.0f * RoomScale, -768.0f * RoomScale, r->z - 336.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	d = CreateDoor(r->zone, r->x - 509.0f * RoomScale, -768.0f * RoomScale, r->z - 1037.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	d->locked = 1;
	d->DisableWaypoint = 1;
	it = CreateItem("Night Vision Goggles", "nvgoggles", r->x + 56.0154f * RoomScale, r->y - 648.0f * RoomScale, r->z + 749.638f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 200;
	rotate_entity(it->collider, 0, r->angle + irand(245, 1), 0, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2gw, room2gw_b
static void f_room2gw(Rooms *r) {
	U int bd_temp = 0;
	U Decals * de = NULL;
	U Rooms * r2 = NULL;
	if (s_eq(r->RoomTemplate->Name, "room2gw_b")) {
		r->Objects[2] = create_pivot(r->obj);
		position_entity(r->Objects[2], r->x - 156.825f * RoomScale, -37.3458f * RoomScale, r->z + 121.364f * RoomScale, 1);
		de = CreateDecal(3, r->x - 156.825f * RoomScale, -37.3458f * RoomScale, r->z + 121.364f * RoomScale, 90, frnd(360, 0), 0);
		de->Size = 0.5f;
		scale_sprite(de->obj, de->Size, de->Size);
		entity_parent(de->obj, r->obj, 1);
		r->Objects[0] = create_pivot(0);
		position_entity(r->Objects[0], r->x + 280.0f * RoomScale, r->y + 345.0f * RoomScale, r->z - 340.0f * RoomScale, 1);
		entity_parent(r->Objects[0], r->obj, 1);
	}
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 336.0f * RoomScale, 0.0f, r->z - 382.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z - 606.679f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z - 606.679f * RoomScale, 1);
	r->RoomDoors[0]->dir = 0;
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	r->RoomDoors[0]->locked = 1;
	r->RoomDoors[0]->MTFClose = 0;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 336.0f * RoomScale, 0.0f, r->z + 462.0f * RoomScale, 180, r, 0, 0, 0, "", 0);
	position_entity(r->RoomDoors[1]->buttons[0], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[1]->buttons[0], 1), r->z - 606.679f * RoomScale, 1);
	position_entity(r->RoomDoors[1]->buttons[1], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[1]->buttons[1], 1), r->z - 606.679f * RoomScale, 1);
	r->RoomDoors[1]->dir = 0;
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 1;
	r->RoomDoors[1]->locked = 1;
	r->RoomDoors[1]->MTFClose = 0;
	for (r2 = lfirst(Rooms); r2; r2 = lafter(r2)) {
		if (r2 != r) {
			if (((s_eq(r2->RoomTemplate->Name, "room2gw")) | (s_eq(r2->RoomTemplate->Name, "room2gw_b")))) {
				r->Objects[3] = copy_entity(r2->Objects[3], r->obj);
				break;
			}
		}
	}
	if (r->Objects[3] == 0) {
		r->Objects[3] = LoadMesh_Strict("GFX\\map\\room2gw_pipes.b3d", r->obj);
	}
	entity_pick_mode(r->Objects[3], 2, 1);
	if (s_eq(r->RoomTemplate->Name, "room2gw")) {
		r->Objects[0] = create_pivot(0);
		position_entity(r->Objects[0], r->x + 344.0f * RoomScale, 128.0f * RoomScale, r->z, 0);
		entity_parent(r->Objects[0], r->obj, 1);
		bd_temp = 0;
		if (room2gw_brokendoor) {
			if (room2gw_x == r->x) {
				if (room2gw_z == r->z) {
					bd_temp = 1;
				}
			}
		}
		if ((room2gw_brokendoor == 0 && irand(1, 2) == 1) || bd_temp) {
			r->Objects[1] = copy_entity(DoorOBJ, 0);
			scale_entity(r->Objects[1], (204.0f * RoomScale) / mesh_width(r->Objects[1]), 312.0f * RoomScale / mesh_height(r->Objects[1]), 16.0f * RoomScale / mesh_depth(r->Objects[1]), 0);
			entity_type(r->Objects[1], HIT_MAP, 0);
			position_entity(r->Objects[1], r->x + 336.0f * RoomScale, 0.0f, r->z + 462.0f * RoomScale, 0);
			rotate_entity(r->Objects[1], 0, 180 + 180, 0, 0);
			entity_parent(r->Objects[1], r->obj, 1);
			move_entity(r->Objects[1], 120.0f, 0, 5.0f);
			room2gw_brokendoor = 1;
			room2gw_x = r->x;
			room2gw_z = r->z;
			free_entity(r->RoomDoors[1]->obj2);
			r->RoomDoors[1]->obj2 = 0;
		}
	}
}

// room3gw
static void f_room3gw(Rooms *r) {
	U Doors * d = NULL;
	U Rooms * r2 = NULL;
	d = CreateDoor(r->zone, r->x - 728.0f * RoomScale, 0.0f, r->z - 458.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	d->locked = 0;
	d = CreateDoor(r->zone, r->x - 223.0f * RoomScale, 0.0f, r->z - 736.0f * RoomScale, -90, r, 0, 0, 3, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	d->locked = 0;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 459.0f * RoomScale, 0.0f, r->z + 339.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z - 606.679f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z - 606.679f * RoomScale, 1);
	r->RoomDoors[0]->dir = 0;
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	r->RoomDoors[0]->locked = 1;
	r->RoomDoors[0]->MTFClose = 0;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 385.0f * RoomScale, 0.0f, r->z + 339.0f * RoomScale, 270, r, 0, 0, 0, "", 0);
	position_entity(r->RoomDoors[1]->buttons[0], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[1]->buttons[0], 1), r->z - 606.679f * RoomScale, 1);
	position_entity(r->RoomDoors[1]->buttons[1], r->x + 580.822f * RoomScale, entity_y(r->RoomDoors[1]->buttons[1], 1), r->z - 606.679f * RoomScale, 1);
	r->RoomDoors[1]->dir = 0;
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 1;
	r->RoomDoors[1]->locked = 1;
	r->RoomDoors[1]->MTFClose = 0;
	free_entity(r->RoomDoors[1]->obj2);
	r->RoomDoors[1]->obj2 = 0;
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x - 48.0f * RoomScale, 128.0f * RoomScale, r->z + 320.0f * RoomScale, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	for (r2 = lfirst(Rooms); r2; r2 = lafter(r2)) {
		if (r2 != r) {
			if (s_eq(r2->RoomTemplate->Name, "room3gw")) {
				r->Objects[3] = copy_entity(r2->Objects[3], r->obj);
				break;
			}
		}
	}
	if (r->Objects[3] == 0) {
		r->Objects[3] = LoadMesh_Strict("GFX\\map\\room3gw_pipes.b3d", r->obj);
	}
	entity_pick_mode(r->Objects[3], 2, 1);
}

// room1162
static void f_room1162(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x + 248.0f * RoomScale, 0.0f, r->z - 736.0f * RoomScale, 90, r, 0, 0, 2, "", 0);
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 1012.0f * RoomScale, r->y + 128.0f * RoomScale, r->z - 640.0f * RoomScale, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	entity_pick_mode(r->Objects[0], 1, 1);
	it = CreateItem("Document SCP-1162", "paper", r->x + 863.227f * RoomScale, r->y + 152.0f * RoomScale, r->z - 953.231f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	sc = CreateSecurityCam(r->x - 192.0f * RoomScale, r->y + 704.0f * RoomScale, r->z + 192.0f * RoomScale, r, 0);
	sc->angle = 225;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
}

// room2scps2
static void f_room2scps2(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 288.0f * RoomScale, r->y, r->z + 576.0f * RoomScale, 90, r, 0, 0, 3, "", 0);
	r->RoomDoors[0]->open = 0;
	r->RoomDoors[0]->locked = 1;
	d = CreateDoor(r->zone, r->x + 777.0f * RoomScale, r->y, r->z + 671.0f * RoomScale, 90, r, 0, 0, 4, "", 0);
	d = CreateDoor(r->zone, r->x + 556.0f * RoomScale, r->y, r->z + 296.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 576.0f * RoomScale, r->y + 160.0f * RoomScale, r->z + 632.0f * RoomScale, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	it = CreateItem("SCP-1499", "scp1499", r->x + 600.0f * RoomScale, r->y + 176.0f * RoomScale, r->z - 228.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle, 0, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-1499", "paper", r->x + 840.0f * RoomScale, r->y + 260.0f * RoomScale, r->z + 224.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-500", "paper", r->x + 1152.0f * RoomScale, r->y + 224.0f * RoomScale, r->z + 336.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Emily Ross' Badge", "badge", r->x + 364.0f * RoomScale, r->y + 5.0f * RoomScale, r->z + 716.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	sc = CreateSecurityCam(r->x + 850.0f * RoomScale, r->y + 350.0f * RoomScale, r->z + 876.0f * RoomScale, r, 0);
	sc->angle = 220;
	sc->turn = 30;
	turn_entity(sc->CameraObj, 30, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	sc = CreateSecurityCam(r->x + 600.0f * RoomScale, r->y + 514.0f * RoomScale, r->z + 150.0f * RoomScale, r, 0);
	sc->angle = 180;
	sc->turn = 30;
	turn_entity(sc->CameraObj, 30, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
}

// room3offices
static void f_room3offices(Rooms *r) {
	U Doors * d = NULL;
	d = CreateDoor(r->zone, r->x + 736.0f * RoomScale, 0.0f, r->z + 240.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	position_entity(d->buttons[0], r->x + 892.0f * RoomScale, entity_y(d->buttons[0], 1), r->z + 224.0f * RoomScale, 1);
	position_entity(d->buttons[1], r->x + 892.0f * RoomScale, entity_y(d->buttons[1], 1), r->z + 255.0f * RoomScale, 1);
	free_entity(d->obj2);
	d->obj2 = 0;
	r->Objects[0] = LoadMesh_Strict("GFX\\map\\room3offices_hb.b3d", r->obj);
	entity_pick_mode(r->Objects[0], 2, 1);
	entity_type(r->Objects[0], HIT_MAP, 0);
	entity_alpha(r->Objects[0], 0.0f);
}

// room2offices4
static void f_room2offices4(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	d = CreateDoor(0, r->x - 240.0f * RoomScale, 0.0f, r->z, 90, r, 0, 0, 0, "", 0);
	position_entity(d->buttons[0], r->x - 230.0f * RoomScale, entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	position_entity(d->buttons[1], r->x - 250.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d->open = 0;
	d->AutoClose = 0;
	it = CreateItem("Sticky Note", "paper", r->x - 991.0f * RoomScale, r->y - 242.0f * RoomScale, r->z + 904.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2sl
static void f_room2sl(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U SecurityCams * sc = NULL;
	U float scale = 0;
	U int screen = 0;
	scale = RoomScale * 4.5f * 0.4f;
	r->Textures[0] = load_anim_texture("GFX\\SL_monitors_checkpoint.jpg", 1, 512, 512, 0, 4);
	r->Textures[1] = load_anim_texture("GFX\\Sl_monitors.jpg", 1, 256, 256, 0, 8);
	for (i = 0; i <= 14; i += 1) {
		if (i != 7) {
			r->Objects[i] = copy_entity(Monitor, 0);
			scale_entity(r->Objects[i], scale, scale, scale, 0);
			if (((i != 4) & (i != 13))) {
				screen = create_sprite(0);
				entity_fx(screen, 17);
				sprite_view_mode(screen, 2);
				scale_sprite(screen, mesh_width(Monitor) * scale * 0.95f * 0.5f, mesh_height(Monitor) * scale * 0.95f * 0.5f);
				if (i == 0) {
					entity_texture(screen, r->Textures[1], 0, 0);
				} else if (i == 2) {
					entity_texture(screen, r->Textures[1], 2, 0);
				} else if (i == 3) {
					entity_texture(screen, r->Textures[1], 1, 0);
				} else if (i == 8) {
					entity_texture(screen, r->Textures[1], 4, 0);
				} else if (i == 9) {
					entity_texture(screen, r->Textures[1], 5, 0);
				} else if (i == 10) {
					entity_texture(screen, r->Textures[1], 3, 0);
				} else if (i == 11) {
					entity_texture(screen, r->Textures[1], 7, 0);
				} else {
					entity_texture(screen, r->Textures[0], 3, 0);
				}
				entity_parent(screen, r->Objects[i], 1);
			} else if (i == 4) {
				r->Objects[20] = create_sprite(0);
				entity_fx(r->Objects[20], 17);
				sprite_view_mode(r->Objects[20], 2);
				scale_sprite(r->Objects[20], mesh_width(Monitor) * scale * 0.95f * 0.5f, mesh_height(Monitor) * scale * 0.95f * 0.5f);
				entity_texture(r->Objects[20], r->Textures[0], 2, 0);
				entity_parent(r->Objects[20], r->Objects[i], 1);
			} else {
				r->Objects[21] = create_sprite(0);
				entity_fx(r->Objects[21], 17);
				sprite_view_mode(r->Objects[21], 2);
				scale_sprite(r->Objects[21], mesh_width(Monitor) * scale * 0.95f * 0.5f, mesh_height(Monitor) * scale * 0.95f * 0.5f);
				entity_texture(r->Objects[21], r->Textures[1], 6, 0);
				entity_parent(r->Objects[21], r->Objects[i], 1);
			}
		}
	}
	for (i = 0; i <= 2; i += 1) {
		position_entity(r->Objects[i], r->x - 207.94f * RoomScale, r->y + (648.0f + (112 * i)) * RoomScale, r->z - 60.0686f * RoomScale, 0);
		rotate_entity(r->Objects[i], 0, 105 + r->angle, 0, 0);
		entity_parent(r->Objects[i], r->obj, 1);
	}
	for (i = 3; i <= 5; i += 1) {
		position_entity(r->Objects[i], r->x - 231.489f * RoomScale, r->y + (648.0f + (112 * (i - 3))) * RoomScale, r->z + 95.7443f * RoomScale, 0);
		rotate_entity(r->Objects[i], 0, 90 + r->angle, 0, 0);
		entity_parent(r->Objects[i], r->obj, 1);
	}
	for (i = 6; i <= 8; i += 2) {
		position_entity(r->Objects[i], r->x - 231.489f * RoomScale, r->y + (648.0f + (112 * (i - 6))) * RoomScale, r->z + 255.744f * RoomScale, 0);
		rotate_entity(r->Objects[i], 0, 90 + r->angle, 0, 0);
		entity_parent(r->Objects[i], r->obj, 1);
	}
	for (i = 9; i <= 11; i += 1) {
		position_entity(r->Objects[i], r->x - 231.489f * RoomScale, r->y + (648.0f + (112 * (i - 9))) * RoomScale, r->z + 415.744f * RoomScale, 0);
		rotate_entity(r->Objects[i], 0, 90 + r->angle, 0, 0);
		entity_parent(r->Objects[i], r->obj, 1);
	}
	for (i = 12; i <= 14; i += 1) {
		position_entity(r->Objects[i], r->x - 208.138f * RoomScale, r->y + (648.0f + (112 * (i - 12))) * RoomScale, r->z + 571.583f * RoomScale, 0);
		rotate_entity(r->Objects[i], 0, 75 + r->angle, 0, 0);
		entity_parent(r->Objects[i], r->obj, 1);
	}
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 480.0f * RoomScale, r->y, r->z - 640.0f * RoomScale, 90, r, 0, 0, 3, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 576.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z - 480 * RoomScale, 1);
	rotate_entity(r->RoomDoors[0]->buttons[0], 0, 270, 0, 0);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 544.0f * RoomScale, r->y + 480.0f * RoomScale, r->z + 256.0f * RoomScale, 270, r, 0, 0, 3, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	free_entity(r->RoomDoors[1]->obj2);
	r->RoomDoors[1]->obj2 = 0;
	d = CreateDoor(r->zone, r->x + 1504.0f * RoomScale, r->y + 480.0f * RoomScale, r->z + 960.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d->AutoClose = 0;
	d->locked = 1;
	r->Objects[7] = create_pivot(0);
	position_entity(r->Objects[7], r->x, r->y + 100.0f * RoomScale, r->z - 800.0f * RoomScale, 1);
	entity_parent(r->Objects[7], r->obj, 1);
	r->Objects[15] = create_pivot(0);
	position_entity(r->Objects[15], r->x + 700.0f * RoomScale, r->y + 700.0f * RoomScale, r->z + 256.0f * RoomScale, 1);
	entity_parent(r->Objects[15], r->obj, 1);
	r->Objects[16] = create_pivot(0);
	position_entity(r->Objects[16], r->x - 60.0f * RoomScale, r->y + 700.0f * RoomScale, r->z + 200.0f * RoomScale, 1);
	entity_parent(r->Objects[16], r->obj, 1);
	r->Objects[17] = create_pivot(0);
	position_entity(r->Objects[17], r->x - 48.0f * RoomScale, r->y + 540.0f * RoomScale, r->z + 656.0f * RoomScale, 1);
	entity_parent(r->Objects[17], r->obj, 1);
	r->Objects[9 * 2] = copy_entity(LeverBaseOBJ, 0);
	r->Objects[9 * 2 + 1] = copy_entity(LeverOBJ, 0);
	r->Levers[0] = r->Objects[9 * 2 + 1];
	for (i = 0; i <= 1; i += 1) {
		scale_entity(r->Objects[9 * 2 + i], 0.04f, 0.04f, 0.04f, 0);
		position_entity(r->Objects[9 * 2 + i], r->x - 49 * RoomScale, r->y + 689 * RoomScale, r->z + 912 * RoomScale, 1);
		entity_parent(r->Objects[9 * 2 + i], r->obj, 1);
	}
	rotate_entity(r->Objects[9 * 2], 0, 0, 0, 0);
	rotate_entity(r->Objects[9 * 2 + 1], 10, 0 - 180, 0, 0);
	entity_pick_mode(r->Objects[9 * 2 + 1], 1, 0);
	entity_radius(r->Objects[9 * 2 + 1], 0.1f, 0.1f);
	sc = CreateSecurityCam(r->x - 159.0f * RoomScale, r->y + 384.0f * RoomScale, r->z - 929.0f * RoomScale, r, 1);
	sc->angle = 315;
	sc->room = r;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	position_entity(sc->ScrObj, r->x - 231.489f * RoomScale, r->y + 760.0f * RoomScale, r->z + 255.744f * RoomScale, 0);
	turn_entity(sc->ScrObj, 0, 90, 0, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
}

// room2_4
static void f_room2_4(Rooms *r) {

	r->Objects[6] = create_pivot(0);
	position_entity(r->Objects[6], r->x + 640.0f * RoomScale, 8.0f * RoomScale, r->z - 896.0f * RoomScale, 0);
	entity_parent(r->Objects[6], r->obj, 1);
}

// room3z2
static void f_room3z2(Rooms *r) {
	U Rooms * r2 = NULL;
	for (r2 = lfirst(Rooms); r2; r2 = lafter(r2)) {
		if (((s_eq(r2->RoomTemplate->Name, r->RoomTemplate->Name)) & (r2 != r))) {
			r->Objects[0] = copy_entity(r2->Objects[0], r->obj);
			break;
		}
	}
	if (r->Objects[0] == 0) {
		r->Objects[0] = LoadMesh_Strict("GFX\\map\\room3z2_hb.b3d", r->obj);
	}
	entity_pick_mode(r->Objects[0], 2, 1);
	entity_type(r->Objects[0], HIT_MAP, 0);
	entity_alpha(r->Objects[0], 0.0f);
}

// lockroom3
static void f_lockroom3(Rooms *r) {
	U Doors * d = NULL;
	U Doors * d2 = NULL;
	U float scale = 0;
	d = CreateDoor(r->zone, r->x - 736.0f * RoomScale, 0, r->z - 104.0f * RoomScale, 0, r, 1, 0, 0, "", 0);
	d->timer = 70 * 5;
	d->AutoClose = 0;
	d->open = 0;
	d->locked = 1;
	entity_parent(d->buttons[0], 0, 1);
	position_entity(d->buttons[0], r->x - 288.0f * RoomScale, 0.7f, r->z - 640.0f * RoomScale, 0);
	entity_parent(d->buttons[0], r->obj, 1);
	free_entity(d->buttons[1]);
	d->buttons[1] = 0;
	d2 = CreateDoor(r->zone, r->x + 104.0f * RoomScale, 0, r->z + 736.0f * RoomScale, 270, r, 1, 0, 0, "", 0);
	d2->timer = 70 * 5;
	d2->AutoClose = 0;
	d2->open = 0;
	d2->locked = 1;
	entity_parent(d2->buttons[0], 0, 1);
	position_entity(d2->buttons[0], r->x + 640.0f * RoomScale, 0.7f, r->z + 288.0f * RoomScale, 0);
	rotate_entity(d2->buttons[0], 0, 90, 0, 0);
	entity_parent(d2->buttons[0], r->obj, 1);
	free_entity(d2->buttons[1]);
	d2->buttons[1] = 0;
	d->LinkedDoor = d2;
	d2->LinkedDoor = d;
	scale = RoomScale * 4.5f * 0.4f;
	r->Objects[0] = copy_entity(Monitor, 0);
	scale_entity(r->Objects[0], scale, scale, scale, 0);
	position_entity(r->Objects[0], r->x + 668 * RoomScale, 1.1f, r->z - 96.0f * RoomScale, 1);
	rotate_entity(r->Objects[0], 0, 90, 0, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = copy_entity(Monitor, 0);
	scale_entity(r->Objects[1], scale, scale, scale, 0);
	position_entity(r->Objects[1], r->x + 96.0f * RoomScale, 1.1f, r->z - 668.0f * RoomScale, 1);
	entity_parent(r->Objects[1], r->obj, 1);
}

// medibay
static void f_medibay(Rooms *r) {
	U Items * it = NULL;
	r->Objects[0] = LoadMesh_Strict("GFX\\map\\medibay_props.b3d", r->obj);
	entity_type(r->Objects[0], HIT_MAP, 0);
	entity_pick_mode(r->Objects[0], 2, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x - 762.0f * RoomScale, r->y + 0.0f * RoomScale, r->z - 346.0f * RoomScale, 1);
	r->Objects[2] = create_pivot(r->obj);
	position_entity(r->Objects[2], (entity_x(r->Objects[1], 1) + (126.0f * RoomScale)), entity_y(r->Objects[1], 1), entity_z(r->Objects[1], 1), 1);
	it = CreateItem("First Aid Kit", "firstaid", r->x - 506.0f * RoomScale, r->y + 192.0f * RoomScale, r->z - 322.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Syringe", "syringe", r->x - 333.0f * RoomScale, r->y + 100.0f * RoomScale, r->z + 97.3f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Syringe", "syringe", r->x - 340.0f * RoomScale, r->y + 100.0f * RoomScale, r->z + 52.3f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 264.0f * RoomScale, r->y - 0.0f * RoomScale, r->z + 640.0f * RoomScale, 90, r, 0, 0, 3, "", 0);
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x - 820.0f * RoomScale, r->y, r->z - 318.399f * RoomScale, 1);
}

// room2cpit
static void f_room2cpit(Rooms *r) {
	U Doors * d = NULL;
	U Emitters * em = NULL;
	U Items * it = NULL;
	em = CreateEmitter(r->x + 512.0f * RoomScale, -76 * RoomScale, r->z - 688 * RoomScale, 0);
	turn_entity(em->Obj, -90, 0, 0, 0);
	entity_parent(em->Obj, r->obj, 1);
	em->RandAngle = 55;
	em->Speed = 0.0005f;
	em->Achange = -0.015f;
	em->SizeChange = 0.007f;
	d = CreateDoor(r->zone, r->x - 256.0f * RoomScale, 0.0f, r->z - 752.0f * RoomScale, 90, r, 0, 2, 3, "", 0);
	d->locked = 1;
	d->open = 0;
	d->AutoClose = 0;
	d->MTFClose = 0;
	d->DisableWaypoint = 1;
	position_entity(d->buttons[0], r->x - 240.0f * RoomScale, entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	it = CreateItem("Dr L's Note", "paper", r->x - 160.0f * RoomScale, 32.0f * RoomScale, r->z - 353.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// dimension1499
static void f_dimension1499(Rooms *r) {

	r->Levers[1] = LoadMesh_Strict("GFX\\map\\dimension1499\\1499object0_cull.b3d", r->obj);
	entity_type(r->Levers[1], HIT_MAP, 0);
	entity_alpha(r->Levers[1], 0);
	r->Levers[0] = create_pivot(0);
	position_entity(r->Levers[0], r->x + 205.0f * RoomScale, r->y + 200.0f * RoomScale, r->z + 2287.0f * RoomScale, 0);
	entity_parent(r->Levers[0], r->obj, 1);
}

const FillEntry Fills3[] = {
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
	{0, 0},
};
