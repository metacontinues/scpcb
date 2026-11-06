// newgame.c: Main.bb's InitNewGame
#include "g.h"

void InitNewGame(void) {
	DrawLoading(45, 0);
	HideDistance = 15;
	HeartBeatRate = 70;
	AccessCode = 0;
	for (int i = 0; i <= 3; i++) AccessCode = bbInt(AccessCode + bbRand(1, 9) * powf(10, i));
	if (!bbLen(SelectedMap)) CreateMap();
	else RuntimeError("custom maps (LoadMap) aren't ported yet");
	// TODO InitWayPoints, then Curr173/Curr106 = CreateNPC(...) w/ Curr106\State = 70*60*Rand(12,17).
	// they roll rng, so until NPCs are in, everything after the map rolls differently than blitz
	DrawLoading(79, 0);
	each(Doors, d) {
		bbEntityParent(d->obj, 0, 1);
		if (d->obj2) bbEntityParent(d->obj2, 0, 1);
		if (d->frameobj) bbEntityParent(d->frameobj, 0, 1);
		for (int i = 0; i <= 1; i++) if (d->buttons[i]) bbEntityParent(d->buttons[i], 0, 1);
		if (d->obj2 && d->dir == 0) { bbMoveEntity(d->obj, 0, 0, 8 * RoomScale); bbMoveEntity(d->obj2, 0, 0, 8 * RoomScale); }
	}
	each(Items, it) { bbEntityType(it->collider, HIT_ITEM, 0); bbEntityParent(it->collider, 0, 1); }
	DrawLoading(80, 0);
	each(SecurityCams, sc) { sc->angle = bbEntityYaw(sc->obj, 0) + sc->angle; bbEntityParent(sc->obj, 0, 1); }
	each(Rooms, r) {
		for (int i = 0; i <= MaxRoomLights; i++) if (r->Lights[i]) bbEntityParent(r->Lights[i], 0, 1);
		if (!r->RoomTemplate->DisableDecals) {
			if (bbRand(1, 4) == 1) {
				// blitz codegen order for these args: x's Rnd, z's Rnd, Rand(360), then Rand(2, 3)
				float rx = bbRnd(-2, 2), rz = bbRnd(-2, 2); int a = bbRand(360, 1), id = bbRand(2, 3);
				Decals *de = CreateDecal(id, bbEntityX(r->obj, 0) + rx, .003f, bbEntityZ(r->obj, 0) + rz, 90, a, 0);
				de->Size = bbRnd(.1f, .4f); bbScaleSprite(de->obj, de->Size, de->Size);
				bbEntityAlpha(de->obj, bbRnd(.85f, .95f));
			}
			if (bbRand(1, 4) == 1) {
				float rx = bbRnd(-2, 2), rz = bbRnd(-2, 2); int a = bbRand(360, 1); // x, z, then Rand(360)
				Decals *de = CreateDecal(0, bbEntityX(r->obj, 0) + rx, .003f, bbEntityZ(r->obj, 0) + rz, 90, a, 0);
				de->Size = bbRnd(.5f, .7f); bbEntityAlpha(de->obj, .7f); de->ID = 1; bbScaleSprite(de->obj, de->Size, de->Size);
				bbEntityAlpha(de->obj, bbRnd(.7f, .85f));
			}
		}
		if (bbSeq(r->RoomTemplate->Name, "start") && !IntroEnabled) {
			bbPositionEntity(Collider, bbEntityX(r->obj, 0) + 3584 * RoomScale, 704 * RoomScale, bbEntityZ(r->obj, 0) + 1024 * RoomScale, 0);
			PlayerRoom = r;
			static const char *docs[] = {"Class D Orientation Leaflet", "Document SCP-173"};
			for (int k = 0; k < 2; k++) {
				Items *it = CreateItem(docs[k], "paper", 1, 1, 1, 0, 0, 0, 1, 0);
				it->Picked = 1; it->Dropped = -1; it->itemtemplate->found = 1;
				Inventory[k] = it;
				bbHideEntity(it->collider);
				bbEntityType(it->collider, HIT_ITEM, 0);
				bbEntityParent(it->collider, 0, 1);
				ItemAmount++;
			}
		} else if (bbSeq(r->RoomTemplate->Name, "173") && IntroEnabled) {
			bbPositionEntity(Collider, bbEntityX(r->obj, 0), 1, bbEntityZ(r->obj, 0), 0);
			PlayerRoom = r;
		}
	}
	each(RoomTemplates, rt) bbFreeEntity(rt->obj);
	bbDeleteEach(TempWayPoints);
	bbTurnEntity(Collider, 0, bbRand(160, 200), 0, 0);
	bbResetEntity(Collider);
	// TODO InitEvents (+ the room2nuke/room106/room2sl event states set here)
	bbMoveMouse(viewport_center_x, viewport_center_y);
	AASetFont(Font1);
	bbHidePointer();
	BlinkTimer = -10; BlurTimer = 100; Stamina = 100;
	for (int i = 0; i <= 70; i++) { // let things settle before the first frame
		FPSfactor = 1;
		bbFlushKeys();
		MovePlayer();
		UpdateDoors();
		// TODO UpdateNPCs
		bbUpdateWorld(1);
		if (bbInt(i * .27f) != bbInt((i - 1) * .27f)) DrawLoading(80 + bbInt(i * .27f), 0);
	}
	FreeTextureCache();
	DrawLoading(100, 0);
	bbFlushKeys(); bbFlushMouse();
	DropSpeed = 0;
	PrevTime = bbMilliSecs();
}
