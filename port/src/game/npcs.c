// npcs.c: NPCs.bb's CreateNPC/RemoveNPC, the UpdateNPCs frame (per-type cases live in npc*.c), helpers
// + Main.bb's AnimateNPC/SetNPCFrame/Animate2/TeleportEntity
#include "g.h"

TLIST(NPCs);

#define INI "DATA\\NPCs.ini"
#define RN (PlayerRoom->RoomTemplate->Name)
static float inif(const char *sec, const char *k) { return GetINIFloat(INI, sec, k, 0); }
static int npivot(NPCs *n, float r) { n->Collider = bbCreatePivot(0); bbEntityRadius(n->Collider, r, r); return n->Collider; }
static NPCs *sametype(NPCs *n) { each(NPCs, n2) if (n2->NPCtype == n->NPCtype && n2 != n) return n2; return NULL; }
static int count(NPCs *n) { int c = 0; each(NPCs, n2) if (n2->NPCtype == n->NPCtype && n2 != n) c++; return c; }
// MeshCullBox is a no-op here (no per-entity frustum culling), so CreateNPC's calls are left out

NPCs *CreateNPC(int type, float x, float y, float z) {
	NPCs *n = bbNew(NPCs);
	float t;
	n->BlinkTimer = 1; n->InFacility = 1; n->TextureID = -1; // field defaults
	n->NPCtype = type;
	n->GravityMult = 1; n->MaxGravity = .2f; n->CollRadius = .2f; n->FallingPickDistance = 10;
	switch (type) {
	case NPCtype173:
		n->NVName = "SCP-173";
		bbEntityRadius(npivot(n, 0), .23f, .32f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->Gravity = 1;
		n->obj = LoadMesh_Strict("GFX\\npcs\\173_2.b3d", 0);
		if (bbSeq(bbLeft(bbCurrentDate(), 7), "31 Oct ")) { // jack-o-lantern on halloween
			HalloweenTex = 1;
			int tx = LoadTexture_Strict("GFX\\npcs\\173h.pt", 1);
			bbEntityTexture(n->obj, tx, 0, 0);
			bbFreeTexture(tx);
		}
		t = inif("SCP-173", "scale") / bbMeshDepth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-173", "speed") / 100;
		n->obj2 = LoadMesh_Strict("GFX\\173box.b3d", 0);
		bbScaleEntity(n->obj2, RoomScale, RoomScale, RoomScale, 0);
		bbHideEntity(n->obj2);
		n->CollRadius = .32f;
		break;
	case NPCtypeOldMan: {
		n->NVName = "SCP-106";
		npivot(n, .2f);
		n->GravityMult = 0; n->MaxGravity = 0;
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\106_2.b3d", 0);
		t = inif("SCP-106", "scale") / 2.2f;
		bbScaleEntity(n->obj, t, t, t, 0);
		int eyes = LoadTexture_Strict("GFX\\npcs\\oldmaneyes.jpg", 1);
		n->Speed = inif("SCP-106", "speed") / 100;
		n->obj2 = bbCreateSprite(0);
		bbScaleSprite(n->obj2, .03f, .03f);
		bbEntityTexture(n->obj2, eyes, 0, 0);
		bbEntityBlend(n->obj2, 3);
		bbEntityFX(n->obj2, 1 + 8);
		bbSpriteViewMode(n->obj2, 2);
		bbFreeTexture(eyes);
		break;
	}
	case NPCtypeGuard:
		n->NVName = "Human";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(GuardObj, 0);
		n->Speed = inif("Guard", "speed") / 100;
		t = inif("Guard", "scale") / 2.5f;
		bbScaleEntity(n->obj, t, t, t, 0);
		break;
	case NPCtypeMTF:
		n->NVName = "Human";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(MTFObj, 0);
		n->Speed = inif("MTF", "speed") / 100;
		t = inif("MTF", "scale") / 2.5f;
		bbScaleEntity(n->obj, t, t, t, 0);
		if (!MTFSFX[0]) {
			for (int i = 0; i < 4; i++) MTFSFX[i] = LoadSound_Strict(sf("SFX\\Character\\MTF\\ClassD%d.ogg", i + 1));
			MTFSFX[5] = LoadSound_Strict("SFX\\Character\\MTF\\Beep.ogg");
			MTFSFX[6] = LoadSound_Strict("SFX\\Character\\MTF\\Breath.ogg");
		}
		if (!MTFrooms[6]) {
			static const char *nm[] = {"room106", "roompj", "room079", "room2poffices", "914", "coffin", "start"};
			each(Rooms, r) for (int i = 0; i < 7; i++) if (bbSeq(bbLower(r->RoomTemplate->Name), nm[i])) MTFrooms[i] = r;
		}
		break;
	case NPCtypeD:
		n->NVName = "Human";
		npivot(n, .32f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(ClassDObj, 0);
		t = .5f / bbMeshWidth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		n->CollRadius = .32f;
		break;
	case NPCtype372:
		npivot(n, .2f);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\372.b3d", 0);
		t = .35f / bbMeshWidth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		break;
	case NPCtype5131:
		n->NVName = "SCP-513-1";
		npivot(n, .2f);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\bll.b3d", 0);
		n->obj2 = bbCopyEntity(n->obj, 0);
		bbEntityAlpha(n->obj2, .6f);
		t = 1.8f / bbMeshWidth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		bbScaleEntity(n->obj2, t, t, t, 0);
		break;
	case NPCtype096:
		n->NVName = "SCP-096";
		npivot(n, .26f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp096.b3d", 0);
		n->Speed = inif("SCP-096", "speed") / 100;
		t = inif("SCP-096", "scale") / 3;
		bbScaleEntity(n->obj, t, t, t, 0);
		n->CollRadius = .26f;
		break;
	case NPCtype049:
		n->NVName = "SCP-049";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(NPC049OBJ, 0);
		n->Speed = inif("SCP-049", "speed") / 100;
		t = inif("SCP-049", "scale");
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Sound = LoadSound_Strict("SFX\\Horror\\Horror12.ogg");
		if (!HorrorSFX[13]) HorrorSFX[13] = LoadSound_Strict("SFX\\Horror\\Horror13.ogg");
		n->CanUseElevator = 1;
		break;
	case NPCtypeZombie:
		n->NVName = "Human";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(NPC0492OBJ, 0);
		t = inif("SCP-049-2", "scale") / 2.5f;
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-049-2", "speed") / 100;
		bbSetAnimTime(n->obj, 107, 0);
		n->Sound = LoadSound_Strict("SFX\\SCP\\049\\0492Breath.ogg");
		n->HP = 100;
		break;
	case NPCtypeApache:
		n->NVName = "Human";
		n->GravityMult = 0; n->MaxGravity = 0;
		npivot(n, .2f);
		n->obj = bbCopyEntity(ApacheObj, 0);
		n->obj2 = bbCopyEntity(ApacheRotorObj, 0);
		bbEntityParent(n->obj2, n->obj, 1);
		for (int i = -1; i <= 1; i += 2) {
			int r2 = bbCopyEntity(n->obj2, n->obj2);
			bbRotateEntity(r2, 0, 4.0f * i, 0, 0);
			bbEntityAlpha(r2, .5f);
		}
		n->obj3 = LoadAnimMesh_Strict("GFX\\apacherotor2.b3d", n->obj);
		bbPositionEntity(n->obj3, 0, 2.15f, -5.48f, 0);
		bbEntityType(n->Collider, HIT_APACHE, 0);
		bbEntityRadius(n->Collider, 3, 3);
		for (int i = -1; i <= 1; i += 2) {
			int l = bbCreateLight(2, n->obj);
			bbLightRange(l, 2); bbLightColor(l, 255, 255, 255);
			bbPositionEntity(l, 1.65f * i, 1.17f, -.25f, 0);
			int ls = bbCreateSprite(n->obj);
			bbPositionEntity(ls, 1.65f * i, 1.17f, 0, 0); // blitz passes -0.25 as the 5th (global) arg, which rounds to 0
			bbScaleSprite(ls, .13f, .13f);
			bbEntityTexture(ls, LightSpriteTex[0], 0, 0);
			bbEntityBlend(ls, 3);
			bbEntityFX(ls, 1 + 8);
		}
		bbScaleEntity(n->obj, .6f, .6f, .6f, 0);
		break;
	case NPCtypeTentacle: {
		n->NVName = "Unidentified";
		n->Collider = bbCreatePivot(0);
		NPCs *o = sametype(n);
		if (o) n->obj = bbCopyEntity(o->obj, 0);
		if (!n->obj) { n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\035tentacle.b3d", 0); bbScaleEntity(n->obj, .065f, .065f, .065f, 0); }
		bbSetAnimTime(n->obj, 283, 0);
		break;
	}
	case NPCtype860: {
		n->NVName = "Unidentified";
		npivot(n, .25f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\forestmonster.b3d", 0);
		bbEntityFX(n->obj, 1);
		int tx = LoadTexture_Strict("GFX\\npcs\\860_eyes.png", 1 + 2);
		n->obj2 = bbCreateSprite(0);
		bbScaleSprite(n->obj2, .1f, .1f);
		bbEntityTexture(n->obj2, tx, 0, 0);
		bbFreeTexture(tx);
		bbEntityFX(n->obj2, 1 + 8);
		bbEntityBlend(n->obj2, BL_ADD);
		bbSpriteViewMode(n->obj2, 2);
		n->Speed = inif("forestmonster", "speed") / 100;
		t = inif("forestmonster", "scale") / 20;
		bbScaleEntity(n->obj, t, t, t, 0);
		n->CollRadius = .25f;
		break;
	}
	case NPCtype939: {
		int c = count(n), i = 0; // blitz's i keeps 0 for a 4th 939
		if (c == 0) i = 53;
		if (c == 1) i = 89;
		if (c == 2) i = 96;
		n->NVName = cat("SCP-939-", bbStr(i));
		npivot(n, .3f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		NPCs *o = sametype(n);
		if (o) n->obj = bbCopyEntity(o->obj, 0);
		if (!n->obj) {
			n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\scp-939.b3d", 0);
			t = inif("SCP-939", "scale") / 2.5f;
			bbScaleEntity(n->obj, t, t, t, 0);
		}
		n->Speed = inif("SCP-939", "speed") / 100;
		n->CollRadius = .3f;
		break;
	}
	case NPCtype066:
		n->NVName = "SCP-066";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\NPCs\\scp-066.b3d", 0);
		t = inif("SCP-066", "scale") / 2.5f;
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = inif("SCP-066", "speed") / 100;
		break;
	case NPCtype966: {
		n->NVName = cat("SCP-966-", bbStr(count(n) + 1));
		npivot(n, .2f);
		NPCs *o = sametype(n);
		if (o) n->obj = bbCopyEntity(o->obj, 0);
		if (!n->obj) n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp-966.b3d", 0);
		bbEntityFX(n->obj, 1);
		t = inif("SCP-966", "scale") / 40;
		bbScaleEntity(n->obj, t, t, t, 0);
		bbSetAnimTime(n->obj, 15, 0);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->Speed = inif("SCP-966", "speed") / 100;
		break;
	}
	case NPCtype1048a: // no collider, like blitz
		n->NVName = "SCP-1048-A";
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048a.b3d", 0);
		bbScaleEntity(n->obj, .05f, .05f, .05f, 0);
		bbSetAnimTime(n->obj, 2, 0);
		n->Sound = LoadSound_Strict("SFX\\SCP\\1048A\\Shriek.ogg");
		n->Sound2 = LoadSound_Strict("SFX\\SCP\\1048A\\Growth.ogg");
		break;
	case NPCtype1499: {
		n->NVName = "Unidentified";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		NPCs *o = sametype(n);
		if (o) n->obj = bbCopyEntity(o->obj, 0);
		if (!n->obj) n->obj = LoadAnimMesh_Strict("GFX\\npcs\\1499-1.b3d", 0);
		n->Speed = inif("SCP-1499-1", "speed") / 100 * bbRnd(.9f, 1.1f);
		t = inif("SCP-1499-1", "scale") / 4 * bbRnd(.8f, 1);
		bbScaleEntity(n->obj, t, t, t, 0);
		bbEntityFX(n->obj, 1);
		// TODO EntityAutoFade(obj, HideDistance*2.5, HideDistance*2.95): not in the runtime yet
		break;
	}
	case NPCtype008:
		n->NVName = "Human";
		npivot(n, .2f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = LoadAnimMesh_Strict("GFX\\npcs\\zombiesurgeon.b3d", 0);
		t = .5f / bbMeshWidth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		SetNPCFrame(n, 11);
		n->Sound = LoadSound_Strict("SFX\\SCP\\049\\0492Breath.ogg");
		n->HP = 120;
		break;
	case NPCtypeClerk:
		n->NVName = "Human";
		npivot(n, .32f);
		bbEntityType(n->Collider, HIT_PLAYER, 0);
		n->obj = bbCopyEntity(ClerkOBJ, 0);
		t = .5f / bbMeshWidth(n->obj);
		bbScaleEntity(n->obj, t, t, t, 0);
		n->Speed = 2.0f / 100;
		n->CollRadius = .32f;
		break;
	}
	if (n->Collider) bbPositionEntity(n->Collider, x, y, z, 1);
	bbPositionEntity(n->obj, x, y, z, 1);
	if (n->Collider) bbResetEntity(n->Collider);
	n->ID = 0;
	n->ID = FindFreeNPCID();
	NPCSpeedChange(n);
	return n;
}

static void stopchn(int chn, int stream) {
	if (!stream) { if (chn && bbChannelPlaying(chn)) bbStopChannel(chn); }
	else if (chn) StopStream_Strict(chn);
}

void RemoveNPC(NPCs *n) {
	if (!n) return;
	if (n->obj2) { bbFreeEntity(n->obj2); n->obj2 = 0; }
	if (n->obj3) { bbFreeEntity(n->obj3); n->obj3 = 0; }
	if (n->obj4) { bbFreeEntity(n->obj4); n->obj4 = 0; }
	stopchn(n->SoundChn, n->SoundChn_IsStream);
	stopchn(n->SoundChn2, n->SoundChn2_IsStream);
	if (n->Sound) FreeSound_Strict(n->Sound);
	if (n->Sound2) FreeSound_Strict(n->Sound2);
	bbFreeEntity(n->obj); n->obj = 0;
	if (n->Collider) bbFreeEntity(n->Collider);
	n->Collider = 0;
	bbDelete(n);
}

// the gravity part at the end of UpdateNPCs' loop
static void gravity(NPCs *n) {
	int gd = bbInt(Distance(bbEntityX(Collider, 0), bbEntityZ(Collider, 0), bbEntityX(n->Collider, 0), bbEntityZ(n->Collider, 0))); // blitz Local w/o suffix: int
	if (!(gd < HideDistance * .7f || n->NPCtype == NPCtype1499) || n->InFacility != InFacility) { n->DropSpeed = 0; return; }
	bbTranslateEntity(n->Collider, 0, n->DropSpeed, 0, 0);
	int floor = 0;
	for (int i = 1; i <= bbCountCollisions(n->Collider); i++) if (bbCollisionY(n->Collider, i) < bbEntityY(n->Collider, 0) - .01f) { floor = 1; break; }
	if (floor) { n->DropSpeed = 0; return; }
	if (!ShouldEntitiesFall) { n->DropSpeed = 0; return; }
	int upd = 0;
	if (n->InFacility == 1) {
		if (!bbSeq(RN, "173")) { each(Events, e) if (bbSeq(e->EventName, "room860") && e->EventState == 1) { upd = 1; break; } }
		else upd = 1;
		if (!upd) each(Rooms, r) {
			float mx = 4, nx = 0, mz = 4, nz = 0;
			if (r->MaxX != 0 || r->MinX != 0 || r->MaxZ != 0 || r->MinZ != 0) { mx = r->MaxX; nx = r->MinX; mz = r->MaxZ; nz = r->MinZ; }
			if (fabsf(bbEntityX(n->Collider, 0) - bbEntityX(r->obj, 0)) > fabsf(mx - nx)) continue;
			if (fabsf(bbEntityZ(n->Collider, 0) - bbEntityZ(r->obj, 0)) > fabsf(mz - nz)) continue;
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
		default: break; // TODO the other types (NPCs.bb 1242-5122), they only spawn from events so far
		}
		if (n->IsDead) bbEntityType(n->Collider, HIT_DEAD, 0);
		gravity(n);
	}
	// TODO MTF_CameraCheckTimer's announcements (only MTF units start that timer)
}

void TeleportCloser(NPCs *n) {
	float cd = 0;
	WayPoints *cw = NULL;
	each(WayPoints, w) {
		if (w->door) continue;
		float xt = fabsf(bbEntityX(w->obj, 1) - bbEntityX(n->Collider, 1));
		if (!(xt < 10 && xt > 1)) continue;
		float zt = fabsf(bbEntityZ(w->obj, 1) - bbEntityZ(n->Collider, 1));
		if (!(zt < 10 && zt > 1)) continue;
		if (bbEntityDistance(Collider, w->obj) > 16 - 8 * SelectedDifficulty->aggressiveNPCs) {
			float nd = bbEntityDistance(Collider, w->obj); // closest to the player
			if (nd < cd || !cw) { cd = nd; cw = w; }
		}
	}
	if (!cw) return;
	float wy = bbEntityY(cw->obj, 1);
	if (n->InFacility != 1 || SelectedDifficulty->aggressiveNPCs || (wy <= 7 && wy >= -10)) {
		bbPositionEntity(n->Collider, bbEntityX(cw->obj, 1), wy + .15f, bbEntityZ(cw->obj, 1), 1);
		bbResetEntity(n->Collider);
		n->PathStatus = 0; n->PathTimer = 0; n->PathLocation = 0;
	}
}

int OtherNPCSeesMeNPC(NPCs *me, NPCs *other) {
	if (other->BlinkTimer <= 0) return 0;
	return bbEntityDistance(other->Collider, me->Collider) < 6 && fabsf(bbDeltaYaw(other->Collider, me->Collider)) < 60;
}

// 0 no, 1 seen, 2 heard, 3 caught on camera (mtf only), 4 through glass
int MeNPCSeesPlayer(NPCs *me, int disablesoundoncrouch) {
	if (NoTarget) return 0;
	if (!PlayerDetected || me->NPCtype != NPCtypeMTF) {
		if (me->BlinkTimer <= 0) return 0;
		if (bbEntityDistance(Collider, me->Collider) > 8 - CrouchState + PlayerSoundVolume) return 0;
		if (PlayerSoundVolume > 1) {
			if (fabsf(bbDeltaYaw(me->Collider, Collider)) > 60 && bbEntityVisible(me->Collider, Collider)) return 1;
			if (!bbEntityVisible(me->Collider, Collider)) return disablesoundoncrouch && Crouch ? 0 : 2;
		} else if (fabsf(bbDeltaYaw(me->Collider, Collider)) > 60) return 0;
		return bbEntityVisible(me->Collider, Collider);
	}
	if (bbEntityDistance(Collider, me->Collider) > 8 - CrouchState + PlayerSoundVolume) return 3;
	if (bbEntityVisible(me->Collider, Camera)) return 1;
	return PlayerSoundVolume > 1 ? 2 : 3;
}

void MoveToPocketDimension(void) {
	each(Rooms, r) {
		if (!bbSeq(r->RoomTemplate->Name, "pocketdimension")) continue;
		FallTimer = 0;
		UpdateDoors();
		UpdateRooms();
		bbShowEntity(Collider);
		PlaySound_Strict(Use914SFX);
		PlaySound_Strict(OldManSFX[5]);
		bbPositionEntity(Collider, bbEntityX(r->obj, 0), .8f, bbEntityZ(r->obj, 0), 0);
		DropSpeed = 0;
		bbResetEntity(Collider);
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
	if (bbSeq(RN, "pocketdimension") || bbSeq(RN, "gatea") || bbSeq(RN, "dimension1499") || bbSeq(RN, "173")) return 0;
	if (bbSeq(RN, "exit1") && bbEntityY(Collider, 0) > 1040 * RoomScale) return 0;
	int temp = 0;
	each(Events, e) if (bbSeq(e->EventName, "room860") && e->EventState == 1) { temp = 1; break; }
	if (bbSeq(RN, "room860") && temp) return 0;
	if (!canSpawnIn049Chamber && !SelectedDifficulty->aggressiveNPCs && bbSeq(RN, "room049") && bbEntityY(Collider, 0) <= -2848 * RoomScale) return 0;
	return 1;
}

// 0 outside (dimension1499), 1 facility, 2 tunnels
int CheckForNPCInFacility(NPCs *n) {
	float y = bbEntityY(n->Collider, 0);
	if (y > 100) return 0;
	if (y < -10) return 2;
	if (y > 7 && y <= 100) return 2;
	return 1;
}

void FindNextElevator(NPCs *n) {
	each(ElevatorObj, eo) {
		if (eo->InFacility != n->InFacility || fabsf(bbEntityY(eo->obj, 1) - bbEntityY(n->Collider, 0)) >= 10) continue;
		each(ElevatorObj, eo2) {
			if (eo2 == eo || eo2->InFacility != n->InFacility || fabsf(bbEntityY(eo2->obj, 1) - bbEntityY(n->Collider, 0)) >= 10) continue;
			if (bbEntityDistance(eo2->obj, n->Collider) < bbEntityDistance(eo->obj, n->Collider)) {
				n->PathStatus = FindPath(n, bbEntityX(eo2->obj, 1), bbEntityY(eo2->obj, 1), bbEntityZ(eo2->obj, 1));
				n->CurrElevator = eo2;
				break;
			}
		}
		if (!n->CurrElevator) {
			n->PathStatus = FindPath(n, bbEntityX(eo->obj, 1), bbEntityY(eo->obj, 1), bbEntityZ(eo->obj, 1));
			n->CurrElevator = eo;
		}
		if (n->PathStatus != 1) n->CurrElevator = NULL;
		break;
	}
}

void GoToElevator(NPCs *n) {
	if (n->PathStatus == 1) return;
	ElevatorObj *ce = n->CurrElevator;
	bbPointEntity(n->obj, ce->obj, 0);
	bbRotateEntity(n->Collider, 0, CurveAngle(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 20), 0, 0);
	float r = 280 * RoomScale;
	int inside = fabsf(bbEntityX(n->Collider, 0) - bbEntityX(ce->obj, 1)) < r && fabsf(bbEntityZ(n->Collider, 0) - bbEntityZ(ce->obj, 1)) < r
		&& fabsf(bbEntityY(n->Collider, 0) - bbEntityY(ce->obj, 1)) < r;
	float dist = bbEntityDistance(n->Collider, ce->door->frameobj);
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
		if (start < quit) { int tmp = bbInt(start); start = quit; quit = tmp; } // blitz swaps through an int temp
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
	bbSetAnimTime(n->obj, frame, 0);
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
	bbSetAnimTime(e, t, 0);
	return t;
}

// Main.bb: drop e onto the floor below (dir 0) or the ceiling above (dir 1) x,y,z
void TeleportEntity(int e, float x, float y, float z, float customradius, int isglobal, float pickrange, int dir) {
	int pvt = bbCreatePivot(0);
	bbPositionEntity(pvt, x, y + .05f, z, isglobal);
	bbRotateEntity(pvt, dir == 0 ? 90 : -90, 0, 0, 0);
	if (bbEntityPick(pvt, pickrange)) bbPositionEntity(e, x, bbPickedY() + customradius + (dir == 0 ? .02f : -.02f), z, isglobal);
	else bbPositionEntity(e, x, y, z, isglobal);
	bbFreeEntity(pvt);
	bbResetEntity(e);
}
