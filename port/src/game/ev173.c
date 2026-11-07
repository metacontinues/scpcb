// ev173.c: the intro ("173": cell, escort, 173's chamber) and "alarm" (the start room after the breach). UpdateEvents.bb
// event fns return 1 to stop the event loop for this frame
#include "g.h"

#define R (e->room)
#define N(i) bbLive(R->NPC[i])
#define RS RoomScale
static float X(int o) { return bbEntityX(o, 0); }
static float Y(int o) { return bbEntityY(o, 0); }
static float Z(int o) { return bbEntityZ(o, 0); }
static float XG(int o) { return bbEntityX(o, 1); }
static float ZG(int o) { return bbEntityZ(o, 1); }
static float fl(void) { return FPSfactor; }

// swap an npc's voice line
static void say(NPCs *n, const char *file) {
	FreeSound_Strict(n->Sound);
	n->Sound = LoadSound_Strict(file);
	n->SoundChn = PlaySound2(n->Sound, Camera, n->Collider, 10, 1);
}
static void opendoor(Doors *d) { d->locked = 0; UseDoor(d, 0, 1); d->locked = 1; } // the locked-door toggle the intro does a lot
static void face(NPCs *n, int t, float smooth) {
	bbPointEntity(n->obj, t, 0);
	bbRotateEntity(n->Collider, 0, CurveValue(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), smooth), 0, 1);
}

// the 2 guards + class d that guard the player's escape
static void spot5(Events *e) {
	NPCs *n3 = N(3), *n4 = N(4), *n5 = N(5);
	if (n5->State == 11) return;
	// both escorts are away from him and you're close: he stops you
	if (bbEntityDistance(n3->Collider, n5->Collider) > 5 && bbEntityDistance(n4->Collider, n5->Collider) > 5 && bbEntityDistance(n5->Collider, Collider) < 3.5f) {
		n5->State = 11; n5->State3 = 1;
		n5->SoundChn2 = PlaySound2(n5->Sound2, Camera, n5->Collider, 10, 1);
		n5->Reload = 70 * 3;
	}
}

// the PA message the intercom reads out while you're escorted
static str pamsg(void) {
	if (bbRand(3, 1) == 1) return cat("scripted\\scripted", bbStr(bbRand(1, 5)), ".ogg|off.ogg|");
	str s = cat("1\\attention", bbStr(bbRand(1, 2)), ".ogg"), who = "";
	switch (bbRand(3, 1)) {
	case 1: who = "crew"; s = cat(s, "|2\\crew", bbStr(bbRand(0, 5)), ".ogg"); break;
	case 2: who = "scientist"; s = cat(s, "|2\\scientist", bbStr(bbRand(0, 19)), ".ogg"); break;
	case 3: who = "security"; s = cat(s, "|2\\security", bbStr(bbRand(0, 5)), ".ogg"); break;
	}
	if (bbSeq(who, "scientist") && bbRand(2, 1) == 1) { // call on line
		s = cat(s, "|3\\callonline.ogg");
		s = cat(s, "|numbers\\", bbStr(bbRand(1, 9)), ".ogg");
		if (bbRand(2, 1) == 1) s = cat(s, "|numbers\\", bbStr(bbRand(1, 9)), ".ogg");
	} else {
		s = cat(s, "|3\\report", bbStr(bbRand(0, 1)), ".ogg");
		if (bbSeq(who, "crew")) {
			s = cat(s, "|4\\crew", bbStr(bbRand(0, 6)), ".ogg");
			if (bbRand(2, 1) == 1) s = cat(s, "|5\\crew", bbStr(bbRand(0, 6)), ".ogg");
		} else if (bbSeq(who, "scientist")) {
			s = cat(s, "|4\\scientist", bbStr(bbRand(0, 7)), ".ogg");
			if (bbRand(2, 1) == 1) s = cat(s, "|5\\scientist0.ogg");
		} else if (bbSeq(who, "security")) {
			s = cat(s, "|4\\security", bbStr(bbRand(0, 5)), ".ogg");
			if (bbRand(2, 1) == 1) s = cat(s, "|5\\security", bbStr(bbRand(1, 2)), ".ogg");
		}
	}
	return cat(s, "|off.ogg|");
}

// ---- the walk from the cell to 173's chamber (EventState3 > 0)

static void wakeup(Events *e) { // waking up in the cell (cell() places the camera right after)
	if (e->EventState3 == 1) {
		bbHideEntity(Collider);
		bbRotateEntity(Camera, -70, 0, 0, 0);
		CurrMusicVolume = MusicVolume;
		StopStream_Strict(MusicCHN);
		MusicCHN = StreamSound_Strict(cat("SFX\\Music\\", Music[13], ".ogg"), CurrMusicVolume, FMOD_MODE);
		NowPlaying = ShouldPlay;
		PlaySound_Strict(IntroSFX[11]);
		BlurTimer = 500;
		bbShowEntity(Light); bbEntityAlpha(Light, .5f);
	}
	if (e->EventState3 < 3) e->EventState3 += fl() / 100;
	else if (e->EventState3 < 15 || e->EventState3 >= 50) e->EventState3 += fl() / 30;
}

static int cell(Events *e) {
	float x, y, z, s3;
	wakeup(e);
	s3 = e->EventState3;
	if (s3 < 15) {
		x = X(R->obj) - (3224.0f + 1024) * RS; y = 136 * RS; z = Z(R->obj) + 8 * RS;
		if (s3 < 14) { // getting up, scripted camera
			mouse_x_speed_1 = mouse_y_speed_1 = 0;
			if (s3 - fl() / 30 < 12 && s3 > 12) PlaySound2(StepSFX[0][0][0], Camera, Collider, 8, .3f);
			bbShowEntity(Light); bbEntityAlpha(Light, .9f - s3 / 2);
			x += (X(R->obj) - (3048.0f + 1024) * RS - x) * Max((s3 - 10) / 4, 0);
			if (s3 < 10) y += .2f * Min(Max((s3 - 3) / 5, 0), 1);
			else y = (y + .2f) + (.302f + .6f - (y + .2f)) * Max((s3 - 10) / 4, 0);
			z += (Z(R->obj) + 104 * RS - z) * Min(Max((s3 - 3) / 5, 0), 1);
			bbRotateEntity(Camera, -70 + 70 * Min(Max((s3 - 3) / 5, 0), 1) + sinf(s3 * 12.857f * DTOR) * 5, -60 * Max((s3 - 10) / 4, 0), sinf(s3 * 25.7f * DTOR) * 8, 0);
			bbPositionEntity(Camera, x, y, z, 0);
			bbHideEntity(Collider);
			bbPositionEntity(Collider, x, .302f, z, 0);
			DropSpeed = 0;
		} else {
			bbHideEntity(Light);
			bbPositionEntity(Collider, X(Collider), .302f, Z(Collider), 0);
			bbResetEntity(Collider); bbShowEntity(Collider);
			DropSpeed = 0;
			e->EventState3 = 15;
			Msg = "Pick up the paper on the desk."; MsgTimer = 70 * 7;
		}
		user_camera_pitch = 0;
		bbRotateEntity(Collider, 0, bbEntityYaw(Camera, 0), 0, 0);
	} else if (s3 < 40 && Inventory[0]) {
		Msg = cat("Press ", KeyName[KEY_INV], " to open the inventory."); MsgTimer = 70 * 7;
		e->EventState3 = 40;
		return 1;
	}
	if (SelectedItem) e->EventState3 += fl() / 5;
	return 0;
}

static void gas(Events *e) { // the cell door shuts and the gas comes on
	for (int k = 0; k < 2; k++) {
		Emitters *em = CreateEmitter(PlayerRoom->x - ((k ? 3168.0f : 2976.0f) + 1024) * RS, 373 * RS, PlayerRoom->z + 204 * RS, 0);
		bbTurnEntity(em->Obj, 90, 0, 0, 1);
		em->RandAngle = 7; em->Speed = .03f; em->SizeChange = .003f; em->Room = PlayerRoom;
	}
}

static void exitcell(Events *e) { // 150..700: ulgrin tells you to come out
	NPCs *n3 = N(3), *n4 = N(4);
	if (n3->State == 7) {
		if (!n3->Sound2) {
			n3->Sound2 = LoadSound_Strict("SFX\\Room\\Intro\\Guard\\Ulgrin\\BeforeDoorOpen.ogg");
			n3->SoundChn2 = PlaySound2(n3->Sound2, Camera, n3->Collider, 10, 1);
		}
		UpdateSoundOrigin(n3->SoundChn2, Camera, n3->Collider, 10, 1);
		if (!bbChannelPlaying(n3->SoundChn2)) {
			n3->Sound = LoadSound_Strict("SFX\\Room\\Intro\\Guard\\Ulgrin\\ExitCell.ogg");
			n3->SoundChn = PlaySound2(n3->Sound, Camera, n3->Collider, 10, 1);
			n3->State = n4->State = N(5)->State = 9;
			opendoor(R->RoomDoors[6]);
		}
		return;
	}
	FreeSound_Strict(n3->Sound2);
	e->EventState3 = Min(e->EventState3 + fl() / 4, 699);
	float s3 = e->EventState3;
	if (Distance(X(Collider), Z(Collider), PlayerRoom->x - (3072 + 1024) * RS, PlayerRoom->z + 192 * RS) > 1.5f) { // out of the cell
		if (s3 > 250) {
			if (n3->SoundChn && bbChannelPlaying(n3->SoundChn)) bbStopChannel(n3->SoundChn);
			say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\Escort%d.ogg", bbRand(1, 2)));
			n3->PathStatus = FindPath(n3, PlayerRoom->x - 320 * RS, .3f, PlayerRoom->z - 704 * RS);
			n4->PathStatus = FindPath(n4, PlayerRoom->x - 320 * RS, .3f, PlayerRoom->z - 704 * RS);
			e->EventState3 = 710;
		}
		return;
	}
	n3->State = 9;
	if (s3 - fl() / 4 < 350 && s3 >= 350) say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\ExitCellRefuse%d.ogg", bbRand(1, 2)));
	else if (s3 - fl() / 4 < 550 && s3 >= 550) say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\CellGas%d.ogg", bbRand(1, 2)));
	else if (s3 > 630) {
		bbPositionEntity(Collider, X(Collider), Y(Collider), Min(Z(Collider), ZG(R->obj) + 490 * RS), 0);
		if (R->RoomDoors[6]->open) { opendoor(R->RoomDoors[6]); gas(e); }
		EyeIrritation = Max(EyeIrritation + fl() * 4, 1);
	}
}

static void escortnags(Events *e, NPCs *n3, NPCs *n4) { // you're lagging behind: refuse, pissed off, kill
	n3->State3 = Max(n3->State3 + fl(), 50);
	if (n3->State3 >= 70 * 8 && n3->State3 - fl() < 70 * 8 && n3->State == 7) {
		if (n4->SoundChn && bbChannelPlaying(n4->SoundChn)) bbStopChannel(n4->SoundChn);
		if (n3->State2 < 2) { say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortRefuse%d.ogg", bbRand(1, 2))); n3->State3 = 50; n3->State2 = 3; }
		else if (n3->State2 == 3) { say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortPissedOff%d.ogg", bbRand(1, 2))); n3->State3 = 50; n3->State2 = 4; }
		else if (n3->State2 == 4) { say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortKill%d.ogg", bbRand(1, 2))); n3->State3 = 50 + 70 * 2.5f; n3->State2 = 5; }
		else if (n3->State2 == 5) {
			for (int i = 3; i <= 5; i++) { N(i)->State = 11; N(i)->State3 = 1; }
		}
	}
	spot5(e);
}

static void escortguards(Events *e, NPCs *n3, NPCs *n4, float dist) { // ulgrin + the other guard keep you walking
	if (dist < Min(Max(4 - n3->State3 * .05f, 1.5f), 4)) {
		if (n3->PathStatus != 1) {
			n3->State = 7;
			face(n3, Collider, 20);
			if (n3->PathStatus == 2) {
				n3->PathStatus = FindPath(n3, PlayerRoom->x - 320 * RS, .3f, PlayerRoom->z - 704 * RS);
				n4->PathStatus = FindPath(n4, PlayerRoom->x - 320 * RS, .3f, PlayerRoom->z - 704 * RS);
				n3->State = 3;
			}
		} else n3->State = 3;
	} else {
		n3->State = 7;
		face(n3, Collider, 20);
		if (dist > 5.5f) {
			n3->PathStatus = 2;
			if (n3->State2 == 0) {
				say(n3, "SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortRun.ogg");
				PlaySound2(e->Sound, Camera, n3->Collider, 10, 1);
				n3->State2 = 1;
			}
			n3->State = 5;
			n3->EnemyX = X(Collider); n3->EnemyY = Y(Collider); n3->EnemyZ = Z(Collider);
		}
	}
	float d4 = bbEntityDistance(Collider, n4->Collider);
	if (d4 > 1.5f && bbEntityDistance(n3->Collider, Collider) < bbEntityDistance(n3->Collider, n4->Collider)) n4->State = 3;
	else { n4->State = 5; n4->EnemyX = X(Collider); n4->EnemyY = Y(Collider); n4->EnemyZ = Z(Collider); }
}

static void arrive(Events *e, NPCs *n3, NPCs *n4) { // at the chamber's door: set up the 173 test
	NPCs *g = R->NPC[0] = CreateNPC(NPCtypeGuard, XG(R->Objects[0]), bbEntityY(R->Objects[0], 1), ZG(R->Objects[0]));
	g->Angle = 180;
	R->NPC[1] = CreateNPC(NPCtypeD, XG(R->Objects[1]), .5f, ZG(R->Objects[1]));
	bbPointEntity(R->NPC[1]->Collider, R->Objects[5], 0);
	R->NPC[2] = CreateNPC(NPCtypeD, XG(R->Objects[2]), .5f, ZG(R->Objects[2]));
	bbPointEntity(R->NPC[2]->Collider, R->Objects[5], 0);
	ChangeNPCTextureID(R->NPC[2], 6);
	n3->State = 9;
	NPCs *n7 = N(7);
	if (n7->SoundChn && bbChannelPlaying(n7->SoundChn)) { bbStopChannel(n7->SoundChn); FreeSound_Strict(n7->Sound); n7->Sound = 0; }
	bbFreeEntity(R->Objects[9]); R->Objects[9] = 0;
	bbFreeEntity(R->Objects[10]); R->Objects[10] = 0;
	if (N(5)) RemoveNPC(N(5));
	for (int i = 8; i <= 10; i++) if (N(i)) RemoveNPC(N(i));
	say(n3, sf("SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortDone%d.ogg", bbRand(1, 5)));
	NPCs *n6 = N(6);
	bbPositionEntity(n6->Collider, XG(R->obj) - 1190 * RS, 450 * RS, ZG(R->obj) + 456 * RS, 1);
	bbResetEntity(n6->Collider);
	bbPointEntity(n6->Collider, R->obj, 0);
	n6->CurrSpeed = 0; n6->State = 0;
	e->EventState3 = 905;
	opendoor(R->RoomDoors[3]);
	n4->State = 9;
}

static void escort(Events *e) { // 710..900: the walk
	NPCs *n3 = N(3), *n4 = N(4);
	n4->Angle = 0;
	if (X(Collider) < XG(R->obj) - 5376 * RS && !bbLen(e->EventStr)) e->EventStr = pamsg();
	NPCs *n6 = N(6);
	if (n6) { // the scientist at the door
		if (n6->State == 0) {
			if (R->RoomDoors[7]->open && Distance(X(Collider), Z(Collider), XG(R->obj) - 3328 * RS, ZG(R->obj) - 1232 * RS) < 5) {
				n6->State = 1;
				if (bbSeq(e->EventStr, "done")) PlaySound_Strict(LoadTempSound(sf("SFX\\Room\\Intro\\PA\\scripted\\announcement%d.ogg", bbRand(1, 7))));
			}
		} else if (Z(n6->Collider) > ZG(R->obj) - 64 * RS) {
			bbRotateEntity(n6->Collider, 0, CurveAngle(90, bbEntityYaw(n6->Collider, 0), 15), 0, 0);
			if (R->RoomDoors[7]->open) UseDoor(R->RoomDoors[7], 0, 1);
			if (R->RoomDoors[7]->openstate < 1) n6->State = 0;
		}
	}
	NPCs *n8 = N(8);
	if (n8) { // the 2 guards walking a class d
		if (n8->State == 7) {
			if (Distance(X(Collider), Z(Collider), XG(R->obj) - 6688 * RS, ZG(R->obj) - 1252 * RS) < 2.5f) { n8->State = 10; N(9)->State = 1; N(10)->State = 10; }
		} else if (X(n8->Collider) < XG(R->obj) - 7100 * RS) for (int i = 8; i <= 10; i++) N(i)->State = 0;
	}
	NPCs *n5 = N(5);
	n5->SoundChn = LoopSound2(n5->Sound, n5->SoundChn, Camera, n5->obj, 2, .5f);
	if (bbLen(e->EventStr) && !bbSeq(e->EventStr, "done")) { // the PA, one clip at a time
		if (!e->SoundCHN) e->SoundCHN = PlaySound_Strict(LoadTempSound("SFX\\Room\\Intro\\PA\\on.ogg"));
		if (!bbChannelPlaying(e->SoundCHN)) {
			str clip = bbLeft(e->EventStr, bbInstr(e->EventStr, "|", 1) - 1);
			e->SoundCHN = PlaySound_Strict(LoadTempSound(cat("SFX\\Room\\Intro\\PA\\", clip)));
			e->EventStr = bbRight(e->EventStr, bbLen(e->EventStr) - bbLen(clip) - 1);
			if (!bbLen(e->EventStr)) {
				FreeSound_Strict(n3->Sound);
				int t = bbRand(1, 5);
				n3->Sound = LoadSound_Strict(sf("SFX\\Room\\Intro\\Guard\\Conversation%da.ogg", t));
				n3->SoundChn = PlaySound2(n3->Sound, Camera, n3->Collider, 10, 1);
				n4->Sound = LoadSound_Strict(sf("SFX\\Room\\Intro\\Guard\\Conversation%db.ogg", t));
				n4->SoundChn = PlaySound2(n4->Sound, Camera, n4->Collider, 10, 1);
				e->EventStr = "done";
			}
		}
	}
	float dist = Distance(X(Collider), Z(Collider), X(n3->Collider), Z(n3->Collider));
	if (dist < 3) n3->State3 = Min(Max(n3->State3 - fl(), 0), 50);
	else escortnags(e, n3, n4);
	if (n5->State == 11) UpdateSoundOrigin(n5->SoundChn2, Camera, n5->Collider, 10, 1);
	if (n3->State != 11) escortguards(e, n3, n4, dist);
	Doors *d2 = R->RoomDoors[2];
	dist = Distance(X(Collider), Z(Collider), XG(d2->frameobj), ZG(d2->frameobj));
	if (Distance(X(n3->Collider), Z(n3->Collider), XG(d2->frameobj), ZG(d2->frameobj)) < 4.5f && dist < 5) arrive(e, n3, n4);
}

static void document(Events *e) { // 905: "oh and by the way", ulgrin hands you 173's document
	NPCs *n3 = N(3);
	if (!bbChannelPlaying(n3->SoundChn) && n3->Frame < 358) {
		n3->State = 8;
		say(n3, "SFX\\Room\\Intro\\Guard\\Ulgrin\\OhAndByTheWay.ogg");
		SetNPCFrame(n3, 358);
	} else if (n3->Frame >= 358) {
		bbPointEntity(n3->Collider, Collider, 0);
		bbRotateEntity(n3->Collider, 0, bbEntityYaw(n3->Collider, 0), 0, 0);
		if (n3->Frame <= 481.5f) AnimateNPC(n3, 358, 482, .4f, 0);
		else {
			AnimateNPC(n3, 483, 607, .2f, 1);
			if (bbEntityDistance(Collider, n3->Collider) < 1.5f && bbEntityInView(n3->obj, Camera)) {
				DrawHandIcon = 1;
				if (MouseHit1) {
					SelectedItem = CreateItem("Document SCP-173", "paper", 0, 0, 0, 0, 0, 0, 1, 0);
					bbEntityType(SelectedItem->collider, HIT_ITEM, 0);
					PickItem(SelectedItem);
					opendoor(R->RoomDoors[2]);
					e->EventState3 = 910;
					SetNPCFrame(n3, 608);
				}
			}
		}
	}
}

static int walk(Events *e) {
	NPCs *n3 = N(3);
	ShouldPlay = 13;
	CurrSpeed = Min(CurrSpeed - CurrSpeed * (.008f / bbEntityDistance(n3->Collider, Collider)) * fl(), CurrSpeed); // match the guards' pace
	float s3 = e->EventState3;
	if (s3 < 170) { if (cell(e)) return 1; }
	else if (s3 >= 150 && s3 < 700) exitcell(e);
	else if (s3 < 800) { e->EventState3 += fl() / 4; spot5(e); }
	else if (s3 < 900) escort(e);
	else if (s3 <= 905) document(e);
	else if (n3->Frame <= 620.5f && n3->State == 8) AnimateNPC(n3, 608, 621, .4f, 0);
	else {
		n3->Angle = bbEntityYaw(n3->Collider, 0);
		n3->State = N(4)->State = 9;
		if (Distance(X(Collider), Z(Collider), X(R->obj), Z(R->obj)) < 4) {
			opendoor(R->RoomDoors[2]);
			e->EventState3 = 0;
			n3->State = N(4)->State = 0;
			UseDoor(R->RoomDoors[1], 0, 1);
		}
	}
	NPCs *n7 = N(7);
	if (n7) { // the scientist at his desk
		bbRotateEntity(n7->Collider, 0, 180 + sinf(MilliSecs2() / 20 * DTOR) * 3, 0, 1);
		bbPositionEntity(n7->Collider, XG(R->obj) - 3361 * RS, -315 * RS, ZG(R->obj) - 2165 * RS, 0);
		bbResetEntity(n7->Collider);
		n7->State = 6;
		SetNPCFrame(n7, 182);
		if (N(6)->State == 1 && n7->Sound) {
			if (n7->SoundChn && !bbChannelPlaying(n7->SoundChn)) { FreeSound_Strict(n7->Sound); n7->Sound = 0; }
			if (n7->Sound) n7->SoundChn = LoopSound2(n7->Sound, n7->SoundChn, Camera, n7->Collider, 7, 1);
		}
	}
	for (int i = 3; i <= 4; i++) {
		NPCs *n = N(i);
		if (!n->Sound) continue;
		if (!bbChannelPlaying(n->SoundChn)) { FreeSound_Strict(n->Sound); n->Sound = 0; }
		else n->SoundChn = LoopSound2(n->Sound, n->SoundChn, Camera, n->Collider, 10, 1);
	}
	return 0;
}

// ---- 173's chamber (EventState3 = 0)

static void setup(Events *e) { // first frame in the room: everyone's spawned
	IntroSFX[0] = LoadSound_Strict("SFX\\Room\\Intro\\Scientist\\Franklin\\EnterChamber.ogg");
	IntroSFX[1] = LoadSound_Strict("SFX\\Room\\Intro\\Scientist\\Franklin\\Approach173.ogg");
	IntroSFX[2] = LoadSound_Strict("SFX\\Room\\Intro\\Scientist\\Franklin\\Problem.ogg");
	for (int i = 4; i <= 6; i++) IntroSFX[i] = LoadSound_Strict(sf("SFX\\Room\\Intro\\Scientist\\Franklin\\Refuse%d.ogg", i - 3));
	IntroSFX[16] = LoadSound_Strict("SFX\\Room\\Intro\\Horror.ogg");
	IntroSFX[17] = LoadSound_Strict("SFX\\Room\\Intro\\See173.ogg");
	IntroSFX[18] = LoadSound_Strict("SFX\\Room\\Intro\\173Chamber.ogg");
	Curr173->Idle = 1;
	NPCs *n = R->NPC[3] = CreateNPC(NPCtypeGuard, R->x - 4096 * RS + bbRnd(-.3f, .3f), .3f, R->z + bbRand(860, 896) * RS);
	bbRotateEntity(n->Collider, 0, R->angle + 180, 0, 0); n->State = 7;
	n = R->NPC[4] = CreateNPC(NPCtypeGuard, R->x - 3840 * RS, .3f, R->z + 768 * RS);
	bbRotateEntity(n->Collider, 0, R->angle + 135, 0, 0); n->State = 7;
	n = R->NPC[5] = CreateNPC(NPCtypeGuard, R->x - 8288 * RS, .3f, R->z + 1096 * RS);
	n->Sound = LoadSound_Strict(sf("SFX\\Room\\Intro\\Guard\\Music%d.ogg", bbRand(1, 5)));
	bbRotateEntity(n->Collider, 0, R->angle + 180, 0, 1); n->State = 7;
	n->Sound2 = LoadSound_Strict("SFX\\Room\\Intro\\Guard\\PlayerEscape.ogg");
	n = R->NPC[6] = CreateNPC(NPCtypeD, R->x - 3712 * RS, -.3f, R->z - 2208 * RS);
	ChangeNPCTextureID(n, 3);
	n = R->NPC[7] = CreateNPC(NPCtypeD, R->x - 3712 * RS, -.3f, R->z - 2208 * RS);
	n->Sound = LoadSound_Strict("SFX\\Room\\Intro\\Scientist\\Conversation.ogg");
	ChangeNPCTextureID(n, 2);
	int pvt = bbCreatePivot(0);
	bbRotateEntity(pvt, 90, 0, 0, 0);
	n = R->NPC[8] = CreateNPC(NPCtypeGuard, R->x - 3800 * RS, 1, R->z - 3900 * RS); n->State = 7;
	n = R->NPC[9] = CreateNPC(NPCtypeD, R->x - 4000 * RS, 1.1f, R->z - 3900 * RS); n->State2 = 1;
	int tex = LoadTexture_Strict("GFX\\npcs\\classd3.jpg", 1);
	bbEntityTexture(n->obj, tex, 0, 0); bbFreeTexture(tex);
	n = R->NPC[10] = CreateNPC(NPCtypeGuard, R->x - 4200 * RS, 1, R->z - 3900 * RS); n->State = 7;
	for (int i = 8; i <= 10; i++) { // onto the floor
		NPCs *m = R->NPC[i];
		bbPositionEntity(pvt, X(m->Collider), Y(m->Collider), Z(m->Collider), 0);
		bbEntityPick(pvt, 20);
		if (bbPickedEntity()) {
			bbPositionEntity(m->Collider, bbPickedX(), bbPickedY(), bbPickedZ(), 1);
			bbAlignToVector(m->Collider, -bbPickedNX(), -bbPickedNY(), -bbPickedNZ(), 3, 1);
			bbRotateEntity(m->Collider, 0, 90, 0, 0);
		}
	}
	bbFreeEntity(pvt);
	bbPositionEntity(Curr173->Collider, XG(R->Objects[5]), .5f, ZG(R->Objects[5]), 0);
	bbResetEntity(Curr173->Collider);
	bbPositionEntity(Collider, PlayerRoom->x - (3072 + 1024) * RS, .3f, PlayerRoom->z + 192 * RS, 0);
	bbResetEntity(Collider);
	e->EventState = 1; e->EventState3 = 1;
}

static void franklinanim(NPCs *n6) {
	if (bbAnimTime(n6->obj) >= 325) Animate2(n6->obj, bbAnimTime(n6->obj), 326, 328, .02f, 0);
	else Animate2(n6->obj, bbAnimTime(n6->obj), 320, 328, .05f, 0);
}

static int briefing(Events *e) { // < 10000: the class ds walk in, you're told to follow
	NPCs *n6 = N(6);
	if (n6->SoundChn) {
		if (bbChannelPlaying(n6->SoundChn)) { n6->State = 6; franklinanim(n6); }
		else Animate2(n6->obj, bbAnimTime(n6->obj), 328, 320, -.02f, 0);
	}
	if (IntroSFX[17] && bbEntityVisible(Curr173->Collider, Collider) && bbEntityInView(Curr173->obj, Camera)) {
		Msg = cat("Press ", KeyName[KEY_BLINK], " to blink."); MsgTimer = 70 * 4;
		PlaySound_Strict(IntroSFX[17]);
		IntroSFX[17] = 0;
	}
	e->EventState = Min(e->EventState + fl() / 3, 5000);
	float s = e->EventState;
	if (s >= 130 && s - fl() / 3 < 130) n6->SoundChn = PlaySound_Strict(IntroSFX[0]);
	else if (s > 230) {
		int temp = 1;
		for (int i = 1; i <= 2; i++) {
			NPCs *n = R->NPC[i];
			int o = R->Objects[i + 2];
			if (Distance(X(n->Collider), Z(n->Collider), XG(o), ZG(o)) > .3f) {
				bbPointEntity(n->obj, o, 0);
				bbRotateEntity(n->Collider, 0, CurveValue(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 15), 0, 0);
				if (s > 200 + i * 30) n->State = 1;
				temp = 0;
			} else {
				n->State = 0;
				bbPointEntity(n->obj, R->Objects[5], 0);
				bbRotateEntity(n->Collider, 0, CurveValue(bbEntityYaw(n->obj, 0), bbEntityYaw(n->Collider, 0), 15), 0, 0);
			}
		}
		if (X(Collider) < X(R->obj) + 408 * RS) { // you won't go in
			if (s >= 450 && s - fl() / 3 < 450) n6->SoundChn = PlaySound_Strict(IntroSFX[4]);
			else if (s >= 650 && s - fl() / 3 < 650) n6->SoundChn = PlaySound_Strict(IntroSFX[5]);
			else if (s >= 850 && s - fl() / 3 < 850) { UseDoor(R->RoomDoors[1], 0, 1); n6->SoundChn = PlaySound_Strict(IntroSFX[6]); }
			else if (s > 1000) { // fire at will
				NPCs *g = N(0);
				g->State = 1; g->State2 = 10; g->State3 = 1;
				N(3)->State = 11;
				opendoor(R->RoomDoors[2]);
				e->EventState2 = 1;
				return 1;
			}
			if (s > 850) bbPositionEntity(Collider, Min(X(Collider), X(R->obj) + 352 * RS), Y(Collider), Z(Collider), 0);
		} else if (temp) { e->EventState = 10000; UseDoor(R->RoomDoors[1], 0, 1); }
	}
	n6->State = 7;
	face(n6, Collider, 20);
	bbPositionEntity(Curr173->Collider, XG(R->Objects[5]), Y(Curr173->Collider), ZG(R->Objects[5]), 0);
	bbRotateEntity(Curr173->Collider, 0, 0, 0, 1);
	bbResetEntity(Curr173->Collider);
	return 0;
}

static void test(Events *e) { // < 14000: inside w/ 173, the door, the problem, the lights
	e->EventState = Min(e->EventState + fl(), 13000);
	float s = e->EventState;
	if (s < 10300) bbPositionEntity(Collider, Max(X(Collider), X(R->obj) + 352 * RS), Y(Collider), Z(Collider), 0);
	NPCs *n6 = N(6);
	n6->State = 6;
	bbPointEntity(n6->obj, Curr173->Collider, 0);
	bbRotateEntity(n6->Collider, 0, CurveValue(bbEntityYaw(n6->obj, 0), bbEntityYaw(n6->Collider, 0), 50), 0, 1);
	if (s >= 10300 && s - fl() < 10300) { // "please approach SCP-173"
		e->SoundCHN = PlaySound_Strict(IntroSFX[1]);
		bbPositionEntity(Collider, Max(X(Collider), X(R->obj) + 352 * RS), Y(Collider), Z(Collider), 0);
	} else if (s >= 10440 && s - fl() < 10440) { UseDoor(R->RoomDoors[1], 0, 1); e->SoundCHN = PlaySound_Strict(IntroSFX[7]); }
	else if (s >= 10740 && s - fl() < 10740) e->SoundCHN = PlaySound_Strict(IntroSFX[2]);
	else if (s >= 11145 && s - fl() < 11145) {
		e->SoundCHN = PlaySound_Strict(IntroSFX[10]);
		R->NPC[1]->Sound = LoadSound_Strict("SFX\\Room\\Intro\\ClassD\\DontLikeThis.ogg");
		PlaySound2(R->NPC[1]->Sound, Camera, R->NPC[2]->Collider, 10, 1);
	} else if (s >= 11561 && s - fl() < 11561) { // lights out
		e->EventState = 14000;
		PlaySound_Strict(IntroSFX[16]);
		R->NPC[2]->Sound = LoadSound_Strict("SFX\\Room\\Intro\\ClassD\\Breen.ogg");
		PlaySound2(R->NPC[2]->Sound, Camera, R->NPC[1]->Collider, 10, 1);
	}
	s = e->EventState; // lights out may have just jumped it to 14000
	if (s >= 10440 && s - fl() < 11561 && X(Collider) < XG(R->RoomDoors[1]->frameobj) && N(0)->State != 12) { // guard alert: get back in there
		NPCs *g = N(0);
		g->Sound = LoadSound_Strict(sf("SFX\\Room\\Intro\\Guard\\Balcony\\Alert%d.ogg", bbRand(1, 2)));
		g->SoundChn = PlaySound2(g->Sound, Camera, g->Collider, 20, 1);
		g->State = 12; g->State2 = 1;
	}
	if (s > 10300) {
		NPCs *n1 = R->NPC[1];
		if (s > 10560) {
			if (s < 10750) { n1->State = 1; n1->CurrSpeed = .005f; }
			else { n1->State = 0; n1->CurrSpeed = CurveValue(0, n1->CurrSpeed, 10); }
		}
		franklinanim(n6);
	}
	bbPositionEntity(Curr173->Collider, XG(R->Objects[5]), Y(Curr173->Collider), ZG(R->Objects[5]), 0);
	bbRotateEntity(Curr173->Collider, 0, 0, 0, 1);
	bbResetEntity(Curr173->Collider);
}

static void guardvoice(NPCs *g, const char *file) {
	if (g->Sound) { bbStopChannel(g->SoundChn); FreeSound_Strict(g->Sound); g->Sound = 0; }
	g->Sound = LoadSound_Strict(file);
	g->SoundChn = PlaySound2(g->Sound, Camera, g->Collider, 20, 1);
}

static void kills(Events *e) { // < 20000: 173 kills both class ds in the dark
	NPCs *n1 = R->NPC[1], *n2 = R->NPC[2], *g = N(0), *n6 = N(6);
	int pvt = bbCreatePivot(0); // you can't look away
	bbPositionEntity(pvt, X(Camera), bbEntityY(Curr173->Collider, 1) - .05f, Z(Camera), 0);
	bbPointEntity(pvt, Curr173->Collider, 0);
	bbRotateEntity(Collider, bbEntityPitch(Collider, 0), CurveAngle(bbEntityYaw(pvt, 0), bbEntityYaw(Collider, 0), 40), 0, 0);
	bbTurnEntity(pvt, 90, 0, 0, 0);
	user_camera_pitch = CurveAngle(bbEntityPitch(pvt, 0), user_camera_pitch + 90, 40);
	user_camera_pitch -= 90;
	bbFreeEntity(pvt);
	n6->State = 6;
	bbPointEntity(n6->obj, Curr173->Collider, 0);
	bbRotateEntity(n6->Collider, 0, CurveValue(bbEntityYaw(n6->obj, 0), bbEntityYaw(n6->Collider, 0), 20), 0, 1);
	Animate2(n6->obj, bbAnimTime(n6->obj), 357, 381, .05f, 1);
	e->EventState = Min(e->EventState + fl(), 19000);
	float s = e->EventState;
	if (s < 14100) { // lights go out, the first class d
		if (s < 14060) {
			BlinkTimer = Max((14000 - s) / 2 - bbRnd(0, 1), -10);
			if (BlinkTimer == -10) {
				bbPointEntity(Curr173->Collider, n1->obj, 0);
				bbRotateEntity(Curr173->Collider, 0, bbEntityYaw(Curr173->Collider, 0), 0, 0);
				bbMoveEntity(Curr173->Collider, 0, 0, Curr173->Speed * .6f * fl());
				Curr173->SoundChn = LoopSound2(StoneDragSFX, Curr173->SoundChn, Camera, Curr173->Collider, 10, Curr173->State);
				Curr173->State = CurveValue(1, Curr173->State, 3);
			} else Curr173->State = Max(0, Curr173->State - fl() / 20);
		} else if (s < 14065) {
			BlinkTimer = -10;
			if (n1->State == 0) PlaySound2(NeckSnapSFX[bbRand(0, 2)], Camera, Curr173->Collider, 10, 1);
			bbSetAnimTime(n1->obj, 0, 0);
			n1->State = 6;
			bbPositionEntity(Curr173->Collider, X(n1->obj), Y(Curr173->Collider), Z(n1->obj), 0);
			bbResetEntity(Curr173->Collider);
			bbPointEntity(Curr173->Collider, n2->Collider, 0);
			n2->State = 3;
			bbRotateEntity(n2->Collider, 0, bbEntityYaw(n2->Collider, 0), 0, 0);
			Animate2(n2->obj, bbAnimTime(n2->obj), 406, 382, -.01f * 15, 1);
			bbMoveEntity(n2->Collider, 0, 0, -.01f * fl());
			g->State = 12; // "what the..."
			g->Angle = 180;
			guardvoice(g, sf("SFX\\Room\\Intro\\Guard\\Balcony\\WTF%d.ogg", bbRand(1, 2)));
			g->State2 = 0;
		} else {
			Animate2(n1->obj, bbAnimTime(n1->obj), 0, 19, .2f, 0);
			if (!n2->Sound) { n2->Sound = LoadSound_Strict("SFX\\Room\\Intro\\ClassD\\Gasp.ogg"); PlaySound2(n2->Sound, Camera, n2->Collider, 8, 1); }
		}
		if (s > 14080 && s - fl() < 14080) PlaySound_Strict(IntroSFX[12]);
		CameraShake = 3;
	} else if (s < 14200) { // the other one
		Animate2(n1->obj, bbAnimTime(n1->obj), 0, 19, .2f, 0);
		g->State = 8;
		if (s > 14105) {
			if (n2->State != 6) PlaySound2(NeckSnapSFX[1], Camera, n2->Collider, 8, 1);
			n2->State = 6;
			bbPositionEntity(Curr173->Collider, X(n2->obj), Y(Curr173->Collider), Z(n2->obj), 0);
			bbResetEntity(Curr173->Collider);
			bbPointEntity(Curr173->Collider, Collider, 0);
		}
		if (s < 14130) { bbSetAnimTime(n2->obj, 50, 0); BlinkTimer = -10; LightBlink = 1; }
		else { Animate2(n2->obj, bbAnimTime(n2->obj), 50, 60, .2f, 0); Curr173->Idle = 0; }
		if (s > 14100 && s - fl() < 14100) PlaySound_Strict(IntroSFX[8]);
		if (s < 14150) CameraShake = 5;
	} else {
		Animate2(n2->obj, bbAnimTime(n2->obj), 45, 60, .2f, 0);
		if (s > 14300) {
			if (s > 14600 && s < 14700) { BlinkTimer = -10; LightBlink = 1; }
			if (X(Collider) < X(R->obj) + 448 * RS) e->EventState = 20000;
		}
	}
}

// lights out for good: you wake up in the start room, the intro area is torn down
static void tostart(Events *e) {
	each(Rooms, r) {
		if (!bbSeq(r->RoomTemplate->Name, "start")) continue;
		PlayerRoom = r;
		float x = bbEntityX(r->obj, 1) + 3712 * RS, y = 384 * RS, z = bbEntityZ(r->obj, 1) + 1312 * RS;
		bbPositionEntity(Collider, x + (X(Collider) - X(R->obj)), y + Y(Collider) + .4f, z + (Z(Collider) - Z(R->obj)), 0);
		DropSpeed = 0;
		bbResetEntity(Collider);
		for (int i = 0; i <= 2; i++) {
			int c = R->NPC[i]->Collider;
			bbPositionEntity(c, x + (X(c) - X(R->obj)), y + Y(c) + .4f, z + (Z(c) - Z(R->obj)), 0);
			bbResetEntity(c);
		}
		ShouldPlay = 0;
		for (int i = 0; i <= 9; i++) FreeSound_Strict(IntroSFX[i]);
		for (int i = 16; i <= 18; i++) FreeSound_Strict(IntroSFX[i]);
		r->NPC[0] = R->NPC[0];
		r->NPC[0]->State = 8;
		each(Doors, d) if (d->room == R) RemoveDoor(d);
		each(WayPoints, w) if (w->room == R) { bbFreeEntity(w->obj); bbDelete(w); }
		for (int i = 3; i <= 4; i++) RemoveNPC(N(i));
		r->NPC[1] = R->NPC[6];
		Rooms *dead = R;
		bbFreeEntity(dead->obj);
		bbDelete(dead);
		e->room = NULL;
		each(SecurityCams, sc) if (sc->room == dead) bbDelete(sc); // the intro room's cams
		bbShowEntity(Fog);
		bbAmbientLight(Brightness, Brightness, Brightness);
		bbCameraFogRange(Camera, CameraFogNear, CameraFogFar);
		bbCameraFogMode(Camera, 1);
		e->EventState2 = 1;
		break;
	}
}

static void breakout(Events *e) { // < 30000: 173 gets the balcony guard
	e->EventState = Min(e->EventState + fl(), 30000);
	float s = e->EventState;
	NPCs *g = N(0);
	if (s < 20100) { CameraShake = 2; return; }
	if (s < 20200) { // lights out, 173 next to the guard
		if (s > 20105 && s - fl() < 20105) {
			PlaySound_Strict(IntroSFX[9]);
			bbPositionEntity(g->Collider, X(R->obj) - 160 * RS, Y(g->Collider) + .1f, Z(R->obj) + 1280 * RS, 0);
			bbResetEntity(g->Collider);
			guardvoice(g, "SFX\\Room\\Intro\\Guard\\Balcony\\OhSh.ogg");
		}
		if (s > 20105) {
			Curr173->Idle = 1;
			bbPointEntity(g->Collider, Curr173->obj, 0);
			bbPositionEntity(Curr173->Collider, X(R->obj) - 608 * RS, Y(R->obj) + 480 * RS, Z(R->obj) + 1312 * RS, 0);
			bbResetEntity(Curr173->Collider);
			bbPointEntity(Curr173->Collider, g->Collider, 0);
		}
		BlinkTimer = -10; LightBlink = 1; CameraShake = 3;
	} else if (s < 20300) { // the guard opens fire
		bbPointEntity(g->Collider, Curr173->Collider, 0);
		bbMoveEntity(g->Collider, 0, 0, -.002f);
		g->State = 2;
		UpdateSoundOrigin(g->SoundChn, Camera, g->Collider, 20, 1);
		if (s > 20260 && s - fl() < 20260) PlaySound_Strict(IntroSFX[12]);
	} else if (s - fl() < 20300) { // lights out, the guard's dead
		BlinkTimer = -10; LightBlink = 1; CameraShake = 3;
		PlaySound_Strict(IntroSFX[11]);
		PlaySound2(NeckSnapSFX[1], Camera, g->Collider, 8, 1);
		Curr173->Idle = 0;
		e->SoundCHN = PlaySound_Strict(IntroSFX[15]);
		bbPositionEntity(Curr173->Collider, X(PlayerRoom->obj) - 400 * RS, 100, Z(PlayerRoom->obj) + 1072 * RS, 0);
		bbResetEntity(Curr173->Collider);
		tostart(e);
	}
}

static int chamber(Events *e) {
	if (IntroSFX[18]) e->SoundCHN2 = LoopSound2(IntroSFX[18], e->SoundCHN2, Camera, R->Objects[4], 6, 1);
	float s = e->EventState;
	if (s == 0) { if (PlayerRoom == R) setup(e); }
	else if (s < 10000) return briefing(e);
	else if (s < 14000) test(e);
	else if (s < 20000) kills(e);
	else if (s < 30000) breakout(e);
	return 0;
}

int Ev173(Events *e) {
	if (KillTimer >= 0 && e->EventState2 == 0) {
		PlayerZone = 0;
		if (e->EventState3 > 0 ? walk(e) : chamber(e)) return 1;
	} else {
		if (KillTimer < 0 && N(3) && N(3)->State == 1) { LoadEventSound(e, "SFX\\Room\\Intro\\Guard\\Ulgrin\\EscortTerminated.ogg", 0); PlaySound_Strict(e->Sound); }
		for (int i = 0; i <= 6; i++) if (IntroSFX[i]) { FreeSound_Strict(IntroSFX[i]); IntroSFX[i] = 0; }
		FreeSound_Strict(IntroSFX[16]); IntroSFX[16] = 0;
		e->EventState2 = 1;
	}
	if (R && PlayerRoom == R) {
		if (e->EventState >= 10) { bbCameraRange(Camera, .05f, 15); if (N(7)) RemoveNPC(N(7)); }
		else bbCameraRange(Camera, .05f, 40);
		bbCameraFogMode(Camera, 0);
		bbAmbientLight(140, 140, 140);
		bbHideEntity(Fog);
		LightVolume = TempLightVolume = 4;
	} else RemoveEvent(e);
	return 0;
}

// ---- "alarm": the start room after the breach (also where you land if the intro's off)

static void alarmsetup(Events *e) {
	R->RoomDoors[2]->open = 1;
	bbShowEntity(Fog);
	bbAmbientLight(Brightness, Brightness, Brightness);
	bbCameraFogRange(Camera, CameraFogNear, CameraFogFar);
	bbCameraFogMode(Camera, 1);
	if (SelectedDifficulty->saveType == SAVEANYWHERE) { Msg = cat("Press ", KeyName[KEY_SAVE], " to save."); MsgTimer = 70 * 4; }
	else if (SelectedDifficulty->saveType == SAVEONSCREENS) { Msg = "Saving is only permitted on clickable monitors scattered throughout the facility."; MsgTimer = 70 * 8; }
	Curr173->Idle = 0;
	Doors *d1 = R->RoomDoors[1];
	while (d1->openstate < 180) { // snap the door open
		d1->openstate = Min(180, d1->openstate + .8f);
		bbMoveEntity(d1->obj, sinf(d1->openstate * DTOR) / 180, 0, 0);
		bbMoveEntity(d1->obj2, -sinf(d1->openstate * DTOR) / 180, 0, 0);
	}
	if (N(0)) { SetNPCFrame(N(0), 74); N(0)->State = 8; }
	if (!N(1)) {
		R->NPC[1] = CreateNPC(NPCtypeD, 0, 0, 0);
		ChangeNPCTextureID(R->NPC[1], 3);
		R->NPC[1]->texture = "GFX\\npcs\\scientist2.jpg";
		ChangeNPCTextureID(R->NPC[1], 3);
	}
	bbPositionEntity(N(1)->Collider, R->x, .5f, R->z - 1, 1);
	bbResetEntity(N(1)->Collider);
	SetNPCFrame(N(1), 210);
	if (!N(2)) R->NPC[2] = CreateNPC(NPCtypeGuard, 0, 0, 0);
	bbPositionEntity(N(2)->Collider, R->x, .5f, R->z + 528 * RS, 1);
	bbResetEntity(N(2)->Collider);
	N(2)->State = 7;
	bbPointEntity(N(2)->Collider, N(1)->Collider, 0);
	if (!N(0)) { // no intro: the cell block survivors get placed too
		NPCs *n = R->NPC[3] = CreateNPC(NPCtypeGuard, XG(R->Objects[2]), bbEntityY(R->Objects[2], 1), ZG(R->Objects[2]));
		bbRotateEntity(n->Collider, 0, 90, 0, 0);
		SetNPCFrame(n, 286); n->State = 8;
		bbMoveEntity(n->Collider, 1, 0, 0);
		n = R->NPC[4] = CreateNPC(NPCtypeD, XG(R->Objects[3]), .5f, ZG(R->Objects[3]));
		SetNPCFrame(n, 19); n->State = 3;
		bbRotateEntity(n->Collider, 0, 270, 0, 0);
		bbMoveEntity(n->Collider, 0, 0, 2.65f);
		n = R->NPC[5] = CreateNPC(NPCtypeD, XG(R->Objects[4]), .5f, ZG(R->Objects[4]));
		ChangeNPCTextureID(n, 6);
		SetNPCFrame(n, 19); n->State = 3;
		bbRotateEntity(n->Collider, 0, 270, 0, 0);
		bbMoveEntity(n->Collider, .25f, 0, 3);
		bbRotateEntity(n->Collider, 0, 0, 0, 0);
		float x = bbEntityX(R->obj, 1) + 3712 * RS, y = 384 * RS, z = bbEntityZ(R->obj, 1) + 1312 * RS;
		for (int i = 3; i <= 5; i++) {
			int c = R->NPC[i]->Collider;
			bbPositionEntity(c, x + (X(c) - X(R->obj)), y + Y(c) + .4f, z + (Z(c) - Z(R->obj)), 0);
			bbResetEntity(c);
		}
	}
	e->EventState = 1;
}

static void lockdown(Events *e, str trig) { // 900+: the scientist + ulgrin at the door, 173 behind it
	NPCs *n1 = N(1), *n2 = N(2);
	Doors *d5 = R->RoomDoors[5];
	if (e->EventState > 900 && d5->open) {
		if (e->EventState - fl() <= 900) {
			n1->Sound = LoadSound_Strict("SFX\\Room\\Intro\\WhatThe.ogg");
			n1->SoundChn = PlaySound2(n1->Sound, Camera, n1->Collider, 10, 1);
		}
		n1->State = 3;
		n1->CurrSpeed = CurveValue(-.008f, n1->CurrSpeed, 5);
		AnimateNPC(n1, 260, 236, n1->CurrSpeed * 18, 1);
		bbRotateEntity(n1->Collider, 0, 0, 0, 0);
		if (e->EventState > 900 + 2.5f * 70 && n2->State != 1) { // ulgrin backs off
			n2->CurrSpeed = CurveValue(-.012f, n2->CurrSpeed, 5);
			AnimateNPC(n2, 39, 76, n2->CurrSpeed * 40, 1);
			bbMoveEntity(n2->Collider, 0, 0, n2->CurrSpeed * fl());
			n2->State = 8;
			if (Z(n2->Collider) < R->z) {
				bbPointEntity(n2->obj, n1->Collider, 0);
				bbRotateEntity(n2->Collider, 0, CurveAngle(bbEntityYaw(n2->obj, 0) - 180, bbEntityYaw(n2->Collider, 0), 15), 0, 0);
			} else bbRotateEntity(n2->Collider, 0, 0, 0, 0);
		}
		if (e->EventState < 900 + 4 * 70) {
			bbPositionEntity(Curr173->Collider, R->x + 32 * RS, .31f, R->z + 1072 * RS, 1);
			bbRotateEntity(Curr173->Collider, 0, 190, 0, 0);
			if (e->EventState > 900 + 70 && e->EventState < 900 + 2.5f * 70) {
				AnimateNPC(n2, 1539, 1553, .2f, 0);
				bbPointEntity(n2->obj, Curr173->Collider, 0);
				bbRotateEntity(n2->Collider, 0, CurveAngle(bbEntityYaw(n2->obj, 0), bbEntityYaw(n2->Collider, 0), 15), 0, 0);
			}
		} else {
			if (e->EventState - fl() < 900 + 4 * 70) {
				PlaySound_Strict(IntroSFX[11]); LightBlink = 3;
				PlaySound2(StoneDragSFX, Camera, Curr173->Collider, 10, 1);
				bbPointEntity(Curr173->Collider, n2->Collider, 0);
				if (Y(Collider) < 320 * RS) BlinkTimer = -10;
			}
			bbPositionEntity(Curr173->Collider, R->x - 96 * RS, .31f, R->z + 592 * RS, 1);
			bbRotateEntity(Curr173->Collider, 0, 190, 0, 0);
			if (n2->State != 1 && KillTimer >= 0 && Z(n2->Collider) < R->z - 1150 * RS) { // the door slams, 173's loose
				d5->open = 0;
				LightBlink = 3;
				PlaySound_Strict(IntroSFX[11]);
				BlinkTimer = -10;
				PlaySound2(StoneDragSFX, Camera, Curr173->Collider, 10, 1);
				if (bbEntityDistance(Curr173->Collider, Collider) < 2.5f && fabsf(Y(Collider) - Y(Curr173->Collider)) < 1)
					bbPositionEntity(Curr173->Collider, X(Collider), Y(Collider), Z(Collider), 0);
				else bbPositionEntity(Curr173->Collider, 0, 0, 0, 0);
				bbResetEntity(Curr173->Collider);
				Msg = cat("Hold ", KeyName[KEY_SPRINT], " to run."); MsgTimer = 70 * 8;
			}
		}
		if (bbSeq(trig, "173scene_end") && bbEntityVisible(n2->Collider, Collider) && !NoTarget) { n2->State = 1; n2->State3 = 1; } // ulgrin shoots
		else if (n2->State == 1 && !bbEntityVisible(n2->Collider, Collider)) { n2->State = 0; n2->State3 = 0; }
		if (n2->State == 1) d5->open = 1;
	} else {
		CanSave = 1;
		if (n2->State != 1) {
			if (X(Collider) < R->x + 1384 * RS) e->EventState = Max(e->EventState, 900);
			if (d5->openstate == 0) {
				if (N(1)) RemoveNPC(N(1));
				if (N(2)) RemoveNPC(N(2));
				e->EventState2 = 1;
			}
		}
	}
}

int EvAlarm(Events *e) {
	if (!R->RoomDoors[5]) for (int i = 0; i <= 3; i++) if (R->AdjDoor[i]) { R->RoomDoors[5] = R->AdjDoor[i]; R->RoomDoors[5]->open = 1; break; }
	if (e->EventState == 0) { if (PlayerRoom == R) alarmsetup(e); return 0; }
	if (N(0)) AnimateNPC(N(0), 249, 286, .4f, 0);
	str trig = CheckTriggers();
	if (bbSeq(trig, "173scene_timer")) e->EventState += fl();
	else if (bbSeq(trig, "173scene_activated")) e->EventState = Max(e->EventState, 500);
	if (e->EventState < 850) {
		bbPositionEntity(Curr173->Collider, R->x + 32 * RS, .31f, R->z + 1072 * RS, 1);
		bbHideEntity(Curr173->obj);
	}
	if (e->EventState >= 500) {
		e->EventState += fl();
		if (e->EventState2 == 0) { bbShowEntity(Curr173->obj); lockdown(e, trig); }
		// the ceiling pieces come down
		int o0 = R->Objects[0], o1 = R->Objects[1];
		float s = e->EventState;
		bbPositionEntity(o0, XG(o0), -Max(s - 1300, 0) / 4500, ZG(o0), 1);
		bbRotateEntity(o0, -Max(s - 1320, 0) / 130, 0, -Max(s - 1300, 0) / 40, 1);
		bbPositionEntity(o1, XG(o1), -Max(s - 1800, 0) / 5000, ZG(o1), 1);
		bbRotateEntity(o1, -Max(s - 2040, 0) / 135, 0, -Max(s - 2040, 0) / 43, 1);
		if (bbEntityDistance(o0, Collider) < 2.5f && bbRand(300, 1) == 2) PlaySound2(DecaySFX[bbRand(1, 3)], Camera, o0, 3, 1);
	}
	if (e->EventState < 2000 && (!e->SoundCHN || !bbChannelPlaying(e->SoundCHN))) e->SoundCHN = PlaySound_Strict(AlarmSFX[0]);
	if (e->EventState3 < 11) {
		if (!bbChannelPlaying(e->SoundCHN2)) {
			e->EventState3++;
			if (e->Sound2) { FreeSound_Strict(e->Sound2); e->Sound2 = 0; }
			e->Sound2 = LoadSound_Strict(sf("SFX\\Alarm\\Alarm2_%d.ogg", bbInt(e->EventState3)));
			e->SoundCHN2 = PlaySound_Strict(e->Sound2);
		} else if (bbInt(e->EventState3) == 8) CameraShake = 1;
	}
	if (fmodf(e->EventState, 600) > 300 && fmodf(e->EventState + fl(), 600) < 300) { // the commotion through the walls
		int i = (int)floorf((e->EventState - 5000) / 600) + 1;
		if (i == 0) PlaySound_Strict(LoadTempSound("SFX\\Room\\Intro\\PA\\scripted\\scripted6.ogg"));
		if (i > 0 && i < 26 && !CommotionState[i]) {
			PlaySound_Strict(LoadTempSound(sf("SFX\\Room\\Intro\\Commotion\\Commotion%d.ogg", i)));
			CommotionState[i] = 1;
		}
		if (i > 26) {
			if (N(0)) RemoveNPC(N(0));
			bbFreeEntity(R->Objects[0]); bbFreeEntity(R->Objects[1]);
			R->Objects[0] = R->Objects[1] = 0;
			RemoveEvent(e);
		}
	}
	return 0;
}
