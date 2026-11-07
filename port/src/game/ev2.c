// ev2.c: medium UpdateEvents.bb events (106's ambushes, sinkholes, 173 + the glass, the elevator death...) + QuickLoad
#include "g.h"

#define R (e->room)
#define RS RoomScale
#define IN (PlayerRoom == R)
static float XG(int o) { return entity_x(o, 1); }
static float YG(int o) { return entity_y(o, 1); }
static float ZG(int o) { return entity_z(o, 1); }
static float sgn(float v) { return v > 0 ? 1 : v < 0 ? -1 : 0; }

static int testroom(Events *e) { // 1048 scurries across, gas on approach
	if (!IN) return 0;
	int *o7 = &R->Objects[7];
	if (e->EventState == 0) {
		*o7 = LoadAnimMesh_Strict("GFX\\npcs\\scp-1048.b3d", 0);
		scale_entity(*o7, .05f, .05f, .05f, 0);
		t_form_point(entity_x(Collider, 0), entity_y(Collider, 0), entity_z(Collider, 0), 0, R->obj);
		int temp = t_formed_z() == 0 ? -1 : -(int)sgn(t_formed_z());
		t_form_point(-720, 0, 816 * temp, R->obj, 0);
		position_entity(*o7, t_formed_x(), 0, t_formed_z(), 0);
		rotate_entity(*o7, -90, R->angle - 90, 0, 0);
		set_anim_time(*o7, 297, 0);
		e->EventState = 1;
	}
	if (entity_distance(Collider, R->Objects[6]) < 2.5f && e->EventState > 0) {
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\079\\TestroomWarning.ogg"));
		for (int i = 0; i <= 5; i++) {
			int o = R->Objects[i];
			Emitters *em = CreateEmitter(XG(o), YG(o), ZG(o), 0);
			turn_entity(em->Obj, 90, 0, 0, 1);
			em->RandAngle = 5; em->Speed = .042f; em->SizeChange = .0025f;
		}
		e->EventState = -e->EventState;
	}
	if (*o7) {
		Animate2(*o7, anim_time(*o7), 284, 295, .3f, 1);
		move_entity(*o7, 0, -.008f * FPSfactor, 0);
		t_form_point(entity_x(*o7, 0), entity_y(*o7, 0), entity_z(*o7, 0), 0, R->obj);
		if (fabsf(t_formed_x()) > 725) { free_entity(*o7); *o7 = 0; e->EventState *= 2; }
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
		if (d1->open && (d1->openstate > 50 || entity_distance(Collider, d1->frameobj) < .5f)) {
			d1->openstate = Min(d1->openstate, 50);
			d1->open = 0;
			PlaySound2(LoadTempSound("SFX\\Door\\DoorError.ogg"), Camera, d1->frameobj, 10, 1);
		}
		return 0;
	}
	d1->locked = 0;
	Rooms *ga = NULL;
	each(Rooms, r) if (s_eq(r->RoomTemplate->Name, "gatea")) { ga = r; break; }
	if (!llive(Curr096) || Curr096->State == 0 || Curr096->State == 5)
		e->EventState = UpdateElevators(e->EventState, R->RoomDoors[0], ga->RoomDoors[1], R->Objects[0], R->Objects[1], e, 1);
	// TODO else Update096ElevatorEvent (comes w/ 096)
	if (!Contained106 && e->EventState < -1.5f && e->EventState + FPSfactor >= -1.5f) PlaySound_Strict(OldManSFX[3]);
	if (entity_distance(Collider, R->Objects[1]) < 4) { ga->RoomDoors[1]->locked = 1; PlayerRoom = ga; RemoveEvent(e); }
	return 0;
}

static int sinkhole(Events *e) { // 106's sinkhole drags you to the pocket dimension
	if (e->EventState == 0) {
		Decals *de = CreateDecal(0, entity_x(R->obj, 0) + frnd(-.5f, .5f), .01f, entity_z(R->obj, 0) + frnd(-.5f, .5f), 90, irand(360, 1), 0);
		if (de) { de->Size = 2.5f; scale_sprite(de->obj, de->Size, de->Size); }
		e->EventState = 1;
		return 0;
	}
	if (!IN) { e->EventState2 = 0; return 0; }
	if (!e->Sound) e->Sound = LoadSound_Strict("SFX\\Room\\Sinkhole.ogg");
	else e->SoundCHN = LoopSound2(e->Sound, e->SoundCHN, Camera, R->obj, 4.5f, 1.5f);
	float dist = Distance(entity_x(Collider, 0), entity_z(Collider, 0), entity_x(R->obj, 0), entity_z(R->obj, 0));
	if (dist >= 2) return 0;
	CurrStepSFX = 1;
	CurrSpeed = CurveValue(0, CurrSpeed, Max(dist * 50, 1));
	CrouchState = (2 - dist) / 2;
	if (dist >= .5f) return 0;
	if (e->EventState2 == 0) PlaySound_Strict(LoadTempSound("SFX\\Room\\SinkholeFall.ogg"));
	CurrSpeed = CurveValue(0, CurrSpeed, Max(dist * 50, 1));
	float x = CurveValue(entity_x(R->obj, 0), entity_x(Collider, 0), 10);
	float y = CurveValue(entity_y(R->obj, 0) - e->EventState2, entity_y(Collider, 0), 25);
	float z = CurveValue(entity_z(R->obj, 0), entity_z(Collider, 0), 10);
	position_entity(Collider, x, y, z, 1);
	DropSpeed = 0;
	reset_entity(Collider);
	e->EventState2 = Min(e->EventState2 + FPSfactor / 200, 2);
	LightBlink = Min(e->EventState2 * 5, 10);
	BlurTimer = e->EventState2 * 500;
	if (e->EventState2 == 2) MoveToPocketDimension();
	return 0;
}

static int room2cafeteria(Events *e) { // 294 takes two quarters
	if (IN && !Using294 && entity_distance(R->Objects[0], Collider) < 1.5f) {
		GiveAchievement(Achv294, 1);
		if (entity_in_view(R->Objects[0], Camera)) {
			DrawHandIcon = 1;
			if (MouseHit1) {
				int temp = 1; // the output slot's empty
				each(Items, it) if (!it->Picked && entity_x(it->collider, 0) - XG(R->Objects[1]) == 0 && entity_z(it->collider, 0) - ZG(R->Objects[1]) == 0) { temp = 0; break; }
				int inserted = 0;
				if (e->EventState2 < 2 && SelectedItem && (s_eq(SelectedItem->itemtemplate->tempname, "25ct") || s_eq(SelectedItem->itemtemplate->tempname, "coin"))) {
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
	if (e->EventState == 0) { CreateNPC(NPCtype066, entity_x(R->obj, 0), .5f, entity_z(R->obj, 0)); e->EventState = 1; }
	return 0;
}

static int testroom173(Events *e) { // 173 behind the glass, breaks through when you blink
	if (!IN || Curr173->Idle != 0) return 0;
	if (e->EventState == 0) {
		if (R->RoomDoors[0]->open) {
			position_entity(Curr173->Collider, XG(R->Objects[0]), .5f, ZG(R->Objects[0]), 0);
			reset_entity(Curr173->Collider);
			e->EventState = 1;
		}
		return 0;
	}
	int *g = &R->Objects[2];
	if (!*g) {
		int tex = LoadTexture_Strict("GFX\\map\\glass.png", 1 + 2);
		*g = create_sprite(0);
		entity_texture(*g, tex, 0, 0);
		sprite_view_mode(*g, 2);
		scale_sprite(*g, 182 * RS * .5f, 192 * RS * .5f);
		int pvt = create_pivot(R->obj);
		position_entity(pvt, -632, 224, -208, 0);
		position_entity(*g, XG(pvt), YG(pvt), ZG(pvt), 0);
		free_entity(pvt);
		rotate_entity(*g, 0, R->angle, 0, 0);
		turn_entity(*g, 0, 180, 0, 0);
		entity_parent(*g, R->obj, 1);
		free_texture(tex);
	}
	show_entity(*g);
	e->EventState++; // 173's timer for breaking through
	float dist = entity_distance(Collider, R->Objects[1]);
	if (dist < 1) e->EventState = Max(e->EventState, 70 * 12); // close: 173's ready
	else if (dist > 1.4f && e->EventState > 70 * 12 && BlinkTimer <= -10) { // stepped back and blinked: it attacks
		if (entity_distance(Curr173->Collider, R->Objects[0]) > 5) { RemoveEvent(e); return 0; } // 173 wandered off
		PlaySound2(LoadTempSound("SFX\\General\\GlassBreak.ogg"), Camera, Curr173->obj, 10, 1);
		free_entity(*g); *g = 0;
		position_entity(Curr173->Collider, XG(R->Objects[1]), .5f, ZG(R->Objects[1]), 0);
		reset_entity(Curr173->Collider);
		RemoveEvent(e);
	}
	return 0;
}

static int room2elevator(Events *e) { // a guard walks into the elevator and dies
	NPCs *n = llive(R->NPC[0]);
	if (e->EventState == 0) {
		if (R->dist < 8 && R->dist > 0) {
			n = R->NPC[0] = CreateNPC(NPCtypeGuard, XG(R->obj), .5f, ZG(R->obj));
			point_entity(n->Collider, Collider, 0);
			rotate_entity(n->Collider, 0, entity_yaw(n->Collider, 0), 0, 1);
			e->EventState = 1;
		}
		return 0;
	}
	if (e->EventState == 1) {
		if (R->dist < 5 || irand(700, 1) == 1) {
			e->EventState = 2;
			n->State = 5;
			n->EnemyX = XG(R->Objects[1]); n->EnemyY = YG(R->Objects[1]); n->EnemyZ = ZG(R->Objects[1]);
		}
	} else if (e->EventState == 2) {
		if (entity_distance(n->Collider, R->Objects[1]) < 2) {
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
				CreateDecal(3, XG(o), .0005f, ZG(o), 90, frnd(360, 0), 0);
				Decals *de = CreateDecal(17, XG(o), .002f, ZG(o), 90, frnd(360, 0), 0);
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
		Decals *de = CreateDecal(0, entity_x(R->obj, 0), 799 * RS, entity_z(R->obj, 0), -90, irand(360, 1), 0);
		if (de) {
			de->Size = .05f; de->SizeChange = .0015f; entity_alpha(de->obj, .8f); UpdateDecals();
			PlaySound2(DecaySFX[3], Camera, de->obj, 15, 1);
		}
		e->EventState = 1;
	}
	if (e->EventState <= 0) return 0;
	if (!llive(R->NPC[0])) e->EventState += FPSfactor;
	if (e->EventState <= 200) return 0;
	if (!llive(R->NPC[0])) {
		NPCs *n = R->NPC[0] = CreateNPC(NPCtypeD, entity_x(R->obj, 0), 900 * RS, entity_z(R->obj, 0));
		rotate_entity(n->Collider, 0, frnd(360, 0), 0, 1);
		ChangeNPCTextureID(n, 5);
		n->State = 6;
		PlaySound_Strict(HorrorSFX[0]);
		PlaySound2(DecaySFX[2], Camera, n->Collider, 15, 1);
	}
	NPCs *n = R->NPC[0];
	n->FallingPickDistance = 0;
	entity_type(n->Collider, HIT_PLAYER, 0);
	if (entity_y(n->Collider, 0) > .35f) {
		AnimateNPC(n, 1, 10, .12f, 0);
		float dist = entity_distance(Collider, n->Collider);
		if (dist < .8f) { // push the player out from under it
			float d = point_direction(entity_x(Collider, 1), entity_z(Collider, 1), entity_x(n->Collider, 1), entity_z(n->Collider, 1));
			translate_entity(Collider, cosf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0, sinf((-d + 90) * DTOR) * (dist - .8f) * (dist - .8f), 0);
		}
		if (entity_y(n->Collider, 0) > .6f) entity_type(n->Collider, 0, 0);
		return 0;
	}
	e->EventState += FPSfactor;
	AnimateNPC(n, 11, 19, .25f, 0);
	if (!e->Sound) {
		LoadEventSound(e, "SFX\\General\\BodyFall.ogg", 0);
		PlaySound_Strict(e->Sound);
		Decals *de = CreateDecal(0, entity_x(R->obj, 0), .001f, entity_z(R->obj, 0), 90, irand(360, 1), 0);
		if (de) { de->Size = .4f; entity_alpha(de->obj, .8f); UpdateDecals(); }
	}
	if (e->EventState > 400) { if (e->Sound) { FreeSound_Strict(e->Sound); e->Sound = 0; } RemoveEvent(e); }
	return 0;
}

static void pipesdecal(Events *e, int o, float yaw) {
	Decals *d = CreateDecal(0, XG(o), YG(o), ZG(o), 0, yaw, frnd(360, 0));
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
		position_entity(n->Collider, XG(o0), entity_y(Collider, 0) - .15f, ZG(o0), 0);
		point_entity(n->Collider, o1, 0);
		move_entity(n->Collider, 0, 0, entity_distance(o0, o1) * .5f * (s / 50));
		AnimateNPC(n, 284, 333, .02f * 35, 1);
	} else if (s < 200) { // look around
		n->Idle = 1;
		AnimateNPC(n, 334, 494, .2f, 1);
		position_entity(n->Collider, (XG(o0) + XG(o1)) / 2, entity_y(Collider, 0) - .15f, (ZG(o0) + ZG(o1)) / 2, 0);
		rotate_entity(n->Collider, 0, CurveValue(s, entity_yaw(n->Collider, 0), 30), 0, 1);
		if (entity_distance(n->Collider, Collider) < 4) {
			int pvt = create_pivot(0);
			position_entity(pvt, entity_x(n->Collider, 0), entity_y(n->Collider, 0), entity_z(n->Collider, 0), 0);
			point_entity(pvt, Collider, 0);
			if (WrapAngle(entity_yaw(pvt, 0) - entity_yaw(n->Collider, 0)) < 80) { // it saw you
				n->State = -11; n->Idle = 0;
				PlaySound_Strict(HorrorSFX[10]);
				e->EventState = 260;
			}
			free_entity(pvt);
		}
	} else if (s < 250) { // leave
		n->Idle = 1;
		position_entity(n->Collider, XG(o0), entity_y(Collider, 0) - .15f, ZG(o0), 0);
		point_entity(n->Collider, o1, 0);
		move_entity(n->Collider, 0, 0, entity_distance(o0, o1) * ((s - 150) / 100));
		AnimateNPC(n, 284, 333, .02f * 35, 1);
	}
	reset_entity(n->Collider);
	position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .15f, entity_z(n->Collider, 0), 0);
	rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0), 0, 0);
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
		if (IN && Curr173->Idle < 2 && !s_len(e->EventStr) && QuickLoadPercent == -1) { QuickLoadPercent = 0; QuickLoad_CurrEvent = e; e->EventStr = "load0"; }
		return 0;
	}
	NPCs *n0 = R->NPC[0], *n1 = R->NPC[1];
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s < 70 * 3.5f) {
		rotate_entity(n1->Collider, 0, CurveAngle(R->angle + 90, entity_yaw(n1->Collider, 0), 100), 0, 1);
		n0->State = 1;
		if (s > 70 * 3.2f && s - FPSfactor <= 70 * 3.2f) PlaySound2(IntroSFX[15], Camera, R->obj, 15, 1);
	} else if (s < 70 * 6.5f) {
		if (s - FPSfactor < 70 * 3.5f) { n0->State = 0; n1->SoundChn = PlaySound2(n1->Sound, Camera, n1->Collider, 12, 1); }
		if (s > 70 * 4.5f) {
			point_entity(n0->obj, R->obj, 0);
			rotate_entity(n0->Collider, 0, CurveAngle(entity_yaw(n0->obj, 0), entity_yaw(n0->Collider, 0), 30), 0, 1);
		}
		point_entity(n1->obj, R->obj, 0);
		turn_entity(n1->obj, 0, sinf(s * DTOR) * 25, 0, 0);
		rotate_entity(n1->Collider, 0, CurveAngle(entity_yaw(n1->obj, 0), entity_yaw(n1->Collider, 0), 30), 0, 1);
	} else {
		if (s - FPSfactor < 70 * 6.5f) { PlaySound_Strict(HorrorSFX[0]); PlaySound_Strict(LightSFX); }
		BlinkTimer = Max((70 * 6.5f - s) / 5 - frnd(0, 2), -10);
		if (BlinkTimer == -10) {
			if (s > 70 * 7.5f && s - FPSfactor <= 70 * 7.5f) {
				PlaySound2(NeckSnapSFX[0], Camera, n0->Collider, 8, 1);
				// the wallet w/ 2 quarters
				Items *it = CreateItem("Wallet", "wallet", entity_x(n0->Collider, 1), entity_y(n0->Collider, 1), entity_z(n0->Collider, 1), 0, 0, 0, 1, 0);
				entity_type(it->collider, HIT_ITEM, 0);
				point_entity(it->collider, n1->Collider, 0);
				move_entity(it->collider, -.4f, 0, -.2f);
				TeleportEntity(it->collider, entity_x(it->collider, 0), entity_y(it->collider, 0), entity_z(it->collider, 0), -.02f, 1, 10, 0);
				for (int i = 0; i <= 1; i++) {
					Items *q = CreateItem("Quarter", "25ct", 1, 1, 1, 0, 0, 0, 1, 0);
					q->Picked = 1; q->Dropped = -1; q->itemtemplate->found = 1;
					it->SecondInv[i] = q;
					hide_entity(q->collider);
					entity_type(q->collider, HIT_ITEM, 0);
				}
			}
			if (s > 70 * 8 && s - FPSfactor <= 70 * 8) PlaySound2(NeckSnapSFX[1], Camera, n1->Collider, 8, 1);
			SetNPCFrame(n0, 60); n0->State = 8;
			SetNPCFrame(n1, 19); n1->State = 6;
		}
		if (s > 70 * 8.5f) {
			int o0 = R->Objects[0], o1 = R->Objects[1];
			position_entity(Curr173->Collider, (XG(o0) + XG(o1)) / 2, YG(o0), (ZG(o0) + ZG(o1)) / 2, 0);
			point_entity(Curr173->Collider, Collider, 0);
			reset_entity(Curr173->Collider);
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
			else if (n->State <= -10 && entity_distance(n->Collider, Collider) > 5 && !entity_in_view(n->obj, Camera)) { e->EventState = 1; e->EventState2 = 1; }
		} else if (Contained106) RemoveEvent(e);
		return 0;
	}
	if (e->EventState == 1) {
		if (R->dist < 3 || irand(7000, 1) == 1) {
			e->EventState = 2;
			Decals *d = CreateDecal(0, entity_x(R->obj, 0), 445 * RS, entity_z(R->obj, 0), -90, irand(360, 1), 0);
			if (d) {
				d->Size = frnd(.5f, .7f); entity_alpha(d->obj, .7f); d->ID = 1; scale_sprite(d->obj, d->Size, d->Size);
				entity_alpha(d->obj, frnd(.7f, .85f));
			}
			PlaySound_Strict(HorrorSFX[10]);
		} else if (R->dist > 8) {
			if (irand(5, 1) != 1) n->State = -10000;
			n->Idle = 0;
			RemoveEvent(e);
		}
		return 0;
	}
	if (e->EventState2 == 1) ShouldPlay = 10;
	e->EventState += FPSfactor;
	float s = e->EventState;
	if (s <= 180) {
		position_entity(n->Collider, XG(R->obj), entity_y(Collider, 0) + 1 - Min(sinf(s * DTOR) * 1.5f, 1.1f), ZG(R->obj), 1);
		point_entity(n->Collider, Camera, 0);
		AnimateNPC(n, 55, 104, .1f, 1);
		n->Idle = 1; n->State = 1;
		reset_entity(n->Collider);
		n->DropSpeed = 0;
		position_entity(n->obj, entity_x(n->Collider, 0), entity_y(n->Collider, 0) - .15f, entity_z(n->Collider, 0), 0);
		rotate_entity(n->obj, 0, entity_yaw(n->Collider, 0), 0, 0);
		show_entity(n->obj);
	} else if (s < 300) {
		n->Idle = 0; n->State = -10;
		position_entity(n->Collider, XG(R->obj), -3, ZG(R->obj), 1);
		n->PathTimer = 70 * 10; n->PathStatus = 0; n->PathLocation = 0;
		Decals *de = CreateDecal(0, XG(R->obj), .01f, ZG(R->obj), 90, irand(360, 1), 0);
		if (de) { de->Size = .05f; de->SizeChange = .01f; entity_alpha(de->obj, .8f); UpdateDecals(); }
		e->EventState = 300;
	} else if (s < 800) {
		if (entity_y(n->Collider, 0) >= entity_y(Collider, 0) - .05f) RemoveEvent(e);
		else {
			translate_entity(n->Collider, 0, (entity_y(Collider, 1) - .11f - entity_y(n->Collider, 0)) / 50, 0, 0);
			if (entity_y(n->Collider, 0) < -.1f) n->CurrSpeed = 0;
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
	if (s_eq(s, "load0")) {
		QuickLoadPercent = 10;
		if (!llive(R->NPC[0])) R->NPC[0] = CreateNPC(NPCtypeD, XG(R->Objects[0]), YG(R->Objects[0]), ZG(R->Objects[0]));
		ChangeNPCTextureID(R->NPC[0], 4);
		e->EventStr = "load1";
	} else if (s_eq(s, "load1")) { QuickLoadPercent = 20; R->NPC[0]->Sound = LoadSound_Strict("SFX\\Room\\Storeroom\\Escape1.ogg"); e->EventStr = "load2"; }
	else if (s_eq(s, "load2")) { QuickLoadPercent = 35; R->NPC[0]->SoundChn = PlaySound2(R->NPC[0]->Sound, Camera, R->NPC[0]->Collider, 12, 1); e->EventStr = "load3"; }
	else if (s_eq(s, "load3")) {
		QuickLoadPercent = 55;
		if (!llive(R->NPC[1])) R->NPC[1] = CreateNPC(NPCtypeD, XG(R->Objects[1]), YG(R->Objects[1]), ZG(R->Objects[1]));
		ChangeNPCTextureID(R->NPC[1], 2);
		e->EventStr = "load4";
	} else if (s_eq(s, "load4")) { QuickLoadPercent = 80; R->NPC[1]->Sound = LoadSound_Strict("SFX\\Room\\Storeroom\\Escape2.ogg"); e->EventStr = "load5"; }
	else if (s_eq(s, "load5")) {
		QuickLoadPercent = 100;
		point_entity(R->NPC[0]->Collider, R->NPC[1]->Collider, 0);
		point_entity(R->NPC[1]->Collider, R->NPC[0]->Collider, 0);
		e->EventState = 1;
	}
}

static void QuickLoadEvents(void) {
	Events *e = llive(QuickLoad_CurrEvent);
	if (!e) { QuickLoadPercent = -1; return; }
	if (s_eq(e->EventName, "room2closets")) ql_closets(e);
	// TODO room2sl, room3storage, room049, room205, room860, room966, dimension1499 (they come w/ their events)
}

void DrawQuickLoading(void) {
	if (QuickLoadPercent <= -1) { QuickLoadPercent = -1; QuickLoadPercent_DisplayTimer = 0; QuickLoad_CurrEvent = NULL; return; }
	mid_handle(QuickLoadIcon);
	draw_image(QuickLoadIcon, GraphicWidth - 90, GraphicHeight - 150, 0);
	set_color(255, 255, 255);
	AASetFont(Font1);
	AAText(GraphicWidth - 100, GraphicHeight - 90, cat("LOADING: ", s_str(QuickLoadPercent), "%"), 1, 0);
	if (QuickLoadPercent > 99) {
		if (QuickLoadPercent_DisplayTimer < 70) QuickLoadPercent_DisplayTimer = Min(QuickLoadPercent_DisplayTimer + FPSfactor, 70);
		else QuickLoadPercent = -1;
	}
	QuickLoadEvents();
}
