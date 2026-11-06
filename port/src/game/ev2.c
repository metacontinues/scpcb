// ev2.c: medium UpdateEvents.bb events (106's ambushes, sinkholes, 173 + the glass, the elevator death...) + QuickLoad
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return bbEntityX(o, 1); }
static float YG(int o) { return bbEntityY(o, 1); }
static float ZG(int o) { return bbEntityZ(o, 1); }
static float sgn(float v) { return v > 0 ? 1 : v < 0 ? -1 : 0; }

static int testroom(Events *e) { // 1048 scurries across, gas on approach
	if (!IN) return 0;
	int *o7 = &R->Objects[7];
	if (e->EventState == 0) {
		*o7 = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048.b3d", 0);
		bbScaleEntity(*o7, .05f, .05f, .05f, 0);
		bbTFormPoint(bbEntityX(Collider, 0), bbEntityY(Collider, 0), bbEntityZ(Collider, 0), 0, R->obj);
		int temp = bbTFormedZ() == 0 ? -1 : -(int)sgn(bbTFormedZ());
		bbTFormPoint(-720, 0, 816 * temp, R->obj, 0);
		bbPositionEntity(*o7, bbTFormedX(), 0, bbTFormedZ(), 0);
		bbRotateEntity(*o7, -90, R->angle - 90, 0, 0);
		bbSetAnimTime(*o7, 297, 0);
		e->EventState = 1;
	}
	if (bbEntityDistance(Collider, R->Objects[6]) < 2.5f && e->EventState > 0) {
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\079\\TestroomWarning.ogg"));
		for (int i = 0; i <= 5; i++) {
			int o = R->Objects[i];
			Emitters *em = CreateEmitter(XG(o), YG(o), ZG(o), 0);
			bbTurnEntity(em->Obj, 90, 0, 0, 1);
			em->RandAngle = 5; em->Speed = .042f; em->SizeChange = .0025f;
		}
		e->EventState = -e->EventState;
	}
	if (*o7) {
		Animate2(*o7, bbAnimTime(*o7), 284, 295, .3f, 1);
		bbMoveEntity(*o7, 0, -.008f * FPSfactor, 0);
		bbTFormPoint(bbEntityX(*o7, 0), bbEntityY(*o7, 0), bbEntityZ(*o7, 0), 0, R->obj);
		if (fabsf(bbTFormedX()) > 725) { bbFreeEntity(*o7); *o7 = 0; e->EventState *= 2; }
	}
	if (e->EventState == -2) RemoveEvent(e);
	return 0;
}

static int gateaentrance(Events *e) {
	if (!IN) return 0;
	Doors *d1 = R->RoomDoors[1];
	if (!RemoteDoorOn) { d1->locked = 1; return 0; }
	if (e->EventState3 == 0) {
		d1->locked = 0;
		if (d1->open && (d1->openstate > 50 || bbEntityDistance(Collider, d1->frameobj) < .5f)) {
			d1->openstate = Min(d1->openstate, 50);
			d1->open = 0;
			PlaySound2(LoadTempSound("SFX\\Door\\DoorError.ogg"), Camera, d1->frameobj, 10, 1);
		}
		return 0;
	}
	d1->locked = 0;
	Rooms *ga = NULL;
	each(Rooms, r) if (bbSeq(r->RoomTemplate->Name, "gatea")) { ga = r; break; }
	if (!bbLive(Curr096) || Curr096->State == 0 || Curr096->State == 5)
		e->EventState = UpdateElevators(e->EventState, R->RoomDoors[0], ga->RoomDoors[1], R->Objects[0], R->Objects[1], e, 1);
	// TODO else Update096ElevatorEvent (comes w/ 096)
	if (!Contained106 && e->EventState < -1.5f && e->EventState + FPSfactor >= -1.5f) PlaySound_Strict(OldManSFX[3]);
	if (bbEntityDistance(Collider, R->Objects[1]) < 4) { ga->RoomDoors[1]->locked = 1; PlayerRoom = ga; RemoveEvent(e); }
	return 0;
}

static int sinkhole(Events *e) { // 106's sinkhole drags you to the pocket dimension
	if (e->EventState == 0) {
		float rx = bbRnd(-.5f, .5f), rz = bbRnd(-.5f, .5f); int a = bbRand(360, 1); // x, z, then the Rand: codegen order
		Decals *de = CreateDecal(0, bbEntityX(R->obj, 0) + rx, .01f, bbEntityZ(R->obj, 0) + rz, 90, a, 0);
		if (de) { de->Size = 2.5f; bbScaleSprite(de->obj, de->Size, de->Size); }
		e->EventState = 1;
		return 0;
	}
	if (!IN) { e->EventState2 = 0; return 0; }
	if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Room\\Sinkhole.ogg");
	else e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, R->obj, 4.5f, 1.5f);
	float dist = Distance(bbEntityX(Collider, 0), bbEntityZ(Collider, 0), bbEntityX(R->obj, 0), bbEntityZ(R->obj, 0));
	if (dist >= 2) return 0;
	CurrStepSFX = 1;
	CurrSpeed = CurveValue(0, CurrSpeed, Max(dist * 50, 1));
	CrouchState = (2 - dist) / 2;
	if (dist >= .5f) return 0;
	if (e->EventState2 == 0) PlaySound_Strict(LoadTempSound("SFX\\Room\\SinkholeFall.ogg"));
	CurrSpeed = CurveValue(0, CurrSpeed, Max(dist * 50, 1));
	float x = CurveValue(bbEntityX(R->obj, 0), bbEntityX(Collider, 0), 10);
	float y = CurveValue(bbEntityY(R->obj, 0) - e->EventState2, bbEntityY(Collider, 0), 25);
	float z = CurveValue(bbEntityZ(R->obj, 0), bbEntityZ(Collider, 0), 10);
	bbPositionEntity(Collider, x, y, z, 1);
	DropSpeed = 0;
	bbResetEntity(Collider);
	e->EventState2 = Min(e->EventState2 + FPSfactor / 200, 2);
	LightBlink = Min(e->EventState2 * 5, 10);
	BlurTimer = e->EventState2 * 500;
	if (e->EventState2 == 2) MoveToPocketDimension();
	return 0;
}

static int room2cafeteria(Events *e) { // 294 takes two quarters
	if (IN && !Using294 && bbEntityDistance(R->Objects[0], Collider) < 1.5f) {
		GiveAchievement(Achv294, 1);
		if (bbEntityInView(R->Objects[0], Camera)) {
			DrawHandIcon = 1;
			if (MouseHit1) {
				int temp = 1; // the output slot's empty
				each(Items, it) if (!it->Picked && bbEntityX(it->collider, 0) - XG(R->Objects[1]) == 0 && bbEntityZ(it->collider, 0) - ZG(R->Objects[1]) == 0) { temp = 0; break; }
				int inserted = 0;
				if (e->EventState2 < 2 && SelectedItem && (bbSeq(SelectedItem->itemtemplate->tempname, "25ct") || bbSeq(SelectedItem->itemtemplate->tempname, "coin"))) {
					RemoveItem(SelectedItem); SelectedItem = NULL;
					e->EventState2++;
					PlaySound_Strict(LoadTempSound("SFX\\SCP\\294\\coin_drop.ogg"));
					inserted = 1;
				}
				if (e->EventState2 == 2) { Using294 = temp; if (Using294) MouseHit1 = 0; }
				else if (e->EventState2 == 1 && !inserted) { Using294 = 0; Msg = "You need to insert another Quarter in order to use this machine."; MsgTimer = 70 * 5; }
				else if (!inserted) { Using294 = 0; Msg = "You need to insert two Quarters in order to use this machine."; MsgTimer = 70 * 5; }
			}
		}
	}
	if (e->EventState == 0) { CreateNPC(NPCtype066, bbEntityX(R->obj, 0), .5f, bbEntityZ(R->obj, 0)); e->EventState = 1; }
	return 0;
}

static int testroom173(Events *e) { // 173 behind the glass, breaks through when you blink
	if (!IN || Curr173->Idle != 0) return 0;
	if (e->EventState == 0) {
		if (R->RoomDoors[0]->open) {
			bbPositionEntity(Curr173->Collider, XG(R->Objects[0]), .5f, ZG(R->Objects[0]), 0);
			bbResetEntity(Curr173->Collider);
			e->EventState = 1;
		}
		return 0;
	}
	int *g = &R->Objects[2];
	if (!*g) {
		int tex = LoadTexture_Strict("GFX\\map\\glass.png", 1 + 2);
		*g = bbCreateSprite(0);
		bbEntityTexture(*g, tex, 0, 0);
		bbSpriteViewMode(*g, 2);
		bbScaleSprite(*g, 182 * RS * .5f, 192 * RS * .5f);
		int pvt = bbCreatePivot(R->obj);
		bbPositionEntity(pvt, -632, 224, -208, 0);
		bbPositionEntity(*g, XG(pvt), YG(pvt), ZG(pvt), 0);
		bbFreeEntity(pvt);
		bbRotateEntity(*g, 0, R->angle, 0, 0);
		bbTurnEntity(*g, 0, 180, 0, 0);
		bbEntityParent(*g, R->obj, 1);
		bbFreeTexture(tex);
	}
	bbShowEntity(*g);
	e->EventState++; // 173's timer for breaking through
	float dist = bbEntityDistance(Collider, R->Objects[1]);
	if (dist < 1) e->EventState = Max(e->EventState, 70 * 12); // close: 173's ready
	else if (dist > 1.4f && e->EventState > 70 * 12 && BlinkTimer <= -10) { // stepped back and blinked: it attacks
		if (bbEntityDistance(Curr173->Collider, R->Objects[0]) > 5) { RemoveEvent(e); return 0; } // 173 wandered off
		PlaySound2(LoadTempSound("SFX\\General\\GlassBreak.ogg"), Camera, Curr173->obj, 10, 1);
		bbFreeEntity(*g); *g = 0;
		bbPositionEntity(Curr173->Collider, XG(R->Objects[1]), .5f, ZG(R->Objects[1]), 0);
		bbResetEntity(Curr173->Collider);
		RemoveEvent(e);
	}
	return 0;
}

static int room2elevator(Events *e) { // a guard walks into the elevator and dies
	NPCs *n = bbLive(R->NPC[0]);
	if (e->EventState == 0) {
		if (R->dist < 8 && R->dist > 0) {
			n = R->NPC[0] = CreateNPC(NPCtypeGuard, XG(R->obj), .5f, ZG(R->obj));
			bbPointEntity(n->Collider, Collider, 0);
			bbRotateEntity(n->Collider, 0, bbEntityYaw(n->Collider, 0), 0, 1);
			e->EventState = 1;
		}
		return 0;
	}
	if (e->EventState == 1) {
		if ((R->dist < 5) | (bbRand(700, 1) == 1)) { // blitz Or: Rand always rolls
			e->EventState = 2;
			n->State = 5;
			n->EnemyX = XG(R->Objects[1]); n->EnemyY = YG(R->Objects[1]); n->EnemyZ = ZG(R->Objects[1]);
		}
	} else if (e->EventState == 2) {
		if (bbEntityDistance(n->Collider, R->Objects[1]) < 2) {
			R->RoomDoors[0]->open = 0;
			PlaySound2(CloseDoorSFX[3][0], Camera, R->RoomDoors[0]->obj, 8, 1);
			PlaySound_Strict(LoadTempSound("SFX\\Room\\Room2ElevatorDeath.ogg"));
			e->EventState = 2.05f;
		}
	} else if (e->EventState < 13 * 70) {
		e->EventState += FPSfactor;
		float s = e->EventState;
		if (s > 6.7f * 70 && s < 7.4f * 70) CameraShake = 7.4f - s / 70;
		else if (s > 8.6f * 70 && s < 10.6f * 70) CameraShake = 10.6f - s / 70;
		else if (s > 12.6f * 70) {
			CameraShake = 0;
			if (s - FPSfactor < 12.6f * 70 && n) {
				RemoveNPC(n); R->NPC[0] = NULL;
				int o = R->Objects[0];
				CreateDecal(3, XG(o), .0005f, ZG(o), 90, bbRnd(360, 0), 0);
				Decals *de = CreateDecal(17, XG(o), .002f, ZG(o), 90, bbRnd(360, 0), 0);
				if (de) de->Size = .5f;
				o = R->Objects[1];
				de = CreateDecal(3, XG(o), YG(o), ZG(o), 0, R->angle + 270, 0);
				if (de) de->Size = .9f;
			}
			R->RoomDoors[0]->locked = 0;
		}
	} else if (R->RoomDoors[0]->open) { R->RoomDoors[0]->locked = 1; RemoveEvent(e); }
	return 0;
}

static int room1lifts(Events *e) {
	if (!IN) return 0;
	for (int i = 0; i <= 1; i++) {
		UpdateButton(R->Objects[i]);
		if (ClosestButton == R->Objects[i] && MouseHit1) {
			Msg = "The elevator appears to be broken.";
			PlaySound2(ButtonSFX2, Camera, R->Objects[i], 10, 1);
			MsgTimer = 5 * 70;
			MouseHit1 = 0;
		}
	}
	return 0;
}

static int victim106(Events *e) { // a class d falls out of the ceiling
	if (Contained106) return 0;
	if (IN && e->EventState == 0) {
		Decals *de = CreateDecal(0, bbEntityX(R->obj, 0), 799 * RS, bbEntityZ(R->obj, 0), -90, bbRand(360, 1), 0);
		if (de) {
			de->Size = .05f; de->SizeChange = .0015f; bbEntityAlpha(de->obj, .8f); UpdateDecals();
			PlaySound2(DecaySFX[3], Camera, de->obj, 15, 1);
		}
		e->EventState = 1;
	}
	if (e->EventState <= 0) return 0;
	if (!bbLive(R->NPC[0])) e->EventState += FPSfactor;
	if (e->EventState <= 200) return 0;
	if (!bbLive(R->NPC[0])) {
		NPCs *n = R->NPC[0] = CreateNPC(NPCtypeD, bbEntityX(R->obj, 0), 900 * RS, bbEntityZ(R->obj, 0));
		bbRotateEntity(n->Collider, 0, bbRnd(360, 0), 0, 1);
		ChangeNPCTextureID(n, 5);
		n->State = 6;
		PlaySound_Strict(HorrorSFX[0]);
		PlaySound2(DecaySFX[2], Camera, n->Collider, 15, 1);
	}
	NPCs *n = R->NPC[0];
	n->FallingPickDistance = 0;
	bbEntityType(n->Collider, HIT_PLAYER, 0);
	if (bbEntityY(n->Collider, 0) > .35f) {
		AnimateNPC(n, 1, 10, .12f, 0);
		float dist = bbEntityDistance(Collider, n->Collider);
		if (dist < .8f) { // push the player out from under it
			float d = point_direction(bbEntityX(Collider, 1), bbEntityZ(Collider, 1), bbEntityX(n->Collider, 1), bbEntityZ(n->Collider, 1));
			bbTranslateEntity(Collider, cosf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0, sinf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0);
		}
		if (bbEntityY(n->Collider, 0) > .6f) bbEntityType(n->Collider, 0, 0);
		return 0;
	}
	e->EventState += FPSfactor;
	AnimateNPC(n, 11, 19, .25f, 0);
	if (!e->Sound) {
		LoadEventSound(e, "SFX\\General\\BodyFall.ogg", 0);
		PlaySound_Strict(e->Sound);
		Decals *de = CreateDecal(0, bbEntityX(R->obj, 0), .001f, bbEntityZ(R->obj, 0), 90, bbRand(360, 1), 0);
		if (de) { de->Size = .4f; bbEntityAlpha(de->obj, .8f); UpdateDecals(); }
	}
	if (e->EventState > 400) { if (e->Sound) { FreeSound_Strict(e->Sound); e->Sound = 0; } RemoveEvent(e); }
	return 0;
}

static void pipesdecal(Events *e, int o, float yaw) {
	Decals *d = CreateDecal(0, XG(o), YG(o), ZG(o), 0, yaw, bbRnd(360, 0));
	if (!d) return;
	d->Timer = 90000; d->Alpha = .01f; d->AlphaChange = .005f; d->Size = .1f; d->SizeChange = .003f;
}

static int pipes106(Events *e) { // 106 walks across the pipes
	if (Contained106) return 0;
	if (e->EventState == 0) { if (IN) e->EventState = 1; return 0; }
	e->EventState += FPSfactor * .7f;
	NPCs *n = Curr106;
	int o0 = R->Objects[0], o1 = R->Objects[1];
	float s = e->EventState;
	if (s < 50) { // walk to the middle
		n->Idle = 1;
		bbPositionEntity(n->Collider, XG(o0), bbEntityY(Collider, 0) - .15f, ZG(o0), 0);
		bbPointEntity(n->Collider, o1, 0);
		bbMoveEntity(n->Collider, 0, 0, bbEntityDistance(o0, o1) * .5f * (s / 50));
		AnimateNPC(n, 284, 333, .02f * 35, 1);
	} else if (s < 200) { // look around
		n->Idle = 1;
		AnimateNPC(n, 334, 494, .2f, 1);
		bbPositionEntity(n->Collider, (XG(o0) + XG(o1)) / 2, bbEntityY(Collider, 0) - .15f, (ZG(o0) + ZG(o1)) / 2, 0);
		bbRotateEntity(n->Collider, 0, CurveValue(s, bbEntityYaw(n->Collider, 0), 30), 0, 1);
		if (bbEntityDistance(n->Collider, Collider) < 4) {
			int pvt = bbCreatePivot(0);
			bbPositionEntity(pvt, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0), bbEntityZ(n->Collider, 0), 0);
			bbPointEntity(pvt, Collider, 0);
			if (WrapAngle(bbEntityYaw(pvt, 0) - bbEntityYaw(n->Collider, 0)) < 80) { // it saw you
				n->State = -11; n->Idle = 0;
				PlaySound_Strict(HorrorSFX[10]);
				e->EventState = 260;
			}
			bbFreeEntity(pvt);
		}
	} else if (s < 250) { // leave
		n->Idle = 1;
		bbPositionEntity(n->Collider, XG(o0), bbEntityY(Collider, 0) - .15f, ZG(o0), 0);
		bbPointEntity(n->Collider, o1, 0);
		bbMoveEntity(n->Collider, 0, 0, bbEntityDistance(o0, o1) * ((s - 150) / 100));
		AnimateNPC(n, 284, 333, .02f * 35, 1);
	}
	bbResetEntity(n->Collider);
	bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .15f, bbEntityZ(n->Collider, 0), 0);
	bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0), 0, 0);
	s = e->EventState;
	float prev = s - FPSfactor * .7f;
	if (s / 250 > .3f && prev / 250 <= .3f) {
		e->SoundCHN = PlaySound_Strict(HorrorSFX[6]);
		BlurTimer = 800;
		pipesdecal(e, R->Objects[2], R->angle - 90);
	}
	if (s / 250 > .65f && prev / 250 <= .65f) pipesdecal(e, R->Objects[3], R->angle + 90);
	if (s > 250) { n->Idle = 0; RemoveEvent(e); }
	return 0;
}

static int closets(Events *e) { // 2 class ds hiding in the closet. their setup is QuickLoad's (room2closets below)
	if (e->EventState == 0) {
		if (IN && Curr173->Idle < 2 && !bbLen(e->EventStr) && QuickLoadPercent == -1) { QuickLoadPercent = 0; QuickLoad_CurrEvent = e; e->EventStr = "load0"; }
		return 0;
	}
	NPCs *n0 = R->NPC[0], *n1 = R->NPC[1];
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s < 70 * 3.5f) {
		bbRotateEntity(n1->Collider, 0, CurveAngle(R->angle + 90, bbEntityYaw(n1->Collider, 0), 100), 0, 1);
		n0->State = 1;
		if (s > 70 * 3.2f && s - FPSfactor <= 70 * 3.2f) PlaySound2(IntroSFX[15], Camera, R->obj, 15, 1);
	} else if (s < 70 * 6.5f) {
		if (s - FPSfactor < 70 * 3.5f) { n0->State = 0; n1->SoundChn = PlaySound2(n1->Sound, Camera, n1->Collider, 12, 1); }
		if (s > 70 * 4.5f) {
			bbPointEntity(n0->obj, R->obj, 0);
			bbRotateEntity(n0->Collider, 0, CurveAngle(bbEntityYaw(n0->obj, 0), bbEntityYaw(n0->Collider, 0), 30), 0, 1);
		}
		bbPointEntity(n1->obj, R->obj, 0);
		bbTurnEntity(n1->obj, 0, sinf(s * DTOR) * 25, 0, 0);
		bbRotateEntity(n1->Collider, 0, CurveAngle(bbEntityYaw(n1->obj, 0), bbEntityYaw(n1->Collider, 0), 30), 0, 1);
	} else {
		if (s - FPSfactor < 70 * 6.5f) { PlaySound_Strict(HorrorSFX[0]); PlaySound_Strict(LightSFX); }
		BlinkTimer = Max((70 * 6.5f - s) / 5 - bbRnd(0, 2), -10);
		if (BlinkTimer == -10) {
			if (s > 70 * 7.5f && s - FPSfactor <= 70 * 7.5f) {
				PlaySound2(NeckSnapSFX[0], Camera, n0->Collider, 8, 1);
				// the wallet w/ 2 quarters
				Items *it = CreateItem("Wallet", "wallet", bbEntityX(n0->Collider, 1), bbEntityY(n0->Collider, 1), bbEntityZ(n0->Collider, 1), 0, 0, 0, 1, 0);
				bbEntityType(it->collider, HIT_ITEM, 0);
				bbPointEntity(it->collider, n1->Collider, 0);
				bbMoveEntity(it->collider, -.4f, 0, -.2f);
				TeleportEntity(it->collider, bbEntityX(it->collider, 0), bbEntityY(it->collider, 0), bbEntityZ(it->collider, 0), -.02f, 1, 10, 0);
				for (int i = 0; i <= 1; i++) {
					Items *q = CreateItem("Quarter", "25ct", 1, 1, 1, 0, 0, 0, 1, 0);
					q->Picked = 1; q->Dropped = -1; q->itemtemplate->found = 1;
					it->SecondInv[i] = q;
					bbHideEntity(q->collider);
					bbEntityType(q->collider, HIT_ITEM, 0);
				}
			}
			if (s > 70 * 8 && s - FPSfactor <= 70 * 8) PlaySound2(NeckSnapSFX[1], Camera, n1->Collider, 8, 1);
			SetNPCFrame(n0, 60); n0->State = 8;
			SetNPCFrame(n1, 19); n1->State = 6;
		}
		if (s > 70 * 8.5f) {
			int o0 = R->Objects[0], o1 = R->Objects[1];
			bbPositionEntity(Curr173->Collider, (XG(o0) + XG(o1)) / 2, YG(o0), (ZG(o0) + ZG(o1)) / 2, 0);
			bbPointEntity(Curr173->Collider, Collider, 0);
			bbResetEntity(Curr173->Collider);
			RemoveEvent(e);
		}
	}
	return 0;
}

static int tunnel106(Events *e) { // 106 rises out of the tunnel floor
	NPCs *n = Curr106;
	if (e->EventState == 0) {
		if (R->dist < 5 && R->dist > 0) {
			if (n->State >= 0) e->EventState = 1;
			else if (n->State <= -10 && bbEntityDistance(n->Collider, Collider) > 5 && !bbEntityInView(n->obj, Camera)) { e->EventState = 1; e->EventState2 = 1; }
		} else if (Contained106) RemoveEvent(e);
		return 0;
	}
	if (e->EventState == 1) {
		if ((R->dist < 3) | (bbRand(7000, 1) == 1)) { // blitz Or: Rand always rolls
			e->EventState = 2;
			Decals *d = CreateDecal(0, bbEntityX(R->obj, 0), 445 * RS, bbEntityZ(R->obj, 0), -90, bbRand(360, 1), 0);
			if (d) {
				d->Size = bbRnd(.5f, .7f); bbEntityAlpha(d->obj, .7f); d->ID = 1; bbScaleSprite(d->obj, d->Size, d->Size);
				bbEntityAlpha(d->obj, bbRnd(.7f, .85f));
			}
			PlaySound_Strict(HorrorSFX[10]);
		} else if (R->dist > 8) {
			if (bbRand(5, 1) != 1) n->State = -10000;
			n->Idle = 0;
			RemoveEvent(e);
		}
		return 0;
	}
	if (e->EventState2 == 1) ShouldPlay = 10;
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s <= 180) {
		bbPositionEntity(n->Collider, XG(R->obj), bbEntityY(Collider, 0) + 1 - Min(sinf(s * DTOR) * 1.5f, 1.1f), ZG(R->obj), 1);
		bbPointEntity(n->Collider, Camera, 0);
		AnimateNPC(n, 55, 104, .1f, 1);
		n->Idle = 1; n->State = 1;
		bbResetEntity(n->Collider);
		n->DropSpeed = 0;
		bbPositionEntity(n->obj, bbEntityX(n->Collider, 0), bbEntityY(n->Collider, 0) - .15f, bbEntityZ(n->Collider, 0), 0);
		bbRotateEntity(n->obj, 0, bbEntityYaw(n->Collider, 0), 0, 0);
		bbShowEntity(n->obj);
	} else if (s < 300) {
		n->Idle = 0; n->State = -10;
		bbPositionEntity(n->Collider, XG(R->obj), -3, ZG(R->obj), 1);
		n->PathTimer = 70 * 10; n->PathStatus = 0; n->PathLocation = 0;
		Decals *de = CreateDecal(0, XG(R->obj), .01f, ZG(R->obj), 90, bbRand(360, 1), 0);
		if (de) { de->Size = .05f; de->SizeChange = .01f; bbEntityAlpha(de->obj, .8f); UpdateDecals(); }
		e->EventState = 300;
	} else if (s < 800) {
		if (bbEntityY(n->Collider, 0) >= bbEntityY(Collider, 0) - .05f) RemoveEvent(e);
		else {
			bbTranslateEntity(n->Collider, 0, (bbEntityY(Collider, 1) - .11f - bbEntityY(n->Collider, 0)) / 50, 0, 0);
			if (bbEntityY(n->Collider, 0) < -.1f) n->CurrSpeed = 0;
		}
	} else RemoveEvent(e);
	return 0;
}

const EvEntry Evs2[] = {
	{"testroom", testroom}, {"gateaentrance", gateaentrance}, {"106sinkhole", sinkhole}, {"room2cafeteria", room2cafeteria},
	{"testroom173", testroom173}, {"room2elevator", room2elevator}, {"room1lifts", room1lifts}, {"106victim", victim106},
	{"room2pipes106", pipes106}, {"room2closets", closets}, {"tunnel106", tunnel106}, {NULL, NULL},
};

// ---- Menu.bb's DrawQuickLoading + Main.bb's QuickLoadEvents: events that load their stuff over a few frames

static void ql_closets(Events *e) {
	if (e->EventState != 0) return;
	str s = e->EventStr;
	if (bbSeq(s, "load0")) {
		QuickLoadPercent = 10;
		if (!bbLive(R->NPC[0])) R->NPC[0] = CreateNPC(NPCtypeD, XG(R->Objects[0]), YG(R->Objects[0]), ZG(R->Objects[0]));
		ChangeNPCTextureID(R->NPC[0], 4);
		e->EventStr = "load1";
	} else if (bbSeq(s, "load1")) { QuickLoadPercent = 20; R->NPC[0]->Sound = LoadSound_Strict("SFX\\Room\\Storeroom\\Escape1.ogg"); e->EventStr = "load2"; }
	else if (bbSeq(s, "load2")) { QuickLoadPercent = 35; R->NPC[0]->SoundChn = PlaySound2(R->NPC[0]->Sound, Camera, R->NPC[0]->Collider, 12, 1); e->EventStr = "load3"; }
	else if (bbSeq(s, "load3")) {
		QuickLoadPercent = 55;
		if (!bbLive(R->NPC[1])) R->NPC[1] = CreateNPC(NPCtypeD, XG(R->Objects[1]), YG(R->Objects[1]), ZG(R->Objects[1]));
		ChangeNPCTextureID(R->NPC[1], 2);
		e->EventStr = "load4";
	} else if (bbSeq(s, "load4")) { QuickLoadPercent = 80; R->NPC[1]->Sound = LoadSound_Strict("SFX\\Room\\Storeroom\\Escape2.ogg"); e->EventStr = "load5"; }
	else if (bbSeq(s, "load5")) {
		QuickLoadPercent = 100;
		bbPointEntity(R->NPC[0]->Collider, R->NPC[1]->Collider, 0);
		bbPointEntity(R->NPC[1]->Collider, R->NPC[0]->Collider, 0);
		e->EventState = 1;
	}
}

static void QuickLoadEvents(void) {
	Events *e = bbLive(QuickLoad_CurrEvent);
	if (!e) { QuickLoadPercent = -1; return; }
	if (bbSeq(e->EventName, "room2closets")) ql_closets(e);
	// TODO room2sl, room3storage, room049, room205, room860, room966, dimension1499 (they come w/ their events)
}

void DrawQuickLoading(void) {
	if (QuickLoadPercent <= -1) { QuickLoadPercent = -1; QuickLoadPercent_DisplayTimer = 0; QuickLoad_CurrEvent = NULL; return; }
	bbMidHandle(QuickLoadIcon);
	bbDrawImage(QuickLoadIcon, GraphicWidth - 90, GraphicHeight - 150, 0);
	bbColor(255, 255, 255);
	AASetFont(Font1);
	AAText(GraphicWidth - 100, GraphicHeight - 90, cat("LOADING: ", bbStr(QuickLoadPercent), "%"), 1, 0);
	if (QuickLoadPercent > 99) {
		if (QuickLoadPercent_DisplayTimer < 70) QuickLoadPercent_DisplayTimer = Min(QuickLoadPercent_DisplayTimer + FPSfactor, 70);
		else QuickLoadPercent = -1;
	}
	QuickLoadEvents();
}
