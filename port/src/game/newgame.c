// newgame.c: Main.bb's InitNewGame
#include "g.h"

void InitNewGame(void) {
	DrawLoading(45, 0);
	HideDistance = 15;
	HeartBeatRate = 70;
	AccessCode = 0;
	for (int i = 0; i <= 3; i++) AccessCode = iround(AccessCode + irand(1, 9) * powf(10, i));
	if (!s_len(SelectedMap)) CreateMap();
	else RuntimeError("custom maps (LoadMap) aren't ported yet");
	InitWayPoints(45);
	DrawLoading(79, 0);
	Curr173 = CreateNPC(NPCtype173, 0, -30, 0);
	Curr106 = CreateNPC(NPCtypeOldMan, 0, -30, 0);
	Curr106->State = 70 * 60 * irand(12, 17);
	each(Doors, d) {
		entity_parent(d->obj, 0, 1);
		if (d->obj2) entity_parent(d->obj2, 0, 1);
		if (d->frameobj) entity_parent(d->frameobj, 0, 1);
		for (int i = 0; i <= 1; i++) if (d->buttons[i]) entity_parent(d->buttons[i], 0, 1);
		if (d->obj2 && d->dir == 0) { move_entity(d->obj, 0, 0, 8 * RoomScale); move_entity(d->obj2, 0, 0, 8 * RoomScale); }
	}
	each(Items, it) { entity_type(it->collider, HIT_ITEM, 0); entity_parent(it->collider, 0, 1); }
	DrawLoading(80, 0);
	each(SecurityCams, sc) { sc->angle = entity_yaw(sc->obj, 0) + sc->angle; entity_parent(sc->obj, 0, 1); }
	each(Rooms, r) {
		for (int i = 0; i <= MaxRoomLights; i++) if (r->Lights[i]) entity_parent(r->Lights[i], 0, 1);
		if (!r->RoomTemplate->DisableDecals) {
			if (irand(1, 4) == 1) {

				float rx = frnd(-2, 2), rz = frnd(-2, 2); int a = irand(1, 360), id = irand(2, 3);
				Decals *de = CreateDecal(id, entity_x(r->obj, 0) + rx, .003f, entity_z(r->obj, 0) + rz, 90, a, 0);
				de->Size = frnd(.1f, .4f); scale_sprite(de->obj, de->Size, de->Size);
				entity_alpha(de->obj, frnd(.85f, .95f));
			}
			if (irand(1, 4) == 1) {
				float rx = frnd(-2, 2), rz = frnd(-2, 2); int a = irand(1, 360); // x, z, then Rand(360)
				Decals *de = CreateDecal(0, entity_x(r->obj, 0) + rx, .003f, entity_z(r->obj, 0) + rz, 90, a, 0);
				de->Size = frnd(.5f, .7f); entity_alpha(de->obj, .7f); de->ID = 1; scale_sprite(de->obj, de->Size, de->Size);
				entity_alpha(de->obj, frnd(.7f, .85f));
			}
		}
		if (s_eq(r->RoomTemplate->Name, "start") && !IntroEnabled) {
			position_entity(Collider, entity_x(r->obj, 0) + 3584 * RoomScale, 704 * RoomScale, entity_z(r->obj, 0) + 1024 * RoomScale, 0);
			PlayerRoom = r;
			static const char *docs[] = {"Class D Orientation Leaflet", "Document SCP-173"};
			for (int k = 0; k < 2; k++) {
				Items *it = CreateItem(docs[k], "paper", 1, 1, 1, 0, 0, 0, 1, 0);
				it->Picked = 1; it->Dropped = -1; it->itemtemplate->found = 1;
				Inventory[k] = it;
				hide_entity(it->collider);
				entity_type(it->collider, HIT_ITEM, 0);
				entity_parent(it->collider, 0, 1);
				ItemAmount++;
			}
		} else if (s_eq(r->RoomTemplate->Name, "173") && IntroEnabled) {
			position_entity(Collider, entity_x(r->obj, 0), 1, entity_z(r->obj, 0), 0);
			PlayerRoom = r;
		}
	}
	each(RoomTemplates, rt) free_entity(rt->obj);
	ldel_each(TempWayPoints);
	turn_entity(Collider, 0, irand(160, 200), 0, 0);
	reset_entity(Collider);
	if (!s_len(SelectedMap)) InitEvents();
	each(Events, e) {
		if (s_eq(e->EventName, "room2nuke")) e->EventState = 1;
		if (s_eq(e->EventName, "room106")) e->EventState2 = 1;
		if (s_eq(e->EventName, "room2sl")) e->EventState3 = 1;
	}
	move_mouse(viewport_center_x, viewport_center_y);
	AASetFont(Font1);
	hide_pointer();
	BlinkTimer = -10; BlurTimer = 100; Stamina = 100;
	for (int i = 0; i <= 70; i++) { // let things settle before the first frame
		FPSfactor = 1;
		flush_keys();
		MovePlayer();
		UpdateDoors();
		UpdateNPCs();
		update_world(1);
		if (iround(i * .27f) != iround((i - 1) * .27f)) DrawLoading(80 + iround(i * .27f), 0);
	}
	if (getenv("SCPCB_ROOM")) each(Rooms, r) if (s_eq(r->RoomTemplate->Name, getenv("SCPCB_ROOM"))) { // dev: start in that room
		position_entity(Collider, entity_x(r->obj, 0), entity_y(r->obj, 0) + .5f, entity_z(r->obj, 0), 0);
		reset_entity(Collider);
		PlayerRoom = r;
		break;
	}
	// dev: SCPCB_173=d / SCPCB_106=d put 173 / an already risen 106 d units in front of you
	for (int k = 0; k < 2; k++) {
		const char *v = getenv(k ? "SCPCB_106" : "SCPCB_173");
		if (!v) continue;
		NPCs *n = k ? Curr106 : Curr173;
		int p = create_pivot(0);
		position_entity(p, entity_x(Collider, 0), entity_y(Collider, 0), entity_z(Collider, 0), 0);
		rotate_entity(p, 0, entity_yaw(Collider, 0), 0, 0);
		move_entity(p, 0, 0, atof(v));
		position_entity(n->Collider, entity_x(p, 0), entity_y(p, 0), entity_z(p, 0), 0);
		reset_entity(n->Collider);
		free_entity(p);
		if (k) { n->State = -10; SetNPCFrame(n, 259); }
	}
	FreeTextureCache();
	DrawLoading(100, 0);
	flush_keys(); flush_mouse();
	DropSpeed = 0;
	PrevTime = milli_secs();
}
