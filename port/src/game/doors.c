// doors.c: Main.bb's CreateDoor/CreateButton (UpdateDoors & co come w/ the in-game loop)
#include "g.h"

TLIST(Doors);

static int hitmesh(Doors *d) { // shared doorhit.b3d, loaded once
	each(Doors, d2) if (d2 != d && d2->DoorHitOBJ) return bbCopyEntity(d2->DoorHitOBJ, d->frameobj);
	return LoadMesh_Strict("GFX\\doorhit.b3d", d->frameobj);
}

Doors *CreateDoor(int lvl, float x, float y, float z, float angle, Rooms *room, int dopen, int big, int keycard, const char *code, int useCollisionMesh) {
	int parent = room ? room->obj : 0;
	float S = RoomScale;
	Doors *d = bbNew(Doors);
	if (big == 1) {
		d->obj = bbCopyEntity(BigDoorOBJ[0], 0); bbScaleEntity(d->obj, 55 * S, 55 * S, 55 * S, 0);
		d->obj2 = bbCopyEntity(BigDoorOBJ[1], 0); bbScaleEntity(d->obj2, 55 * S, 55 * S, 55 * S, 0);
		d->frameobj = bbCopyEntity(DoorColl, 0);
		bbScaleEntity(d->frameobj, S, S, S, 0);
		bbEntityType(d->frameobj, HIT_MAP, 0);
		bbEntityAlpha(d->frameobj, 0);
	} else if (big == 2) {
		d->obj = bbCopyEntity(HeavyDoorObj[0], 0); bbScaleEntity(d->obj, S, S, S, 0);
		d->obj2 = bbCopyEntity(HeavyDoorObj[1], 0); bbScaleEntity(d->obj2, S, S, S, 0);
		d->frameobj = bbCopyEntity(DoorFrameOBJ, 0);
	} else if (big == 3) {
		each(Doors, d2) if (d2 != d && d2->dir == 3) {
			d->obj = bbCopyEntity(d2->obj, 0); d->obj2 = bbCopyEntity(d2->obj2, 0);
			bbScaleEntity(d->obj, S, S, S, 0); bbScaleEntity(d->obj2, S, S, S, 0);
			break;
		}
		if (!d->obj) {
			d->obj = LoadMesh_Strict("GFX\\map\\elevatordoor.b3d", 0);
			d->obj2 = bbCopyEntity(d->obj, 0);
			bbScaleEntity(d->obj, S, S, S, 0); bbScaleEntity(d->obj2, S, S, S, 0);
		}
		d->frameobj = bbCopyEntity(DoorFrameOBJ, 0);
	} else {
		d->obj = bbCopyEntity(DoorOBJ, 0);
		float sx = 204 * S / bbMeshWidth(d->obj), sy = 312 * S / bbMeshHeight(d->obj), sz = 16 * S / bbMeshDepth(d->obj);
		bbScaleEntity(d->obj, sx, sy, sz, 0);
		d->frameobj = bbCopyEntity(DoorFrameOBJ, 0);
		d->obj2 = bbCopyEntity(DoorOBJ, 0);
		bbScaleEntity(d->obj2, sx, sy, sz, 0);
	}
	bbPositionEntity(d->frameobj, x, y, z, 0);
	bbScaleEntity(d->frameobj, 8.0f / 2048, 8.0f / 2048, 8.0f / 2048, 0);
	bbEntityPickMode(d->frameobj, 2, 1);
	bbEntityType(d->obj, HIT_MAP, 0);
	bbEntityType(d->obj2, HIT_MAP, 0);
	d->ID = DoorTempID++;
	d->KeyCard = keycard;
	d->Code = bbSdup(code);
	d->Level = lvl; d->LevelDest = 66;
	for (int i = 0; i <= 1; i++) {
		if (bbLen(code)) { d->buttons[i] = bbCopyEntity(ButtonCodeOBJ, 0); bbEntityFX(d->buttons[i], 1); }
		else d->buttons[i] = bbCopyEntity(keycard > 0 ? ButtonKeyOBJ : keycard < 0 ? ButtonScannerOBJ : ButtonOBJ, 0);
		bbScaleEntity(d->buttons[i], .03f, .03f, .03f, 0);
	}
	if (big == 1) {
		bbPositionEntity(d->buttons[0], x - 432 * S, y + .7f, z + 192 * S, 0);
		bbPositionEntity(d->buttons[1], x + 432 * S, y + .7f, z - 192 * S, 0);
		bbRotateEntity(d->buttons[0], 0, 90, 0, 0);
		bbRotateEntity(d->buttons[1], 0, 270, 0, 0);
	} else {
		bbPositionEntity(d->buttons[0], x + .6f, y + .7f, z - .1f, 0);
		bbPositionEntity(d->buttons[1], x - .6f, y + .7f, z + .1f, 0);
		bbRotateEntity(d->buttons[1], 0, 180, 0, 0);
	}
	for (int i = 0; i <= 1; i++) { bbEntityParent(d->buttons[i], d->frameobj, 1); bbEntityPickMode(d->buttons[i], 2, 1); }
	bbPositionEntity(d->obj, x, y, z, 0);
	bbRotateEntity(d->obj, 0, angle, 0, 0);
	bbRotateEntity(d->frameobj, 0, angle, 0, 0);
	if (d->obj2) {
		bbPositionEntity(d->obj2, x, y, z, 0);
		bbRotateEntity(d->obj2, 0, big == 1 ? angle : angle + 180, 0, 0);
		bbEntityParent(d->obj2, parent, 1);
	}
	bbEntityParent(d->frameobj, parent, 1);
	bbEntityParent(d->obj, parent, 1);
	d->angle = bbInt(angle);
	d->open = dopen;
	bbEntityPickMode(d->obj, 2, 1);
	if (d->obj2) bbEntityPickMode(d->obj2, 2, 1);
	bbEntityPickMode(d->frameobj, 2, 1);
	if (d->open && !big && bbRand(1, 8) == 1) d->AutoClose = 1;
	d->dir = big;
	d->room = room;
	d->MTFClose = 1;
	if (useCollisionMesh) {
		d->DoorHitOBJ = hitmesh(d);
		bbEntityAlpha(d->DoorHitOBJ, 0);
		bbEntityFX(d->DoorHitOBJ, 1);
		bbEntityType(d->DoorHitOBJ, HIT_MAP, 0);
		bbEntityColor(d->DoorHitOBJ, 255, 0, 0);
		bbHideEntity(d->DoorHitOBJ);
	}
	return d;
}

int CreateButton(float x, float y, float z, float pitch, float yaw, float roll) {
	int obj = bbCopyEntity(ButtonOBJ, 0);
	bbScaleEntity(obj, .03f, .03f, .03f, 0);
	bbPositionEntity(obj, x, y, z, 0);
	bbRotateEntity(obj, pitch, yaw, roll, 0);
	bbEntityPickMode(obj, 2, 1);
	return obj;
}
