// use.c: DrawGUI's "item in hand" part (Main.bb 5606-7092): reading documents, wearing things, pills, kits...
// TODO still: radio, navigator, scp1499, the 294 cup, scp1025, and the clipboard/wallet (OtherOpen) screen
#include "g.h"

#define SI SelectedItem
static int is(const char *t) { return s_eq(SI->itemtemplate->tempname, t); }
static void say(const char *m, int t) { sset(&Msg, (str)m); MsgTimer = t; }
static void center(int img) { draw_image(img, GraphicWidth / 2 - image_width(img) / 2, GraphicHeight / 2 - image_height(img) / 2, 0); }
static str quote(const char *s) { return cat("\"", s, "\""); }
static void nostalgia(int lo, int hi) { PlaySound_Strict(LoadTempSound(sf("SFX\\SCP\\1162\\NostalgiaCancer%d.ogg", irand(lo, hi)))); }

// the progress bar a kit/vest/suit shows while you put it on
static void progress(float per) {
	center(SI->itemtemplate->invimg);
	int w = 300, h = 20, x = GraphicWidth / 2 - w / 2, y = GraphicHeight / 2 + 80;
	draw_rect(x, y, w + 4, h, 0);
	for (int i = 1; i <= iround((w - 2) * (SI->state / 100) / 10); i++) draw_image(BlinkMeterIMG, x + 3 + 10 * (i - 1), y + 3, 0);
	SI->state = Min(SI->state + FPSfactor / per, 100);
}

// nv goggles: kind 1 normal, 2 super, 3 fine
static void goggles(int kind) {
	if (Wearing1499 == 0 && WearingHazmat == 0) {
		if (WearingNightVision == kind) { sset(&Msg, "You removed the goggles."); CameraFogFar = StoredCameraFogFar; }
		else { sset(&Msg, "You put on the goggles."); WearingGasMask = 0; WearingNightVision = 0; StoredCameraFogFar = CameraFogFar; CameraFogFar = 30; }
		WearingNightVision = !WearingNightVision * kind;
	} else if (Wearing1499 > 0) sset(&Msg, "You need to take off SCP-1499 in order to put on the goggles.");
	else sset(&Msg, "You need to take off the hazmat suit in order to put on the goggles.");
	SI = NULL; MsgTimer = 70 * 5;
}

static int touch1123(void) { // returns 1 for blitz's Return
	if (Wearing714 == 1) return 0;
	if (!s_eq(PlayerRoom->RoomTemplate->Name, "room1123")) {
		show_entity(Light); LightFlash = 7;
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg"));
		sset(&DeathMSG, "Subject D-9341 was shot dead after attempting to attack a member of Nine-Tailed Fox. Surveillance tapes show that the subject had been "
			"wandering around the site approximately 9 minutes prior, shouting the phrase \"get rid of the four pests\""
			" in chinese. SCP-1123 was found in [REDACTED] nearby, suggesting the subject had come into physical contact with it. How "
			"exactly SCP-1123 was removed from its containment chamber is still unknown.");
		Kill();
		return 1;
	}
	Events *e = FindEvent("room1123");
	if (e) {
		if (e->EventState == 0) { show_entity(Light); LightFlash = 3; PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg")); }
		e->EventState = Max(1, e->EventState);
	}
	return 0;
}

static void scp500(void) {
	if (!CanUseItem(0, 0, 1)) return;
	GiveAchievement(Achv500, 1);
	say(Infect > 0 ? "You swallowed the pill. Your nausea is fading." : "You swallowed the pill.", 70 * 7);
	DeathTimer = 0; Infect = 0; Stamina = 100;
	for (int i = 0; i <= 5; i++) SCP1025state[i] = 0;
	if (StaminaEffect > 1) { StaminaEffect = 1; StaminaEffectTimer = 0; }
	RemoveItem(SI); SI = NULL;
}

static void veryfinekit(void) {
	if (!CanUseItem(0, 0, 1)) return;
	switch (irand(1, 5)) {
	case 1: Injuries = 3.5f; say("You started bleeding heavily.", 70 * 7); break;
	case 2: Injuries = 0; Bloodloss = 0; say("Your wounds are healing up rapidly.", 70 * 7); break;
	case 3: Injuries = Max(0, Injuries - frnd(.5f, 3.5f)); Bloodloss = Max(0, Bloodloss - frnd(10, 100)); say("You feel much better.", 70 * 7); break;
	case 4: BlurTimer = 10000; Bloodloss = 0; say("You feel nauseated.", 70 * 7); break;
	case 5: {
		BlinkTimer = -10;
		str rn = PlayerRoom->RoomTemplate->Name;
		if (s_eq(rn, "dimension1499") || s_eq(rn, "gatea") || (s_eq(rn, "exit1") && entity_y(Collider, 0) > 1040 * RoomScale)) {
			Injuries = 2.5f; say("You started bleeding heavily.", 70 * 7);
		} else {
			each(Rooms, r) if (s_eq(r->RoomTemplate->Name, "pocketdimension")) {
				position_entity(Collider, entity_x(r->obj, 0), .8f, entity_z(r->obj, 0), 0);
				reset_entity(Collider);
				UpdateDoors(); UpdateRooms();
				PlaySound_Strict(Use914SFX);
				DropSpeed = 0;
				Curr106->State = -2500;
				break;
			}
			say("For some inexplicable reason, you find yourself inside the pocket dimension.", 70 * 8);
		}
	}
	}
	RemoveItem(SI);
}

static void firstaid(void) {
	if (Bloodloss == 0 && Injuries == 0) { say("You do not need to use a first aid kit right now.", 70 * 5); SI = NULL; return; }
	if (!CanUseItem(0, 1, 1)) return;
	CurrSpeed = CurveValue(0, CurrSpeed, 5);
	Crouch = 1;
	progress(5);
	if (SI->state != 100) return;
	if (is("finefirstaid")) {
		Bloodloss = 0;
		Injuries = Max(0, Injuries - 2);
		if (Injuries == 0) sset(&Msg, "You bandaged the wounds and took a painkiller. You feel fine.");
		else if (Injuries > 1) sset(&Msg, "You bandaged the wounds and took a painkiller, but you were not able to stop the bleeding.");
		else sset(&Msg, "You bandaged the wounds and took a painkiller, but you still feel sore.");
		MsgTimer = 70 * 5;
		RemoveItem(SI);
		return;
	}
	Bloodloss = Max(0, Bloodloss - irand(10, 20));
	if (Injuries >= 2.5f) { sset(&Msg, "The wounds were way too severe to staunch the bleeding completely."); Injuries = Max(2.5f, Injuries - frnd(.3f, .7f)); }
	else if (Injuries > 1) {
		Injuries = Max(.5f, Injuries - frnd(.5f, 1));
		sset(&Msg, Injuries > 1 ? "You bandaged the wounds but were unable to staunch the bleeding completely." : "You managed to stop the bleeding.");
	} else { sset(&Msg, Injuries > .5f ? "You took a painkiller, easing the pain slightly." : "You took a painkiller, but it still hurts to walk."); Injuries = .5f; }
	if (is("firstaid2")) switch (irand(1, 6)) {
		case 1: SuperMan = 1; sset(&Msg, "You have becomed overwhelmedwithadrenalineholyshitWOOOOOO~!"); break;
		case 2: InvertMouse = !InvertMouse; sset(&Msg, "You suddenly find it very difficult to turn your head."); break;
		case 3: BlurTimer = 5000; sset(&Msg, "You feel nauseated."); break;
		case 4: BlinkEffect = .6f; BlinkEffectTimer = irand(20, 30); break;
		case 5: Bloodloss = 0; Injuries = 0; sset(&Msg, "You bandaged the wounds. The bleeding stopped completely and you feel fine."); break;
		case 6: sset(&Msg, "You bandaged the wounds and blood started pouring heavily through the bandages."); Injuries = 3.5f; break;
	}
	MsgTimer = 70 * 5;
	RemoveItem(SI);
}

static void eyedrops(float effect, int lo, int hi, float blood) {
	if (!CanUseItem(0, 0, 0)) return;
	if (Wearing714 != 1) { BlinkEffect = effect; BlinkEffectTimer = irand(lo, hi); Bloodloss = blood ? Max(Bloodloss - blood, 0) : Bloodloss; BlurTimer = 200; }
	RemoveItem(SI);
}

static void loaddoc(void) { // paper/ticket: build the readable image the first time
	ItemTemplates *t = SI->itemtemplate;
	str n = t->name;
	if (s_eq(n, "Burnt Note")) { // the 0x code written over it
		t->img = LoadImage_Strict("GFX\\items\\bn.it");
		set_buffer(image_buffer(t->img, 0));
		set_color(0, 0, 0); AAText(277, 469, s_str(AccessCode), 1, 1); set_color(255, 255, 255);
		set_buffer(back_buffer());
	} else if (s_eq(n, "Document SCP-372")) {
		t->img = LoadImage_Strict(t->imgpath);
		t->img = ResizeImage2(t->img, iround(image_width(t->img) * MenuScale), iround(image_height(t->img) * MenuScale));
		set_buffer(image_buffer(t->img, 0));
		set_color(37, 45, 137); AASetFont(Font5);
		int temp = AccessCode * 3 % 10000;
		if (temp < 1000) temp += 1000;
		AAText(iround(383 * MenuScale), iround(734 * MenuScale), s_str(temp), 1, 1);
		set_color(255, 255, 255);
		set_buffer(back_buffer());
	} else if (s_eq(n, "Movie Ticket")) { // not resized, it'd break the masking
		t->img = LoadImage_Strict(t->imgpath);
		if (SI->state == 0) {
			sset(&Msg, quote("Hey, I remember this movie!")); MsgTimer = 70 * 10;
			nostalgia(1, 5);
			SI->state = 1;
		}
	} else {
		t->img = LoadImage_Strict(t->imgpath);
		t->img = ResizeImage2(t->img, iround(image_width(t->img) * MenuScale), iround(image_height(t->img) * MenuScale));
	}
	mask_image(t->img, 255, 0, 255);
}

static void cigarette(void) {
	if (!CanUseItem(0, 0, 1)) return;
	if (SI->state == 0) {
		int gone = 0;
		switch (irand(1, 6)) {
		case 1: sset(&Msg, quote("I don't have anything to light it with. Umm, what about that... Nevermind.")); break;
		case 2: sset(&Msg, "You are unable to get lit."); break;
		case 3: sset(&Msg, quote("I quit that a long time ago.")); gone = 1; break;
		case 4: sset(&Msg, quote("Even if I wanted one, I have nothing to light it with.")); break;
		case 5: sset(&Msg, quote("Could really go for one now... Wish I had a lighter.")); break;
		case 6: sset(&Msg, quote("Don't plan on starting, even at a time like this.")); gone = 1; break;
		}
		if (gone) RemoveItem(SI);
		else SI->state = 1;
	} else sset(&Msg, "You are unable to get lit.");
	MsgTimer = 70 * 5;
}

static void weed(int strong) {
	if (!CanUseItem(0, 0, 1)) return;
	if (Wearing714 == 1) sset(&Msg, quote("DUDE WTF THIS SHIT DOESN'T EVEN WORK"));
	else if (!strong) {
		sset(&Msg, quote("MAN DATS SUM GOOD ASS SHIT"));
		Injuries = Max(Injuries - .5f, 0);
		BlurTimer = 500;
		GiveAchievement(Achv420, 1);
		PlaySound_Strict(LoadTempSound("SFX\\Music\\420J.ogg"));
	} else {
		sset(&DeathMSG, "Subject D-9341 found in a comatose state in [DATA REDACTED]. The subject was holding what appears to be a cigarette while smiling widely. "
			"Chemical analysis of the cigarette has been inconclusive, although it seems to contain a high concentration of an unidentified chemical "
			"whose molecular structure is remarkably similar to that of tetrahydrocannabinol.");
		sset(&Msg, quote("UH WHERE... WHAT WAS I DOING AGAIN... MAN I NEED TO TAKE A NAP..."));
		KillTimer = -1;
	}
	MsgTimer = strong ? 70 * 6 : 70 * 5;
	RemoveItem(SI);
}

static void hazmat(void) {
	if (WearingVest != 0) return;
	CurrSpeed = CurveValue(0, CurrSpeed, 5);
	progress(4);
	if (SI->state != 100) return;
	if (WearingHazmat > 0) { sset(&Msg, "You removed the hazmat suit."); WearingHazmat = 0; DropItem(SI, 1); }
	else {
		WearingHazmat = is("hazmatsuit") ? 1 : is("hazmatsuit2") ? 2 : 3;
		if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
		sset(&Msg, "You put on the hazmat suit.");
		if (WearingNightVision) CameraFogFar = StoredCameraFogFar;
		WearingGasMask = 0; WearingNightVision = 0;
	}
	SI->state = 0; MsgTimer = 70 * 5; SI = NULL;
}

static void vest(void) {
	CurrSpeed = CurveValue(0, CurrSpeed, 5);
	progress(2 + .5f * is("finevest"));
	if (SI->state != 100) return;
	if (WearingVest > 0) { sset(&Msg, "You removed the vest."); WearingVest = 0; DropItem(SI, 1); }
	else {
		if (is("vest")) { sset(&Msg, "You put on the vest and feel slightly encumbered."); WearingVest = 1; }
		else { sset(&Msg, "You put on the vest and feel heavily encumbered."); WearingVest = 2; }
		if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
	}
	SI->state = 0; MsgTimer = 70 * 5; SI = NULL;
}

static void gasmask(void) {
	if (Wearing1499 == 0 && WearingHazmat == 0) {
		if (WearingGasMask) sset(&Msg, "You removed the gas mask.");
		else {
			sset(&Msg, is("supergasmask") ? "You put on the gas mask and you can breathe easier." : "You put on the gas mask.");
			if (WearingNightVision) CameraFogFar = StoredCameraFogFar;
			WearingNightVision = 0; WearingGasMask = 0;
		}
		if (is("gasmask3")) WearingGasMask = WearingGasMask == 0 ? 3 : 0;
		else if (is("supergasmask")) WearingGasMask = WearingGasMask == 0 ? 2 : 0;
		else WearingGasMask = !WearingGasMask;
	} else if (Wearing1499 > 0) sset(&Msg, "You need to take off SCP-1499 in order to put on the gas mask.");
	else sset(&Msg, "You need to take off the hazmat suit in order to put on the gas mask.");
	SI = NULL; MsgTimer = 70 * 5;
}

static void readable(int resize) { // badge/oldpaper: load + mask the image once, then show it
	ItemTemplates *t = SI->itemtemplate;
	if (!t->img) {
		t->img = LoadImage_Strict(t->imgpath);
		if (resize) t->img = ResizeImage2(t->img, iround(image_width(t->img) * MenuScale), iround(image_height(t->img) * MenuScale));
		mask_image(t->img, 255, 0, 255);
	}
	center(t->img);
}

static void syringe(int fine) {
	if (!CanUseItem(0, 1, 1)) return;
	if (fine == 0) { HealTimer = 30; StaminaEffect = .5f; StaminaEffectTimer = 20; sset(&Msg, "You injected yourself with the syringe and feel a slight adrenaline rush."); }
	else if (fine == 1) {
		HealTimer = frnd(20, 40); StaminaEffect = frnd(.5f, .8f); StaminaEffectTimer = frnd(20, 30);
		sset(&Msg, "You injected yourself with the syringe and feel an adrenaline rush.");
	} else switch (irand(1, 3)) {
		case 1: HealTimer = frnd(40, 60); StaminaEffect = .1f; StaminaEffectTimer = 30; sset(&Msg, "You injected yourself with the syringe and feel a huge adrenaline rush."); break;
		case 2: SuperMan = 1; sset(&Msg, "You injected yourself with the syringe and feel a humongous adrenaline rush."); break;
		case 3: VomitTimer = 30; sset(&Msg, "You injected yourself with the syringe and feel a pain in your stomach."); break;
	}
	MsgTimer = 70 * 8;
	RemoveItem(SI);
}

static void select_(void) { // the per-item Select. SelectedItem is non-NULL here
	if (is("nvgoggles")) goggles(1);
	else if (is("supernv")) goggles(2);
	else if (is("finenvgoggles")) goggles(3);
	else if (is("ring")) {
		if (Wearing714 == 2) { sset(&Msg, "You removed the ring."); Wearing714 = 0; }
		else { sset(&Msg, "You put on the ring."); Wearing714 = 2; }
		MsgTimer = 70 * 5; SI = NULL;
	} else if (is("battery")) {}
	else if (is("key1") || is("key2") || is("key3") || is("key4") || is("key5") || is("key6") || is("keyomni") || is("scp860")
		|| is("hand") || is("hand2") || is("25ct")) center(SI->itemtemplate->invimg);
	else if (is("scp513")) {
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\513\\Bell1.ogg"));
		if (!llive(Curr5131)) Curr5131 = CreateNPC(NPCtype5131, 0, 0, 0);
		SI = NULL;
	} else if (is("scp500")) scp500();
	else if (is("veryfinefirstaid")) veryfinekit();
	else if (is("firstaid") || is("finefirstaid") || is("firstaid2")) firstaid();
	else if (is("eyedrops")) eyedrops(.6f, 20, 30, 0);
	else if (is("fineeyedrops")) eyedrops(.4f, 30, 40, 1);
	else if (is("supereyedrops")) {
		if (CanUseItem(0, 0, 0)) {
			if (Wearing714 != 1) { BlinkEffect = 0; BlinkEffectTimer = 60; EyeStuck = 10000; }
			BlurTimer = 1000;
			RemoveItem(SI);
		}
	} else if (is("paper") || is("ticket")) { if (!SI->itemtemplate->img) loaddoc(); center(SI->itemtemplate->img); }
	else if (is("syringe")) syringe(0);
	else if (is("finesyringe")) syringe(1);
	else if (is("veryfinesyringe")) syringe(2);
	else if (is("cigarette")) cigarette();
	else if (is("420")) weed(0);
	else if (is("420s")) weed(1);
	else if (is("scp714")) {
		if (Wearing714 == 1) { sset(&Msg, "You removed the ring."); Wearing714 = 0; }
		else { GiveAchievement(Achv714, 1); sset(&Msg, "You put on the ring."); Wearing714 = 1; }
		MsgTimer = 70 * 5; SI = NULL;
	} else if (is("hazmatsuit") || is("hazmatsuit2") || is("hazmatsuit3")) hazmat();
	else if (is("vest") || is("finevest")) vest();
	else if (is("gasmask") || is("supergasmask") || is("gasmask3")) gasmask();
	else if (is("badge")) {
		readable(0);
		if (SI->state == 0) {
			nostalgia(6, 10);
			if (s_eq(SI->itemtemplate->name, "Old Badge")) { sset(&Msg, quote("Huh? This guy looks just like me!")); MsgTimer = 70 * 10; }
			SI->state = 1;
		}
	} else if (is("key")) {
		if (SI->state == 0) { nostalgia(6, 10); sset(&Msg, quote("Isn't this the key to that old shack? The one where I... No, it can't be.")); MsgTimer = 70 * 10; }
		SI->state = 1; SI = NULL;
	} else if (is("oldpaper")) {
		readable(1);
		if (SI->state == 0 && s_eq(SI->itemtemplate->name, "Disciplinary Hearing DH-S-4137-17092")) {
			BlurTimer = 1000;
			sset(&Msg, quote("Why does this seem so familiar?")); MsgTimer = 70 * 10;
			nostalgia(6, 10);
			SI->state = 1;
		}
	} else if (is("coin")) {
		if (SI->state == 0) nostalgia(1, 5);
		sset(&Msg, "");
		SI->state = 1;
		center(SI->itemtemplate->invimg);
	} else if (is("scp427")) {
		if (I_427->Using == 1) { sset(&Msg, "You closed the locket."); I_427->Using = 0; }
		else { GiveAchievement(Achv427, 1); sset(&Msg, "You opened the locket."); I_427->Using = 1; }
		MsgTimer = 70 * 5; SI = NULL;
	} else if (is("pill")) {
		if (CanUseItem(0, 0, 1)) { say("You swallowed the pill.", 70 * 7); RemoveItem(SI); SI = NULL; }
	} else if (is("scp500death")) {
		if (CanUseItem(0, 0, 1)) {
			say("You swallowed the pill.", 70 * 7);
			if (I_427->Timer < 70 * 360) I_427->Timer = 70 * 360;
			RemoveItem(SI); SI = NULL;
		}
	}
	// TODO radio, 18vradio, fineradio, veryfineradio, navigator/nav, scp1499/super1499, cup, scp1025,
	// and Default: items w/ invSlots open the clipboard/wallet screen (OtherOpen)
}

static int isdoc(Items *it) { str t = it->itemtemplate->tempname; return s_eq(t, "paper") || s_eq(t, "badge") || s_eq(t, "oldpaper") || s_eq(t, "ticket"); }

// returns 1 for blitz's Return out of DrawGUI
int UseItem(void) {
	if (is("1123")) { if (touch1123()) return 1; }
	else select_();
	if (SI && SI->itemtemplate->img && isdoc(SI)) // only one document image stays loaded
		each(Items, a) if (a != SI && isdoc(a) && a->itemtemplate->img && a->itemtemplate->img != SI->itemtemplate->img) { free_image(a->itemtemplate->img); a->itemtemplate->img = 0; }
	if (!SI || !MouseHit2) return 0;
	entity_alpha(Dark, 0); // right click puts it away
	if (is("scp1025")) { if (SI->itemtemplate->img) free_image(SI->itemtemplate->img); SI->itemtemplate->img = 0; }
	else if (is("firstaid") || is("finefirstaid") || is("firstaid2")) SI->state = 0;
	else if (is("vest") || is("finevest")) { SI->state = 0; if (!WearingVest) DropItem(SI, 0); }
	else if (is("hazmatsuit") || is("hazmatsuit2") || is("hazmatsuit3")) { SI->state = 0; if (!WearingHazmat) DropItem(SI, 0); }
	else if (is("scp1499") || is("super1499")) SI->state = 0;
	if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
	SI = NULL;
	return 0;
}
