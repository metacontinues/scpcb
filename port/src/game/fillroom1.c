// fillroom1.c: FillRoom's per-room cases room860 .. room3storage (MapSystem.bb's big Select, same order)
// drafted w/ a throwaway translator, then reviewed by hand against the .bb.

#include "g.h"

#define U __attribute__((unused))

// room860
static void f_room860(Rooms *r) {
	U Doors * d = NULL;
	U Forest * fr = NULL;
	U Items * it = NULL;
	r->Objects[2] = LoadMesh_Strict("GFX\\map\\forest\\door_frame.b3d", 0);
	position_entity(r->Objects[2], r->x + 184.0f * RoomScale, 0, r->z, 1);
	scale_entity(r->Objects[2], 45.0f * RoomScale, 45.0f * RoomScale, 80.0f * RoomScale, 1);
	entity_parent(r->Objects[2], r->obj, 1);
	r->Objects[3] = LoadMesh_Strict("GFX\\map\\forest\\door.b3d", 0);
	position_entity(r->Objects[3], r->x + 112.0f * RoomScale, 0, r->z + 0.05f, 1);
	entity_type(r->Objects[3], HIT_MAP, 0);
	scale_entity(r->Objects[3], 46.0f * RoomScale, 45.0f * RoomScale, 46.0f * RoomScale, 1);
	entity_parent(r->Objects[3], r->obj, 1);
	r->Objects[4] = copy_entity(r->Objects[3], 0);
	position_entity(r->Objects[4], r->x + 256.0f * RoomScale, 0, r->z - 0.05f, 1);
	rotate_entity(r->Objects[4], 0, 180, 0, 0);
	scale_entity(r->Objects[4], 46.0f * RoomScale, 45.0f * RoomScale, 46.0f * RoomScale, 1);
	entity_parent(r->Objects[4], r->obj, 1);
	d = CreateDoor(r->zone, r->x + 928.0f * RoomScale, 0, r->z + 640.0f * RoomScale, 0, r, 0, 0, 0, "ABCD", 0);
	d = CreateDoor(r->zone, r->x + 928.0f * RoomScale, 0, r->z - 640.0f * RoomScale, 0, r, 1, 0, 0, "ABCD", 0);
	d->AutoClose = 0;
	d = CreateDoor(r->zone, r->x + 416.0f * RoomScale, 0, r->z - 640.0f * RoomScale, 0, r, 0, 0, 1, "", 0);
	d = CreateDoor(r->zone, r->x + 416.0f * RoomScale, 0, r->z + 640.0f * RoomScale, 0, r, 0, 0, 1, "", 0);
	if (I_Zone->HasCustomForest == 0) {
		fr = lnew(Forest);
		r->fr = fr;
		GenForestGrid(fr);
		PlaceForest(fr, r->x, r->y + 30.0f, r->z, r);
	}
	it = CreateItem("Document SCP-860-1", "paper", r->x + 672.0f * RoomScale, r->y + 176.0f * RoomScale, r->z + 335.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle + 10, 0, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-860", "paper", r->x + 1152.0f * RoomScale, r->y + 176.0f * RoomScale, r->z - 384.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle + 170, 0, 0);
	entity_parent(it->collider, r->obj, 1);
}

// lockroom
static void f_lockroom(Rooms *r) {
	U Doors * d = NULL;
	U Doors * d2 = NULL;
	U Emitters * em = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x - 736.0f * RoomScale, 0, r->z - 104.0f * RoomScale, 0, r, 1, 0, 0, "", 0);
	d->timer = 70 * 5;
	d->AutoClose = 0;
	d->open = 0;
	entity_parent(d->buttons[0], 0, 1);
	position_entity(d->buttons[0], r->x - 288.0f * RoomScale, 0.7f, r->z - 640.0f * RoomScale, 0);
	entity_parent(d->buttons[0], r->obj, 1);
	free_entity(d->buttons[1]);
	d->buttons[1] = 0;
	d2 = CreateDoor(r->zone, r->x + 104.0f * RoomScale, 0, r->z + 736.0f * RoomScale, 270, r, 1, 0, 0, "", 0);
	d2->timer = 70 * 5;
	d2->AutoClose = 0;
	d2->open = 0;
	entity_parent(d2->buttons[0], 0, 1);
	position_entity(d2->buttons[0], r->x + 640.0f * RoomScale, 0.7f, r->z + 288.0f * RoomScale, 0);
	rotate_entity(d2->buttons[0], 0, 90, 0, 0);
	entity_parent(d2->buttons[0], r->obj, 1);
	free_entity(d2->buttons[1]);
	d2->buttons[1] = 0;
	d->LinkedDoor = d2;
	d2->LinkedDoor = d;
	sc = CreateSecurityCam(r->x - 688.0f * RoomScale, r->y + 384 * RoomScale, r->z + 688.0f * RoomScale, r, 1);
	sc->angle = 45 + 180;
	sc->turn = 45;
	sc->ScrTexture = 1;
	entity_texture(sc->ScrObj, ScreenTexs[sc->ScrTexture], 0, 0);
	turn_entity(sc->CameraObj, 40, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	position_entity(sc->ScrObj, r->x + 668 * RoomScale, 1.1f, r->z - 96.0f * RoomScale, 0);
	turn_entity(sc->ScrObj, 0, 90, 0, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
	sc = CreateSecurityCam(r->x - 112.0f * RoomScale, r->y + 384 * RoomScale, r->z + 112.0f * RoomScale, r, 1);
	sc->angle = 45;
	sc->turn = 45;
	sc->ScrTexture = 1;
	entity_texture(sc->ScrObj, ScreenTexs[sc->ScrTexture], 0, 0);
	turn_entity(sc->CameraObj, 40, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	position_entity(sc->ScrObj, r->x + 96.0f * RoomScale, 1.1f, r->z - 668.0f * RoomScale, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
	em = CreateEmitter(r->x - 175.0f * RoomScale, 370.0f * RoomScale, r->z + 656.0f * RoomScale, 0);
	turn_entity(em->Obj, 90, 0, 0, 1);
	entity_parent(em->Obj, r->obj, 1);
	em->RandAngle = 20;
	em->Speed = 0.05f;
	em->SizeChange = 0.007f;
	em->Achange = -0.006f;
	em->Gravity = -0.24f;
	em = CreateEmitter(r->x - 655.0f * RoomScale, 370.0f * RoomScale, r->z + 240.0f * RoomScale, 0);
	turn_entity(em->Obj, 90, 0, 0, 1);
	entity_parent(em->Obj, r->obj, 1);
	em->RandAngle = 20;
	em->Speed = 0.05f;
	em->SizeChange = 0.007f;
	em->Achange = -0.006f;
	em->Gravity = -0.24f;
}

// lockroom2
static void f_lockroom2(Rooms *r) {
	U Decals * de = NULL;
	U int i = 0;
	U SecurityCams * sc = NULL;
	for (i = 0; i <= 5; i += 1) {
		float rn3 = frnd(0, 0.001f);
		float rn4 = frnd(-392, 520);
		float rn5 = frnd(0, 360);
		float rn2 = frnd(-392, 520);
		int rn1 = irand(2, 3);
		de = CreateDecal(rn1, r->x + rn2 * RoomScale, 3.0f * RoomScale + rn3, r->z + rn4 * RoomScale, 90, rn5, 0);
		de->Size = frnd(0.3f, 0.6f);
		scale_sprite(de->obj, de->Size, de->Size);
		float rn8 = frnd(0, 0.001f);
		float rn9 = frnd(-392, 520);
		float rn10 = frnd(0, 360);
		float rn7 = frnd(-392, 520);
		int rn6 = irand(15, 16);
		CreateDecal(rn6, r->x + rn7 * RoomScale, 3.0f * RoomScale + rn8, r->z + rn9 * RoomScale, 90, rn10, 0);
		de->Size = frnd(0.1f, 0.6f);
		scale_sprite(de->obj, de->Size, de->Size);
		float rn13 = frnd(0, 0.001f);
		float rn14 = frnd(-0.5f, 0.5f);
		float rn15 = frnd(0, 360);
		float rn12 = frnd(-0.5f, 0.5f);
		int rn11 = irand(15, 16);
		CreateDecal(rn11, r->x + rn12, 3.0f * RoomScale + rn13, r->z + rn14, 90, rn15, 0);
		de->Size = frnd(0.1f, 0.6f);
		scale_sprite(de->obj, de->Size, de->Size);
	}
	sc = CreateSecurityCam(r->x + 512.0f * RoomScale, r->y + 384 * RoomScale, r->z + 384.0f * RoomScale, r, 1);
	sc->angle = 45 + 90;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 40, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	position_entity(sc->ScrObj, r->x + 668 * RoomScale, 1.1f, r->z - 96.0f * RoomScale, 0);
	turn_entity(sc->ScrObj, 0, 90, 0, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
	sc = CreateSecurityCam(r->x - 384.0f * RoomScale, r->y + 384 * RoomScale, r->z - 512.0f * RoomScale, r, 1);
	sc->angle = 45 + 90 + 180;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 40, 0, 0, 0);
	entity_parent(sc->obj, r->obj, 1);
	position_entity(sc->ScrObj, r->x + 96.0f * RoomScale, 1.1f, r->z - 668.0f * RoomScale, 0);
	entity_parent(sc->ScrObj, r->obj, 1);
}

// gatea
static void f_gatea(Rooms *r) {
	U Doors * d2 = NULL;
	U Rooms * r2 = NULL;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 4064.0f * RoomScale, r->y - 1280.0f * RoomScale, r->z + 3952.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	d2 = CreateDoor(r->zone, r->x, r->y, r->z - 1024.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d2->AutoClose = 0;
	d2->open = 0;
	d2->locked = 1;
	d2 = CreateDoor(r->zone, r->x - 1440 * RoomScale, r->y - 480.0f * RoomScale, r->z + 2328.0f * RoomScale, 0, r, 0, 0, 2, "", 0);
	if (s_eq(SelectedEnding, "A2")) {
		d2->AutoClose = 0;
		d2->open = 1;
		d2->locked = 1;
	} else {
		d2->AutoClose = 0;
		d2->open = 0;
		d2->locked = 0;
	}
	position_entity(d2->buttons[0], r->x - 1320.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z + 2288.0f * RoomScale, 1);
	position_entity(d2->buttons[1], r->x - 1584 * RoomScale, entity_y(d2->buttons[0], 1), r->z + 2488.0f * RoomScale, 1);
	rotate_entity(d2->buttons[1], 0, 90, 0, 1);
	d2 = CreateDoor(r->zone, r->x - 1440 * RoomScale, r->y - 480.0f * RoomScale, r->z + 4352.0f * RoomScale, 0, r, 0, 0, 2, "", 0);
	if (s_eq(SelectedEnding, "A2")) {
		d2->AutoClose = 0;
		d2->open = 1;
		d2->locked = 1;
	} else {
		d2->AutoClose = 0;
		d2->open = 0;
		d2->locked = 0;
	}
	position_entity(d2->buttons[0], r->x - 1320.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z + 4384.0f * RoomScale, 1);
	rotate_entity(d2->buttons[0], 0, 180, 0, 1);
	position_entity(d2->buttons[1], r->x - 1584.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z + 4232.0f * RoomScale, 1);
	rotate_entity(d2->buttons[1], 0, 90, 0, 1);
	for (r2 = lfirst(Rooms); r2; r2 = lafter(r2)) {
		if (s_eq(r2->RoomTemplate->Name, "exit1")) {
			r->Objects[1] = r2->Objects[1];
			r->Objects[2] = r2->Objects[2];
		} else if (s_eq(r2->RoomTemplate->Name, "gateaentrance")) {
			r->RoomDoors[1] = CreateDoor(0, r->x + 1544.0f * RoomScale, r->y, r->z - 64.0f * RoomScale, 90, r, 0, 3, 0, "", 0);
			r->RoomDoors[1]->AutoClose = 0;
			r->RoomDoors[1]->open = 0;
			position_entity(r->RoomDoors[1]->buttons[0], r->x + 1584 * RoomScale, entity_y(r->RoomDoors[1]->buttons[0], 1), r->z + 80 * RoomScale, 1);
			position_entity(r->RoomDoors[1]->buttons[1], r->x + 1456 * RoomScale, entity_y(r->RoomDoors[1]->buttons[1], 1), r->z - 208 * RoomScale, 1);
			r2->Objects[1] = create_pivot(0);
			position_entity(r2->Objects[1], r->x + 1848.0f * RoomScale, r->y + 240.0f * RoomScale, r->z - 64.0f * RoomScale, 1);
			entity_parent(r2->Objects[1], r->obj, 1);
		}
	}
	r->Objects[3] = create_pivot(0);
	position_entity(r->Objects[3], r->x + 1216.0f * RoomScale, r->y, r->z + 2112.0f * RoomScale, 1);
	entity_parent(r->Objects[3], r->obj, 1);
	r->Objects[4] = create_pivot(0);
	position_entity(r->Objects[4], r->x, r->y + 96.0f * RoomScale, r->z + 6400.0f * RoomScale, 1);
	entity_parent(r->Objects[4], r->obj, 1);
	r->Objects[5] = create_pivot(0);
	position_entity(r->Objects[5], r->x + 1784.0f * RoomScale, r->y + 2124.0f * RoomScale, r->z + 4512.0f * RoomScale, 1);
	entity_parent(r->Objects[5], r->obj, 1);
	r->Objects[6] = create_pivot(0);
	position_entity(r->Objects[6], r->x - 5048.0f * RoomScale, r->y + 1912.0f * RoomScale, r->z + 4656.0f * RoomScale, 1);
	entity_parent(r->Objects[6], r->obj, 1);
	r->Objects[7] = create_pivot(0);
	position_entity(r->Objects[7], r->x + 1824.0f * RoomScale, r->y + 224.0f * RoomScale, r->z + 7056.0f * RoomScale, 1);
	entity_parent(r->Objects[7], r->obj, 1);
	r->Objects[8] = create_pivot(0);
	position_entity(r->Objects[8], r->x - 1824.0f * RoomScale, r->y + 224.0f * RoomScale, r->z + 7056.0f * RoomScale, 1);
	entity_parent(r->Objects[8], r->obj, 1);
	r->Objects[9] = create_pivot(0);
	position_entity(r->Objects[9], r->x + 2624.0f * RoomScale, r->y + 992.0f * RoomScale, r->z + 6157.0f * RoomScale, 1);
	entity_parent(r->Objects[9], r->obj, 1);
	r->Objects[11] = create_pivot(0);
	position_entity(r->Objects[11], r->x - 4064.0f * RoomScale, r->y - 1248.0f * RoomScale, r->z - 1696.0f * RoomScale, 1);
	entity_parent(r->Objects[11], r->obj, 1);
	r->Objects[13] = LoadMesh_Strict("GFX\\map\\gateawall1.b3d", r->obj);
	position_entity(r->Objects[13], r->x - 4308.0f * RoomScale, r->y - 1045.0f * RoomScale, r->z + 544.0f * RoomScale, 1);
	entity_color(r->Objects[13], 25, 25, 25);
	entity_type(r->Objects[13], HIT_MAP, 0);
	r->Objects[14] = LoadMesh_Strict("GFX\\map\\gateawall2.b3d", r->obj);
	position_entity(r->Objects[14], r->x - 3820.0f * RoomScale, r->y - 1045.0f * RoomScale, r->z + 544.0f * RoomScale, 1);
	entity_color(r->Objects[14], 25, 25, 25);
	entity_type(r->Objects[14], HIT_MAP, 0);
	r->Objects[15] = create_pivot(r->obj);
	position_entity(r->Objects[15], r->x - 3568.0f * RoomScale, r->y - 1089.0f * RoomScale, r->z + 4944.0f * RoomScale, 1);
	r->Objects[16] = LoadMesh_Strict("GFX\\map\\gatea_hitbox1.b3d", r->obj);
	entity_pick_mode(r->Objects[16], 2, 1);
	entity_type(r->Objects[16], HIT_MAP, 0);
	entity_alpha(r->Objects[16], 0.0f);
}

// gateaentrance
static void f_gateaentrance(Rooms *r) {

	r->RoomDoors[0] = CreateDoor(0, r->x + 744.0f * RoomScale, 0, r->z + 512.0f * RoomScale, 90, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 688 * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z + 368 * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 784 * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z + 656 * RoomScale, 1);
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 1048.0f * RoomScale, 0, r->z + 512.0f * RoomScale, 1);
	entity_parent(r->Objects[0], r->obj, 1);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x, 0, r->z - 360.0f * RoomScale, 0, r, 0, 1, 5, "", 0);
	r->RoomDoors[1]->dir = 1;
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	position_entity(r->RoomDoors[1]->buttons[1], r->x + 416 * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z - 576 * RoomScale, 1);
	rotate_entity(r->RoomDoors[1]->buttons[1], 0, r->angle - 90, 0, 1);
	position_entity(r->RoomDoors[1]->buttons[0], r->x, 20.0f, r->z, 1);
}

// exit1
static void f_exit1(Rooms *r) {
	U Doors * d = NULL;
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 4356.0f * RoomScale, 9767.0f * RoomScale, r->z + 2588.0f * RoomScale, 1);
	r->RoomDoors[4] = CreateDoor(r->zone, r->x, 0, r->z - 320.0f * RoomScale, 0, r, 0, 1, 5, "", 0);
	r->RoomDoors[4]->dir = 1;
	r->RoomDoors[4]->AutoClose = 0;
	r->RoomDoors[4]->open = 0;
	position_entity(r->RoomDoors[4]->buttons[1], r->x + 352 * RoomScale, 0.7f, r->z - 528 * RoomScale, 1);
	rotate_entity(r->RoomDoors[4]->buttons[1], 0, r->angle - 90, 0, 1);
	position_entity(r->RoomDoors[4]->buttons[0], r->x, 7.0f, r->z, 1);
	r->Objects[3] = create_pivot(0);
	position_entity(r->Objects[3], r->x - 7680.0f * RoomScale, 10992.0f * RoomScale, r->z - 27048.0f * RoomScale, 1);
	entity_parent(r->Objects[3], r->obj, 1);
	r->Objects[4] = create_pivot(0);
	position_entity(r->Objects[4], r->x + 5203.36f * RoomScale, 12128.0f * RoomScale, r->z - 1739.19f * RoomScale, 1);
	entity_parent(r->Objects[4], r->obj, 1);
	r->Objects[5] = create_pivot(0);
	position_entity(r->Objects[5], r->x + 4363.02f * RoomScale, 10536.0f * RoomScale, r->z + 2766.16f * RoomScale, 1);
	entity_parent(r->Objects[5], r->obj, 1);
	r->Objects[6] = create_pivot(0);
	position_entity(r->Objects[6], r->x + 5192.0f * RoomScale, 12192.0f * RoomScale, r->z - 1760.0f * RoomScale, 1);
	entity_parent(r->Objects[6], r->obj, 1);
	r->Objects[7] = create_pivot(0);
	position_entity(r->Objects[7], r->x + 5192.0f * RoomScale, 12192.0f * RoomScale, r->z - 4352.0f * RoomScale, 1);
	entity_parent(r->Objects[7], r->obj, 1);
	r->RoomDoors[0] = CreateDoor(0, r->x + 720.0f * RoomScale, 0, r->z + 1432.0f * RoomScale, 0, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	move_entity(r->RoomDoors[0]->buttons[0], 0, 0, 22.0f * RoomScale);
	move_entity(r->RoomDoors[0]->buttons[1], 0, 0, 22.0f * RoomScale);
	r->Objects[8] = create_pivot(0);
	position_entity(r->Objects[8], r->x + 720.0f * RoomScale, 0, r->z + 1744.0f * RoomScale, 1);
	entity_parent(r->Objects[8], r->obj, 1);
	r->RoomDoors[1] = CreateDoor(0, r->x - 5424.0f * RoomScale, 10784.0f * RoomScale, r->z - 1380.0f * RoomScale, 0, r, 0, 3, 0, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	move_entity(r->RoomDoors[1]->buttons[0], 0, 0, 22.0f * RoomScale);
	move_entity(r->RoomDoors[1]->buttons[1], 0, 0, 22.0f * RoomScale);
	r->Objects[9] = create_pivot(0);
	position_entity(r->Objects[9], r->x - 5424.0f * RoomScale, 10784.0f * RoomScale, r->z - 1068.0f * RoomScale, 1);
	entity_parent(r->Objects[9], r->obj, 1);
	r->RoomDoors[2] = CreateDoor(0, r->x + 4352.0f * RoomScale, 10784.0f * RoomScale, r->z - 492.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	r->RoomDoors[3] = CreateDoor(0, r->x + 4352.0f * RoomScale, 10784.0f * RoomScale, r->z + 500.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[3]->AutoClose = 0;
	r->RoomDoors[3]->open = 0;
	r->Objects[10] = create_pivot(0);
	position_entity(r->Objects[10], r->x + 4352.0f * RoomScale, 10778.0f * RoomScale, r->z + 1344.0f * RoomScale, 1);
	entity_parent(r->Objects[10], r->obj, 1);
	r->Objects[11] = create_pivot(0);
	position_entity(r->Objects[11], r->x + 2816.0f * RoomScale, 11024.0f * RoomScale, r->z - 2816.0f * RoomScale, 1);
	entity_parent(r->Objects[11], r->obj, 1);
	r->RoomDoors[5] = CreateDoor(0, r->x + 3248.0f * RoomScale, 9856.0f * RoomScale, r->z + 6400.0f * RoomScale, 0, r, 0, 0, 0, "ABCD", 0);
	r->RoomDoors[5]->AutoClose = 0;
	r->RoomDoors[5]->open = 0;
	d = CreateDoor(0, r->x + 3072.0f * RoomScale, 9856.0f * RoomScale, r->z + 5800.0f * RoomScale, 90, r, 0, 0, 3, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	r->Objects[14] = create_pivot(0);
	position_entity(r->Objects[14], r->x + 3536.0f * RoomScale, 10256.0f * RoomScale, r->z + 5512.0f * RoomScale, 1);
	entity_parent(r->Objects[14], r->obj, 1);
	r->Objects[15] = create_pivot(0);
	position_entity(r->Objects[15], r->x + 3536.0f * RoomScale, 10256.0f * RoomScale, r->z + 5824.0f * RoomScale, 1);
	entity_parent(r->Objects[15], r->obj, 1);
	r->Objects[16] = create_pivot(0);
	position_entity(r->Objects[16], r->x + 3856.0f * RoomScale, 10256.0f * RoomScale, r->z + 5512.0f * RoomScale, 1);
	entity_parent(r->Objects[16], r->obj, 1);
	r->Objects[17] = create_pivot(0);
	position_entity(r->Objects[17], r->x + 3856.0f * RoomScale, 10256.0f * RoomScale, r->z + 5824.0f * RoomScale, 1);
	entity_parent(r->Objects[17], r->obj, 1);
	r->Objects[18] = create_pivot(0);
	position_entity(r->Objects[18], r->x + 3250.0f * RoomScale, 9896.0f * RoomScale, r->z + 6623.0f * RoomScale, 1);
	entity_parent(r->Objects[18], r->obj, 1);
	r->Objects[19] = create_pivot(0);
	position_entity(r->Objects[19], r->x + 3808.0f * RoomScale, 12320.0f * RoomScale, r->z - 13568.0f * RoomScale, 1);
	entity_parent(r->Objects[19], r->obj, 1);
}

// roompj
static void f_roompj(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	it = CreateItem("Document SCP-372", "paper", r->x + 800.0f * RoomScale, r->y + 176.0f * RoomScale, r->z + 1108.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle, 0, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Radio Transceiver", "radio", r->x + 800.0f * RoomScale, r->y + 112.0f * RoomScale, r->z + 944.0f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 80.0f;
	entity_parent(it->collider, r->obj, 1);
	r->Objects[3] = LoadMesh_Strict("GFX\\map\\372_hb.b3d", r->obj);
	entity_pick_mode(r->Objects[3], 2, 1);
	entity_type(r->Objects[3], HIT_MAP, 0);
	entity_alpha(r->Objects[3], 0.0f);
	d = CreateDoor(r->zone, r->x, r->y, r->z - 368.0f * RoomScale, 0, r, 1, 1, 2, "", 0);
	d->AutoClose = 0;
	position_entity(d->buttons[0], r->x - 496.0f * RoomScale, 0.7f, r->z - 272.0f * RoomScale, 1);
	turn_entity(d->buttons[0], 0, 90, 0, 0);
}

// room079
static void f_room079(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	d = CreateDoor(r->zone, r->x, -448.0f * RoomScale, r->z + 1136.0f * RoomScale, 0, r, 0, 1, 4, "", 0);
	d->dir = 1;
	d->AutoClose = 0;
	d->open = 0;
	position_entity(d->buttons[1], r->x + 224.0f * RoomScale, -250 * RoomScale, r->z + 918.0f * RoomScale, 1);
	position_entity(d->buttons[0], r->x - 240.0f * RoomScale, -250 * RoomScale, r->z + 1366.0f * RoomScale, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 1456.0f * RoomScale, -448.0f * RoomScale, r->z + 976.0f * RoomScale, 0, r, 0, 1, 3, "", 0);
	r->RoomDoors[0]->dir = 1;
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 1760.0f * RoomScale, -250 * RoomScale, r->z + 1236.0f * RoomScale, 1);
	turn_entity(r->RoomDoors[0]->buttons[0], 0, -90 - 90, 0, 1);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 1760.0f * RoomScale, -240 * RoomScale, r->z + 740.0f * RoomScale, 1);
	turn_entity(r->RoomDoors[0]->buttons[1], 0, 90 - 90, 0, 1);
	CreateDoor(0, r->x + 1144.0f * RoomScale, -448.0f * RoomScale, r->z + 704.0f * RoomScale, 90, r, 0, 0, -1, "", 0);
	r->Objects[0] = LoadAnimMesh_Strict("GFX\\map\\079.b3d", 0);
	scale_entity(r->Objects[0], 1.3f, 1.3f, 1.3f, 1);
	position_entity(r->Objects[0], r->x + 1856.0f * RoomScale, -560.0f * RoomScale, r->z - 672.0f * RoomScale, 1);
	entity_parent(r->Objects[0], r->obj, 1);
	turn_entity(r->Objects[0], 0, 180, 0, 1);
	r->Objects[1] = create_sprite(r->Objects[0]);
	sprite_view_mode(r->Objects[1], 2);
	position_entity(r->Objects[1], 0.082f, 0.119f, 0.010f, 0);
	scale_sprite(r->Objects[1], 0.18f * 0.5f, 0.145f * 0.5f);
	turn_entity(r->Objects[1], 0, 13.0f, 0, 0);
	move_entity(r->Objects[1], 0, 0, -0.022f);
	entity_texture(r->Objects[1], OldAiPics[0], 0, 0);
	hide_entity(r->Objects[1]);
	r->Objects[2] = create_pivot(r->obj);
	position_entity(r->Objects[2], r->x + 1184.0f * RoomScale, -448.0f * RoomScale, r->z + 1792.0f * RoomScale, 1);
	de = CreateDecal(3, r->x + 1184.0f * RoomScale, -448.0f * RoomScale + 0.01f, r->z + 1792.0f * RoomScale, 90, frnd(0, 360), 0);
	de->Size = 0.5f;
	scale_sprite(de->obj, de->Size, de->Size);
	entity_parent(de->obj, r->obj, 1);
}

// checkpoint1
static void f_checkpoint1(Rooms *r) {
	U SecurityCams * sc = NULL;
	r->RoomDoors[0] = CreateDoor(0, r->x + 48.0f * RoomScale, 0, r->z - 128.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	position_entity(r->RoomDoors[0]->buttons[0], r->x - 152.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z - 352.0f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[1], r->x - 152.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z + 96.0f * RoomScale, 1);
	r->RoomDoors[1] = CreateDoor(0, r->x - 352.0f * RoomScale, 0, r->z - 128.0f * RoomScale, 0, r, 0, 0, 3, "", 0);
	r->RoomDoors[1]->LinkedDoor = r->RoomDoors[0];
	r->RoomDoors[0]->LinkedDoor = r->RoomDoors[1];
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 720.0f * RoomScale, 120.0f * RoomScale, r->z + 333.0f * RoomScale, 1);
	r->RoomDoors[0]->timer = 70 * 5;
	r->RoomDoors[1]->timer = 70 * 5;
	sc = CreateSecurityCam(r->x + 192.0f * RoomScale, r->y + 704.0f * RoomScale, r->z - 960.0f * RoomScale, r, 0);
	sc->angle = 45;
	sc->turn = 0;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
	r->Objects[2] = copy_entity(Monitor2, r->obj);
	scale_entity(r->Objects[2], 2.0f, 2.0f, 2.0f, 0);
	position_entity(r->Objects[2], r->x - 152.0f * RoomScale, 384.0f * RoomScale, r->z + 124.0f * RoomScale, 1);
	rotate_entity(r->Objects[2], 0, 180, 0, 0);
	entity_fx(r->Objects[2], 1);
	r->Objects[3] = copy_entity(Monitor2, r->obj);
	scale_entity(r->Objects[3], 2.0f, 2.0f, 2.0f, 0);
	position_entity(r->Objects[3], r->x - 152.0f * RoomScale, 384.0f * RoomScale, r->z - 380.0f * RoomScale, 1);
	rotate_entity(r->Objects[3], 0, 0, 0, 0);
	entity_fx(r->Objects[3], 1);
	if ((*MT(floorf(r->x / 8.0f), floorf(r->z / 8.0f) - 1)) == 0) {
		CreateDoor(r->zone, r->x, 0, r->z - 4.0f, 0, r, 0, 0, 0, "GEAR", 0);
	}
}

// checkpoint2
static void f_checkpoint2(Rooms *r) {

	r->RoomDoors[0] = CreateDoor(0, r->x - 48.0f * RoomScale, 0, r->z + 128.0f * RoomScale, 0, r, 0, 0, 5, "", 0);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 152.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z - 96.0f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 152.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z + 352.0f * RoomScale, 1);
	r->RoomDoors[1] = CreateDoor(0, r->x + 352.0f * RoomScale, 0, r->z + 128.0f * RoomScale, 0, r, 0, 0, 5, "", 0);
	r->RoomDoors[1]->LinkedDoor = r->RoomDoors[0];
	r->RoomDoors[0]->LinkedDoor = r->RoomDoors[1];
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x - 720.0f * RoomScale, 120.0f * RoomScale, r->z + 464.0f * RoomScale, 1);
	r->Objects[2] = copy_entity(Monitor3, r->obj);
	scale_entity(r->Objects[2], 2.0f, 2.0f, 2.0f, 0);
	position_entity(r->Objects[2], r->x + 152.0f * RoomScale, 384.0f * RoomScale, r->z + 380.0f * RoomScale, 1);
	rotate_entity(r->Objects[2], 0, 180, 0, 0);
	entity_fx(r->Objects[2], 1);
	r->Objects[3] = copy_entity(Monitor3, r->obj);
	scale_entity(r->Objects[3], 2.0f, 2.0f, 2.0f, 0);
	position_entity(r->Objects[3], r->x + 152.0f * RoomScale, 384.0f * RoomScale, r->z - 124.0f * RoomScale, 1);
	rotate_entity(r->Objects[3], 0, 0, 0, 0);
	entity_fx(r->Objects[3], 1);
	r->RoomDoors[0]->timer = 70 * 5;
	r->RoomDoors[1]->timer = 70 * 5;
	if ((*MT(floorf(r->x / 8.0f), floorf(r->z / 8.0f) - 1)) == 0) {
		CreateDoor(r->zone, r->x, 0, r->z - 4.0f, 0, r, 0, 0, 0, "GEAR", 0);
	}
}

// room2pit
static void f_room2pit(Rooms *r) {
	U Emitters * em = NULL;
	U int i = 0;
	U int xtemp = 0;
	U int ztemp = 0;
	i = 0;
	for (xtemp = -1; xtemp <= 1; xtemp += 2) {
		for (ztemp = -1; ztemp <= 1; ztemp += 1) {
			em = CreateEmitter(r->x + 202.0f * RoomScale * xtemp, 8.0f * RoomScale, r->z + 256.0f * RoomScale * ztemp, 0);
			em->RandAngle = 30;
			em->Speed = 0.0045f;
			em->SizeChange = 0.007f;
			em->Achange = -0.016f;
			r->Objects[i] = em->Obj;
			if (i < 3) {
				turn_entity(em->Obj, 0, -90, 0, 1);
			} else {
				turn_entity(em->Obj, 0, 90, 0, 1);
			}
			turn_entity(em->Obj, -45, 0, 0, 1);
			entity_parent(em->Obj, r->obj, 1);
			i = i + 1;
		}
	}
	r->Objects[6] = create_pivot(0);
	position_entity(r->Objects[6], r->x + 640.0f * RoomScale, 8.0f * RoomScale, r->z - 896.0f * RoomScale, 0);
	entity_parent(r->Objects[6], r->obj, 1);
	r->Objects[7] = create_pivot(0);
	position_entity(r->Objects[7], r->x - 864.0f * RoomScale, -400.0f * RoomScale, r->z - 632.0f * RoomScale, 0);
	entity_parent(r->Objects[7], r->obj, 1);
}

// room2testroom2
static void f_room2testroom2(Rooms *r) {
	U int Glasstex = 0;
	U Doors * d = NULL;
	U Items * it = NULL;
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x - 640.0f * RoomScale, 0.5f, r->z - 912.0f * RoomScale, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = create_pivot(0);
	position_entity(r->Objects[1], r->x - 669.0f * RoomScale, 0.5f, r->z - 16.0f * RoomScale, 0);
	entity_parent(r->Objects[1], r->obj, 1);
	Glasstex = LoadTexture_Strict("GFX\\map\\glass.png", 1 + 2);
	r->Objects[2] = create_sprite(0);
	entity_texture(r->Objects[2], Glasstex, 0, 0);
	sprite_view_mode(r->Objects[2], 2);
	scale_sprite(r->Objects[2], 182.0f * RoomScale * 0.5f, 192.0f * RoomScale * 0.5f);
	position_entity(r->Objects[2], r->x - 632.0f * RoomScale, 224.0f * RoomScale, r->z - 208.0f * RoomScale, 0);
	turn_entity(r->Objects[2], 0, 180, 0, 0);
	entity_parent(r->Objects[2], r->obj, 1);
	hide_entity(r->Objects[2]);
	free_texture(Glasstex);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 240.0f * RoomScale, 0.0f, r->z + 640.0f * RoomScale, 90, r, 0, 0, 1, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	d = CreateDoor(r->zone, r->x - 512.0f * RoomScale, 0.0f, r->z + 384.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	it = CreateItem("Level 2 Key Card", "key2", r->x - 914.0f * RoomScale, r->y + 137.0f * RoomScale, r->z + 61.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("S-NAV 300 Navigator", "nav", r->x - 312.0f * RoomScale, r->y + 264.0f * RoomScale, r->z + 176.0f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 20;
	entity_parent(it->collider, r->obj, 1);
}

// room3tunnel
static void f_room3tunnel(Rooms *r) {

	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x - 190.0f * RoomScale, 4.0f * RoomScale, r->z + 190.0f * RoomScale, 1);
}

// room2toilets
static void f_room2toilets(Rooms *r) {

	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 1040.0f * RoomScale, 192.0f * RoomScale, r->z, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = create_pivot(0);
	position_entity(r->Objects[1], r->x + 1530.0f * RoomScale, 0.5f, r->z + 512.0f * RoomScale, 0);
	entity_parent(r->Objects[1], r->obj, 1);
	r->Objects[2] = create_pivot(0);
	position_entity(r->Objects[2], r->x + 1535.0f * RoomScale, r->y + 150.0f * RoomScale, r->z + 512.0f * RoomScale, 0);
	entity_parent(r->Objects[2], r->obj, 1);
}

// room2storage
static void f_room2storage(Rooms *r) {
	U int i = 0;
	U Items * it = NULL;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 1288.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x - 760.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 264.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	r->RoomDoors[3] = CreateDoor(r->zone, r->x + 264.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	r->RoomDoors[4] = CreateDoor(r->zone, r->x + 760.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	r->RoomDoors[5] = CreateDoor(r->zone, r->x + 1288.0f * RoomScale, 0, r->z, 270, r, 0, 0, 0, "", 0);
	for (i = 0; i <= 5; i += 1) {
		move_entity(r->RoomDoors[i]->buttons[0], 0, 0, -8.0f);
		move_entity(r->RoomDoors[i]->buttons[1], 0, 0, -8.0f);
		r->RoomDoors[i]->AutoClose = 0;
		r->RoomDoors[i]->open = 0;
	}
	it = CreateItem("Document SCP-939", "paper", r->x + 352.0f * RoomScale, r->y + 176.0f * RoomScale, r->z + 256.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle + 4, 0, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("9V Battery", "bat", r->x + 352.0f * RoomScale, r->y + 112.0f * RoomScale, r->z + 448.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Empty Cup", "emptycup", r->x - 672 * RoomScale, 240 * RoomScale, r->z + 288.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Level 1 Key Card", "key1", r->x - 672.0f * RoomScale, r->y + 240.0f * RoomScale, r->z + 224.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2sroom
static void f_room2sroom(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	d = CreateDoor(r->zone, r->x + 1440.0f * RoomScale, 224.0f * RoomScale, r->z + 32.0f * RoomScale, 90, r, 0, 0, 4, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	it = CreateItem("Some SCP-420-J", "420", r->x + 1776.0f * RoomScale, r->y + 400.0f * RoomScale, r->z + 427.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Some SCP-420-J", "420", r->x + 1808.0f * RoomScale, r->y + 400.0f * RoomScale, r->z + 435.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Level 5 Key Card", "key5", r->x + 2232.0f * RoomScale, r->y + 392.0f * RoomScale, r->z + 387.0f * RoomScale, 0, 0, 0, 1, 0);
	rotate_entity(it->collider, 0, r->angle, 0, 1);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Nuclear Device Document", "paper", r->x + 2248.0f * RoomScale, r->y + 440.0f * RoomScale, r->z + 372.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Radio Transceiver", "radio", r->x + 2240.0f * RoomScale, r->y + 320.0f * RoomScale, r->z + 128.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2shaft
static void f_room2shaft(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U Items * it = NULL;
	d = CreateDoor(r->zone, r->x + 1552.0f * RoomScale, r->y, r->z + 552.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	position_entity(d->buttons[0], entity_x(d->buttons[0], 1), entity_y(d->buttons[0], 1), r->z + 518.0f * RoomScale, 1);
	position_entity(d->buttons[1], entity_x(d->buttons[1], 1), entity_y(d->buttons[1], 1), r->z + 575.0f * RoomScale, 1);
	d->AutoClose = 0;
	d->open = 0;
	d = CreateDoor(r->zone, r->x + 256.0f * RoomScale, r->y, r->z + 744.0f * RoomScale, 90, r, 0, 0, 2, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	it = CreateItem("Level 3 Key Card", "key3", r->x + 1119.0f * RoomScale, r->y + 233.0f * RoomScale, r->z + 494.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("First Aid Kit", "firstaid", r->x + 1035.0f * RoomScale, r->y + 145.0f * RoomScale, r->z + 56.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	rotate_entity(it->collider, 0, 90, 0, 0);
	it = CreateItem("9V Battery", "bat", r->x + 1930.0f * RoomScale, r->y + 97.0f * RoomScale, r->z + 256.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("9V Battery", "bat", r->x + 1061.0f * RoomScale, r->y + 161.0f * RoomScale, r->z + 494.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("ReVision Eyedrops", "eyedrops", r->x + 1930.0f * RoomScale, r->y + 225.0f * RoomScale, r->z + 128.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 1560.0f * RoomScale, r->y, r->z + 250.0f * RoomScale, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x + 1344.0f * RoomScale, -752.0f * RoomScale, r->z - 384.0f * RoomScale, 1);
	de = CreateDecal(3, r->x + 1334.0f * RoomScale, -796.0f * RoomScale + 0.01f, r->z - 220.0f * RoomScale, 90, frnd(0, 360), 0);
	de->Size = 0.25f;
	scale_sprite(de->obj, de->Size, de->Size);
	entity_parent(de->obj, r->obj, 1);
	r->Objects[2] = CreateButton(r->x + 1181.0f * RoomScale, r->y + 180.0f * RoomScale, r->z - 512.0f * RoomScale, 0, 270, 0);
	entity_parent(r->Objects[2], r->obj, 1);
}

// room2poffices
static void f_room2poffices(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	d = CreateDoor(r->zone, r->x + 240.0f * RoomScale, 0.0f, r->z + 448.0f * RoomScale, 90, r, 0, 0, 0, s_str(AccessCode), 0);
	position_entity(d->buttons[0], r->x + 248.0f * RoomScale, entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	position_entity(d->buttons[1], r->x + 232.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d->AutoClose = 0;
	d->open = 0;
	d = CreateDoor(r->zone, r->x - 496.0f * RoomScale, 0.0f, r->z, 90, r, 0, 0, 0, "ABCD", 0);
	position_entity(d->buttons[0], r->x - 488.0f * RoomScale, entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	position_entity(d->buttons[1], r->x - 504.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d->AutoClose = 0;
	d->open = 0;
	d->locked = 1;
	d = CreateDoor(r->zone, r->x + 240.0f * RoomScale, 0.0f, r->z - 576.0f * RoomScale, 90, r, 0, 0, 0, "7816", 0);
	position_entity(d->buttons[0], r->x + 248.0f * RoomScale, entity_y(d->buttons[0], 1), entity_z(d->buttons[0], 1), 1);
	position_entity(d->buttons[1], r->x + 232.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d->AutoClose = 0;
	d->open = 0;
	it = CreateItem("Mysterious Note", "paper", r->x + 736.0f * RoomScale, r->y + 224.0f * RoomScale, r->z + 544.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Ballistic Vest", "vest", r->x + 608.0f * RoomScale, r->y + 112.0f * RoomScale, r->z + 32.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	rotate_entity(it->collider, 0, 90, 0, 0);
	it = CreateItem("Incident Report SCP-106-0204", "paper", r->x + 704.0f * RoomScale, r->y + 183.0f * RoomScale, r->z - 576.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Journal Page", "paper", r->x + 912 * RoomScale, r->y + 176.0f * RoomScale, r->z - 160.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("First Aid Kit", "firstaid", r->x + 912.0f * RoomScale, r->y + 112.0f * RoomScale, r->z - 336.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	rotate_entity(it->collider, 0, 90, 0, 0);
}

// room2poffices2
static void f_room2poffices2(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U Items * it = NULL;
	d = CreateDoor(r->zone, r->x + 240.0f * RoomScale, 0.0f, r->z + 48.0f * RoomScale, 270, r, 0, 0, 3, "", 0);
	position_entity(d->buttons[0], r->x + 224.0f * RoomScale, entity_y(d->buttons[0], 1), r->z + 176.0f * RoomScale, 1);
	position_entity(d->buttons[1], r->x + 256.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	d->AutoClose = 0;
	d->open = 0;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 432.0f * RoomScale, 0.0f, r->z, 90, r, 0, 0, 0, "1234", 0);
	position_entity(r->RoomDoors[0]->buttons[0], r->x - 416.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z + 176.0f * RoomScale, 1);
	free_entity(r->RoomDoors[0]->buttons[1]);
	r->RoomDoors[0]->buttons[1] = 0;
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	r->RoomDoors[0]->locked = 1;
	de = CreateDecal(0, r->x - 808.0f * RoomScale, 0.005f, r->z - 72.0f * RoomScale, 90, irand(1, 360), 0);
	entity_parent(de->obj, r->obj, 1);
	de = CreateDecal(2, r->x - 808.0f * RoomScale, 0.01f, r->z - 72.0f * RoomScale, 90, irand(1, 360), 0);
	de->Size = 0.3f;
	scale_sprite(de->obj, de->Size, de->Size);
	entity_parent(de->obj, r->obj, 1);
	de = CreateDecal(0, r->x - 432.0f * RoomScale, 0.01f, r->z, 90, irand(1, 360), 0);
	entity_parent(de->obj, r->obj, 1);
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x - 808.0f * RoomScale, 1.0f, r->z - 72.0f * RoomScale, 1);
	it = CreateItem("Dr. L's Burnt Note", "paper", r->x - 688.0f * RoomScale, 1.0f, r->z - 16.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Dr L's Burnt Note", "paper", r->x - 808.0f * RoomScale, 1.0f, r->z - 72.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("The Modular Site Project", "paper", r->x + 622.0f * RoomScale, r->y + 125.0f * RoomScale, r->z - 73.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2elevator
static void f_room2elevator(Rooms *r) {

	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 888.0f * RoomScale, 240.0f * RoomScale, r->z, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x + 1024.0f * RoomScale - 0.01f, 120.0f * RoomScale, r->z, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 448.0f * RoomScale, 0.0f, r->z, 90, r, 0, 3, 0, "", 0);
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 416.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[1], 1), r->z - 208.0f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 480.0f * RoomScale, entity_y(r->RoomDoors[0]->buttons[0], 1), r->z + 184.0f * RoomScale, 1);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	r->RoomDoors[0]->locked = 1;
}

// room2cafeteria
static void f_room2cafeteria(Rooms *r) {
	U Items * it = NULL;
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 1847.0f * RoomScale, -240.0f * RoomScale, r->z - 321 * RoomScale, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x + 1780.0f * RoomScale, -248.0f * RoomScale, r->z - 276 * RoomScale, 1);
	it = CreateItem("cup", "cup", r->x - 508.0f * RoomScale, -187 * RoomScale, r->z + 284.0f * RoomScale, 240, 175, 70, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it->name = (str)"Cup of Orange Juice";
	it = CreateItem("cup", "cup", r->x + 1412 * RoomScale, -187 * RoomScale, r->z - 716.0f * RoomScale, 87, 62, 45, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it->name = (str)"Cup of Coffee";
	it = CreateItem("Empty Cup", "emptycup", r->x - 540 * RoomScale, -187 * RoomScale, r->z + 124.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Quarter", "25ct", r->x - 447.0f * RoomScale, r->y - 334.0f * RoomScale, r->z + 36.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Quarter", "25ct", r->x + 1409.0f * RoomScale, r->y - 334.0f * RoomScale, r->z - 732.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room2nuke
static void f_room2nuke(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x + 576.0f * RoomScale, 0.0f, r->z + 152.0f * RoomScale, 90, r, 0, 0, 5, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	position_entity(d->buttons[0], r->x + 602.0f * RoomScale, entity_y(d->buttons[0], 1), r->z + 20.0f * RoomScale, 1);
	position_entity(d->buttons[1], r->x + 550.0f * RoomScale, entity_y(d->buttons[1], 1), r->z + 20.0f * RoomScale, 1);
	free_entity(d->obj2);
	d->obj2 = 0;
	d = CreateDoor(r->zone, r->x - 544.0f * RoomScale, 1504.0f * RoomScale, r->z + 738.0f * RoomScale, 90, r, 0, 0, 5, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	position_entity(d->buttons[0], entity_x(d->buttons[0], 1), entity_y(d->buttons[0], 1), r->z + 608.0f * RoomScale, 1);
	position_entity(d->buttons[1], entity_x(d->buttons[1], 1), entity_y(d->buttons[1], 1), r->z + 608.0f * RoomScale, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 1192.0f * RoomScale, 0.0f, r->z, 90, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	r->Objects[4] = create_pivot(0);
	position_entity(r->Objects[4], r->x + 1496.0f * RoomScale, 240.0f * RoomScale, r->z, 0);
	entity_parent(r->Objects[4], r->obj, 1);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 680.0f * RoomScale, 1504.0f * RoomScale, r->z, 90, r, 0, 3, 0, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	r->Objects[5] = create_pivot(0);
	position_entity(r->Objects[5], r->x + 984.0f * RoomScale, 1744.0f * RoomScale, r->z, 0);
	entity_parent(r->Objects[5], r->obj, 1);
	for (n = 0; n <= 1; n += 1) {
		r->Objects[n * 2] = copy_entity(LeverBaseOBJ, 0);
		r->Objects[n * 2 + 1] = copy_entity(LeverOBJ, 0);
		r->Levers[n] = r->Objects[n * 2 + 1];
		for (i = 0; i <= 1; i += 1) {
			scale_entity(r->Objects[n * 2 + i], 0.04f, 0.04f, 0.04f, 0);
			position_entity(r->Objects[n * 2 + i], r->x - 975.0f * RoomScale, r->y + 1712.0f * RoomScale, r->z - (502.0f - 132.0f * n) * RoomScale, 1);
			entity_parent(r->Objects[n * 2 + i], r->obj, 1);
		}
		rotate_entity(r->Objects[n * 2], 0, -90 - 180, 0, 0);
		rotate_entity(r->Objects[n * 2 + 1], 10, -90 - 180 - 180, 0, 0);
		entity_pick_mode(r->Objects[n * 2 + 1], 1, 0);
		entity_radius(r->Objects[n * 2 + 1], 0.1f, 0.1f);
	}
	it = CreateItem("Nuclear Device Document", "paper", r->x - 768.0f * RoomScale, r->y + 1684.0f * RoomScale, r->z - 768.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Ballistic Vest", "vest", r->x - 944.0f * RoomScale, r->y + 1652.0f * RoomScale, r->z - 656.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	rotate_entity(it->collider, 0, -90, 0, 0);
	sc = CreateSecurityCam(r->x + 624.0f * RoomScale, r->y + 1888.0f * RoomScale, r->z - 312.0f * RoomScale, r, 0);
	sc->angle = 90;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
	r->Objects[6] = create_pivot(0);
	position_entity(r->Objects[6], r->x + 1110.0f * RoomScale, r->y + 36.0f * RoomScale, r->z - 208.0f * RoomScale, 0);
	entity_parent(r->Objects[6], r->obj, 1);
}

// room2tunnel
static void f_room2tunnel(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U Items * it = NULL;
	U int temp = 0;
	r->Objects[0] = create_pivot(0);
	position_entity(r->Objects[0], r->x + 2640.0f * RoomScale, -2496.0f * RoomScale, r->z + 400.0f * RoomScale, 0);
	entity_parent(r->Objects[0], r->obj, 1);
	r->Objects[1] = create_pivot(0);
	position_entity(r->Objects[1], r->x - 4336.0f * RoomScale, -2496.0f * RoomScale, r->z - 2512.0f * RoomScale, 0);
	entity_parent(r->Objects[1], r->obj, 1);
	r->Objects[2] = create_pivot(0);
	rotate_entity(r->Objects[2], 0.0f, 180.0f, 0.0f, 1);
	position_entity(r->Objects[2], r->x + 552.0f * RoomScale, 240.0f * RoomScale, r->z + 656.0f * RoomScale, 0);
	entity_parent(r->Objects[2], r->obj, 1);
	r->Objects[4] = create_pivot(0);
	position_entity(r->Objects[4], r->x - 552.0f * RoomScale, 240.0f * RoomScale, r->z - 656.0f * RoomScale, 0);
	entity_parent(r->Objects[4], r->obj, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 264.0f * RoomScale, 0.0f, r->z + 656.0f * RoomScale, 90, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	position_entity(r->RoomDoors[0]->buttons[1], r->x + 224.0f * RoomScale, 0.7f, r->z + 480.0f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 304.0f * RoomScale, 0.7f, r->z + 832.0f * RoomScale, 1);
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 264.0f * RoomScale, 0.0f, r->z - 656.0f * RoomScale, 90, r, 1, 3, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 1;
	position_entity(r->RoomDoors[2]->buttons[0], r->x - 224.0f * RoomScale, 0.7f, r->z - 480.0f * RoomScale, 1);
	position_entity(r->RoomDoors[2]->buttons[1], r->x - 304.0f * RoomScale, 0.7f, r->z - 832.0f * RoomScale, 1);
	temp = (((iround(AccessCode) * 3)) % 10000);
	if (temp < 1000) {
		temp = temp + 1000;
	}
	d = CreateDoor(0, r->x, r->y, r->z, 0, r, 0, 1, 0, s_str(temp), 0);
	position_entity(d->buttons[0], r->x + 224.0f * RoomScale, r->y + 0.7f, r->z - 384.0f * RoomScale, 1);
	rotate_entity(d->buttons[0], 0, -90, 0, 1);
	position_entity(d->buttons[1], r->x - 224.0f * RoomScale, r->y + 0.7f, r->z + 384.0f * RoomScale, 1);
	rotate_entity(d->buttons[1], 0, 90, 0, 1);
	de = CreateDecal(0, r->x + 64.0f * RoomScale, 0.005f, r->z + 144.0f * RoomScale, 90, irand(1, 360), 0);
	entity_parent(de->obj, r->obj, 1);
	it = CreateItem("Scorched Note", "paper", r->x + 64.0f * RoomScale, r->y + 144.0f * RoomScale, r->z - 384.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// 008
static void f_008(Rooms *r) {
	U int Glasstex = 0;
	U Doors * d = NULL;
	U Doors * d2 = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x + 292.0f * RoomScale, 130.0f * RoomScale, r->z + 516.0f * RoomScale, 1);
	r->Objects[1] = LoadMesh_Strict("GFX\\map\\008_2.b3d", 0);
	scale_entity(r->Objects[1], RoomScale, RoomScale, RoomScale, 0);
	position_entity(r->Objects[1], r->x + 292 * RoomScale, 151 * RoomScale, r->z + 576.0f * RoomScale, 0);
	entity_parent(r->Objects[1], r->obj, 1);
	rotate_entity(r->Objects[1], 89, 0, 0, 1);
	r->Levers[0] = r->Objects[1];
	Glasstex = LoadTexture_Strict("GFX\\map\\glass.png", 1 + 2);
	r->Objects[2] = create_sprite(0);
	entity_texture(r->Objects[2], Glasstex, 0, 0);
	sprite_view_mode(r->Objects[2], 2);
	scale_sprite(r->Objects[2], 256.0f * RoomScale * 0.5f, 194.0f * RoomScale * 0.5f);
	position_entity(r->Objects[2], r->x - 176.0f * RoomScale, 224.0f * RoomScale, r->z + 448.0f * RoomScale, 0);
	turn_entity(r->Objects[2], 0, 90, 0, 0);
	entity_parent(r->Objects[2], r->obj, 1);
	free_texture(Glasstex);
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x - 445.0f * RoomScale, 120.0f * RoomScale, r->z + 544.0f * RoomScale, 1);
	r->Objects[4] = create_pivot(r->obj);
	position_entity(r->Objects[4], r->x + 67.0f * RoomScale, 120.0f * RoomScale, r->z + 464.0f * RoomScale, 1);
	r->Objects[5] = create_sprite(0);
	position_entity(r->Objects[5], r->x - 158 * RoomScale, 368 * RoomScale, r->z + 298.0f * RoomScale, 0);
	scale_sprite(r->Objects[5], 0.02f, 0.02f);
	entity_texture(r->Objects[5], LightSpriteTex[1], 0, 0);
	entity_blend(r->Objects[5], 3);
	entity_parent(r->Objects[5], r->obj, 1);
	hide_entity(r->Objects[5]);
	d = CreateDoor(r->zone, r->x + 296.0f * RoomScale, 0, r->z - 672.0f * RoomScale, 180, r, 1, 0, 4, "", 0);
	d->AutoClose = 0;
	position_entity(d->buttons[1], r->x + 164.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	free_entity(d->buttons[0]);
	d->buttons[0] = 0;
	free_entity(d->obj2);
	d->obj2 = 0;
	r->RoomDoors[0] = d;
	d2 = CreateDoor(r->zone, r->x + 296.0f * RoomScale, 0, r->z - 144.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d2->AutoClose = 0;
	position_entity(d2->buttons[0], r->x + 432.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z - 480.0f * RoomScale, 1);
	rotate_entity(d2->buttons[0], 0, -90, 0, 1);
	position_entity(d2->buttons[1], r->x + 164.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z - 128.0f * RoomScale, 1);
	free_entity(d2->obj2);
	d2->obj2 = 0;
	r->RoomDoors[1] = d2;
	d->LinkedDoor = d2;
	d2->LinkedDoor = d;
	d = CreateDoor(r->zone, r->x - 384.0f * RoomScale, 0, r->z - 672.0f * RoomScale, 0, r, 0, 0, 4, "", 0);
	d->AutoClose = 0;
	d->locked = 1;
	r->RoomDoors[2] = d;
	it = CreateItem("Hazmat Suit", "hazmatsuit", r->x - 76.0f * RoomScale, 0.5f, r->z - 396.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	rotate_entity(it->collider, 0, 90, 0, 0);
	it = CreateItem("Document SCP-008", "paper", r->x - 245.0f * RoomScale, r->y + 192.0f * RoomScale, r->z + 368.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	r->Objects[6] = create_pivot(r->obj);
	position_entity(r->Objects[6], r->x + 160 * RoomScale, 672 * RoomScale, r->z - 384.0f * RoomScale, 1);
	r->Objects[7] = create_pivot(r->obj);
	position_entity(r->Objects[7], r->x, 672 * RoomScale, r->z + 352.0f * RoomScale, 1);
	sc = CreateSecurityCam(r->x + 578.956f * RoomScale, r->y + 444.956f * RoomScale, r->z + 772.0f * RoomScale, r, 0);
	sc->angle = 135;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
}

// room035
static void f_room035(Rooms *r) {
	U Doors * d = NULL;
	U Doors * d2 = NULL;
	U Emitters * em = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	d = CreateDoor(r->zone, r->x - 296.0f * RoomScale, 0, r->z - 672.0f * RoomScale, 180, r, 1, 0, 5, "", 0);
	d->AutoClose = 0;
	d->locked = 1;
	r->RoomDoors[0] = d;
	position_entity(d->buttons[1], r->x - 164.0f * RoomScale, entity_y(d->buttons[1], 1), entity_z(d->buttons[1], 1), 1);
	free_entity(d->buttons[0]);
	d->buttons[0] = 0;
	free_entity(d->obj2);
	d->obj2 = 0;
	d2 = CreateDoor(r->zone, r->x - 296.0f * RoomScale, 0, r->z - 144.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d2->AutoClose = 0;
	d2->locked = 1;
	r->RoomDoors[1] = d2;
	position_entity(d2->buttons[0], r->x - 432.0f * RoomScale, entity_y(d2->buttons[0], 1), r->z - 480.0f * RoomScale, 1);
	rotate_entity(d2->buttons[0], 0, 90, 0, 1);
	free_entity(d2->buttons[1]);
	d2->buttons[1] = 0;
	free_entity(d2->obj2);
	d2->obj2 = 0;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x + 384.0f * RoomScale, 0, r->z - 672.0f * RoomScale, 180, r, 0, 0, 5, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[3] = CreateDoor(0, r->x + 768.0f * RoomScale, 0, r->z + 512.0f * RoomScale, 90, r, 0, 0, 0, "5731", 0);
	r->RoomDoors[3]->AutoClose = 0;
	d->LinkedDoor = d2;
	d2->LinkedDoor = d;
	for (i = 0; i <= 1; i += 1) {
		r->Objects[i * 2] = copy_entity(LeverBaseOBJ, 0);
		r->Objects[i * 2 + 1] = copy_entity(LeverOBJ, 0);
		r->Levers[i] = r->Objects[i * 2 + 1];
		for (n = 0; n <= 1; n += 1) {
			scale_entity(r->Objects[i * 2 + n], 0.04f, 0.04f, 0.04f, 0);
			position_entity(r->Objects[i * 2 + n], r->x + 210.0f * RoomScale, r->y + 224.0f * RoomScale, r->z - (208 - i * 76) * RoomScale, 1);
			entity_parent(r->Objects[i * 2 + n], r->obj, 1);
		}
		rotate_entity(r->Objects[i * 2], 0, -90 - 180, 0, 0);
		rotate_entity(r->Objects[i * 2 + 1], -80, -90, 0, 0);
		entity_pick_mode(r->Objects[i * 2 + 1], 1, 0);
		entity_radius(r->Objects[i * 2 + 1], 0.1f, 0.1f);
	}
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x + 456 * RoomScale, 0.5f, r->z + 400.0f * RoomScale, 1);
	r->Objects[4] = create_pivot(r->obj);
	position_entity(r->Objects[4], r->x - 576 * RoomScale, 0.5f, r->z + 640.0f * RoomScale, 1);
	for (i = 0; i <= 1; i += 1) {
		em = CreateEmitter(r->x - 272.0f * RoomScale, 10, r->z + (624.0f - i * 512) * RoomScale, 0);
		turn_entity(em->Obj, 90, 0, 0, 1);
		entity_parent(em->Obj, r->obj, 1);
		em->RandAngle = 15;
		em->Speed = 0.05f;
		em->SizeChange = 0.007f;
		em->Achange = -0.006f;
		em->Gravity = -0.24f;
		r->Objects[5 + i] = em->Obj;
	}
	r->Objects[7] = create_pivot(r->obj);
	position_entity(r->Objects[7], r->x - 720 * RoomScale, 0.5f, r->z + 880.0f * RoomScale, 1);
	r->Objects[8] = create_pivot(r->obj);
	position_entity(r->Objects[8], r->x + 176 * RoomScale, 0.5f, r->z - 144.0f * RoomScale, 1);
	it = CreateItem("SCP-035 Addendum", "paper", r->x + 248.0f * RoomScale, r->y + 220.0f * RoomScale, r->z + 576.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Radio Transceiver", "radio", r->x - 544.0f * RoomScale, 0.5f, r->z + 704.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("SCP-500-01", "scp500", r->x + 1168 * RoomScale, 224 * RoomScale, r->z + 576 * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Metal Panel", "scp148", r->x - 360 * RoomScale, 0.5f, r->z + 644 * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-035", "paper", r->x + 1168.0f * RoomScale, 104.0f * RoomScale, r->z + 608.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room513
static void f_room513(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x - 704.0f * RoomScale, 0, r->z + 304.0f * RoomScale, 0, r, 0, 0, 2, "", 0);
	d->AutoClose = 0;
	position_entity(d->buttons[0], entity_x(d->buttons[0], 1), entity_y(d->buttons[0], 1), r->z + 288.0f * RoomScale, 1);
	position_entity(d->buttons[1], entity_x(d->buttons[1], 1), entity_y(d->buttons[1], 1), r->z + 320.0f * RoomScale, 1);
	sc = CreateSecurityCam(r->x - 312.0f * RoomScale, r->y + 414 * RoomScale, r->z + 656 * RoomScale, r, 0);
	sc->FollowPlayer = 1;
	it = CreateItem("SCP-513", "scp513", r->x - 32.0f * RoomScale, r->y + 196.0f * RoomScale, r->z + 688.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Blood-stained Note", "paper", r->x + 736.0f * RoomScale, 1.0f, r->z + 48.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-513", "paper", r->x - 480.0f * RoomScale, 104.0f * RoomScale, r->z - 176.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
}

// room966
static void f_room966(Rooms *r) {
	U Doors * d = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x - 400.0f * RoomScale, 0, r->z, -90, r, 0, 0, 3, "", 0);
	d = CreateDoor(r->zone, r->x, 0, r->z - 480.0f * RoomScale, 180, r, 0, 0, 3, "", 0);
	sc = CreateSecurityCam(r->x - 312.0f * RoomScale, r->y + 414 * RoomScale, r->z + 656 * RoomScale, r, 0);
	sc->angle = 225;
	sc->turn = 45;
	turn_entity(sc->CameraObj, 20, 0, 0, 0);
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x, 0.5f, r->z + 512.0f * RoomScale, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x + 64.0f * RoomScale, 0.5f, r->z - 640.0f * RoomScale, 1);
	r->Objects[2] = create_pivot(r->obj);
	position_entity(r->Objects[2], r->x, 0.5f, r->z, 1);
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x + 320.0f * RoomScale, 0.5f, r->z + 704.0f * RoomScale, 1);
	it = CreateItem("Night Vision Goggles", "nvgoggles", r->x + 320.0f * RoomScale, 0.5f, r->z + 704.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it->state = 300;
}

// room3storage
static void f_room3storage(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U Emitters * em = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	U float x = 0;
	U float z = 0;
	r->Objects[0] = create_pivot(r->obj);
	position_entity(r->Objects[0], r->x, 240.0f * RoomScale, r->z + 752.0f * RoomScale, 1);
	r->Objects[1] = create_pivot(r->obj);
	position_entity(r->Objects[1], r->x + 5840.0f * RoomScale, -5392.0f * RoomScale, r->z + 1360.0f * RoomScale, 1);
	r->Objects[2] = create_pivot(r->obj);
	position_entity(r->Objects[2], r->x + 608.0f * RoomScale, 240.0f * RoomScale, r->z - 624.0f * RoomScale, 1);
	r->Objects[3] = create_pivot(r->obj);
	position_entity(r->Objects[3], r->x - 456.0f * RoomScale, -5392.0f * RoomScale, r->z - 1136 * RoomScale, 1);
	r->Objects[4] = create_pivot(r->obj);
	position_entity(r->Objects[4], r->x + 2128.0f * RoomScale, -5550.0f * RoomScale, r->z + 2048.0f * RoomScale, 1);
	r->Objects[5] = create_pivot(r->obj);
	position_entity(r->Objects[5], r->x + 2128.0f * RoomScale, -5550.0f * RoomScale, r->z - 1136.0f * RoomScale, 1);
	r->Objects[6] = create_pivot(r->obj);
	position_entity(r->Objects[6], r->x + 3824.0f * RoomScale, -5550.0f * RoomScale, r->z - 1168.0f * RoomScale, 1);
	r->Objects[7] = create_pivot(r->obj);
	position_entity(r->Objects[7], r->x + 3760.0f * RoomScale, -5550.0f * RoomScale, r->z + 2048.0f * RoomScale, 1);
	r->Objects[8] = create_pivot(r->obj);
	position_entity(r->Objects[8], r->x + 4848.0f * RoomScale, -5550.0f * RoomScale, r->z + 112.0f * RoomScale, 1);
	r->Objects[9] = create_pivot(r->obj);
	position_entity(r->Objects[9], r->x + 592.0f * RoomScale, -5550.0f * RoomScale, r->z + 6352.0f * RoomScale, 1);
	r->Objects[10] = create_pivot(r->obj);
	position_entity(r->Objects[10], r->x + 2928.0f * RoomScale, -5550.0f * RoomScale, r->z + 6352.0f * RoomScale, 1);
	r->Objects[11] = create_pivot(r->obj);
	position_entity(r->Objects[11], r->x + 2928.0f * RoomScale, -5550.0f * RoomScale, r->z + 5200.0f * RoomScale, 1);
	r->Objects[12] = create_pivot(r->obj);
	position_entity(r->Objects[12], r->x + 592.0f * RoomScale, -5550.0f * RoomScale, r->z + 5200.0f * RoomScale, 1);
	r->Objects[13] = create_pivot(r->obj);
	position_entity(r->Objects[13], r->x + 1136.0f * RoomScale, -5550.0f * RoomScale, r->z + 2944.0f * RoomScale, 1);
	r->Objects[14] = create_pivot(r->obj);
	position_entity(r->Objects[14], r->x + 1104.0f * RoomScale, -5550.0f * RoomScale, r->z + 1184.0f * RoomScale, 1);
	r->Objects[15] = create_pivot(r->obj);
	position_entity(r->Objects[15], r->x - 464.0f * RoomScale, -5550.0f * RoomScale, r->z + 1216.0f * RoomScale, 1);
	r->Objects[16] = create_pivot(r->obj);
	position_entity(r->Objects[16], r->x - 432.0f * RoomScale, -5550.0f * RoomScale, r->z + 2976.0f * RoomScale, 1);
	r->Objects[20] = LoadMesh_Strict("GFX\\map\\room3storage_hb.b3d", r->obj);
	entity_pick_mode(r->Objects[20], 2, 1);
	entity_type(r->Objects[20], HIT_MAP, 0);
	entity_alpha(r->Objects[20], 0.0f);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x, 0.0f, r->z + 448.0f * RoomScale, 0, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	position_entity(r->RoomDoors[0]->buttons[1], r->x - 160.0f * RoomScale, 0.7f, r->z + 480.0f * RoomScale, 1);
	position_entity(r->RoomDoors[0]->buttons[0], r->x + 160.0f * RoomScale, 0.7f, r->z + 416.0f * RoomScale, 1);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 5840.0f * RoomScale, -5632.0f * RoomScale, r->z + 1048.0f * RoomScale, 0, r, 0, 3, 0, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	position_entity(r->RoomDoors[1]->buttons[0], r->x + 6000.0f * RoomScale, entity_y(r->RoomDoors[1]->buttons[0], 1), r->z + 1008.0f * RoomScale, 1);
	position_entity(r->RoomDoors[1]->buttons[1], r->x + 5680.0f * RoomScale, entity_y(r->RoomDoors[1]->buttons[1], 1), r->z + 1088.0f * RoomScale, 1);
	r->RoomDoors[2] = CreateDoor(r->zone, r->x + 608.0f * RoomScale, 0.0f, r->z - 312.0f * RoomScale, 0, r, 1, 3, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 1;
	position_entity(r->RoomDoors[2]->buttons[1], r->x + 448.0f * RoomScale, 0.7f, r->z - 272.0f * RoomScale, 1);
	position_entity(r->RoomDoors[2]->buttons[0], r->x + 768.0f * RoomScale, 0.7f, r->z - 352.0f * RoomScale, 1);
	r->RoomDoors[3] = CreateDoor(r->zone, r->x - 456.0f * RoomScale, -5632.0f * RoomScale, r->z - 824.0f * RoomScale, 0, r, 0, 3, 0, "", 0);
	r->RoomDoors[3]->AutoClose = 0;
	r->RoomDoors[3]->open = 0;
	position_entity(r->RoomDoors[3]->buttons[0], r->x - 280.0f * RoomScale, entity_y(r->RoomDoors[3]->buttons[0], 1), r->z - 864.0f * RoomScale, 1);
	position_entity(r->RoomDoors[3]->buttons[1], r->x - 632.0f * RoomScale, entity_y(r->RoomDoors[3]->buttons[1], 1), r->z - 784.0f * RoomScale, 1);
	em = CreateEmitter(r->x + 5218.0f * RoomScale, -5584.0f * RoomScale, r->z - 600 * RoomScale, 0);
	turn_entity(em->Obj, 20, -100, 0, 1);
	entity_parent(em->Obj, r->obj, 1);
	em->Room = r;
	em->RandAngle = 15;
	em->Speed = 0.03f;
	em->SizeChange = 0.01f;
	em->Achange = -0.006f;
	em->Gravity = -0.2f;
	{ int sel1 = irand(1, 3);
	if (sel1 == 1) {
		x = 2312;
		z = -952;
	} else if (sel1 == 2) {
		x = 3032;
		z = 1288;
	} else if (sel1 == 3) {
		x = 2824;
		z = 2808;
	} }
	it = CreateItem("Black Severed Hand", "hand2", r->x + x * RoomScale, -5596.0f * RoomScale + 1.0f, r->z + z * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it = CreateItem("Night Vision Goggles", "nvgoggles", r->x + 1936.0f * RoomScale, r->y - 5496.0f * RoomScale, r->z - 944.0f * RoomScale, 0, 0, 0, 1, 0);
	entity_parent(it->collider, r->obj, 1);
	it->state = 450;
	de = CreateDecal(3, r->x + x * RoomScale, -5632.0f * RoomScale + 0.01f, r->z + z * RoomScale, 90, frnd(0, 360), 0);
	de->Size = 0.5f;
	scale_sprite(de->obj, de->Size, de->Size);
	entity_parent(de->obj, r->obj, 1);
	for (n = 10; n <= 11; n += 1) {
		r->Objects[n * 2] = copy_entity(LeverBaseOBJ, 0);
		r->Objects[n * 2 + 1] = copy_entity(LeverOBJ, 0);
		r->Levers[n - 10] = r->Objects[n * 2 + 1];
		for (i = 0; i <= 1; i += 1) {
			scale_entity(r->Objects[n * 2 + i], 0.04f, 0.04f, 0.04f, 0);
			if (n == 10) {
				position_entity(r->Objects[n * 2 + i], r->x + 3101 * RoomScale, r->y - 5461 * RoomScale, r->z + 6568 * RoomScale, 1);
			} else {
				position_entity(r->Objects[n * 2 + i], r->x + 1209 * RoomScale, r->y - 5461 * RoomScale, r->z + 3164 * RoomScale, 1);
			}
			entity_parent(r->Objects[n * 2 + i], r->obj, 1);
		}
		rotate_entity(r->Objects[n * 2], 0, 0, 0, 0);
		rotate_entity(r->Objects[n * 2 + 1], -10, 0 - 180, 0, 0);
		entity_pick_mode(r->Objects[n * 2 + 1], 1, 0);
		entity_radius(r->Objects[n * 2 + 1], 0.1f, 0.1f);
	}
	r->RoomDoors[4] = CreateDoor(r->zone, r->x + 56 * RoomScale, r->y - 5632 * RoomScale, r->z + 6344 * RoomScale, 90, r, 0, 2, 0, "", 0);
	r->RoomDoors[4]->AutoClose = 0;
	r->RoomDoors[4]->open = 0;
	for (i = 0; i <= 1; i += 1) {
		free_entity(r->RoomDoors[4]->buttons[i]);
		r->RoomDoors[4]->buttons[i] = 0;
	}
	d = CreateDoor(r->zone, r->x + 1157.0f * RoomScale, r->y - 5632.0f * RoomScale, r->z + 660.0f * RoomScale, 0, r, 0, 2, 0, "", 0);
	d->locked = 1;
	d->open = 0;
	d->AutoClose = 0;
	for (i = 0; i <= 1; i += 1) {
		free_entity(d->buttons[i]);
		d->buttons[i] = 0;
	}
	d = CreateDoor(r->zone, r->x + 234.0f * RoomScale, r->y - 5632.0f * RoomScale, r->z + 5239.0f * RoomScale, 90, r, 0, 2, 0, "", 0);
	d->locked = 1;
	d->open = 0;
	d->AutoClose = 0;
	for (i = 0; i <= 1; i += 1) {
		free_entity(d->buttons[i]);
		d->buttons[i] = 0;
	}
	d = CreateDoor(r->zone, r->x + 3446.0f * RoomScale, r->y - 5632.0f * RoomScale, r->z + 6369.0f * RoomScale, 90, r, 0, 2, 0, "", 0);
	d->locked = 1;
	d->open = 0;
	d->AutoClose = 0;
	for (i = 0; i <= 1; i += 1) {
		free_entity(d->buttons[i]);
		d->buttons[i] = 0;
	}
}

const FillEntry Fills1[] = {
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
	{0, 0},
};
