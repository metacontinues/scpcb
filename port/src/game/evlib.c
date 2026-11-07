// evlib.c: what the events lean on: levers, buttons, elevators (MapSystem.bb), triggers, PA streams,
// angle helpers, LoadEventSound (Main.bb), checkpoint monitors
#include "g.h"

int LoadEventSound(Events *e, const char *file, int num) {
	int *s = num == 0 ? &e->Sound : num == 1 ? &e->Sound2 : NULL;
	if (!s) return 0;
	if (*s) { FreeSound_Strict(*s); *s = 0; }
	return *s = LoadSound_Strict(file);
}

float point_direction(float x1, float z1, float x2, float z2) { return atan2f(z1 - z2, x1 - x2) * RTOD; }
float point_distance(float x1, float z1, float x2, float z2) { float dx = x1 - x2, dy = z1 - z2; return sqrtf(dx * dx + dy * dy); }
float angleDist(float a0, float a1) { float b = a0 - a1; return b < -180 ? b + 360 : b > 180 ? b - 360 : b; }

// name of the trigger box the player is in, "" if none
str CheckTriggers(void) {
	Rooms *r = PlayerRoom;
	for (int i = 0; i < r->TriggerboxAmount; i++) {
		int t = r->Triggerbox[i];
		entity_alpha(t, 1);
		float sx = entity_scale_x(t, 1), sy = Max(entity_scale_y(t, 1), .001f), sz = entity_scale_z(t, 1);
		GetMeshExtents(t);
		entity_color(t, 255, 255, 255); entity_alpha(t, 0); // TODO DebugHUD: yellow, .2
		float x = entity_x(Collider, 0), y = entity_y(Collider, 0), z = entity_z(Collider, 0);
		if (x > sx * Mesh_MinX + r->x && x < sx * Mesh_MaxX + r->x && y > sy * Mesh_MinY + r->y && y < sy * Mesh_MaxY + r->y
			&& z > sz * Mesh_MinZ + r->z && z < sz * Mesh_MaxZ + r->z) return r->TriggerboxName[i];
	}
	return "";
}

void PlayAnnouncement(const char *file) {
	if (IntercomStreamCHN) { StopStream_Strict(IntercomStreamCHN); IntercomStreamCHN = 0; }
	IntercomStreamCHN = StreamSound_Strict(file, SFXVolume, 0);
}

void UpdateStreamSounds(void) {
	if (FPSfactor > 0) {
		if (IntercomStreamCHN) SetStreamVolume_Strict(IntercomStreamCHN, SFXVolume);
		each(Events, e) {
			if (e->SoundCHN && e->SoundCHN_isStream) SetStreamVolume_Strict(e->SoundCHN, SFXVolume);
			if (e->SoundCHN2 && e->SoundCHN2_isStream) SetStreamVolume_Strict(e->SoundCHN2, SFXVolume);
		}
	}
	str rn = PlayerRoom->RoomTemplate->Name;
	if (PlayerInReachableRoom(0) || s_eq(rn, "exit1") || s_eq(rn, "gatea")) return;
	if (IntercomStreamCHN) { StopStream_Strict(IntercomStreamCHN); IntercomStreamCHN = 0; }
	if (s_eq(rn, "dimension1499")) return;
	each(Events, e) {
		if (e->SoundCHN && e->SoundCHN_isStream) { StopStream_Strict(e->SoundCHN); e->SoundCHN = 0; e->SoundCHN_isStream = 0; }
		if (e->SoundCHN2 && e->SoundCHN2_isStream) { StopStream_Strict(e->SoundCHN2); e->SoundCHN2 = 0; e->SoundCHN2_isStream = 0; }
	}
}

// grab + drag a lever w/ the mouse. returns 1 when it's up
int UpdateLever(int obj, int locked) {
	float dist = entity_distance(Camera, obj);
	if (dist < 8) {
		if (dist < .8f && !locked && entity_in_view(obj, Camera)) {
			entity_pick(Camera, .65f);
			if (picked_entity() == obj) { DrawHandIcon = 1; if (MouseHit1) GrabbedEntity = obj; }
			float prev = entity_pitch(obj, 0);
			if ((MouseDown1 || MouseHit1) && GrabbedEntity && GrabbedEntity == obj) {
				DrawHandIcon = 1;
				rotate_entity(GrabbedEntity, Max(Min(entity_pitch(obj, 0) + Max(Min(mouse_y_speed_1 * 8, 30), -30), 80), -80), entity_yaw(obj, 0), 0, 0);
				DrawArrowIcon[0] = DrawArrowIcon[2] = 1;
			}
			if (entity_pitch(obj, 1) > 75) { if (prev <= 75) PlaySound2(LeverSFX, Camera, obj, 1, 1); }
			else if (entity_pitch(obj, 1) < -75) { if (prev >= -75) PlaySound2(LeverSFX, Camera, obj, 1, 1); }
		}
		if (!MouseDown1 && !MouseHit1) {
			rotate_entity(obj, CurveValue(entity_pitch(obj, 1) > 0 ? 80 : -80, entity_pitch(obj, 0), 10), entity_yaw(obj, 0), 0, 0);
			GrabbedEntity = 0;
		}
	}
	return entity_pitch(obj, 1) > 0;
}

void UpdateButton(int obj) {
	float dist = entity_distance(Collider, obj);
	if (dist >= .8f) return;
	int t = create_pivot(0);
	position_entity(t, entity_x(Camera, 0), entity_y(Camera, 0), entity_z(Camera, 0), 0);
	point_entity(t, obj, 0);
	if (entity_pick(t, .65f) == obj && (!ClosestButton || dist < entity_distance(Collider, ClosestButton))) ClosestButton = obj;
	free_entity(t);
}

// ---- elevators

static int inbox(int e, int room) { // within an elevator car's 280-unit box
	float r = 280 * RoomScale + .015f * FPSfactor;
	return fabsf(entity_x(e, 0) - entity_x(room, 1)) < r && fabsf(entity_z(e, 0) - entity_z(room, 1)) < r && fabsf(entity_y(e, 0) - entity_y(room, 1)) < r;
}

// carry e from the `from` car to the `to` car, keeping its spot and facing relative to the car
static void carry(int e, int from, int to, int ignorerot, float radius) {
	float x, z, lim = 280 * RoomScale - .22f;
	if (!ignorerot) {
		float dist = Distance(entity_x(e, 1), entity_z(e, 1), entity_x(from, 1), entity_z(from, 1));
		float dir = point_direction(entity_x(e, 1), entity_z(e, 1), entity_x(from, 1), entity_z(from, 1));
		dir += entity_yaw(to, 1) - entity_yaw(from, 1);
		dir = WrapAngle(dir);
		x = Max(Min(cosf(dir * DTOR) * dist, lim), -lim);
		z = Max(Min(sinf(dir * DTOR) * dist, lim), -lim);
		rotate_entity(e, entity_pitch(e, 1), entity_yaw(to, 1) + angleDist(entity_yaw(e, 1), entity_yaw(from, 1)), entity_roll(e, 1), 1);
	} else {
		x = Max(Min(entity_x(e, 0) - entity_x(from, 1), lim), -lim);
		z = Max(Min(entity_z(e, 0) - entity_z(from, 1), lim), -lim);
	}
	TeleportEntity(e, entity_x(to, 1) + x, .1f * FPSfactor + entity_y(to, 1) + (entity_y(e, 0) - entity_y(from, 1)), entity_z(to, 1) + z, radius, 1, 2, 0);
}

// one leg of the ride: State counts down (-) toward room2 or up (+) toward room1
static float ride(float State, Doors *dfrom, Doors *dto, int from, int to, Events *ev, int ignorerot, int down) {
	int inside = 0;
	State += down ? -FPSfactor : FPSfactor;
	if (inbox(Collider, from)) {
		inside = 1;
		if (!ev->SoundCHN || !channel_playing(ev->SoundCHN)) ev->SoundCHN = PlaySound_Strict(ElevatorMoveSFX);
		CameraShake = sinf(fabsf(State) / 3 * DTOR) * .3f;
	}
	if (down ? State >= -500 : State <= 500) return State;
	dfrom->locked = 1; dto->locked = 0;
	State = 0;
	if (inside) {
		carry(Collider, from, to, ignorerot, .3f);
		UpdateDoorsTimer = 0; DropSpeed = 0;
		UpdateDoors(); UpdateRooms();
		dto->SoundCHN = PlaySound_Strict(OpenDoorSFX[3][irand(0, 2)]);
	}
	each(NPCs, n) if (inbox(n->Collider, from)) {
		carry(n->Collider, from, to, ignorerot, n->CollRadius);
		if (n == Curr173) Curr173->IdleTimer = 10;
	}
	each(Items, it) if (inbox(it->collider, from)) carry(it->collider, from, to, ignorerot, .01f);
	UseDoor(dto, 0, !inside);
	dfrom->open = 0;
	PlaySound2(ElevatorBeepSFX, Camera, from, 4, 1);
	return State;
}

float UpdateElevators(float State, Doors *door1, Doors *door2, int room1, int room2, Events *ev, int ignorerot) {
	door1->IsElevatorDoor = door2->IsElevatorDoor = 1;
	if (door1->open && !door2->open && door1->openstate == 180) {
		State = -1;
		door1->locked = 0;
		if ((ClosestButton == door2->buttons[0] || ClosestButton == door2->buttons[1]) && MouseHit1) UseDoor(door1, 0, 1);
	} else if (door2->open && !door1->open && door2->openstate == 180) {
		State = 1;
		door2->locked = 0;
		if ((ClosestButton == door1->buttons[0] || ClosestButton == door1->buttons[1]) && MouseHit1) UseDoor(door2, 0, 1);
	} else if (fabsf(door1->openstate - door2->openstate) < .2f) door1->IsElevatorDoor = door2->IsElevatorDoor = 2;
	door1->locked = door2->locked = 1;
	if (door1->open) { door1->IsElevatorDoor = 3; if (inbox(Collider, room1)) { door1->locked = 0; door1->IsElevatorDoor = 1; } }
	if (door2->open) { door2->IsElevatorDoor = 3; if (inbox(Collider, room2)) { door2->locked = 0; door2->IsElevatorDoor = 1; } }
	if (!door1->open && !door2->open) {
		door1->locked = door2->locked = 1;
		if (door1->openstate == 0 && door2->openstate == 0) {
			if (State < 0) State = ride(State, door1, door2, room1, room2, ev, ignorerot, 1);
			else State = ride(State, door2, door1, room2, room1, ev, ignorerot, 0);
		}
	}
	return State;
}

// ---- checkpoint monitors (MapSystem.bb)

static void paintmon(int ent, int tex) {
	for (int i = 1; i < count_surfaces(ent); i++) {
		surf *sf = get_surface(ent, i);
		hbrush b = get_surface_brush(sf);
		if (!b) continue;
		htex t1 = get_brush_texture(b, 0);
		if (t1) {
			str name = StripPath(texture_name(t1));
			if (!s_eq(s_lower(name), "monitortexture.jpg")) { brush_texture(b, tex, 0, 0); paint_surface(sf, b); }
			if (s_len(name)) free_texture(t1);
		}
		free_brush(b);
	}
}

void UpdateCheckpointMonitors(int numb) {
	if (numb == 0) { UpdateCheckpoint1 = 1; paintmon(Monitor2, MonitorTimer < 50 ? MonitorTexture2 : MonitorTexture4); }
	else { UpdateCheckpoint2 = 1; paintmon(Monitor3, MonitorTimer2 < 50 ? MonitorTexture2 : MonitorTexture3); }
}

void TurnCheckpointMonitorsOff(int numb) {
	if (numb == 0) { UpdateCheckpoint1 = 0; MonitorTimer = 0; paintmon(Monitor2, MonitorTextureOff); }
	else { UpdateCheckpoint2 = 0; MonitorTimer2 = 0; paintmon(Monitor3, MonitorTextureOff); }
}
