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
		bbEntityAlpha(t, 1);
		float sx = bbEntityScaleX(t, 1), sy = Max(bbEntityScaleY(t, 1), .001f), sz = bbEntityScaleZ(t, 1);
		GetMeshExtents(t);
		bbEntityColor(t, 255, 255, 255); bbEntityAlpha(t, 0); // TODO DebugHUD: yellow, .2
		float x = bbEntityX(Collider, 0), y = bbEntityY(Collider, 0), z = bbEntityZ(Collider, 0);
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
	if (PlayerInReachableRoom(0) || bbSeq(rn, "exit1") || bbSeq(rn, "gatea")) return;
	if (IntercomStreamCHN) { StopStream_Strict(IntercomStreamCHN); IntercomStreamCHN = 0; }
	if (bbSeq(rn, "dimension1499")) return;
	each(Events, e) {
		if (e->SoundCHN && e->SoundCHN_isStream) { StopStream_Strict(e->SoundCHN); e->SoundCHN = 0; e->SoundCHN_isStream = 0; }
		if (e->SoundCHN2 && e->SoundCHN2_isStream) { StopStream_Strict(e->SoundCHN2); e->SoundCHN2 = 0; e->SoundCHN2_isStream = 0; }
	}
}

// grab + drag a lever w/ the mouse. returns 1 when it's up
int UpdateLever(int obj, int locked) {
	float dist = bbEntityDistance(Camera, obj);
	if (dist < 8) {
		if (dist < .8f && !locked && bbEntityInView(obj, Camera)) {
			bbEntityPick(Camera, .65f);
			if (bbPickedEntity() == obj) { DrawHandIcon = 1; if (MouseHit1) GrabbedEntity = obj; }
			float prev = bbEntityPitch(obj, 0);
			if ((MouseDown1 || MouseHit1) && GrabbedEntity && GrabbedEntity == obj) {
				DrawHandIcon = 1;
				bbRotateEntity(GrabbedEntity, Max(Min(bbEntityPitch(obj, 0) + Max(Min(mouse_y_speed_1 * 8, 30), -30), 80), -80), bbEntityYaw(obj, 0), 0, 0);
				DrawArrowIcon[0] = DrawArrowIcon[2] = 1;
			}
			if (bbEntityPitch(obj, 1) > 75) { if (prev <= 75) PlaySound2(LeverSFX, Camera, obj, 1, 1); }
			else if (bbEntityPitch(obj, 1) < -75) { if (prev >= -75) PlaySound2(LeverSFX, Camera, obj, 1, 1); }
		}
		if (!MouseDown1 && !MouseHit1) {
			bbRotateEntity(obj, CurveValue(bbEntityPitch(obj, 1) > 0 ? 80 : -80, bbEntityPitch(obj, 0), 10), bbEntityYaw(obj, 0), 0, 0);
			GrabbedEntity = 0;
		}
	}
	return bbEntityPitch(obj, 1) > 0;
}

void UpdateButton(int obj) {
	float dist = bbEntityDistance(Collider, obj);
	if (dist >= .8f) return;
	int t = bbCreatePivot(0);
	bbPositionEntity(t, bbEntityX(Camera, 0), bbEntityY(Camera, 0), bbEntityZ(Camera, 0), 0);
	bbPointEntity(t, obj, 0);
	if (bbEntityPick(t, .65f) == obj && (!ClosestButton || dist < bbEntityDistance(Collider, ClosestButton))) ClosestButton = obj;
	bbFreeEntity(t);
}

// ---- elevators

static int inbox(int e, int room) { // within an elevator car's 280-unit box
	float r = 280 * RoomScale + .015f * FPSfactor;
	return fabsf(bbEntityX(e, 0) - bbEntityX(room, 1)) < r && fabsf(bbEntityZ(e, 0) - bbEntityZ(room, 1)) < r && fabsf(bbEntityY(e, 0) - bbEntityY(room, 1)) < r;
}

// carry e from the `from` car to the `to` car, keeping its spot and facing relative to the car
static void carry(int e, int from, int to, int ignorerot, float radius) {
	float x, z, lim = 280 * RoomScale - .22f;
	if (!ignorerot) {
		float dist = Distance(bbEntityX(e, 1), bbEntityZ(e, 1), bbEntityX(from, 1), bbEntityZ(from, 1));
		float dir = point_direction(bbEntityX(e, 1), bbEntityZ(e, 1), bbEntityX(from, 1), bbEntityZ(from, 1));
		dir += bbEntityYaw(to, 1) - bbEntityYaw(from, 1);
		dir = WrapAngle(dir);
		x = Max(Min(cosf(dir * DTOR) * dist, lim), -lim);
		z = Max(Min(sinf(dir * DTOR) * dist, lim), -lim);
		bbRotateEntity(e, bbEntityPitch(e, 1), bbEntityYaw(to, 1) + angleDist(bbEntityYaw(e, 1), bbEntityYaw(from, 1)), bbEntityRoll(e, 1), 1);
	} else {
		x = Max(Min(bbEntityX(e, 0) - bbEntityX(from, 1), lim), -lim);
		z = Max(Min(bbEntityZ(e, 0) - bbEntityZ(from, 1), lim), -lim);
	}
	TeleportEntity(e, bbEntityX(to, 1) + x, .1f * FPSfactor + bbEntityY(to, 1) + (bbEntityY(e, 0) - bbEntityY(from, 1)), bbEntityZ(to, 1) + z, radius, 1, 2, 0);
}

// one leg of the ride: State counts down (-) toward room2 or up (+) toward room1
static float ride(float State, Doors *dfrom, Doors *dto, int from, int to, Events *ev, int ignorerot, int down) {
	int inside = 0;
	State += down ? -FPSfactor : FPSfactor;
	if (inbox(Collider, from)) {
		inside = 1;
		if (!ev->SoundCHN || !bbChannelPlaying(ev->SoundCHN)) ev->SoundCHN = PlaySound_Strict(ElevatorMoveSFX);
		CameraShake = sinf(fabsf(State) / 3 * DTOR) * .3f;
	}
	if (down ? State >= -500 : State <= 500) return State;
	dfrom->locked = 1; dto->locked = 0;
	State = 0;
	if (inside) {
		carry(Collider, from, to, ignorerot, .3f);
		UpdateDoorsTimer = 0; DropSpeed = 0;
		UpdateDoors(); UpdateRooms();
		dto->SoundCHN = PlaySound_Strict(OpenDoorSFX[3][bbRand(0, 2)]);
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
	for (int i = 2; i <= bbCountSurfaces(ent); i++) {
		surf *sf = bbGetSurface(ent, i);
		hbrush b = bbGetSurfaceBrush(sf);
		if (!b) continue;
		htex t1 = bbGetBrushTexture(b, 0);
		if (t1) {
			str name = StripPath(bbTextureName(t1));
			if (!bbSeq(bbLower(name), "monitortexture.jpg")) { bbBrushTexture(b, tex, 0, 0); bbPaintSurface(sf, b); }
			if (bbLen(name)) bbFreeTexture(t1);
		}
		bbFreeBrush(b);
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
