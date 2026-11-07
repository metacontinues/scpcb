// npcs.c: NPCs.bb's CreateNPC/RemoveNPC, the UpdateNPCs frame (per-type cases live in npc*.c), helpers
// + Main.bb's AnimateNPC/SetNPCFrame/Animate2/TeleportEntity
#include "g.h"

TLIST(NPCs);

#define INI "DATA\\NPCs.ini"
#define RN (PlayerRoom->RoomTemplate->Name)
static float inif(const char *sec, const char *k) { return GetINIFloat(INI, sec, k, 0); }
static int npivot(NPCs *n, float r) { n->Collider = create_pivot(0); entity_radius(n->Collider, r, r); return n->Collider; }
static NPCs *sametype(NPCs *n) { each(NPCs, n2) if (n2->NPCtype == n->NPCtype && n2 != n) return n2; return NULL; }
static int count(NPCs *n) { int c = 0; each(NPCs, n2) if (n2->NPCtype == n->NPCtype && n2 != n) c++; return c; }
// MeshCullBox is a no-op here (no per-entity frustum culling), so CreateNPC's calls are left out

NPCs *CreateNPC(int type, float x, float y, float z) {
	NPCs *n = lnew(NPCs);
	float t;
	n->BlinkTimer = 1; n->InFacility = 1; n->TextureID = -1; // field defaults
	n->NPCtype = type;
	n->GravityMult = 1; n->MaxGravity = .2f; n->CollRadius = .2f; n->FallingPickDistance = 10;
	switch (type) {
	case NPCtype173:
		n->NVName = "SCP-173";
		entity_radius(npivot(n, 0), .23f, .32f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->Gravity = 1;
		n->obj = LoadMesh_Strict("GFX\\npcs\\173_2.b3d", 0);
		if (s_eq(s_left(current_date(), 7), "31 Oct ")) { // jack-o-lantern on halloween
			HalloweenTex = 1;
			int tx = LoadTexture_Strict("GFX\\npcs\\173h.pt", 1);
			entity_texture(n->obj, tx, 0, 0);
			free_texture(tx);
		}
		t = inif("SCP-173", "scale") / mesh_depth(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-173", "speed") / 100;
		n->obj2 = LoadMesh_Strict("GFX\\173box.b3d", 0);
		scale_entity(n->obj2, RoomScale, RoomScale, RoomScale, 0);
		hide_entity(n->obj2);
		n->CollRadius = .32f;
		break;
	case NPCtypeOldMan: {
		n->NVName = "SCP-106";
		npivot(n, .2f);
		n->GravityMult = 0; n->MaxGravity = 0;
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\106_2.b3d", 0);
		t = inif("SCP-106", "scale") / 2.2f;
		scale_entity(n->obj, t, t, t, 0);
		int eyes = LoadTexture_Strict("GFX\\npcs\\oldmaneyes.jpg", 1);
		n->Speed = inif("SCP-106", "speed") / 100;
		n->obj2 = create_sprite(0);
		scale_sprite(n->obj2, .03f, .03f);
		entity_texture(n->obj2, eyes, 0, 0);
		entity_blend(n->obj2, 3);
		entity_fx(n->obj2, 1 + 8);
		sprite_view_mode(n->obj2, 2);
		free_texture(eyes);
		break;
	}
	case NPCtypeGuard:
		n->NVName = "Human";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(GuardObj, 0);
		n->Speed = inif("Guard", "speed") / 100;
		t = inif("Guard", "scale") / 2.5f;
		scale_entity(n->obj, t, t, t, 0);
		break;
	case NPCtypeMTF:
		n->NVName = "Human";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(MTFObj, 0);
		n->Speed = inif("MTF", "speed") / 100;
		t = inif("MTF", "scale") / 2.5f;
		scale_entity(n->obj, t, t, t, 0);
		if (!MTFSFX[0]) {
			for (int i = 0; i < 4; i++) MTFSFX[i] = LoadSound_Strict(sf("SFX\\Character\\MTF\\ClassD%d.ogg", i + 1));
			MTFSFX[5] = LoadSound_Strict("SFX\\Character\\MTF\\Beep.ogg");
			MTFSFX[6] = LoadSound_Strict("SFX\\Character\\MTF\\Breath.ogg");
		}
		if (!MTFrooms[6]) {
			static const char *nm[] = {"room106", "roompj", "room079", "room2poffices", "914", "coffin", "start"};
			each(Rooms, r) for (int i = 0; i < 7; i++) if (s_eq(s_lower(r->RoomTemplate->Name), nm[i])) MTFrooms[i] = r;
		}
		break;
	case NPCtypeD:
		n->NVName = "Human";
		npivot(n, .32f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(ClassDObj, 0);
		t = .5f / mesh_width(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		n->CollRadius = .32f;
		break;
	case NPCtype372:
		npivot(n, .2f);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\372.b3d", 0);
		t = .35f / mesh_width(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		break;
	case NPCtype5131:
		n->NVName = "SCP-513-1";
		npivot(n, .2f);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\bll.b3d", 0);
		n->obj2 = copy_entity(n->obj, 0);
		entity_alpha(n->obj2, .6f);
		t = 1.8f / mesh_width(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		scale_entity(n->obj2, t, t, t, 0);
		break;
	case NPCtype096:
		n->NVName = "SCP-096";
		npivot(n, .26f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp096.b3d", 0);
		n->Speed = inif("SCP-096", "speed") / 100;
		t = inif("SCP-096", "scale") / 3;
		scale_entity(n->obj, t, t, t, 0);
		n->CollRadius = .26f;
		break;
	case NPCtype049:
		n->NVName = "SCP-049";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(NPC049OBJ, 0);
		n->Speed = inif("SCP-049", "speed") / 100;
		t = inif("SCP-049", "scale");
		scale_entity(n->obj, t, t, t, 0);
		n->Sound = LoadSound_Strict("SFX\\Horror\\Horror12.ogg");
		if (!HorrorSFX[13]) HorrorSFX[13] = LoadSound_Strict("SFX\\Horror\\Horror13.ogg");
		n->CanUseElevator = 1;
		break;
	case NPCtypeZombie:
		n->NVName = "Human";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(NPC0492OBJ, 0);
		t = inif("SCP-049-2", "scale") / 2.5f;
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-049-2", "speed") / 100;
		set_anim_time(n->obj, 107, 0);
		n->Sound = LoadSound_Strict("SFX\\SCP\\049\\0492Breath.ogg");
		n->HP = 100;
		break;
	case NPCtypeApache:
		n->NVName = "Human";
		n->GravityMult = 0; n->MaxGravity = 0;
		npivot(n, .2f);
		n->obj = copy_entity(ApacheObj, 0);
		n->obj2 = copy_entity(ApacheRotorObj, 0);
		entity_parent(n->obj2, n->obj, 1);
		for (int i = -1; i <= 1; i += 2) {
			int r2 = copy_entity(n->obj2, n->obj2);
			rotate_entity(r2, 0, 4.0f * i, 0, 0);
			entity_alpha(r2, .5f);
		}
		n->obj3 = LoadAnimMesh_Strict("GFX\\apacherotor2.b3d", n->obj);
		position_entity(n->obj3, 0, 2.15f, -5.48f, 0);
		entity_type(n->Collider, HIT_APACHE, 0);
		entity_radius(n->Collider, 3, 3);
		for (int i = -1; i <= 1; i += 2) {
			int l = create_light(2, n->obj);
			light_range(l, 2); light_color(l, 255, 255, 255);
			position_entity(l, 1.65f * i, 1.17f, -.25f, 0);
			int ls = create_sprite(n->obj);
			position_entity(ls, 1.65f * i, 1.17f, -.25f, 0);
			scale_sprite(ls, .13f, .13f);
			entity_texture(ls, LightSpriteTex[0], 0, 0);
			entity_blend(ls, 3);
			entity_fx(ls, 1 + 8);
		}
		scale_entity(n->obj, .6f, .6f, .6f, 0);
		break;
	case NPCtypeTentacle: {
		n->NVName = "Unidentified";
		n->Collider = create_pivot(0);
		NPCs *o = sametype(n);
		if (o) n->obj = copy_entity(o->obj, 0);
		if (!n->obj) { n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\035tentacle.b3d", 0); scale_entity(n->obj, .065f, .065f, .065f, 0); }
		set_anim_time(n->obj, 283, 0);
		break;
	}
	case NPCtype860: {
		n->NVName = "Unidentified";
		npivot(n, .25f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\forestmonster.b3d", 0);
		entity_fx(n->obj, 1);
		int tx = LoadTexture_Strict("GFX\\npcs\\860_eyes.png", 1 + 2);
		n->obj2 = create_sprite(0);
		scale_sprite(n->obj2, .1f, .1f);
		entity_texture(n->obj2, tx, 0, 0);
		free_texture(tx);
		entity_fx(n->obj2, 1 + 8);
		entity_blend(n->obj2, BL_ADD);
		sprite_view_mode(n->obj2, 2);
		n->Speed = inif("forestmonster", "speed") / 100;
		t = inif("forestmonster", "scale") / 20;
		scale_entity(n->obj, t, t, t, 0);
		n->CollRadius = .25f;
		break;
	}
	case NPCtype939: {
		int c = count(n), i = 0; // blitz's i keeps 0 for a 4th 939
		if (c == 0) i = 53;
		if (c == 1) i = 89;
		if (c == 2) i = 96;
		n->NVName = cat("SCP-939-", s_str(i));
		npivot(n, .3f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		NPCs *o = sametype(n);
		if (o) n->obj = copy_entity(o->obj, 0);
		if (!n->obj) {
			n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\scp-939.b3d", 0);
			t = inif("SCP-939", "scale") / 2.5f;
			scale_entity(n->obj, t, t, t, 0);
		}
		n->Speed = inif("SCP-939", "speed") / 100;
		n->CollRadius = .3f;
		break;
	}
	case NPCtype066:
		n->NVName = "SCP-066";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\scp-066.b3d", 0);
		t = inif("SCP-066", "scale") / 2.5f;
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-066", "speed") / 100;
		break;
	case NPCtype966: {
		n->NVName = cat("SCP-966-", s_str(count(n) + 1));
		npivot(n, .2f);
		NPCs *o = sametype(n);
		if (o) n->obj = copy_entity(o->obj, 0);
		if (!n->obj) n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp-966.b3d", 0);
		entity_fx(n->obj, 1);
		t = inif("SCP-966", "scale") / 40;
		scale_entity(n->obj, t, t, t, 0);
		set_anim_time(n->obj, 15, 0);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->Speed = inif("SCP-966", "speed") / 100;
		break;
	}
	case NPCtype1048a: // no collider, like blitz
		n->NVName = "SCP-1048-A";
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048a.b3d", 0);
		scale_entity(n->obj, .05f, .05f, .05f, 0);
		set_anim_time(n->obj, 2, 0);
		n->Sound = LoadSound_Strict("SFX\\SCP\\1048A\\Shriek.ogg");
		n->Sound2 = LoadSound_Strict("SFX\\SCP\\1048A\\Growth.ogg");
		break;
	case NPCtype1499: {
		n->NVName = "Unidentified";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		NPCs *o = sametype(n);
		if (o) n->obj = copy_entity(o->obj, 0);
		if (!n->obj) n->obj = LoadAnimMesh_Strict("GFX\\npcs\\1499-1.b3d", 0);
		n->Speed = inif("SCP-1499-1", "speed") / 100 * frnd(.9f, 1.1f);
		t = inif("SCP-1499-1", "scale") / 4 * frnd(.8f, 1);
		scale_entity(n->obj, t, t, t, 0);
		entity_fx(n->obj, 1);
		// TODO EntityAutoFade(obj, HideDistance*2.5, HideDistance*2.95): not in the runtime yet
		break;
	}
	case NPCtype008:
		n->NVName = "Human";
		npivot(n, .2f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\zombiesurgeon.b3d", 0);
		t = .5f / mesh_width(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		SetNPCFrame(n, 11);
		n->Sound = LoadSound_Strict("SFX\\SCP\\049\\0492Breath.ogg");
		n->HP = 120;
		break;
	case NPCtypeClerk:
		n->NVName = "Human";
		npivot(n, .32f);
		entity_type(n->Collider, HIT_PLAYER, 0);
		n->obj = copy_entity(ClerkOBJ, 0);
		t = .5f / mesh_width(n->obj);
		scale_entity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		n->CollRadius = .32f;
		break;
	}
	if (n->Collider) position_entity(n->Collider, x, y, z, 1);
	position_entity(n->obj, x, y, z, 1);
	if (n->Collider) reset_entity(n->Collider);
	n->ID = 0;
	n->ID = FindFreeNPCID();
	NPCSpeedChange(n);
	return n;
}

static void stopchn(int chn, int stream) {
	if (!stream) { if (chn && channel_playing(chn)) stop_channel(chn); }
	else if (chn) StopStream_Strict(chn);
}

void RemoveNPC(NPCs *n) {
	if (!n) return;
	if (n->obj2) { free_entity(n->obj2); n->obj2 = 0; }
	if (n->obj3) { free_entity(n->obj3); n->obj3 = 0; }
	if (n->obj4) { free_entity(n->obj4); n->obj4 = 0; }
	stopchn(n->SoundChn, n->SoundChn_IsStream);
	stopchn(n->SoundChn2, n->SoundChn2_IsStream);
	if (n->Sound) FreeSound_Strict(n->Sound);
	if (n->Sound2) FreeSound_Strict(n->Sound2);
	free_entity(n->obj); n->obj = 0;
	if (n->Collider) free_entity(n->Collider);
	n->Collider = 0;
	ldel(n);
}

// the gravity part at the end of UpdateNPCs' loop
static void gravity(NPCs *n) {
	int gd = iround(Distance(entity_x(Collider, 0), entity_z(Collider, 0), entity_x(n->Collider, 0), entity_z(n->Collider, 0))); // blitz Local w/o suffix: int
	if (!(gd < HideDistance * .7f || n->NPCtype == NPCtype1499) || n->InFacility != InFacility) { n->DropSpeed = 0; return; }
	translate_entity(n->Collider, 0, n->DropSpeed, 0, 0);
	int floor = 0;
	for (int i = 1; i <= count_collisions(n->Collider); i++) if (collision_y(n->Collider, i) < entity_y(n->Collider, 0) - .01f) { floor = 1; break; }
	if (floor) { n->DropSpeed = 0; return; }
	if (!ShouldEntitiesFall) { n->DropSpeed = 0; return; }
	int upd = 0;
	if (n->InFacility == 1) {
		if (!s_eq(RN, "173")) { each(Events, e) if (s_eq(e->EventName, "room860") && e->EventState == 1) { upd = 1; break; } }
		else upd = 1;
		if (!upd) each(Rooms, r) {
			float mx = 4, nx = 0, mz = 4, nz = 0;
			if (r->MaxX != 0 || r->MinX != 0 || r->MaxZ != 0 || r->MinZ != 0) { mx = r->MaxX; nx = r->MinX; mz = r->MaxZ; nz = r->MinZ; }
			if (fabsf(entity_x(n->Collider, 0) - entity_x(r->obj, 0)) > fabsf(mx - nx)) continue;
			if (fabsf(entity_z(n->Collider, 0) - entity_z(r->obj, 0)) > fabsf(mz - nz)) continue;
			if (r == PlayerRoom || IsRoomAdjacent(PlayerRoom, r)) { upd = 1; break; }
			for (int i = 0; i <= 3; i++) if (IsRoomAdjacent(PlayerRoom->Adjacent[i], r)) { upd = 1; break; } // blitz's Exit only leaves this For
		}
	} else upd = 1;
	if (upd || n->FallingPickDistance <= 0) n->DropSpeed = Max(n->DropSpeed - .005f * FPSfactor * n->GravityMult, -n->MaxGravity);
	else n->DropSpeed = 0;
}

void UpdateNPCs(void) {
	each(NPCs, n) {
		n->InFacility = CheckForNPCInFacility(n);
		switch (n->NPCtype) {
		case NPCtype173: Update173(n); break;
		case NPCtypeOldMan: Update106(n); break;
		case NPCtypeGuard: UpdateGuard(n); break;
		case NPCtypeD: case NPCtypeClerk: UpdateClassD(n); break;
		default: break; // TODO the other types (NPCs.bb 1242-5122), they only spawn from events so far
		}
		if (n->IsDead) entity_type(n->Collider, HIT_DEAD, 0);
		gravity(n);
	}
	// TODO MTF_CameraCheckTimer's announcements (only MTF units start that timer)
}

void TeleportCloser(NPCs *n) {
	float cd = 0;
	WayPoints *cw = NULL;
	each(WayPoints, w) {
		if (w->door) continue;
		float xt = fabsf(entity_x(w->obj, 1) - entity_x(n->Collider, 1));
		if (!(xt < 10 && xt > 1)) continue;
		float zt = fabsf(entity_z(w->obj, 1) - entity_z(n->Collider, 1));
		if (!(zt < 10 && zt > 1)) continue;
		if (entity_distance(Collider, w->obj) > 16 - 8 * SelectedDifficulty->aggressiveNPCs) {
			float nd = entity_distance(Collider, w->obj); // closest to the player
			if (nd < cd || !cw) { cd = nd; cw = w; }
		}
	}
	if (!cw) return;
	float wy = entity_y(cw->obj, 1);
	if (n->InFacility != 1 || SelectedDifficulty->aggressiveNPCs || (wy <= 7 && wy >= -10)) {
		position_entity(n->Collider, entity_x(cw->obj, 1), wy + .15f, entity_z(cw->obj, 1), 1);
		reset_entity(n->Collider);
		n->PathStatus = 0; n->PathTimer = 0; n->PathLocation = 0;
	}
}

int OtherNPCSeesMeNPC(NPCs *me, NPCs *other) {
	if (other->BlinkTimer <= 0) return 0;
	return entity_distance(other->Collider, me->Collider) < 6 && fabsf(delta_yaw(other->Collider, me->Collider)) < 60;
}

// 0 no, 1 seen, 2 heard, 3 caught on camera (mtf only), 4 through glass
int MeNPCSeesPlayer(NPCs *me, int disablesoundoncrouch) {
	if (NoTarget) return 0;
	if (!PlayerDetected || me->NPCtype != NPCtypeMTF) {
		if (me->BlinkTimer <= 0) return 0;
		if (entity_distance(Collider, me->Collider) > 8 - CrouchState + PlayerSoundVolume) return 0;
		if (PlayerSoundVolume > 1) {
			if (fabsf(delta_yaw(me->Collider, Collider)) > 60 && entity_visible(me->Collider, Collider)) return 1;
			if (!entity_visible(me->Collider, Collider)) return disablesoundoncrouch && Crouch ? 0 : 2;
		} else if (fabsf(delta_yaw(me->Collider, Collider)) > 60) return 0;
		return entity_visible(me->Collider, Collider);
	}
	if (entity_distance(Collider, me->Collider) > 8 - CrouchState + PlayerSoundVolume) return 3;
	if (entity_visible(me->Collider, Camera)) return 1;
	return PlayerSoundVolume > 1 ? 2 : 3;
}

void MoveToPocketDimension(void) {
	each(Rooms, r) {
		if (!s_eq(r->RoomTemplate->Name, "pocketdimension")) continue;
		FallTimer = 0;
		UpdateDoors();
		UpdateRooms();
		show_entity(Collider);
		PlaySound_Strict(Use914SFX);
		PlaySound_Strict(OldManSFX[5]);
		position_entity(Collider, entity_x(r->obj, 0), .8f, entity_z(r->obj, 0), 0);
		DropSpeed = 0;
		reset_entity(Collider);
		BlinkTimer = -10;
		Injuries += .5f;
		PlayerRoom = r;
		return;
	}
}

int FindFreeNPCID(void) {
	for (int id = 1;; id++) {
		int taken = 0;
		each(NPCs, n2) if (n2->ID == id) { taken = 1; break; }
		if (!taken) return id;
	}
}

void ForceSetNPCID(NPCs *n, int id) {
	n->ID = id;
	each(NPCs, n2) if (n2 != n && n2->ID == id) n2->ID = FindFreeNPCID();
}

void NPCSpeedChange(NPCs *n) {
	switch (n->NPCtype) {
	case NPCtype173: case NPCtypeOldMan: case NPCtype096: case NPCtype049: case NPCtype939: case NPCtypeMTF:
		if (SelectedDifficulty->otherFactors == NORMAL) n->Speed *= 1.1f;
		else if (SelectedDifficulty->otherFactors == HARD) n->Speed *= 1.2f;
	}
}

int PlayerInReachableRoom(int canSpawnIn049Chamber) {
	if (s_eq(RN, "pocketdimension") || s_eq(RN, "gatea") || s_eq(RN, "dimension1499") || s_eq(RN, "173")) return 0;
	if (s_eq(RN, "exit1") && entity_y(Collider, 0) > 1040 * RoomScale) return 0;
	int temp = 0;
	each(Events, e) if (s_eq(e->EventName, "room860") && e->EventState == 1) { temp = 1; break; }
	if (s_eq(RN, "room860") && temp) return 0;
	if (!canSpawnIn049Chamber && !SelectedDifficulty->aggressiveNPCs && s_eq(RN, "room049") && entity_y(Collider, 0) <= -2848 * RoomScale) return 0;
	return 1;
}

// 0 outside (dimension1499), 1 facility, 2 tunnels
int CheckForNPCInFacility(NPCs *n) {
	float y = entity_y(n->Collider, 0);
	if (y > 100) return 0;
	if (y < -10) return 2;
	if (y > 7 && y <= 100) return 2;
	return 1;
}

void FindNextElevator(NPCs *n) {
	each(ElevatorObj, eo) {
		if (eo->InFacility != n->InFacility || fabsf(entity_y(eo->obj, 1) - entity_y(n->Collider, 0)) >= 10) continue;
		each(ElevatorObj, eo2) {
			if (eo2 == eo || eo2->InFacility != n->InFacility || fabsf(entity_y(eo2->obj, 1) - entity_y(n->Collider, 0)) >= 10) continue;
			if (entity_distance(eo2->obj, n->Collider) < entity_distance(eo->obj, n->Collider)) {
				n->PathStatus = FindPath(n, entity_x(eo2->obj, 1), entity_y(eo2->obj, 1), entity_z(eo2->obj, 1));
				n->CurrElevator = eo2;
				break;
			}
		}
		if (!n->CurrElevator) {
			n->PathStatus = FindPath(n, entity_x(eo->obj, 1), entity_y(eo->obj, 1), entity_z(eo->obj, 1));
			n->CurrElevator = eo;
		}
		if (n->PathStatus != 1) n->CurrElevator = NULL;
		break;
	}
}

void GoToElevator(NPCs *n) {
	if (n->PathStatus == 1) return;
	ElevatorObj *ce = n->CurrElevator;
	point_entity(n->obj, ce->obj, 0);
	rotate_entity(n->Collider, 0, CurveAngle(entity_yaw(n->obj, 0), entity_yaw(n->Collider, 0), 20), 0, 0);
	float r = 280 * RoomScale;
	int inside = fabsf(entity_x(n->Collider, 0) - entity_x(ce->obj, 1)) < r && fabsf(entity_z(n->Collider, 0) - entity_z(ce->obj, 1)) < r
		&& fabsf(entity_y(n->Collider, 0) - entity_y(ce->obj, 1)) < r;
	float dist = entity_distance(n->Collider, ce->door->frameobj);
	if (ce->door->open) { if (dist > .4f && dist < .7f && inside) UseDoor(ce->door, 0, 1); }
	else if (dist < .7f) { n->CurrSpeed = 0; ce->door->NPCCalledElevator = 1; }
}

void FinishWalking(NPCs *n, float startframe, float endframe, float speed) {
	if (!n) return;
	float c = (endframe - startframe) / 2;
	if (n->Frame >= c) AnimateNPC(n, startframe, endframe, speed, 0);
	else AnimateNPC(n, endframe, startframe, -speed, 0);
}

void AnimateNPC(NPCs *n, float start, float quit, float speed, int loop) {
	float t;
	if (speed > 0) {
		t = Max(Min(n->Frame + speed * FPSfactor, quit), start);
		if (loop && t >= quit) t = start;
	} else {
		if (start < quit) { int tmp = iround(start); start = quit; quit = tmp; } // blitz swaps through an int temp
		if (loop) {
			t = n->Frame + speed * FPSfactor;
			if (t < quit) t = start;
			else if (t > start) t = quit;
		} else t = Max(Min(n->Frame + speed * FPSfactor, start), quit);
	}
	SetNPCFrame(n, t);
}

void SetNPCFrame(NPCs *n, float frame) {
	if (fabsf(n->Frame - frame) < .001f) return;
	set_anim_time(n->obj, frame, 0);
	n->Frame = frame;
}

float Animate2(int e, float curr, int start, int quit, float speed, int loop) {
	float t;
	if (speed > 0) {
		t = Max(Min(curr + speed * FPSfactor, quit), start);
		if (loop && t >= quit) t = start;
	} else {
		if (start < quit) { int tmp = start; start = quit; quit = tmp; }
		if (loop) {
			t = curr + speed * FPSfactor;
			if (t < quit) t = start;
			if (t > start) t = quit;
		} else t = Max(Min(curr + speed * FPSfactor, start), quit);
	}
	set_anim_time(e, t, 0);
	return t;
}

// Main.bb: drop e onto the floor below (dir 0) or the ceiling above (dir 1) x,y,z
void TeleportEntity(int e, float x, float y, float z, float customradius, int isglobal, float pickrange, int dir) {
	int pvt = create_pivot(0);
	position_entity(pvt, x, y + .05f, z, isglobal);
	rotate_entity(pvt, dir == 0 ? 90 : -90, 0, 0, 0);
	if (entity_pick(pvt, pickrange)) position_entity(e, x, picked_y() + customradius + (dir == 0 ? .02f : -.02f), z, isglobal);
	else position_entity(e, x, y, z, isglobal);
	free_entity(pvt);
	reset_entity(e);
}
