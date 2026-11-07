// doors.c: Main.bb's CreateDoor/CreateButton (UpdateDoors & co come w/ the in-game loop)
#include "g.h"

static void doorfree(Doors *d) { free(d->Code); }
TLIST_DTOR(Doors, doorfree);

static int hitmesh(Doors *d) { // shared doorhit.b3d, loaded once
	each(Doors, d2) if (d2 != d && d2->DoorHitOBJ) return copy_entity(d2->DoorHitOBJ, d->frameobj);
	return LoadMesh_Strict("GFX\\doorhit.b3d", d->frameobj);
}

Doors *CreateDoor(int lvl, float x, float y, float z, float angle, Rooms *room, int dopen, int big, int keycard, const char *code, int useCollisionMesh) {
	int parent = room ? room->obj : 0;
	float S = RoomScale;
	Doors *d = lnew(Doors);
	if (big == 1) {
		d->obj = copy_entity(BigDoorOBJ[0], 0); scale_entity(d->obj, 55 * S, 55 * S, 55 * S, 0);
		d->obj2 = copy_entity(BigDoorOBJ[1], 0); scale_entity(d->obj2, 55 * S, 55 * S, 55 * S, 0);
		d->frameobj = copy_entity(DoorColl, 0);
		scale_entity(d->frameobj, S, S, S, 0);
		entity_type(d->frameobj, HIT_MAP, 0);
		entity_alpha(d->frameobj, 0);
	} else if (big == 2) {
		d->obj = copy_entity(HeavyDoorObj[0], 0); scale_entity(d->obj, S, S, S, 0);
		d->obj2 = copy_entity(HeavyDoorObj[1], 0); scale_entity(d->obj2, S, S, S, 0);
		d->frameobj = copy_entity(DoorFrameOBJ, 0);
	} else if (big == 3) {
		each(Doors, d2) if (d2 != d && d2->dir == 3) {
			d->obj = copy_entity(d2->obj, 0); d->obj2 = copy_entity(d2->obj2, 0);
			scale_entity(d->obj, S, S, S, 0); scale_entity(d->obj2, S, S, S, 0);
			break;
		}
		if (!d->obj) {
			d->obj = LoadMesh_Strict("GFX\\map\\elevatordoor.b3d", 0);
			d->obj2 = copy_entity(d->obj, 0);
			scale_entity(d->obj, S, S, S, 0); scale_entity(d->obj2, S, S, S, 0);
		}
		d->frameobj = copy_entity(DoorFrameOBJ, 0);
	} else {
		d->obj = copy_entity(DoorOBJ, 0);
		float sx = 204 * S / mesh_width(d->obj), sy = 312 * S / mesh_height(d->obj), sz = 16 * S / mesh_depth(d->obj);
		scale_entity(d->obj, sx, sy, sz, 0);
		d->frameobj = copy_entity(DoorFrameOBJ, 0);
		d->obj2 = copy_entity(DoorOBJ, 0);
		scale_entity(d->obj2, sx, sy, sz, 0);
	}
	position_entity(d->frameobj, x, y, z, 0);
	scale_entity(d->frameobj, 8.0f / 2048, 8.0f / 2048, 8.0f / 2048, 0);
	entity_pick_mode(d->frameobj, 2, 1);
	entity_type(d->obj, HIT_MAP, 0);
	entity_type(d->obj2, HIT_MAP, 0);
	d->ID = DoorTempID++;
	d->KeyCard = keycard;
	sset(&d->Code, s_dup(code));
	d->Level = lvl; d->LevelDest = 66;
	for (int i = 0; i <= 1; i++) {
		if (s_len(code)) { d->buttons[i] = copy_entity(ButtonCodeOBJ, 0); entity_fx(d->buttons[i], 1); }
		else d->buttons[i] = copy_entity(keycard > 0 ? ButtonKeyOBJ : keycard < 0 ? ButtonScannerOBJ : ButtonOBJ, 0);
		scale_entity(d->buttons[i], .03f, .03f, .03f, 0);
	}
	if (big == 1) {
		position_entity(d->buttons[0], x - 432 * S, y + .7f, z + 192 * S, 0);
		position_entity(d->buttons[1], x + 432 * S, y + .7f, z - 192 * S, 0);
		rotate_entity(d->buttons[0], 0, 90, 0, 0);
		rotate_entity(d->buttons[1], 0, 270, 0, 0);
	} else {
		position_entity(d->buttons[0], x + .6f, y + .7f, z - .1f, 0);
		position_entity(d->buttons[1], x - .6f, y + .7f, z + .1f, 0);
		rotate_entity(d->buttons[1], 0, 180, 0, 0);
	}
	for (int i = 0; i <= 1; i++) { entity_parent(d->buttons[i], d->frameobj, 1); entity_pick_mode(d->buttons[i], 2, 1); }
	position_entity(d->obj, x, y, z, 0);
	rotate_entity(d->obj, 0, angle, 0, 0);
	rotate_entity(d->frameobj, 0, angle, 0, 0);
	if (d->obj2) {
		position_entity(d->obj2, x, y, z, 0);
		rotate_entity(d->obj2, 0, big == 1 ? angle : angle + 180, 0, 0);
		entity_parent(d->obj2, parent, 1);
	}
	entity_parent(d->frameobj, parent, 1);
	entity_parent(d->obj, parent, 1);
	d->angle = iround(angle);
	d->open = dopen;
	entity_pick_mode(d->obj, 2, 1);
	if (d->obj2) entity_pick_mode(d->obj2, 2, 1);
	entity_pick_mode(d->frameobj, 2, 1);
	if (d->open && !big && irand(1, 8) == 1) d->AutoClose = 1;
	d->dir = big;
	d->room = room;
	d->MTFClose = 1;
	if (useCollisionMesh) {
		d->DoorHitOBJ = hitmesh(d);
		entity_alpha(d->DoorHitOBJ, 0);
		entity_fx(d->DoorHitOBJ, 1);
		entity_type(d->DoorHitOBJ, HIT_MAP, 0);
		entity_color(d->DoorHitOBJ, 255, 0, 0);
		hide_entity(d->DoorHitOBJ);
	}
	return d;
}

int CreateButton(float x, float y, float z, float pitch, float yaw, float roll) {
	int obj = copy_entity(ButtonOBJ, 0);
	scale_entity(obj, .03f, .03f, .03f, 0);
	position_entity(obj, x, y, z, 0);
	rotate_entity(obj, pitch, yaw, roll, 0);
	entity_pick_mode(obj, 2, 1);
	return obj;
}

// ---- per frame (Main.bb's UpdateDoors / UseDoor / RemoveDoor)
int ValidRoom2slCamRoom(Rooms *r) {
	if (!r) return 0;
	static const char *n[] = {"room2closets", "room1archive", "room3z3", "room1lifts", "checkpoint1", "room2nuke", "008", "room1162", "room966", "room2ccont"};
	for (int i = 0; i < 10; i++) if (s_eq(r->RoomTemplate->Name, n[i])) return 1;
	return 0;
}

static void showdoor(Doors *d, int show) {
	int e[] = {d->obj, d->frameobj, d->obj2, d->buttons[0], d->buttons[1]};
	for (int i = 0; i < 5; i++) if (e[i]) { if (show) show_entity(e[i]); else hide_entity(e[i]); }
}

// how far each door kind slides per frame while opening/closing: obj divisor, obj2 divisor, speed
static void slide(Doors *d, int dir) { // dir 1 opening, -1 closing
	float s = sinf(d->openstate * DTOR), f = FPSfactor, fo = d->fastopen;
	switch (d->dir) {
	case 0:
		d->openstate = dir > 0 ? Min(180, d->openstate + f * 2 * (fo + 1)) : Max(0, d->openstate - f * 2 * (fo + 1));
		s = sinf(d->openstate * DTOR);
		move_entity(d->obj, dir > 0 ? s * (fo * 2 + 1) * f / 80 : s * -f * (fo + 1) / 80, 0, 0);
		if (d->obj2) move_entity(d->obj2, s * (fo + 1) * f * dir / 80, 0, 0);
		break;
	case 1:
		d->openstate = dir > 0 ? Min(180, d->openstate + f * .8f) : Max(0, d->openstate - f * .8f);
		s = sinf(d->openstate * DTOR);
		move_entity(d->obj, s * f * dir / 180, 0, 0);
		if (d->obj2) move_entity(d->obj2, -s * f * dir / 180, 0, 0);
		break;
	case 2:
		d->openstate = dir > 0 ? Min(180, d->openstate + f * 2 * (fo + 1)) : Max(0, d->openstate - f * 2 * (fo + 1));
		s = sinf(d->openstate * DTOR);
		move_entity(d->obj, s * (fo + 1) * f * dir / 85, 0, 0);
		if (d->obj2) move_entity(d->obj2, dir > 0 ? s * (fo * 2 + 1) * f / 120 : s * (fo + 1) * -f / 120, 0, 0);
		break;
	case 3:
		d->openstate = dir > 0 ? Min(180, d->openstate + f * 2 * (fo + 1)) : Max(0, d->openstate - f * 2 * (fo + 1));
		s = sinf(d->openstate * DTOR);
		move_entity(d->obj, dir > 0 ? s * (fo * 2 + 1) * f / 162 : s * -f * (fo + 1) / 162, 0, 0);
		if (d->obj2) move_entity(d->obj2, dir > 0 ? s * (fo * 2 + 1) * f / 162 : s * (fo + 1) * -f / 162, 0, 0);
		break;
	case 4: // 914 only
		d->openstate = dir > 0 ? Min(180, d->openstate + f * 1.4f) : Max(0, d->openstate - f * 1.4f);
		s = sinf(d->openstate * DTOR);
		move_entity(d->obj, s * f * dir / 114, 0, 0);
		break;
	}
}

// the dust when big containment doors (dir 1) shut
static void bigdust(Doors *d) {
	for (int i = 0, n = irand(75, 99); i <= n; i++) {
		int pvt = create_pivot(0);
		// all three args weigh the same, so the y..z tail outweighs x and goes first: y, z, then x
		float ry = frnd(0, 1.2f), rz = frnd(-.2f, .2f), rx = frnd(-.2f, .2f);
		position_entity(pvt, entity_x(d->frameobj, 1) + rx, entity_y(d->frameobj, 1) + ry, entity_z(d->frameobj, 1) + rz, 0);
		rotate_entity(pvt, 0, frnd(0, 360), 0, 0);
		Particles *p = CreateParticle(entity_x(pvt, 0), entity_y(pvt, 0), entity_z(pvt, 0), 2, .002f, 0, 300);
		p->speed = .005f;
		float py = frnd(0, 360), pp = frnd(-20, 20); // Rnd(360) first, see MouseLook
		rotate_entity(p->pvt, pp, py, 0, 0);
		p->SizeChange = -.00001f; p->size = .01f;
		scale_sprite(p->obj, p->size, p->size);
		p->Achange = -.01f;
		entity_order(p->obj, -1);
		free_entity(pvt);
	}
}

static float sgnf(float v) { return v > 0 ? 1 : v < 0 ? -1 : 0; }

void UpdateDoors(void) {
	if (UpdateDoorsTimer <= 0) {
		each(Doors, d) {
			d->dist = fabsf(entity_x(Collider, 0) - entity_x(d->obj, 1)) + fabsf(entity_z(Collider, 0) - entity_z(d->obj, 1));
			showdoor(d, d->dist <= HideDistance * 2);
			if (s_eq(PlayerRoom->RoomTemplate->Name, "room2sl") && ValidRoom2slCamRoom(d->room)) showdoor(d, 1);
		}
		UpdateDoorsTimer = 30;
	} else UpdateDoorsTimer = Max(UpdateDoorsTimer - FPSfactor, 0);
	ClosestButton = 0; ClosestDoor = NULL;
	each(Doors, d) {
		if (d->dist < HideDistance * 2 || d->IsElevatorDoor > 0) {
			if ((d->openstate >= 180 || d->openstate <= 0) && GrabbedEntity == 0) for (int i = 0; i <= 1; i++) {
				int b = d->buttons[i];
				if (!b || fabsf(entity_x(Collider, 0) - entity_x(b, 1)) >= 1 || fabsf(entity_z(Collider, 0) - entity_z(b, 1)) >= 1) continue;
				float dist = Distance(entity_x(Collider, 1), entity_z(Collider, 1), entity_x(b, 1), entity_z(b, 1));
				if (dist >= .7f) continue;
				int t = create_pivot(0);
				position_entity(t, entity_x(Camera, 0), entity_y(Camera, 0), entity_z(Camera, 0), 0);
				point_entity(t, b, 0);
				if (entity_pick(t, .6f) == b) {
					if (!ClosestButton || dist < entity_distance(Collider, ClosestButton)) { ClosestButton = b; ClosestDoor = d; }
				}
				free_entity(t);
			}
			if (d->open) {
				if (d->openstate < 180) slide(d, 1);
				else {
					d->fastopen = 0;
					reset_entity(d->obj);
					if (d->obj2) reset_entity(d->obj2);
					if (d->timerstate > 0) {
						d->timerstate = Max(0, d->timerstate - FPSfactor);
						if (d->timerstate + FPSfactor > 110 && d->timerstate <= 110) d->SoundCHN = PlaySound2(CautionSFX, Camera, d->obj, 10, 1);
						int snd = d->dir == 1 ? irand(0, 1) : irand(0, 2);
						if (d->timerstate == 0) { d->open = !d->open; d->SoundCHN = PlaySound2(CloseDoorSFX[d->dir][snd], Camera, d->obj, 10, 1); }
					}
					if (d->AutoClose && RemoteDoorOn == 1 && entity_distance(Camera, d->obj) < 2.1f) {
						if (!Wearing714) PlaySound_Strict(HorrorSFX[7]);
						d->open = 0;
						d->SoundCHN = PlaySound2(CloseDoorSFX[(int)Min(d->dir, 1)][irand(0, 2)], Camera, d->obj, 10, 1);
						d->AutoClose = 0;
					}
				}
			} else if (d->openstate > 0) {
				slide(d, -1);
				if (d->dir == 1 && d->openstate < 15 && d->openstate + FPSfactor >= 15 && ParticleAmount == 2) bigdust(d);
				// keep the player out of a closing door
				float fx = entity_x(d->frameobj, 1), fz = entity_z(d->frameobj, 1), cx = entity_x(Collider, 0), cz = entity_z(Collider, 0);
				if (d->angle == 0 || d->angle == 180) {
					if (fabsf(fz - cz) < .15f && fabsf(fx - cx) < .7f * (d->dir * 2 + 1))
						position_entity(Collider, cx, entity_y(Collider, 0), CurveValue(fz + .15f * sgnf(cz - fz), cz, 5), 0);
				} else if (fabsf(fx - cx) < .15f && fabsf(fz - cz) < .7f * (d->dir * 2 + 1))
					position_entity(Collider, CurveValue(fx + .15f * sgnf(cx - fx), cx, 5), entity_y(Collider, 0), cz, 0);
				if (d->DoorHitOBJ) show_entity(d->DoorHitOBJ);
			} else {
				d->fastopen = 0;
				float fx = entity_x(d->frameobj, 1), fy = entity_y(d->frameobj, 1), fz = entity_z(d->frameobj, 1);
				position_entity(d->obj, fx, fy, fz, 0);
				if (d->obj2) position_entity(d->obj2, fx, fy, fz, 0);
				if (d->obj2 && d->dir == 0) { move_entity(d->obj, 0, 0, 8 * RoomScale); move_entity(d->obj2, 0, 0, 8 * RoomScale); }
				if (d->DoorHitOBJ) hide_entity(d->DoorHitOBJ);
			}
		}
		UpdateSoundOrigin(d->SoundCHN, Camera, d->frameobj, 10, 1);
		if (d->DoorHitOBJ) entity_alpha(d->DoorHitOBJ, 0); // TODO DebugHUD: .5
	}
}

static int keycardlevel(const char *t) {
	static const char *k[] = {"key1", "key2", "key3", "key4", "key5", "key6"};
	for (int i = 0; i < 6; i++) if (s_eq(t, k[i])) return i + 1;
	return -1;
}
static void lockmsg(void) {
	if ((!s_has(Msg, "The keycard") && !s_has(Msg, "A keycard with")) || MsgTimer < 70 * 3) { sset(&Msg, "A keycard is required to operate this door."); MsgTimer = 70 * 7; }
}

void UseDoor(Doors *d, int showmsg, int playsfx) {
	if (d->KeyCard > 0) {
		if (!SelectedItem) { if (showmsg) lockmsg(); return; }
		int temp = keycardlevel(SelectedItem->itemtemplate->tempname);
		if (temp == -1) { if (showmsg) lockmsg(); return; }
		SelectedItem = NULL;
		if (temp >= d->KeyCard) {
			if (showmsg) {
				if (d->locked) { PlaySound_Strict(KeyCardSFX2); sset(&Msg, "The keycard was inserted into the slot but nothing happened."); MsgTimer = 70 * 7; return; }
				PlaySound_Strict(KeyCardSFX1); sset(&Msg, "The keycard was inserted into the slot."); MsgTimer = 70 * 7;
			}
		} else {
			if (showmsg) {
				PlaySound_Strict(KeyCardSFX2);
				sset(&Msg, d->locked ? "The keycard was inserted into the slot but nothing happened."
					: cat("A keycard with security clearance ", s_str(d->KeyCard), " or higher is required to operate this door."));
				MsgTimer = 70 * 7;
			}
			return;
		}
	} else if (d->KeyCard < 0) {
		int temp = 0;
		if (SelectedItem) {
			str t = SelectedItem->itemtemplate->tempname;
			temp = (s_eq(t, "hand") & (d->KeyCard == -1)) | (s_eq(t, "hand2") & (d->KeyCard == -2));
		}
		SelectedItem = NULL;
		if (!temp) {
			if (showmsg) {
				PlaySound_Strict(ScannerSFX2);
				sset(&Msg, "You placed your palm onto the scanner. The scanner reads: \"DNA does not match known sample. Access denied.\"");
				MsgTimer = 70 * 10;
			}
			return;
		}
		PlaySound_Strict(ScannerSFX1);
		if (!s_has(Msg, "You placed your") || MsgTimer < 70 * 3) sset(&Msg, "You place the palm of the hand onto the scanner. The scanner reads: \"DNA verified. Access granted.\"");
		MsgTimer = 70 * 10;
	} else if (d->locked) {
		if (!showmsg) return;
		if (!(d->IsElevatorDoor > 0)) {
			PlaySound_Strict(ButtonSFX2);
			if (!s_eq(PlayerRoom->RoomTemplate->Name, "room2elevator")) sset(&Msg, d->open ? "You pushed the button but nothing happened." : "The door appears to be locked.");
			else sset(&Msg, "The elevator appears to be broken.");
			MsgTimer = 70 * 5;
		} else if (d->IsElevatorDoor == 1) { sset(&Msg, "You called the elevator."); MsgTimer = 70 * 5; }
		else if (d->IsElevatorDoor == 3) { sset(&Msg, "The elevator is already on this floor."); MsgTimer = 70 * 5; }
		else if (!s_eq(Msg, "You called the elevator.")) {
			if (s_eq(Msg, "You already called the elevator.") || MsgTimer < 70 * 3) {
				switch (irand(1, 10)) {
				case 1: sset(&Msg, "Stop spamming the button."); break;
				case 2: sset(&Msg, "Pressing it harder does not make the elevator come faster."); break;
				case 3: sset(&Msg, "If you continue pressing this button I will generate a Memory Access Violation."); break;
				default: sset(&Msg, "You already called the elevator.");
				}
				MsgTimer = 70 * 7;
			}
		} else { sset(&Msg, "You already called the elevator."); MsgTimer = 70 * 7; }
		return;
	}
	d->open = !d->open;
	if (d->LinkedDoor) d->LinkedDoor->open = !d->LinkedDoor->open;
	int snd = d->dir == 1 ? irand(0, 1) : irand(0, 2);
	if (d->open) {
		if (d->LinkedDoor) d->LinkedDoor->timerstate = d->LinkedDoor->timer;
		d->timerstate = d->timer;
	}
	if (playsfx) {
		d->SoundCHN = PlaySound2(d->open ? OpenDoorSFX[d->dir][snd] : CloseDoorSFX[d->dir][snd], Camera, d->obj, 10, 1);
		UpdateSoundOrigin(d->SoundCHN, Camera, d->obj, 10, 1);
	}
}

void RemoveDoor(Doors *d) {
	if (!d->n.ls) return;
	if (ClosestDoor == d) ClosestDoor = NULL;
	if (SelectedDoor == d) SelectedDoor = NULL;
	each(Rooms, r) {
		for (int i = 0; i < 8; i++) if (r->RoomDoors[i] == d) r->RoomDoors[i] = NULL;
		for (int i = 0; i < 5; i++) if (r->AdjDoor[i] == d) r->AdjDoor[i] = NULL;
	}
	each(Doors, o) if (o->LinkedDoor == d) o->LinkedDoor = NULL;
	each(WayPoints, w) if (w->door == d) w->door = NULL;
	each(ElevatorObj, eo) if (eo->door == d) eo->door = NULL;
	for (int i = 0; i <= 1; i++) if (d->buttons[i]) entity_parent(d->buttons[i], 0, 1);
	int e[] = {d->obj, d->obj2, d->frameobj, d->buttons[0], d->buttons[1]};
	for (int i = 0; i < 5; i++) if (e[i]) free_entity(e[i]);
	ldel(d);
}
