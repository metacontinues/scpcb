// fillroom2.c: FillRoom's per-room cases room049 .. 173 (MapSystem.bb's big Select, same order)
// drafted w/ a throwaway translator, then reviewed by hand against the .bb. rng calls that share one
// blitz statement are split into temps, left to right (blitz's own arg order is unverified)
#include "g.h"

#define U __attribute__((unused))

// room049
static void f_room049(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int n = 0;
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x + 640.0f * RoomScale, 240.0f * RoomScale, r->z + 656.0f * RoomScale, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x + 3211.0f * RoomScale, (-3280.0f) * RoomScale, r->z + 1824.0f * RoomScale, 1);
	r->Objects[2] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[2], r->x - 672.0f * RoomScale, 240.0f * RoomScale, r->z - 93.0f * RoomScale, 1);
	r->Objects[3] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[3], r->x - 2766.0f * RoomScale, (-3280.0f) * RoomScale, r->z - 1277.0f * RoomScale, 1);
	r->Objects[4] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[4], r->x + 528.0f * RoomScale, (-3440.0f) * RoomScale, r->z + 96.0f * RoomScale, 1);
	r->Objects[5] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[5], r->x + 64.0f * RoomScale, (-3440.0f) * RoomScale, r->z - 1000.0f * RoomScale, 1);
	for (n = 0; n <= 1; n += 1) {
		r->Objects[n * 2 + 6] = bbCopyEntity(LeverBaseOBJ, 0);
		r->Objects[n * 2 + 7] = bbCopyEntity(LeverOBJ, 0);
		r->Levers[n] = r->Objects[n * 2 + 7];
		for (i = 0; i <= 1; i += 1) {
			bbScaleEntity(r->Objects[n * 2 + 6 + i], 0.03f, 0.03f, 0.03f, 0);
			if (n == 0) {
				bbPositionEntity(r->Objects[n * 2 + 6 + i], r->x + 852.0f * RoomScale, r->y - 3374.0f * RoomScale, r->z - 854.0f * RoomScale, 1);
			} else if (n == 1) {
				bbPositionEntity(r->Objects[n * 2 + 6 + i], r->x - 834.0f * RoomScale, r->y - 3400.0f * RoomScale, r->z + 1093.0f * RoomScale, 1);
			}
			bbEntityParent(r->Objects[n * 2 + 6 + i], r->obj, 1);
		}
		bbRotateEntity(r->Objects[n * 2 + 6], 0, 180 + 90 * (!(n)), 0, 0);
		bbRotateEntity(r->Objects[n * 2 + 7], 81 - 92 * n, 90 * (!(n)), 0, 0);
		bbEntityPickMode(r->Objects[n * 2 + 7], 1, 0);
		bbEntityRadius(r->Objects[n * 2 + 7], 0.1f, 0.1f);
	}
	r->RoomDoors[0] = CreateDoor(r->zone, r->x + 330.0f * RoomScale, 0.0f, r->z + 656.0f * RoomScale, 90, r, 1, 3, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	bbPositionEntity(r->RoomDoors[0]->buttons[1], r->x + 288.0f * RoomScale, 0.7f, r->z + 512.0f * RoomScale, 1);
	bbPositionEntity(r->RoomDoors[0]->buttons[0], r->x + 368.0f * RoomScale, 0.7f, r->z + 840.0f * RoomScale, 1);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 2898.0f * RoomScale, (-3520.0f) * RoomScale, r->z + 1824.0f * RoomScale, 90, r, 0, 3, 0, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	bbPositionEntity(r->RoomDoors[1]->buttons[1], r->x + 2881.0f * RoomScale, bbEntityY(r->RoomDoors[1]->buttons[1], 1), r->z + 1663.0f * RoomScale, 1);
	bbPositionEntity(r->RoomDoors[1]->buttons[0], r->x + 2936.0f * RoomScale, bbEntityY(r->RoomDoors[1]->buttons[0], 1), r->z + 2009.0f * RoomScale, 1);
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 672.0f * RoomScale, 0.0f, r->z - 408.0f * RoomScale, 0, r, 1, 3, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 1;
	bbPositionEntity(r->RoomDoors[2]->buttons[0], r->x - 487.0f * RoomScale, 0.7f, r->z - 447.0f * RoomScale, 1);
	bbPositionEntity(r->RoomDoors[2]->buttons[1], r->x - 857.0f * RoomScale, 0.7f, r->z - 369.0f * RoomScale, 1);
	r->RoomDoors[3] = CreateDoor(r->zone, r->x - 2766.0f * RoomScale, (-3520.0f) * RoomScale, r->z - 1592.0f * RoomScale, 0, r, 0, 3, 0, "", 0);
	r->RoomDoors[3]->AutoClose = 0;
	r->RoomDoors[3]->open = 0;
	bbPositionEntity(r->RoomDoors[3]->buttons[0], r->x - 2581.0f * RoomScale, bbEntityY(r->RoomDoors[3]->buttons[0], 1), r->z - 1631.0f * RoomScale, 1);
	bbPositionEntity(r->RoomDoors[3]->buttons[1], r->x - 2951.0f * RoomScale, bbEntityY(r->RoomDoors[3]->buttons[1], 1), r->z - 1553.0f * RoomScale, 1);
	r->RoomDoors[4] = CreateDoor(r->zone, r->x + 272.0f * RoomScale, (-3552.0f) * RoomScale, r->z + 104.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	r->RoomDoors[4]->AutoClose = 0;
	r->RoomDoors[4]->open = 1;
	r->RoomDoors[4]->locked = 1;
	r->RoomDoors[5] = CreateDoor(r->zone, r->x + 264.0f * RoomScale, (-3520.0f) * RoomScale, r->z - 1824.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	r->RoomDoors[5]->AutoClose = 0;
	r->RoomDoors[5]->open = 1;
	r->RoomDoors[5]->locked = 1;
	r->RoomDoors[6] = CreateDoor(r->zone, r->x - 264.0f * RoomScale, (-3520.0f) * RoomScale, r->z + 1824.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	r->RoomDoors[6]->AutoClose = 0;
	r->RoomDoors[6]->open = 1;
	r->RoomDoors[6]->locked = 1;
	d = CreateDoor(0, r->x, 0, r->z, 0, r, 0, 2, -2, "", 0);
	it = CreateItem("Document SCP-049", "paper", r->x - 608.0f * RoomScale, r->y - 3332.0f * RoomScale, r->z + 876.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Level 4 Key Card", "key4", r->x - 512.0f * RoomScale, r->y - 3412.0f * RoomScale, r->z + 864.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("First Aid Kit", "firstaid", r->x + 385.0f * RoomScale, r->y - 3412.0f * RoomScale, r->z + 271.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	d = CreateDoor(r->zone, r->x - 272.0f * RoomScale, r->y - 3552.0f * RoomScale, r->z + 98.0f * RoomScale, 90, r, 1, 1, 0, "", 0);
	d->AutoClose = 0;
	d->open = 1;
	d->MTFClose = 0;
	d->locked = 1;
	for (i = 0; i <= 1; i += 1) {
		bbFreeEntity(d->buttons[i]);
		d->buttons[i] = 0;
	}
	d = CreateDoor(r->zone, r->x - 2990.0f * RoomScale, r->y - 3520.0f * RoomScale, r->z - 1824.0f * RoomScale, 90, r, 0, 2, 0, "", 0);
	d->locked = 1;
	d->DisableWaypoint = 1;
	d = CreateDoor(r->zone, r->x - 896.0f * RoomScale, r->y, r->z - 640 * RoomScale, 90, r, 0, 2, 0, "", 0);
	d->locked = 1;
	d->DisableWaypoint = 1;
	r->Objects[10] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[10], r->x - 832.0f * RoomScale, r->y - 3484.0f * RoomScale, r->z + 1572.0f * RoomScale, 1);
	r->Objects[11] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[11], r->x + 2642.0f * RoomScale, r->y - 3516.0f * RoomScale, r->z + 1822.0f * RoomScale, 1);
	r->Objects[12] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[12], r->x - 2666.0f * RoomScale, r->y - 3516.0f * RoomScale, r->z - 1792.0f * RoomScale, 1);
}

// room2_2
static void f_room2_2(Rooms *r) {
	U Rooms * r2 = NULL;
	for (r2 = bbFirst(Rooms); r2; r2 = bbAfter(r2)) {
		if (r2 != r) {
			if (bbSeq(r2->RoomTemplate->Name, "room2_2")) {
				r->Objects[0] = bbCopyEntity(r2->Objects[0], 0);
				break;
			}
		}
	}
	if (r->Objects[0] == 0) {
		r->Objects[0] = LoadMesh_Strict("GFX\\map\\fan.b3d", 0);
	}
	bbScaleEntity(r->Objects[0], RoomScale, RoomScale, RoomScale, 0);
	bbPositionEntity(r->Objects[0], r->x - 248 * RoomScale, 528 * RoomScale, r->z, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
}

// room012
static void f_room012(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U int i = 0;
	U Items * it = NULL;
	U int tex = 0;
	d = CreateDoor(r->zone, r->x + 264.0f * RoomScale, 0.0f, r->z + 672.0f * RoomScale, 270, r, 0, 0, 3, "", 0);
	bbPositionEntity(d->buttons[0], r->x + 224.0f * RoomScale, bbEntityY(d->buttons[0], 1), r->z + 540.0f * RoomScale, 1);
	bbPositionEntity(d->buttons[1], r->x + 304.0f * RoomScale, bbEntityY(d->buttons[1], 1), r->z + 840.0f * RoomScale, 1);
	bbTurnEntity(d->buttons[1], 0, 0, 0, 1);
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 512.0f * RoomScale, (-768.0f) * RoomScale, r->z - 336.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	r->RoomDoors[0]->locked = 1;
	bbPositionEntity(r->RoomDoors[0]->buttons[0], r->x + 176.0f * RoomScale, (-512.0f) * RoomScale, r->z - 364.0f * RoomScale, 1);
	bbFreeEntity(r->RoomDoors[0]->buttons[1]);
	r->RoomDoors[0]->buttons[1] = 0;
	r->Objects[0] = bbCopyEntity(LeverBaseOBJ, 0);
	r->Objects[1] = bbCopyEntity(LeverOBJ, 0);
	r->Levers[0] = r->Objects[1];
	for (i = 0; i <= 1; i += 1) {
		bbScaleEntity(r->Objects[i], 0.04f, 0.04f, 0.04f, 0);
		bbPositionEntity(r->Objects[i], r->x + 240.0f * RoomScale, r->y - 512.0f * RoomScale, r->z - 364 * RoomScale, 1);
		bbEntityParent(r->Objects[i], r->obj, 1);
	}
	bbRotateEntity(r->Objects[1], 10, -180, 0, 0);
	bbEntityPickMode(r->Objects[1], 1, 0);
	bbEntityRadius(r->Objects[1], 0.1f, 0.1f);
	r->Objects[2] = LoadMesh_Strict("GFX\\map\\room012_2.b3d", 0);
	bbScaleEntity(r->Objects[2], RoomScale, RoomScale, RoomScale, 0);
	bbPositionEntity(r->Objects[2], r->x - 360 * RoomScale, (-130) * RoomScale, r->z + 456.0f * RoomScale, 0);
	bbEntityParent(r->Objects[2], r->obj, 1);
	r->Objects[3] = bbCreateSprite(0);
	bbPositionEntity(r->Objects[3], r->x - 43.5f * RoomScale, (-574) * RoomScale, r->z - 362.0f * RoomScale, 0);
	bbScaleSprite(r->Objects[3], 0.015f, 0.015f);
	bbEntityTexture(r->Objects[3], LightSpriteTex[1], 0, 0);
	bbEntityBlend(r->Objects[3], 3);
	bbEntityParent(r->Objects[3], r->obj, 1);
	bbHideEntity(r->Objects[3]);
	r->Objects[4] = LoadMesh_Strict("GFX\\map\\room012_3.b3d", 0);
	tex = LoadTexture_Strict("GFX\\map\\scp-012_0.jpg", 1);
	bbEntityTexture(r->Objects[4], tex, 0, 1);
	bbScaleEntity(r->Objects[4], RoomScale, RoomScale, RoomScale, 0);
	bbPositionEntity(r->Objects[4], r->x - 360 * RoomScale, (-130) * RoomScale, r->z + 456.0f * RoomScale, 0);
	bbEntityParent(r->Objects[4], r->Objects[2], 1);
	it = CreateItem("Document SCP-012", "paper", r->x - 56.0f * RoomScale, r->y - 576.0f * RoomScale, r->z - 408.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Severed Hand", "hand", r->x - 784 * RoomScale, (-576) * RoomScale + 0.3f, r->z + 640 * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	de = CreateDecal(3, r->x - 784 * RoomScale, (-768) * RoomScale + 0.01f, r->z + 640 * RoomScale, 90, bbRnd(0, 360), 0);
	de->Size = 0.5f;
	bbScaleSprite(de->obj, de->Size, de->Size);
	bbEntityParent(de->obj, r->obj, 1);
}

// tunnel2
static void f_tunnel2(Rooms *r) {

	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x, 544.0f * RoomScale, r->z + 512.0f * RoomScale, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x, 544.0f * RoomScale, r->z - 512.0f * RoomScale, 1);
}

// room2pipes
static void f_room2pipes(Rooms *r) {

	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x + 368.0f * RoomScale, 0.0f, r->z, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x - 368.0f * RoomScale, 0.0f, r->z, 1);
	r->Objects[2] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[2], r->x + 224.0f * RoomScale - 0.005f, 192.0f * RoomScale, r->z, 1);
	r->Objects[3] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[3], r->x - 224.0f * RoomScale + 0.005f, 192.0f * RoomScale, r->z, 1);
}

// room3pit
static void f_room3pit(Rooms *r) {
	U Emitters * em = NULL;
	em = CreateEmitter(r->x + 512.0f * RoomScale, (-76) * RoomScale, r->z - 688 * RoomScale, 0);
	bbTurnEntity(em->Obj, -90, 0, 0, 0);
	bbEntityParent(em->Obj, r->obj, 1);
	em->RandAngle = 55;
	em->Speed = 0.0005f;
	em->Achange = -0.015f;
	em->SizeChange = 0.007f;
	em = CreateEmitter(r->x - 512.0f * RoomScale, (-76) * RoomScale, r->z - 688 * RoomScale, 0);
	bbTurnEntity(em->Obj, -90, 0, 0, 0);
	bbEntityParent(em->Obj, r->obj, 1);
	em->RandAngle = 55;
	em->Speed = 0.0005f;
	em->Achange = -0.015f;
	em->SizeChange = 0.007f;
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x + 704.0f * RoomScale, 112.0f * RoomScale, r->z - 416.0f * RoomScale, 1);
}

// room2servers
static void f_room2servers(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U int n = 0;
	d = CreateDoor(0, r->x, 0, r->z, 0, r, 0, 2, 0, "", 0);
	d->locked = 1;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 208.0f * RoomScale, 0.0f, r->z - 736.0f * RoomScale, 90, r, 1, 0, 0, "", 1);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x - 208.0f * RoomScale, 0.0f, r->z + 736.0f * RoomScale, 90, r, 1, 0, 0, "", 1);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 672.0f * RoomScale, 0.0f, r->z - 1024.0f * RoomScale, 0, r, 0, 0, 0, "GEAR", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->DisableWaypoint = 1;
	bbFreeEntity(r->RoomDoors[2]->buttons[0]);
	r->RoomDoors[2]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[2]->buttons[1]);
	r->RoomDoors[2]->buttons[1] = 0;
	for (n = 0; n <= 2; n += 1) {
		r->Objects[n * 2] = bbCopyEntity(LeverBaseOBJ, 0);
		r->Objects[n * 2 + 1] = bbCopyEntity(LeverOBJ, 0);
		r->Levers[n] = r->Objects[n * 2 + 1];
		for (i = 0; i <= 1; i += 1) {
			bbScaleEntity(r->Objects[n * 2 + i], 0.03f, 0.03f, 0.03f, 0);
			if (n == 0) {
				bbPositionEntity(r->Objects[n * 2 + i], r->x - 1260.0f * RoomScale, r->y + 234.0f * RoomScale, r->z + 750 * RoomScale, 1);
			} else if (n == 1) {
				bbPositionEntity(r->Objects[n * 2 + i], r->x - 920.0f * RoomScale, r->y + 164.0f * RoomScale, r->z + 898 * RoomScale, 1);
			} else if (n == 2) {
				bbPositionEntity(r->Objects[n * 2 + i], r->x - 837.0f * RoomScale, r->y + 152.0f * RoomScale, r->z + 886 * RoomScale, 1);
			}
			bbEntityParent(r->Objects[n * 2 + i], r->obj, 1);
		}
		bbRotateEntity(r->Objects[n * 2 + 1], 81, -180, 0, 0);
		bbEntityPickMode(r->Objects[n * 2 + 1], 1, 0);
		bbEntityRadius(r->Objects[n * 2 + 1], 0.1f, 0.1f);
	}
	bbRotateEntity(r->Objects[2 + 1], -81, -180, 0, 0);
	bbRotateEntity(r->Objects[4 + 1], -81, -180, 0, 0);
	r->Objects[6] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[6], r->x - 320 * RoomScale, 0.5f, r->z, 1);
	r->Objects[7] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[7], r->x - 1328.0f * RoomScale, 0.5f, r->z + 528 * RoomScale, 1);
	r->Objects[8] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[8], r->x - 1376.0f * RoomScale, 0.5f, r->z + 32 * RoomScale, 1);
	r->Objects[9] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[9], r->x - 848 * RoomScale, 0.5f, r->z + 576 * RoomScale, 1);
}

// room3servers
static void f_room3servers(Rooms *r) {
	U Items * it = NULL;
	U int tex = 0;
	it = CreateItem("9V Battery", "bat", r->x - 132.0f * RoomScale, r->y - 368.0f * RoomScale, r->z - 648.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x - 76.0f * RoomScale, r->y - 368.0f * RoomScale, r->z - 648.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x - 196.0f * RoomScale, r->y - 368.0f * RoomScale, r->z - 648.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	it = CreateItem("S-NAV 300 Navigator", "nav", r->x + 124.0f * RoomScale, r->y - 368.0f * RoomScale, r->z - 648.0f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 20;
	bbEntityParent(it->collider, r->obj, 1);
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x + 736.0f * RoomScale, (-512.0f) * RoomScale, r->z - 400.0f * RoomScale, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x - 552.0f * RoomScale, (-512.0f) * RoomScale, r->z - 528.0f * RoomScale, 1);
	r->Objects[2] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[2], r->x + 736.0f * RoomScale, (-512.0f) * RoomScale, r->z + 272.0f * RoomScale, 1);
	r->Objects[3] = LoadMesh_Strict("GFX\\npcs\\duck_low_res.b3d", 0);
	bbScaleEntity(r->Objects[3], 0.07f, 0.07f, 0.07f, 0);
	tex = LoadTexture_Strict("GFX\\npcs\\duck2.png", 1);
	bbEntityTexture(r->Objects[3], tex, 0, 0);
	bbPositionEntity(r->Objects[3], r->x + 928.0f * RoomScale, (-640) * RoomScale, r->z + 704.0f * RoomScale, 0);
	bbEntityParent(r->Objects[3], r->obj, 1);
}

// room3servers2
static void f_room3servers2(Rooms *r) {
	U Items * it = NULL;
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x - 504.0f * RoomScale, (-512.0f) * RoomScale, r->z + 271.0f * RoomScale, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x + 628.0f * RoomScale, (-512.0f) * RoomScale, r->z + 271.0f * RoomScale, 1);
	r->Objects[2] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[2], r->x - 532.0f * RoomScale, (-512.0f) * RoomScale, r->z - 877.0f * RoomScale, 1);
	it = CreateItem("Document SCP-970", "paper", r->x + 960.0f * RoomScale, r->y - 448.0f * RoomScale, r->z + 251.0f * RoomScale, 0, 0, 0, 1, 0);
	bbRotateEntity(it->collider, 0, r->angle, 0, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Gas Mask", "gasmask", r->x + 954.0f * RoomScale, r->y - 504.0f * RoomScale, r->z + 235.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
}

// testroom
static void f_testroom(Rooms *r) {
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	U int xtemp = 0;
	U int ztemp = 0;
	for (xtemp = 0; xtemp <= 1; xtemp += 1) {
		for (ztemp = -1; ztemp <= 1; ztemp += 1) {
			r->Objects[xtemp * 3 + (ztemp + 1)] = bbCreatePivot(0);
			bbPositionEntity(r->Objects[xtemp * 3 + (ztemp + 1)], r->x + ((-236.0f + 280.0f * xtemp)) * RoomScale, (-700.0f) * RoomScale, r->z + 384.0f * ztemp * RoomScale, 0);
			bbEntityParent(r->Objects[xtemp * 3 + (ztemp + 1)], r->obj, 1);
		}
	}
	r->Objects[6] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[6], r->x + 754.0f * RoomScale, r->y - 1248.0f * RoomScale, r->z, 0);
	bbEntityParent(r->Objects[6], r->obj, 1);
	sc = CreateSecurityCam(r->x + 744.0f * RoomScale, r->y - 856.0f * RoomScale, r->z + 236.0f * RoomScale, r, 0);
	sc->FollowPlayer = 1;
	CreateDoor(0, r->x + 720.0f * RoomScale, 0, r->z, 0, r, 0, 2, -1, "", 0);
	CreateDoor(0, r->x - 624.0f * RoomScale, (-1280.0f) * RoomScale, r->z, 90, r, 1, 0, 0, "", 0);
	it = CreateItem("Document SCP-682", "paper", r->x + 656.0f * RoomScale, r->y - 1200.0f * RoomScale, r->z - 16.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
}

// room2closets
static void f_room2closets(Rooms *r) {
	U Items * clipboard = NULL;
	U Doors * d = NULL;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	it = CreateItem("Document SCP-1048", "paper", r->x + 736.0f * RoomScale, r->y + 176.0f * RoomScale, r->z + 736.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Gas Mask", "gasmask", r->x + 736.0f * RoomScale, r->y + 176.0f * RoomScale, r->z + 544.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("9V Battery", "bat", r->x + 736.0f * RoomScale, r->y + 176.0f * RoomScale, r->z - 448.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x + 730.0f * RoomScale, r->y + 176.0f * RoomScale, r->z - 496.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x + 740.0f * RoomScale, r->y + 176.0f * RoomScale, r->z - 560.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	it = CreateItem("Level 1 Key Card", "key1", r->x + 736.0f * RoomScale, r->y + 240.0f * RoomScale, r->z + 752.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	clipboard = CreateItem("Clipboard", "clipboard", r->x + 736.0f * RoomScale, r->y + 224.0f * RoomScale, r->z - 480.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Incident Report SCP-1048-A", "paper", r->x + 736.0f * RoomScale, r->y + 224.0f * RoomScale, r->z - 480.0f * RoomScale, 0, 0, 0, 1, 0);
	bbHideEntity(it->collider);
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x - 1120 * RoomScale, (-256) * RoomScale, r->z + 896 * RoomScale, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x - 1232 * RoomScale, (-256) * RoomScale, r->z - 160 * RoomScale, 1);
	d = CreateDoor(0, r->x - 240.0f * RoomScale, 0.0f, r->z, 90, r, 0, 0, 0, "", 0);
	bbPositionEntity(d->buttons[0], r->x - 230.0f * RoomScale, bbEntityY(d->buttons[0], 1), bbEntityZ(d->buttons[0], 1), 1);
	bbPositionEntity(d->buttons[1], r->x - 250.0f * RoomScale, bbEntityY(d->buttons[1], 1), bbEntityZ(d->buttons[1], 1), 1);
	d->open = 0;
	d->AutoClose = 0;
	sc = CreateSecurityCam(r->x, r->y + 704 * RoomScale, r->z + 863 * RoomScale, r, 0);
	sc->angle = 180;
	sc->turn = 45;
	bbTurnEntity(sc->CameraObj, 20, 0, 0, 0);
}

// room2offices
static void f_room2offices(Rooms *r) {
	U Items * it = NULL;
	U WayPoints * w = NULL;
	U WayPoints * w2 = NULL;
	it = CreateItem("Document SCP-106", "paper", r->x + 404.0f * RoomScale, r->y + 145.0f * RoomScale, r->z + 559.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Level 2 Key Card", "key2", r->x - 156.0f * RoomScale, r->y + 151.0f * RoomScale, r->z + 72.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("S-NAV 300 Navigator", "nav", r->x + 305.0f * RoomScale, r->y + 153.0f * RoomScale, r->z + 944.0f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 20;
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Notification", "paper", r->x - 137.0f * RoomScale, r->y + 153.0f * RoomScale, r->z + 464.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	w = CreateWaypoint(r->x - 32.0f * RoomScale, r->y + 66.0f * RoomScale, r->z + 288.0f * RoomScale, NULL, r);
	w2 = CreateWaypoint(r->x, r->y + 66.0f * RoomScale, r->z - 448.0f * RoomScale, NULL, r);
	w->connected[0] = w2;
	w->dist[0] = bbEntityDistance(w->obj, w2->obj);
	w2->connected[0] = w;
	w2->dist[0] = w->dist[0];
}

// room2offices2
static void f_room2offices2(Rooms *r) {
	U Items * it = NULL;
	U int temp = 0;
	it = CreateItem("Level 1 Key Card", "key1", r->x - 368.0f * RoomScale, r->y - 48.0f * RoomScale, r->z + 80.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-895", "paper", r->x - 800.0f * RoomScale, r->y - 48.0f * RoomScale, r->z + 368.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	if (bbRand(1, 2) == 1) {
		it = CreateItem("Document SCP-860", "paper", r->x - 800.0f * RoomScale, r->y - 48.0f * RoomScale, r->z - 464.0f * RoomScale, 0, 0, 0, 1, 0);
	} else {
		it = CreateItem("SCP-093 Recovered Materials", "paper", r->x - 800.0f * RoomScale, r->y - 48.0f * RoomScale, r->z - 464.0f * RoomScale, 0, 0, 0, 1, 0);
	}
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("S-NAV 300 Navigator", "nav", r->x - 336.0f * RoomScale, r->y - 48.0f * RoomScale, r->z - 480.0f * RoomScale, 0, 0, 0, 1, 0);
	it->state = 28;
	bbEntityParent(it->collider, r->obj, 1);
	r->Objects[0] = LoadMesh_Strict("GFX\\npcs\\duck_low_res.b3d", 0);
	bbScaleEntity(r->Objects[0], 0.07f, 0.07f, 0.07f, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x - 808.0f * RoomScale, (-72.0f) * RoomScale, r->z - 40.0f * RoomScale, 1);
	r->Objects[2] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[2], r->x - 488.0f * RoomScale, 160.0f * RoomScale, r->z + 700.0f * RoomScale, 1);
	r->Objects[3] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[3], r->x - 488.0f * RoomScale, 160.0f * RoomScale, r->z - 668.0f * RoomScale, 1);
	r->Objects[4] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[4], r->x - 572.0f * RoomScale, 350.0f * RoomScale, r->z - 4.0f * RoomScale, 1);
	temp = bbRand(1, 4);
	bbPositionEntity(r->Objects[0], bbEntityX(r->Objects[temp], 1), bbEntityY(r->Objects[temp], 1), bbEntityZ(r->Objects[temp], 1), 1);
}

// room2offices3
static void f_room2offices3(Rooms *r) {
	U int i = 0;
	U Items * it = NULL;
	if (bbRand(1, 2) == 1) {
		it = CreateItem("Mobile Task Forces", "paper", r->x + 744.0f * RoomScale, r->y + 240.0f * RoomScale, r->z + 944.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	} else {
		it = CreateItem("Security Clearance Levels", "paper", r->x + 680.0f * RoomScale, r->y + 240.0f * RoomScale, r->z + 944.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	it = CreateItem("Object Classes", "paper", r->x + 160.0f * RoomScale, r->y + 240.0f * RoomScale, r->z + 568.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Document", "paper", r->x - 1440.0f * RoomScale, r->y + 624.0f * RoomScale, r->z + 152.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Radio Transceiver", "radio", r->x - 1184.0f * RoomScale, r->y + 480.0f * RoomScale, r->z - 800.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	{ int to_ = bbRand(0, 1);
	for (i = 0; i <= to_; i += 1) {
		it = CreateItem("ReVision Eyedrops", "eyedrops", r->x - 1529.0f * RoomScale, r->y + 563.0f * RoomScale, r->z - 572.0f * RoomScale + i * 0.05f, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	} }
	it = CreateItem("9V Battery", "bat", r->x - 1545.0f * RoomScale, r->y + 603.0f * RoomScale, r->z - 372.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x - 1540.0f * RoomScale, r->y + 603.0f * RoomScale, r->z - 340.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	if (bbRand(1, 2) == 1) {
		it = CreateItem("9V Battery", "bat", r->x - 1529.0f * RoomScale, r->y + 603.0f * RoomScale, r->z - 308.0f * RoomScale, 0, 0, 0, 1, 0);
		bbEntityParent(it->collider, r->obj, 1);
	}
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 1056.0f * RoomScale, 384.0f * RoomScale, r->z + 290.0f * RoomScale, 90, r, 1, 0, 0, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 1;
	bbPositionEntity(r->RoomDoors[0]->buttons[0], bbEntityX(r->RoomDoors[0]->buttons[0], 1), bbEntityY(r->RoomDoors[0]->buttons[0], 1), r->z + 161.0f * RoomScale, 1);
	bbPositionEntity(r->RoomDoors[0]->buttons[1], bbEntityX(r->RoomDoors[0]->buttons[1], 1), bbEntityY(r->RoomDoors[0]->buttons[1], 1), r->z + 161.0f * RoomScale, 1);
}

// start
static void f_start(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U int i = 0;
	U SecurityCams * sc = NULL;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 4000.0f * RoomScale, 384.0f * RoomScale, r->z + 1696.0f * RoomScale, 90, r, 1, 1, 0, "", 0);
	r->RoomDoors[1]->locked = 0;
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->dir = 1;
	r->RoomDoors[1]->open = 1;
	bbFreeEntity(r->RoomDoors[1]->buttons[0]);
	r->RoomDoors[1]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[1]->buttons[1]);
	r->RoomDoors[1]->buttons[1] = 0;
	r->RoomDoors[1]->MTFClose = 0;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x + 2704.0f * RoomScale, 384.0f * RoomScale, r->z + 624.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	bbFreeEntity(r->RoomDoors[2]->buttons[0]);
	r->RoomDoors[2]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[2]->buttons[1]);
	r->RoomDoors[2]->buttons[1] = 0;
	r->RoomDoors[2]->MTFClose = 0;
	d = CreateDoor(r->zone, r->x + 1392.0f * RoomScale, 384.0f * RoomScale, r->z + 64.0f * RoomScale, 90, r, 1, 0, 0, "", 0);
	d->AutoClose = 0;
	d->MTFClose = 0;
	d->locked = 1;
	d = CreateDoor(r->zone, r->x - 640.0f * RoomScale, 384.0f * RoomScale, r->z + 64.0f * RoomScale, 90, r, 0, 0, 0, "", 0);
	d->locked = 1;
	d->AutoClose = 0;
	d = CreateDoor(r->zone, r->x + 1280.0f * RoomScale, 384.0f * RoomScale, r->z + 312.0f * RoomScale, 180, r, 1, 0, 0, "", 0);
	d->locked = 1;
	d->AutoClose = 0;
	bbPositionEntity(d->buttons[0], r->x + 1120.0f * RoomScale, bbEntityY(d->buttons[0], 1), r->z + 328.0f * RoomScale, 1);
	bbPositionEntity(d->buttons[1], r->x + 1120.0f * RoomScale, bbEntityY(d->buttons[1], 1), r->z + 296.0f * RoomScale, 1);
	bbFreeEntity(d->obj2);
	d->obj2 = 0;
	d->MTFClose = 0;
	d = CreateDoor(r->zone, r->x, 0, r->z + 1184.0f * RoomScale, 0, r, 0, 0, 0, "", 0);
	d->locked = 1;
	r->Objects[0] = LoadMesh_Strict("GFX\\map\\IntroDesk.b3d", 0);
	bbScaleEntity(r->Objects[0], RoomScale, RoomScale, RoomScale, 0);
	bbPositionEntity(r->Objects[0], r->x + 272.0f * RoomScale, 0, r->z + 400.0f * RoomScale, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	de = CreateDecal(0, r->x + 272.0f * RoomScale, 0.005f, r->z + 262.0f * RoomScale, 90, bbRand(1, 360), 0);
	bbEntityParent(de->obj, r->obj, 1);
	r->Objects[1] = LoadMesh_Strict("GFX\\map\\IntroDrawer.b3d", 0);
	bbScaleEntity(r->Objects[1], RoomScale, RoomScale, RoomScale, 0);
	bbPositionEntity(r->Objects[1], r->x + 448.0f * RoomScale, 0, r->z + 192.0f * RoomScale, 0);
	bbEntityParent(r->Objects[1], r->obj, 1);
	de = CreateDecal(0, r->x + 456.0f * RoomScale, 0.005f, r->z + 135.0f * RoomScale, 90, bbRand(1, 360), 0);
	bbEntityParent(de->obj, r->obj, 1);
	sc = CreateSecurityCam(r->x - 336.0f * RoomScale, r->y + 352 * RoomScale, r->z + 48.0f * RoomScale, r, 1);
	sc->angle = 270;
	sc->turn = 45;
	sc->room = r;
	bbTurnEntity(sc->CameraObj, 20, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	bbPositionEntity(sc->ScrObj, r->x + 1456 * RoomScale, 608 * RoomScale, r->z + 352.0f * RoomScale, 0);
	bbTurnEntity(sc->ScrObj, 0, 90, 0, 0);
	bbEntityParent(sc->ScrObj, r->obj, 1);
	r->Objects[2] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[2], bbEntityX(r->obj, 0) + 40.0f * RoomScale, 460.0f * RoomScale, bbEntityZ(r->obj, 0) + 1072.0f * RoomScale, 0);
	r->Objects[3] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[3], bbEntityX(r->obj, 0) - 80.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 526.0f * RoomScale, 0);
	r->Objects[4] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[4], bbEntityX(r->obj, 0) - 128.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 320.0f * RoomScale, 0);
	r->Objects[5] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[5], bbEntityX(r->obj, 0) + 660.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 526.0f * RoomScale, 0);
	r->Objects[6] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[6], bbEntityX(r->obj, 0) + 700 * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 320.0f * RoomScale, 0);
	r->Objects[7] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[7], bbEntityX(r->obj, 0) + 1472.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 912.0f * RoomScale, 0);
	for (i = 2; i <= 7; i += 1) {
		bbEntityParent(r->Objects[i], r->obj, 1);
	}
	CreateDevilEmitter(r->x + 3384.0f * RoomScale, r->y + 510.0f * RoomScale, r->z + 2400.0f * RoomScale, r, 1, 4);
}

// room2scps
static void f_room2scps(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U float dx = 0;
	U float dz = 0;
	U int i = 0;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x + 264.0f * RoomScale, 0, r->z, 90, r, 1, 0, 3, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	bbPositionEntity(d->buttons[0], r->x + 320.0f * RoomScale, bbEntityY(d->buttons[0], 1), bbEntityZ(d->buttons[0], 1), 1);
	bbPositionEntity(d->buttons[1], r->x + 224.0f * RoomScale, bbEntityY(d->buttons[1], 1), bbEntityZ(d->buttons[1], 1), 1);
	d = CreateDoor(r->zone, r->x - 264.0f * RoomScale, 0, r->z, 270, r, 1, 0, 3, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	bbPositionEntity(d->buttons[0], r->x - 320.0f * RoomScale, bbEntityY(d->buttons[0], 1), bbEntityZ(d->buttons[0], 1), 1);
	bbPositionEntity(d->buttons[1], r->x - 224.0f * RoomScale, bbEntityY(d->buttons[1], 1), bbEntityZ(d->buttons[1], 1), 1);
	r->RoomDoors[1] = CreateDoor(r->zone, r->x - 560.0f * RoomScale, 0, r->z - 272.0f * RoomScale, 0, r, 1, 0, 3, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x + 560.0f * RoomScale, 0, r->z - 272.0f * RoomScale, 180, r, 1, 0, 3, "", 0);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	r->RoomDoors[3] = CreateDoor(r->zone, r->x + 560.0f * RoomScale, 0, r->z + 272.0f * RoomScale, 180, r, 1, 0, 3, "", 0);
	r->RoomDoors[3]->AutoClose = 0;
	r->RoomDoors[3]->open = 0;
	r->RoomDoors[4] = CreateDoor(r->zone, r->x - 560.0f * RoomScale, 0, r->z + 272.0f * RoomScale, 0, r, 1, 0, 3, "", 0);
	r->RoomDoors[4]->AutoClose = 0;
	r->RoomDoors[4]->open = 0;
	it = CreateItem("SCP-714", "scp714", r->x - 552.0f * RoomScale, r->y + 220.0f * RoomScale, r->z - 760.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("SCP-1025", "scp1025", r->x + 552.0f * RoomScale, r->y + 224.0f * RoomScale, r->z - 758.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("SCP-860", "scp860", r->x + 568.0f * RoomScale, r->y + 178.0f * RoomScale, r->z + 760.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	sc = CreateSecurityCam(r->x + 560.0f * RoomScale, r->y + 386 * RoomScale, r->z - 416.0f * RoomScale, r, 0);
	sc->angle = 180;
	sc->turn = 30;
	bbTurnEntity(sc->CameraObj, 30, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	sc = CreateSecurityCam(r->x - 560.0f * RoomScale, r->y + 386 * RoomScale, r->z - 416.0f * RoomScale, r, 0);
	sc->angle = 180;
	sc->turn = 30;
	bbTurnEntity(sc->CameraObj, 30, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	sc = CreateSecurityCam(r->x + 560.0f * RoomScale, r->y + 386 * RoomScale, r->z + 480.0f * RoomScale, r, 0);
	sc->angle = 0;
	sc->turn = 30;
	bbTurnEntity(sc->CameraObj, 30, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	sc = CreateSecurityCam(r->x - 560.0f * RoomScale, r->y + 386 * RoomScale, r->z + 480.0f * RoomScale, r, 0);
	sc->angle = 0;
	sc->turn = 30;
	bbTurnEntity(sc->CameraObj, 30, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	it = CreateItem("Document SCP-714", "paper", r->x - 728.0f * RoomScale, r->y + 288.0f * RoomScale, r->z - 360.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Document SCP-427", "paper", r->x - 608.0f * RoomScale, r->y + 66.0f * RoomScale, r->z + 636.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	for (i = 0; i <= 14; i += 1) {
		if (i == 0) {
			dx = -64.0f;
			dz = -516.0f;
		} else if (i == 1) {
			dx = -96.0f;
			dz = -388.0f;
		} else if (i == 2) {
			dx = -128.0f;
			dz = -292.0f;
		} else if (i == 3) {
			dx = -128.0f;
			dz = -132.0f;
		} else if (i == 4) {
			dx = -160.0f;
			dz = -36.0f;
		} else if (i == 5) {
			dx = -192.0f;
			dz = 28.0f;
		} else if (i == 6) {
			dx = -384.0f;
			dz = 28.0f;
		} else if (i == 7) {
			dx = -448.0f;
			dz = 92.0f;
		} else if (i == 8) {
			dx = -480.0f;
			dz = 124.0f;
		} else if (i == 9) {
			dx = -512.0f;
			dz = 156.0f;
		} else if (i == 10) {
			dx = -544.0f;
			dz = 220.0f;
		} else if (i == 11) {
			dx = -544.0f;
			dz = 380.0f;
		} else if (i == 12) {
			dx = -544.0f;
			dz = 476.0f;
		} else if (i == 13) {
			dx = -544.0f;
			dz = 572.0f;
		} else if (i == 14) {
			dx = -544.0f;
			dz = 636.0f;
		}
		int rn1 = bbRand(15, 16);
		int rn2 = bbRand(1, 360);
		de = CreateDecal(rn1, r->x + dx * RoomScale, 0.005f, r->z + dz * RoomScale, 90, rn2, 0);
		if (i > 10) {
			de->Size = bbRnd(0.2f, 0.25f);
		} else {
			de->Size = bbRnd(0.1f, 0.17f);
		}
		bbEntityAlpha(de->obj, 1.0f);
		bbScaleSprite(de->obj, de->Size, de->Size);
		bbEntityParent(de->obj, r->obj, 1);
	}
}

// room205
static void f_room205(Rooms *r) {
	U SecurityCams * sc = NULL;
	r->RoomDoors[1] = CreateDoor(r->zone, r->x + 128.0f * RoomScale, 0, r->z + 640.0f * RoomScale, 90, r, 1, 0, 3, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->open = 0;
	r->RoomDoors[0] = CreateDoor(r->zone, r->x - 1392.0f * RoomScale, (-128.0f) * RoomScale, r->z - 384 * RoomScale, 0, r, 1, 0, 3, "", 1);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	bbFreeEntity(r->RoomDoors[0]->buttons[0]);
	r->RoomDoors[0]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[0]->buttons[1]);
	r->RoomDoors[0]->buttons[1] = 0;
	sc = CreateSecurityCam(r->x - 1152.0f * RoomScale, r->y + 900.0f * RoomScale, r->z + 176.0f * RoomScale, r, 1);
	sc->angle = 90;
	sc->turn = 0;
	bbEntityParent(sc->obj, r->obj, 1);
	sc->AllowSaving = 0;
	sc->RenderInterval = 0;
	bbEntityParent(sc->ScrObj, 0, 1);
	bbPositionEntity(sc->ScrObj, r->x - 1716.0f * RoomScale, r->y + 160.0f * RoomScale, r->z + 176.0f * RoomScale, 1);
	bbTurnEntity(sc->ScrObj, 0, 90, 0, 0);
	bbScaleSprite(sc->ScrObj, 896.0f * 0.5f * RoomScale, 896.0f * 0.5f * RoomScale);
	bbEntityParent(sc->ScrObj, r->obj, 1);
	bbCameraZoom(sc->Cam, 1.5f);
	bbHideEntity(sc->ScrOverlay);
	bbHideEntity(sc->MonitorObj);
	r->Objects[0] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[0], r->x - 1536.0f * RoomScale, r->y + 730.0f * RoomScale, r->z + 192.0f * RoomScale, 1);
	bbRotateEntity(r->Objects[0], 0, -90, 0, 1);
	r->Objects[1] = sc->ScrObj;
}

// endroom
static void f_endroom(Rooms *r) {

	r->RoomDoors[0] = CreateDoor(r->zone, r->x, 0, r->z + 1136 * RoomScale, 0, r, 0, 1, 6, "", 0);
	r->RoomDoors[0]->AutoClose = 0;
	r->RoomDoors[0]->open = 0;
	bbFreeEntity(r->RoomDoors[0]->buttons[0]);
	r->RoomDoors[0]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[0]->buttons[1]);
	r->RoomDoors[0]->buttons[1] = 0;
}

// endroomc
static void f_endroomc(Rooms *r) {
	U Doors * d = NULL;
	d = CreateDoor(r->zone, r->x + 1024 * RoomScale, 0, r->z, 0, r, 0, 2, 0, "", 0);
	d->open = 0;
	d->AutoClose = 0;
	d->locked = 1;
}

// coffin
static void f_coffin(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	U SecurityCams * sc = NULL;
	d = CreateDoor(r->zone, r->x, 0, r->z - 448.0f * RoomScale, 0, r, 0, 1, 2, "", 0);
	d->AutoClose = 0;
	d->open = 0;
	bbPositionEntity(d->buttons[0], r->x - 384.0f * RoomScale, 0.7f, r->z - 280.0f * RoomScale, 1);
	sc = CreateSecurityCam(r->x - 320.0f * RoomScale, r->y + 704 * RoomScale, r->z + 288.0f * RoomScale, r, 1);
	sc->angle = 45 + 180;
	sc->turn = 45;
	sc->CoffinEffect = 1;
	bbTurnEntity(sc->CameraObj, 120, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	CoffinCam = sc;
	bbPositionEntity(sc->ScrObj, r->x - 800 * RoomScale, 288.0f * RoomScale, r->z - 340.0f * RoomScale, 0);
	bbEntityParent(sc->ScrObj, r->obj, 1);
	bbTurnEntity(sc->ScrObj, 0, 180, 0, 0);
	r->Objects[2] = bbCopyEntity(LeverBaseOBJ, 0);
	r->Objects[3] = bbCopyEntity(LeverOBJ, 0);
	r->Levers[0] = r->Objects[3];
	for (i = 0; i <= 1; i += 1) {
		bbScaleEntity(r->Objects[2 + i], 0.04f, 0.04f, 0.04f, 0);
		bbPositionEntity(r->Objects[2 + i], r->x - 800.0f * RoomScale, r->y + 180.0f * RoomScale, r->z - 336 * RoomScale, 1);
		bbEntityParent(r->Objects[2 + i], r->obj, 1);
	}
	bbRotateEntity(r->Objects[2], 0, 180, 0, 0);
	bbRotateEntity(r->Objects[3], 10, 0, 0, 0);
	bbEntityPickMode(r->Objects[3], 1, 0);
	bbEntityRadius(r->Objects[3], 0.1f, 0.1f);
	r->Objects[0] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[0], r->x, (-1320.0f) * RoomScale, r->z + 2304.0f * RoomScale, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	it = CreateItem("Document SCP-895", "paper", r->x - 688.0f * RoomScale, r->y + 133.0f * RoomScale, r->z - 304.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Level 3 Key Card", "key3", r->x + 240.0f * RoomScale, r->y - 1456.0f * RoomScale, r->z + 2064.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("Night Vision Goggles", "nvgoggles", r->x + 280.0f * RoomScale, r->y - 1456.0f * RoomScale, r->z + 2164.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it->state = 400;
	r->Objects[1] = bbCreatePivot(r->obj);
	bbPositionEntity(r->Objects[1], r->x + 96.0f * RoomScale, (-1532.0f) * RoomScale, r->z + 2016.0f * RoomScale, 1);
}

// room2tesla, room2tesla_lcz, room2tesla_hcz
static void f_room2tesla(Rooms *r) {
	U Rooms * r2 = NULL;
	U WayPoints * w = NULL;
	U WayPoints * w2 = NULL;
	r->Objects[0] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[0], r->x - 114.0f * RoomScale, 0.0f, r->z, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	r->Objects[1] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[1], r->x + 114.0f * RoomScale, 0.0f, r->z, 0);
	bbEntityParent(r->Objects[1], r->obj, 1);
	r->Objects[2] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[2], r->x, 0.0f, r->z, 0);
	bbEntityParent(r->Objects[2], r->obj, 1);
	r->Objects[3] = bbCreateSprite(0);
	bbEntityTexture(r->Objects[3], TeslaTexture, 0, 0);
	bbSpriteViewMode(r->Objects[3], 2);
	bbEntityBlend(r->Objects[3], 3);
	bbEntityFX(r->Objects[3], 1 + 8 + 16);
	bbPositionEntity(r->Objects[3], r->x, 0.8f, r->z, 0);
	bbHideEntity(r->Objects[3]);
	bbEntityParent(r->Objects[3], r->obj, 1);
	w = CreateWaypoint(r->x, r->y + 66.0f * RoomScale, r->z + 292.0f * RoomScale, NULL, r);
	w2 = CreateWaypoint(r->x, r->y + 66.0f * RoomScale, r->z - 284.0f * RoomScale, NULL, r);
	w->connected[0] = w2;
	w->dist[0] = bbEntityDistance(w->obj, w2->obj);
	w2->connected[0] = w;
	w2->dist[0] = w->dist[0];
	r->Objects[4] = bbCreateSprite(0);
	bbPositionEntity(r->Objects[4], r->x - 32 * RoomScale, 568 * RoomScale, r->z, 0);
	bbScaleSprite(r->Objects[4], 0.03f, 0.03f);
	bbEntityTexture(r->Objects[4], LightSpriteTex[1], 0, 0);
	bbEntityBlend(r->Objects[4], 3);
	bbEntityParent(r->Objects[4], r->obj, 1);
	bbHideEntity(r->Objects[4]);
	r->Objects[5] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[5], r->x, 0, r->z - 800 * RoomScale, 0);
	bbEntityParent(r->Objects[5], r->obj, 1);
	r->Objects[6] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[6], r->x, 0, r->z + 800 * RoomScale, 0);
	bbEntityParent(r->Objects[6], r->obj, 1);
	for (r2 = bbFirst(Rooms); r2; r2 = bbAfter(r2)) {
		if (r2 != r) {
			if (((((bbSeq(r2->RoomTemplate->Name, "room2tesla")) | (bbSeq(r2->RoomTemplate->Name, "room2tesla_lcz")))) | (bbSeq(r2->RoomTemplate->Name, "room2tesla_hcz")))) {
				r->Objects[7] = bbCopyEntity(r2->Objects[7], r->obj);
				break;
			}
		}
	}
	if (r->Objects[7] == 0) {
		r->Objects[7] = LoadMesh_Strict("GFX\\map\\room2tesla_caution.b3d", r->obj);
	}
}

// room2doors
static void f_room2doors(Rooms *r) {
	U Doors * d = NULL;
	U Doors * d2 = NULL;
	d = CreateDoor(r->zone, r->x, 0, r->z + 528.0f * RoomScale, 0, r, 1, 0, 0, "", 0);
	d->AutoClose = 0;
	bbPositionEntity(d->buttons[0], r->x - 832.0f * RoomScale, 0.7f, r->z + 160.0f * RoomScale, 1);
	bbPositionEntity(d->buttons[1], r->x + 160.0f * RoomScale, 0.7f, r->z + 536.0f * RoomScale, 1);
	d2 = CreateDoor(r->zone, r->x, 0, r->z - 528.0f * RoomScale, 180, r, 1, 0, 0, "", 0);
	d2->AutoClose = 0;
	bbFreeEntity(d2->buttons[0]);
	d2->buttons[0] = 0;
	bbPositionEntity(d2->buttons[1], r->x + 160.0f * RoomScale, 0.7f, r->z - 536.0f * RoomScale, 1);
	r->Objects[0] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[0], r->x - 832.0f * RoomScale, 0.5f, r->z, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	d2->LinkedDoor = d;
	d->LinkedDoor = d2;
	d->open = 0;
	d2->open = 1;
}

// 914
static void f_914(Rooms *r) {
	U Doors * d = NULL;
	U int i = 0;
	U Items * it = NULL;
	r->RoomDoors[2] = CreateDoor(r->zone, r->x, 0, r->z - 368.0f * RoomScale, 0, r, 0, 1, 2, "", 0);
	r->RoomDoors[2]->dir = 1;
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	bbPositionEntity(r->RoomDoors[2]->buttons[0], r->x - 496.0f * RoomScale, 0.7f, r->z - 272.0f * RoomScale, 1);
	bbTurnEntity(r->RoomDoors[2]->buttons[0], 0, 90, 0, 0);
	r->Objects[0] = LoadMesh_Strict("GFX\\map\\914key.x", 0);
	r->Objects[1] = LoadMesh_Strict("GFX\\map\\914knob.x", 0);
	for (i = 0; i <= 1; i += 1) {
		bbScaleEntity(r->Objects[i], RoomScale, RoomScale, RoomScale, 0);
		bbEntityPickMode(r->Objects[i], 2, 1);
	}
	bbPositionEntity(r->Objects[0], r->x, r->y + 190.0f * RoomScale, r->z + 374.0f * RoomScale, 0);
	bbPositionEntity(r->Objects[1], r->x, r->y + 230.0f * RoomScale, r->z + 374.0f * RoomScale, 0);
	bbEntityParent(r->Objects[0], r->obj, 1);
	bbEntityParent(r->Objects[1], r->obj, 1);
	d = CreateDoor(r->zone, r->x - 624.0f * RoomScale, 0.0f, r->z + 528.0f * RoomScale, 180, r, 1, 0, 0, "", 0);
	bbFreeEntity(d->obj2);
	d->obj2 = 0;
	bbFreeEntity(d->buttons[0]);
	d->buttons[0] = 0;
	bbFreeEntity(d->buttons[1]);
	d->buttons[1] = 0;
	d->dir = 4;
	r->RoomDoors[0] = d;
	d->AutoClose = 0;
	d = CreateDoor(r->zone, r->x + 816.0f * RoomScale, 0.0f, r->z + 528.0f * RoomScale, 180, r, 1, 0, 0, "", 0);
	bbFreeEntity(d->obj2);
	d->obj2 = 0;
	bbFreeEntity(d->buttons[0]);
	d->buttons[0] = 0;
	bbFreeEntity(d->buttons[1]);
	d->buttons[1] = 0;
	d->dir = 4;
	r->RoomDoors[1] = d;
	d->AutoClose = 0;
	r->Objects[2] = bbCreatePivot(0);
	r->Objects[3] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[2], r->x - 712.0f * RoomScale, 0.5f, r->z + 640.0f * RoomScale, 0);
	bbPositionEntity(r->Objects[3], r->x + 728.0f * RoomScale, 0.5f, r->z + 640.0f * RoomScale, 0);
	bbEntityParent(r->Objects[2], r->obj, 1);
	bbEntityParent(r->Objects[3], r->obj, 1);
	it = CreateItem("Addendum: 5/14 Test Log", "paper", r->x + 954.0f * RoomScale, r->y + 228.0f * RoomScale, r->z + 127.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	it = CreateItem("First Aid Kit", "firstaid", r->x + 960.0f * RoomScale, r->y + 112.0f * RoomScale, r->z - 40.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
	bbRotateEntity(it->collider, 0, 90, 0, 0);
	it = CreateItem("Dr. L's Note", "paper", r->x - 928.0f * RoomScale, 160.0f * RoomScale, r->z - 160.0f * RoomScale, 0, 0, 0, 1, 0);
	bbEntityParent(it->collider, r->obj, 1);
}

// 173
static void f_173(Rooms *r) {
	U Doors * d = NULL;
	U Decals * de = NULL;
	U int i = 0;
	U SecurityCams * sc = NULL;
	U int tex = 0;
	U int xtemp = 0;
	U int ztemp = 0;
	r->Objects[0] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[0], bbEntityX(r->obj, 0) + 40.0f * RoomScale, 460.0f * RoomScale, bbEntityZ(r->obj, 0) + 1072.0f * RoomScale, 0);
	r->Objects[1] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[1], bbEntityX(r->obj, 0) - 80.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 526.0f * RoomScale, 0);
	r->Objects[2] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[2], bbEntityX(r->obj, 0) - 128.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 320.0f * RoomScale, 0);
	r->Objects[3] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[3], bbEntityX(r->obj, 0) + 660.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 526.0f * RoomScale, 0);
	r->Objects[4] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[4], bbEntityX(r->obj, 0) + 700 * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 320.0f * RoomScale, 0);
	r->Objects[5] = bbCreatePivot(0);
	bbPositionEntity(r->Objects[5], bbEntityX(r->obj, 0) + 1472.0f * RoomScale, 100.0f * RoomScale, bbEntityZ(r->obj, 0) + 912.0f * RoomScale, 0);
	for (i = 0; i <= 5; i += 1) {
		bbEntityParent(r->Objects[i], r->obj, 1);
	}
	r->RoomDoors[1] = CreateDoor(r->zone, bbEntityX(r->obj, 0) + 288.0f * RoomScale, 0, bbEntityZ(r->obj, 0) + 384.0f * RoomScale, 90, r, 0, 1, 0, "", 0);
	r->RoomDoors[1]->AutoClose = 0;
	r->RoomDoors[1]->dir = 1;
	r->RoomDoors[1]->open = 0;
	bbFreeEntity(r->RoomDoors[1]->buttons[0]);
	r->RoomDoors[1]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[1]->buttons[1]);
	r->RoomDoors[1]->buttons[1] = 0;
	int rn1 = bbRand(4, 5);
	float rn2 = bbRnd(0, 360);
	de = CreateDecal(rn1, bbEntityX(r->Objects[5], 1), 0.002f, bbEntityZ(r->Objects[5], 1), 90, rn2, 0);
	de->Size = 1.2f;
	bbScaleSprite(de->obj, de->Size, de->Size);
	for (xtemp = 0; xtemp <= 1; xtemp += 1) {
		for (ztemp = 0; ztemp <= 1; ztemp += 1) {
			int rn3 = bbRand(4, 6);
			float rn4 = bbRnd(-0.5f, 0.5f);
			float rn5 = bbRnd(0.001f, 0.0018f);
			float rn6 = bbRnd(-0.5f, 0.5f);
			float rn7 = bbRnd(0, 360);
			de = CreateDecal(rn3, r->x + 700.0f * RoomScale + xtemp * 700.0f * RoomScale + rn4, rn5, r->z + 600 * ztemp * RoomScale + rn6, 90, rn7, 0);
			de->Size = bbRnd(0.5f, 0.8f);
			de->Alpha = bbRnd(0.8f, 1.0f);
			bbScaleSprite(de->obj, de->Size, de->Size);
		}
	}
	r->RoomDoors[2] = CreateDoor(r->zone, r->x - 1008.0f * RoomScale, 0, r->z - 688.0f * RoomScale, 90, r, 1, 0, 0, "", 1);
	r->RoomDoors[2]->AutoClose = 0;
	r->RoomDoors[2]->open = 0;
	r->RoomDoors[2]->locked = 1;
	bbFreeEntity(r->RoomDoors[2]->buttons[0]);
	r->RoomDoors[2]->buttons[0] = 0;
	bbFreeEntity(r->RoomDoors[2]->buttons[1]);
	r->RoomDoors[2]->buttons[1] = 0;
	r->RoomDoors[3] = CreateDoor(r->zone, r->x - 2320.0f * RoomScale, 0, r->z - 1248.0f * RoomScale, 90, r, 1, 0, 0, "", 0);
	r->RoomDoors[3]->AutoClose = 0;
	r->RoomDoors[3]->open = 1;
	r->RoomDoors[3]->locked = 1;
	r->RoomDoors[4] = CreateDoor(r->zone, r->x - 4352.0f * RoomScale, 0, r->z - 1248.0f * RoomScale, 90, r, 1, 0, 0, "", 0);
	r->RoomDoors[4]->AutoClose = 0;
	r->RoomDoors[4]->open = 1;
	r->RoomDoors[4]->locked = 1;
	r->RoomDoors[7] = CreateDoor(r->zone, r->x - 3712.0f * RoomScale, (-385) * RoomScale, r->z - 128.0f * RoomScale, 0, r, 1, 0, 0, "", 0);
	r->RoomDoors[7]->AutoClose = 0;
	r->RoomDoors[7]->open = 1;
	d = CreateDoor(r->zone, r->x - 3712 * RoomScale, (-385) * RoomScale, r->z - 2336 * RoomScale, 0, r, 0, 0, 0, "", 0);
	d->locked = 1;
	d->DisableWaypoint = 1;
	d = CreateDoor(r->zone, r->x - 6864 * RoomScale, 0, r->z - 1248 * RoomScale, 90, r, 1, 0, 0, "", 0);
	d->AutoClose = 0;
	d->locked = 1;
	d = CreateDoor(r->zone, r->x - 5856 * RoomScale, 0, r->z - 1504 * RoomScale, 0, r, 0, 0, 0, "", 0);
	d->locked = 1;
	d->DisableWaypoint = 1;
	d = CreateDoor(r->zone, r->x - 2432 * RoomScale, 0, r->z - 1000 * RoomScale, 0, r, 0, 0, 0, "", 0);
	bbPositionEntity(d->buttons[0], r->x - 2592 * RoomScale, bbEntityY(d->buttons[0], 1), r->z - 1016 * RoomScale, 1);
	bbPositionEntity(d->buttons[1], r->x - 2592 * RoomScale, bbEntityY(d->buttons[0], 1), r->z - 984 * RoomScale, 1);
	d->locked = 1;
	d->DisableWaypoint = 1;
	tex = LoadTexture_Strict("GFX\\map\\Door02.jpg", 1);
	for (ztemp = 0; ztemp <= 1; ztemp += 1) {
		d = CreateDoor(r->zone, r->x - 5760 * RoomScale, 0, r->z + ((320 + 896 * ztemp)) * RoomScale, 0, r, 0, 0, 0, "", 0);
		d->locked = 1;
		d->DisableWaypoint = 1;
		d = CreateDoor(r->zone, r->x - 8288 * RoomScale, 0, r->z + ((320 + 896 * ztemp)) * RoomScale, 0, r, 0, 0, 0, "", 0);
		d->locked = 1;
		if (ztemp == 0) {
			d->open = 1;
		} else {
			d->DisableWaypoint = 1;
		}
		for (xtemp = 0; xtemp <= 2; xtemp += 1) {
			d = CreateDoor(r->zone, r->x - ((7424.0f - 512.0f * xtemp)) * RoomScale, 0, r->z + ((1008.0f - 480.0f * ztemp)) * RoomScale, 180 * (!(ztemp)), r, 0, 0, 0, "", 0);
			bbEntityTexture(d->obj, tex, 0, 0);
			d->locked = 1;
			bbFreeEntity(d->obj2);
			d->obj2 = 0;
			bbFreeEntity(d->buttons[0]);
			d->buttons[0] = 0;
			bbFreeEntity(d->buttons[1]);
			d->buttons[1] = 0;
			d->DisableWaypoint = 1;
		}
		for (xtemp = 0; xtemp <= 4; xtemp += 1) {
			d = CreateDoor(r->zone, r->x - ((5120.0f - 512.0f * xtemp)) * RoomScale, 0, r->z + ((1008.0f - 480.0f * ztemp)) * RoomScale, 180 * (!(ztemp)), r, 0, 0, 0, "", 0);
			bbEntityTexture(d->obj, tex, 0, 0);
			d->locked = 1;
			bbFreeEntity(d->obj2);
			d->obj2 = 0;
			bbFreeEntity(d->buttons[0]);
			d->buttons[0] = 0;
			bbFreeEntity(d->buttons[1]);
			d->buttons[1] = 0;
			d->DisableWaypoint = 1;
			if (((xtemp == 2) & (ztemp == 1))) {
				r->RoomDoors[6] = d;
			}
		}
	}
	CreateItem("Class D Orientation Leaflet", "paper", r->x - ((2914 + 1024)) * RoomScale, 170.0f * RoomScale, r->z + 40 * RoomScale, 0, 0, 0, 1, 0);
	sc = CreateSecurityCam(r->x - 4048.0f * RoomScale, r->y - 32.0f * RoomScale, r->z - 1232.0f * RoomScale, r, 1);
	sc->angle = 270;
	sc->turn = 45;
	sc->room = r;
	bbTurnEntity(sc->CameraObj, 20, 0, 0, 0);
	bbEntityParent(sc->obj, r->obj, 1);
	bbPositionEntity(sc->ScrObj, r->x - 2256 * RoomScale, 224.0f * RoomScale, r->z - 928.0f * RoomScale, 0);
	bbTurnEntity(sc->ScrObj, 0, 90, 0, 0);
	bbEntityParent(sc->ScrObj, r->obj, 1);
	r->Objects[9] = LoadMesh_Strict("GFX\\map\\173_2.b3d", r->obj);
	bbEntityType(r->Objects[9], HIT_MAP, 0);
	bbEntityPickMode(r->Objects[9], 2, 1);
	r->Objects[10] = LoadMesh_Strict("GFX\\map\\intro_labels.b3d", r->obj);
}

const FillEntry Fills2[] = {
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
	{0, 0},
};
