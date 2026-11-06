// use.c: DrawGUI's "item in hand" part (Main.bb 5606-7092): reading documents, wearing things, pills, kits...
// TODO still: radio, navigator, scp1499, the 294 cup, scp1025, and the clipboard/wallet (OtherOpen) screen
#include "g.h"

#define SI SelectedItem
static int is(const char *t) { return bbSeq(SI->itemtemplate->tempname, t); }
static void say(const char *m, int t) { Msg = (str)m; MsgTimer = t; }
static void center(int img) { bbDrawImage(img, GraphicWidth / 2 - bbImageWidth(img) / 2, GraphicHeight / 2 - bbImageHeight(img) / 2, 0); }
static str quote(const char *s) { return cat("\"", s, "\""); }
static void nostalgia(int lo, int hi) { PlaySound_Strict(LoadTempSound(sf("SFX\\SCP\\1162\\NostalgiaCancer%d.ogg", bbRand(lo, hi)))); }

// the progress bar a kit/vest/suit shows while you put it on
static void progress(float per) {
	center(SI->itemtemplate->invimg);
	int w = 300, h = 20, x = GraphicWidth / 2 - w / 2, y = GraphicHeight / 2 + 80;
	bbRect(x, y, w + 4, h, 0);
	for (int i = 1; i <= bbInt((w - 2) * (SI->state / 100) / 10); i++) bbDrawImage(BlinkMeterIMG, x + 3 + 10 * (i - 1), y + 3, 0);
	SI->state = Min(SI->state + FPSfactor / per, 100);
}

// nv goggles: kind 1 normal, 2 super, 3 fine
static void goggles(int kind) {
	if (Wearing1499 == 0 && WearingHazmat == 0) {
		if (WearingNightVision == kind) { Msg = "You removed the goggles."; CameraFogFar = StoredCameraFogFar; }
		else { Msg = "You put on the goggles."; WearingGasMask = 0; WearingNightVision = 0; StoredCameraFogFar = CameraFogFar; CameraFogFar = 30; }
		WearingNightVision = !WearingNightVision * kind;
	} else if (Wearing1499 > 0) Msg = "You need to take off SCP-1499 in order to put on the goggles.";
	else Msg = "You need to take off the hazmat suit in order to put on the goggles.";
	SI = NULL; MsgTimer = 70 * 5;
}

static int touch1123(void) { // returns 1 for blitz's Return
	if (Wearing714 == 1) return 0;
	if (!bbSeq(PlayerRoom->RoomTemplate->Name, "room1123")) {
		bbShowEntity(Light); LightFlash = 7;
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg"));
		DeathMSG = "Subject D-9341 was shot dead after attempting to attack a member of Nine-Tailed Fox. Surveillance tapes show that the subject had been "
			"wandering around the site approximately 9 minutes prior, shouting the phrase \"get rid of the four pests\""
			" in chinese. SCP-1123 was found in [REDACTED] nearby, suggesting the subject had come into physical contact with it. How "
			"exactly SCP-1123 was removed from its containment chamber is still unknown.";
		Kill();
		return 1;
	}
	Events *e = FindEvent("room1123");
	if (e) {
		if (e->EventState == 0) { bbShowEntity(Light); LightFlash = 3; PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg")); }
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
	switch (bbRand(5, 1)) {
	case 1: Injuries = 3.5f; say("You started bleeding heavily.", 70 * 7); break;
	case 2: Injuries = 0; Bloodloss = 0; say("Your wounds are healing up rapidly.", 70 * 7); break;
	case 3: Injuries = Max(0, Injuries - bbRnd(.5f, 3.5f)); Bloodloss = Max(0, Bloodloss - bbRnd(10, 100)); say("You feel much better.", 70 * 7); break;
	case 4: BlurTimer = 10000; Bloodloss = 0; say("You feel nauseated.", 70 * 7); break;
	case 5: {
		BlinkTimer = -10;
		str rn = PlayerRoom->RoomTemplate->Name;
		if (bbSeq(rn, "dimension1499") || bbSeq(rn, "gatea") || (bbSeq(rn, "exit1") && bbEntityY(Collider, 0) > 1040 * RoomScale)) {
			Injuries = 2.5f; say("You started bleeding heavily.", 70 * 7);
		} else {
			each(Rooms, r) if (bbSeq(r->RoomTemplate->Name, "pocketdimension")) {
				bbPositionEntity(Collider, bbEntityX(r->obj, 0), .8f, bbEntityZ(r->obj, 0), 0);
				bbResetEntity(Collider);
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
		if (Injuries == 0) Msg = "You bandaged the wounds and took a painkiller. You feel fine.";
		else if (Injuries > 1) Msg = "You bandaged the wounds and took a painkiller, but you were not able to stop the bleeding.";
		else Msg = "You bandaged the wounds and took a painkiller, but you still feel sore.";
		MsgTimer = 70 * 5;
		RemoveItem(SI);
		return;
	}
	Bloodloss = Max(0, Bloodloss - bbRand(10, 20));
	if (Injuries >= 2.5f) { Msg = "The wounds were way too severe to staunch the bleeding completely."; Injuries = Max(2.5f, Injuries - bbRnd(.3f, .7f)); }
	else if (Injuries > 1) {
		Injuries = Max(.5f, Injuries - bbRnd(.5f, 1));
		Msg = Injuries > 1 ? "You bandaged the wounds but were unable to staunch the bleeding completely." : "You managed to stop the bleeding.";
	} else { Msg = Injuries > .5f ? "You took a painkiller, easing the pain slightly." : "You took a painkiller, but it still hurts to walk."; Injuries = .5f; }
	if (is("firstaid2")) switch (bbRand(6, 1)) {
		case 1: SuperMan = 1; Msg = "You have becomed overwhelmedwithadrenalineholyshitWOOOOOO~!"; break;
		case 2: InvertMouse = !InvertMouse; Msg = "You suddenly find it very difficult to turn your head."; break;
		case 3: BlurTimer = 5000; Msg = "You feel nauseated."; break;
		case 4: BlinkEffect = .6f; BlinkEffectTimer = bbRand(20, 30); break;
		case 5: Bloodloss = 0; Injuries = 0; Msg = "You bandaged the wounds. The bleeding stopped completely and you feel fine."; break;
		case 6: Msg = "You bandaged the wounds and blood started pouring heavily through the bandages."; Injuries = 3.5f; break;
	}
	MsgTimer = 70 * 5;
	RemoveItem(SI);
}

static void eyedrops(float effect, int lo, int hi, float blood) {
	if (!CanUseItem(0, 0, 0)) return;
	if (Wearing714 != 1) { BlinkEffect = effect; BlinkEffectTimer = bbRand(lo, hi); Bloodloss = blood ? Max(Bloodloss - blood, 0) : Bloodloss; BlurTimer = 200; }
	RemoveItem(SI);
}

static void loaddoc(void) { // paper/ticket: build the readable image the first time
	ItemTemplates *t = SI->itemtemplate;
	str n = t->name;
	if (bbSeq(n, "Burnt Note")) { // the 0x code written over it
		t->img = LoadImage_Strict("GFX\\items\\bn.it");
		bbSetBuffer(bbImageBuffer(t->img, 0));
		bbColor(0, 0, 0); AAText(277, 469, bbStr(AccessCode), 1, 1); bbColor(255, 255, 255);
		bbSetBuffer(bbBackBuffer());
	} else if (bbSeq(n, "Document SCP-372")) {
		t->img = LoadImage_Strict(t->imgpath);
		t->img = ResizeImage2(t->img, bbInt(bbImageWidth(t->img) * MenuScale), bbInt(bbImageHeight(t->img) * MenuScale));
		bbSetBuffer(bbImageBuffer(t->img, 0));
		bbColor(37, 45, 137); AASetFont(Font5);
		int temp = AccessCode * 3 % 10000;
		if (temp < 1000) temp += 1000;
		AAText(bbInt(383 * MenuScale), bbInt(734 * MenuScale), bbStr(temp), 1, 1);
		bbColor(255, 255, 255);
		bbSetBuffer(bbBackBuffer());
	} else if (bbSeq(n, "Movie Ticket")) { // not resized, it'd break the masking
		t->img = LoadImage_Strict(t->imgpath);
		if (SI->state == 0) {
			Msg = quote("Hey, I remember this movie!"); MsgTimer = 70 * 10;
			nostalgia(1, 5);
			SI->state = 1;
		}
	} else {
		t->img = LoadImage_Strict(t->imgpath);
		t->img = ResizeImage2(t->img, bbInt(bbImageWidth(t->img) * MenuScale), bbInt(bbImageHeight(t->img) * MenuScale));
	}
	bbMaskImage(t->img, 255, 0, 255);
}

static void cigarette(void) {
	if (!CanUseItem(0, 0, 1)) return;
	if (SI->state == 0) {
		int gone = 0;
		switch (bbRand(6, 1)) {
		case 1: Msg = quote("I don't have anything to light it with. Umm, what about that... Nevermind."); break;
		case 2: Msg = "You are unable to get lit."; break;
		case 3: Msg = quote("I quit that a long time ago."); gone = 1; break;
		case 4: Msg = quote("Even if I wanted one, I have nothing to light it with."); break;
		case 5: Msg = quote("Could really go for one now... Wish I had a lighter."); break;
		case 6: Msg = quote("Don't plan on starting, even at a time like this."); gone = 1; break;
		}
		Items *it = SI;
		if (gone) RemoveItem(it);
		it->state = 1; // blitz writes this through the now Null SelectedItem (a runtime error there); harmless on the removed item here
	} else Msg = "You are unable to get lit.";
	MsgTimer = 70 * 5;
}

static void weed(int strong) {
	if (!CanUseItem(0, 0, 1)) return;
	if (Wearing714 == 1) Msg = quote("DUDE WTF THIS SHIT DOESN'T EVEN WORK");
	else if (!strong) {
		Msg = quote("MAN DATS SUM GOOD ASS SHIT");
		Injuries = Max(Injuries - .5f, 0);
		BlurTimer = 500;
		GiveAchievement(Achv420, 1);
		PlaySound_Strict(LoadTempSound("SFX\\Music\\420J.ogg"));
	} else {
		DeathMSG = "Subject D-9341 found in a comatose state in [DATA REDACTED]. The subject was holding what appears to be a cigarette while smiling widely. "
			"Chemical analysis of the cigarette has been inconclusive, although it seems to contain a high concentration of an unidentified chemical "
			"whose molecular structure is remarkably similar to that of tetrahydrocannabinol.";
		Msg = quote("UH WHERE... WHAT WAS I DOING AGAIN... MAN I NEED TO TAKE A NAP...");
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
	if (WearingHazmat > 0) { Msg = "You removed the hazmat suit."; WearingHazmat = 0; DropItem(SI, 1); }
	else {
		WearingHazmat = is("hazmatsuit") ? 1 : is("hazmatsuit2") ? 2 : 3;
		if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
		Msg = "You put on the hazmat suit.";
		if (WearingNightVision) CameraFogFar = StoredCameraFogFar;
		WearingGasMask = 0; WearingNightVision = 0;
	}
	SI->state = 0; MsgTimer = 70 * 5; SI = NULL;
}

static void vest(void) {
	CurrSpeed = CurveValue(0, CurrSpeed, 5);
	progress(2 + .5f * is("finevest"));
	if (SI->state != 100) return;
	if (WearingVest > 0) { Msg = "You removed the vest."; WearingVest = 0; DropItem(SI, 1); }
	else {
		if (is("vest")) { Msg = "You put on the vest and feel slightly encumbered."; WearingVest = 1; }
		else { Msg = "You put on the vest and feel heavily encumbered."; WearingVest = 2; }
		if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
	}
	SI->state = 0; MsgTimer = 70 * 5; SI = NULL;
}

static void gasmask(void) {
	if (Wearing1499 == 0 && WearingHazmat == 0) {
		if (WearingGasMask) Msg = "You removed the gas mask.";
		else {
			Msg = is("supergasmask") ? "You put on the gas mask and you can breathe easier." : "You put on the gas mask.";
			if (WearingNightVision) CameraFogFar = StoredCameraFogFar;
			WearingNightVision = 0; WearingGasMask = 0;
		}
		if (is("gasmask3")) WearingGasMask = WearingGasMask == 0 ? 3 : 0;
		else if (is("supergasmask")) WearingGasMask = WearingGasMask == 0 ? 2 : 0;
		else WearingGasMask = !WearingGasMask;
	} else if (Wearing1499 > 0) Msg = "You need to take off SCP-1499 in order to put on the gas mask.";
	else Msg = "You need to take off the hazmat suit in order to put on the gas mask.";
	SI = NULL; MsgTimer = 70 * 5;
}

static void readable(int resize) { // badge/oldpaper: load + mask the image once, then show it
	ItemTemplates *t = SI->itemtemplate;
	if (!t->img) {
		t->img = LoadImage_Strict(t->imgpath);
		if (resize) t->img = ResizeImage2(t->img, bbInt(bbImageWidth(t->img) * MenuScale), bbInt(bbImageHeight(t->img) * MenuScale));
		bbMaskImage(t->img, 255, 0, 255);
	}
	center(t->img);
}

static void syringe(int fine) {
	if (!CanUseItem(0, 1, 1)) return;
	if (fine == 0) { HealTimer = 30; StaminaEffect = .5f; StaminaEffectTimer = 20; Msg = "You injected yourself with the syringe and feel a slight adrenaline rush."; }
	else if (fine == 1) {
		HealTimer = bbRnd(20, 40); StaminaEffect = bbRnd(.5f, .8f); StaminaEffectTimer = bbRnd(20, 30);
		Msg = "You injected yourself with the syringe and feel an adrenaline rush.";
	} else switch (bbRand(3, 1)) {
		case 1: HealTimer = bbRnd(40, 60); StaminaEffect = .1f; StaminaEffectTimer = 30; Msg = "You injected yourself with the syringe and feel a huge adrenaline rush."; break;
		case 2: SuperMan = 1; Msg = "You injected yourself with the syringe and feel a humongous adrenaline rush."; break;
		case 3: VomitTimer = 30; Msg = "You injected yourself with the syringe and feel a pain in your stomach."; break;
	}
	MsgTimer = 70 * 8;
	RemoveItem(SI);
}

static void select_(void) { // the per-item Select. SelectedItem is non-NULL here
	if (is("nvgoggles")) goggles(1);
	else if (is("supernv")) goggles(2);
	else if (is("finenvgoggles")) goggles(3);
	else if (is("ring")) {
		if (Wearing714 == 2) { Msg = "You removed the ring."; Wearing714 = 0; }
		else { Msg = "You put on the ring."; Wearing714 = 2; }
		MsgTimer = 70 * 5; SI = NULL;
	} else if (is("battery")) {}
	else if (is("key1") || is("key2") || is("key3") || is("key4") || is("key5") || is("key6") || is("keyomni") || is("scp860")
		|| is("hand") || is("hand2") || is("25ct")) center(SI->itemtemplate->invimg);
	else if (is("scp513")) {
		PlaySound_Strict(LoadTempSound("SFX\\SCP\\513\\Bell1.ogg"));
		if (!bbLive(Curr5131)) Curr5131 = CreateNPC(NPCtype5131, 0, 0, 0);
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
		if (Wearing714 == 1) { Msg = "You removed the ring."; Wearing714 = 0; }
		else { GiveAchievement(Achv714, 1); Msg = "You put on the ring."; Wearing714 = 1; }
		MsgTimer = 70 * 5; SI = NULL;
	} else if (is("hazmatsuit") || is("hazmatsuit2") || is("hazmatsuit3")) hazmat();
	else if (is("vest") || is("finevest")) vest();
	else if (is("gasmask") || is("supergasmask") || is("gasmask3")) gasmask();
	else if (is("badge")) {
		readable(0);
		if (SI->state == 0) {
			nostalgia(6, 10);
			if (bbSeq(SI->itemtemplate->name, "Old Badge")) { Msg = quote("Huh? This guy looks just like me!"); MsgTimer = 70 * 10; }
			SI->state = 1;
		}
	} else if (is("key")) {
		if (SI->state == 0) { nostalgia(6, 10); Msg = quote("Isn't this the key to that old shack? The one where I... No, it can't be."); MsgTimer = 70 * 10; }
		SI->state = 1; SI = NULL;
	} else if (is("oldpaper")) {
		readable(1);
		if (SI->state == 0 && bbSeq(SI->itemtemplate->name, "Disciplinary Hearing DH-S-4137-17092")) {
			BlurTimer = 1000;
			Msg = quote("Why does this seem so familiar?"); MsgTimer = 70 * 10;
			nostalgia(6, 10);
			SI->state = 1;
		}
	} else if (is("coin")) {
		if (SI->state == 0) nostalgia(1, 5);
		Msg = "";
		SI->state = 1;
		center(SI->itemtemplate->invimg);
	} else if (is("scp427")) {
		if (I_427->Using == 1) { Msg = "You closed the locket."; I_427->Using = 0; }
		else { GiveAchievement(Achv427, 1); Msg = "You opened the locket."; I_427->Using = 1; }
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

static int isdoc(Items *it) { str t = it->itemtemplate->tempname; return bbSeq(t, "paper") || bbSeq(t, "badge") || bbSeq(t, "oldpaper") || bbSeq(t, "ticket"); }

// returns 1 for blitz's Return out of DrawGUI
int UseItem(void) {
	if (is("1123")) { if (touch1123()) return 1; }
	else select_();
	if (SI && SI->itemtemplate->img && isdoc(SI)) // only one document image stays loaded
		each(Items, a) if (a != SI && isdoc(a) && a->itemtemplate->img && a->itemtemplate->img != SI->itemtemplate->img) { bbFreeImage(a->itemtemplate->img); a->itemtemplate->img = 0; }
	if (!SI || !MouseHit2) return 0;
	bbEntityAlpha(Dark, 0); // right click puts it away
	if (is("scp1025")) { if (SI->itemtemplate->img) bbFreeImage(SI->itemtemplate->img); SI->itemtemplate->img = 0; }
	else if (is("firstaid") || is("finefirstaid") || is("firstaid2")) SI->state = 0;
	else if (is("vest") || is("finevest")) { SI->state = 0; if (!WearingVest) DropItem(SI, 0); }
	else if (is("hazmatsuit") || is("hazmatsuit2") || is("hazmatsuit3")) { SI->state = 0; if (!WearingHazmat) DropItem(SI, 0); }
	else if (is("scp1499") || is("super1499")) SI->state = 0;
	if (SI->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SI->itemtemplate->sound]);
	SI = NULL;
	return 0;
}
