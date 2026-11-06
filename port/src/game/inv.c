// inv.c: the inventory half of Main.bb's DrawGUI (esc, the inventory screen, drag/drop/combine, the tail),
// the inventory key from the main loop, Pause/ResumeSounds, CanUseItem. using the item in your hand is use.c
#include "g.h"

static int is(Items *it, const char *t) { return bbSeq(it->itemtemplate->tempname, t); }
static void say(const char *m) { Msg = (str)m; MsgTimer = 70 * 5; }

// ---- sounds that stop while a menu's open

static void pausechn(int c, int stream, int pause) {
	if (!c) return;
	if (stream) { SetStreamPaused_Strict(c, pause); return; }
	if (bbChannelPlaying(c)) { if (pause) bbPauseChannel(c); else bbResumeChannel(c); }
}
static void allsounds(int p) {
	each(Events, e) { pausechn(e->SoundCHN, e->SoundCHN_isStream, p); pausechn(e->SoundCHN2, e->SoundCHN2_isStream, p); }
	each(NPCs, n) { pausechn(n->SoundChn, n->SoundChn_IsStream, p); pausechn(n->SoundChn2, n->SoundChn2_IsStream, p); }
	each(Doors, d) pausechn(d->SoundCHN, 0, p);
	each(DevilEmitters, dem) pausechn(dem->SoundCHN, 0, p);
	pausechn(AmbientSFXCHN, 0, p);
	pausechn(BreathCHN, 0, p);
	if (IntercomStreamCHN) SetStreamPaused_Strict(IntercomStreamCHN, p);
}
void PauseSounds(void) { allsounds(1); }
void ResumeSounds(void) { allsounds(0); }

static void flushmouse(void) { bbMouseXSpeed(); bbMouseYSpeed(); bbMouseZSpeed(); mouse_x_speed_1 = mouse_y_speed_1 = 0; }

// note blitz's headgear check sets the message but still says yes
int CanUseItem(int hazmat, int gasmask, int eyewear) {
	if (!hazmat && WearingHazmat) { say("You can't use that item while wearing a hazmat suit."); return 0; }
	if (!gasmask && (WearingGasMask || Wearing1499)) { say("You can't use that item while wearing a gas mask."); return 0; }
	if (!eyewear && WearingNightVision) Msg = "You can't use that item while wearing headgear.";
	return 1;
}

// main loop: the inventory key
void InventoryKey(void) {
	if (!bbKeyHit(KEY_INV) || VomitTimer < 0 || UnableToMove || IsZombie) return; // TODO Using294
	const char *w = ""; float v = 0;
	if (SelectedItem) { w = SelectedItem->itemtemplate->tempname; v = SelectedItem->state; }
	int wearing = bbSeq(w, "vest") || bbSeq(w, "finevest") || bbSeq(w, "hazmatsuit") || bbSeq(w, "hazmatsuit2") || bbSeq(w, "hazmatsuit3");
	if (wearing && v != 0 && v != 100) return; // half way into a vest/suit
	if (InvOpen) { ResumeSounds(); flushmouse(); }
	else PauseSounds();
	InvOpen = !InvOpen;
	OtherOpen = NULL;
	SelectedItem = NULL;
}

// ---- the inventory screen

static void wornframe(Items *it, int x, int y, int w, int h) { // the grey frame around worn items
	str t = it->itemtemplate->tempname;
	static const struct { const char *t; int *v; int n; } worn[] = {
		{"gasmask", &WearingGasMask, 1}, {"supergasmask", &WearingGasMask, 2}, {"gasmask3", &WearingGasMask, 3},
		{"hazmatsuit", &WearingHazmat, 1}, {"hazmatsuit2", &WearingHazmat, 2}, {"hazmatsuit3", &WearingHazmat, 3},
		{"vest", &WearingVest, 1}, {"finevest", &WearingVest, 2}, {"scp714", &Wearing714, 1},
		{"nvgoggles", &WearingNightVision, 1}, {"supernv", &WearingNightVision, 2}, {"scp1499", &Wearing1499, 1},
		{"super1499", &Wearing1499, 2}, {"finenvgoggles", &WearingNightVision, 3},
	};
	bbColor(200, 200, 200);
	for (size_t i = 0; i < sizeof worn / sizeof *worn; i++)
		if (bbSeq(t, worn[i].t)) { if (*worn[i].v == worn[i].n) bbRect(x - 3, y - 3, w + 6, h + 6, 1); return; }
	if (bbSeq(t, "scp427") && I_427->Using == 1) bbRect(x - 3, y - 3, w + 6, h + 6, 1);
}

static void unslot(Items *it) { for (int z = 0; z < MaxItemAmount; z++) if (Inventory[z] == it) Inventory[z] = NULL; }

// put SelectedItem into a clipboard/wallet: returns what got added, NULL if it's full
static Items *intocase(Items *c, int wallet) {
	str b = SelectedItem->itemtemplate->tempname;
	for (int k = 0; k < c->invSlots; k++) {
		if (c->SecondInv[k] || !SelectedItem) continue;
		c->SecondInv[k] = SelectedItem;
		c->state = 1;
		if (!wallet) bbSetAnimTime(c->model, 0, 0);
		else if (!bbSeq(b, "25ct") && !bbSeq(b, "coin") && !bbSeq(b, "key") && !bbSeq(b, "scp860")) bbSetAnimTime(c->model, 3, 0);
		c->invimg = c->itemtemplate->invimg;
		for (int r = 0; r < MaxItemAmount; r++) if (Inventory[r] == SelectedItem) { Inventory[r] = NULL; PlaySound_Strict(PickSFX[SelectedItem->itemtemplate->sound]); }
		Items *a = SelectedItem;
		SelectedItem = NULL;
		return a;
	}
	return NULL;
}

static void tocase(Items *c) { // paper & co dropped on a clipboard or wallet
	str b = SelectedItem->itemtemplate->tempname, b2 = SelectedItem->itemtemplate->name;
	int card = bbSeq(b2, "Playing Card") || bbSeq(b2, "Mastercard");
	if (is(c, "clipboard")) {
		if (!((!bbSeq(b, "misc") && !bbSeq(b, "25ct") && !bbSeq(b, "coin") && !bbSeq(b, "key") && !bbSeq(b, "scp860") && !bbSeq(b, "scp714")) || card)) { say("You cannot combine these two items."); return; }
		Items *a = intocase(c, 0);
		if (!a) Msg = "The paperclip is not strong enough to hold any more items.";
		else if (is(a, "paper") || is(a, "oldpaper")) Msg = "This document was added to the clipboard.";
		else if (is(a, "badge")) Msg = cat(a->itemtemplate->name, " was added to the clipboard.");
		else Msg = cat("The ", a->itemtemplate->name, " was added to the clipboard.");
		MsgTimer = 70 * 5;
	} else if (is(c, "wallet")) {
		if (!((!bbSeq(b, "misc") && !bbSeq(b, "paper") && !bbSeq(b, "oldpaper")) || card)) { say("You cannot combine these two items."); return; }
		Items *a = intocase(c, 1);
		Msg = a ? cat("You put ", a->itemtemplate->name, " into the wallet.") : "The wallet is full.";
		MsgTimer = 70 * 5;
	} else say("You cannot combine these two items.");
}

static void recharge(Items *t, float full, const char *m) {
	if (SelectedItem->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[SelectedItem->itemtemplate->sound]);
	RemoveItem(SelectedItem);
	SelectedItem = NULL;
	t->state = full;
	say(m);
}

static void battery(Items *t, int v18) { // a battery dropped on a navigator/radio/goggles
	str n = t->itemtemplate->name, tn = t->itemtemplate->tempname;
	if (bbSeq(n, "S-NAV Navigator") || bbSeq(n, "S-NAV 300 Navigator") || bbSeq(n, "S-NAV 310 Navigator")) {
		if (v18) say("The battery does not fit inside this navigator.");
		else recharge(t, 100, "You replaced the navigator's battery.");
	} else if (bbSeq(n, "S-NAV Navigator Ultimate")) say("There seems to be no place for batteries in this navigator.");
	else if (bbSeq(n, "Radio Transceiver")) {
		if (bbSeq(tn, "fineradio") || bbSeq(tn, "veryfineradio")) say("There seems to be no place for batteries in this radio.");
		else if (bbSeq(tn, "18vradio")) { if (v18) recharge(t, 100, "You replaced the radio's battery."); else say("The battery does not fit inside this radio."); }
		else if (bbSeq(tn, "radio") && !v18) recharge(t, 100, "You replaced the radio's battery.");
	} else if (bbSeq(n, "Night Vision Goggles") && !v18) {
		if (bbSeq(tn, "nvgoggles") || bbSeq(tn, "supernv")) recharge(t, 1000, "You replaced the goggles' battery.");
		else say("There seems to be no place for batteries in these night vision goggles.");
	} else say("You cannot combine these two items.");
}

static void combine(Items *t) { // SelectedItem dropped onto t
	static const char *papers[] = {"paper", "key1", "key2", "key3", "key4", "key5", "key6", "misc", "oldpaper", "badge", "ticket", "25ct", "coin", "key", "scp860"};
	for (size_t i = 0; i < sizeof papers / sizeof *papers; i++) if (is(SelectedItem, papers[i])) { tocase(t); SelectedItem = NULL; return; }
	if (is(SelectedItem, "battery") || is(SelectedItem, "bat")) battery(t, 0);
	else if (is(SelectedItem, "18vbat")) battery(t, 1);
	else say("You cannot combine these two items.");
}

static void release(int slot) { // let go of the dragged item
	if (slot == 66) { // outside the slots: drop it
		str t = SelectedItem->itemtemplate->tempname;
		if (bbSeq(t, "vest") || bbSeq(t, "finevest") || bbSeq(t, "hazmatsuit") || bbSeq(t, "hazmatsuit2") || bbSeq(t, "hazmatsuit3"))
			say("Double click on this item to take it off.");
		else if ((bbSeq(t, "scp1499") || bbSeq(t, "super1499")) && Wearing1499 > 0) say("Double click on this item to take it off.");
		else { DropItem(SelectedItem, 1); SelectedItem = NULL; InvOpen = 0; }
		bbMoveMouse(viewport_center_x, viewport_center_y);
	} else if (!Inventory[slot]) { unslot(SelectedItem); Inventory[slot] = SelectedItem; SelectedItem = NULL; }
	else if (Inventory[slot] != SelectedItem) combine(Inventory[slot]);
	SelectedItem = NULL;
}

static void fogtweak(void) { // outdoors the inventory/sub-inventory brightens the fog
	str rn = PlayerRoom->RoomTemplate->Name;
	int a = bbSeq(rn, "gatea"), b = bbSeq(rn, "exit1") && bbEntityY(Collider, 0) > 1040 * RoomScale;
	if (!a && !b) return;
	bbHideEntity(Fog);
	bbCameraFogRange(Camera, 5, a ? 30 : 45);
	bbCameraFogColor(Camera, 200, 200, 200);
	bbCameraClsColor(Camera, 200, 200, 200);
	bbCameraRange(Camera, .05f, a ? 30 : 60);
}

// returns 1 for blitz's Return out of DrawGUI
static int screen(void) {
	fogtweak();
	SelectedDoor = NULL;
	int w = 70, h = 70, sp = 35, slot = 66;
	int x0 = GraphicWidth / 2 - (w * MaxItemAmount / 2 + sp * (MaxItemAmount / 2 - 1)) / 2, x = x0, y = GraphicHeight / 2 - h;
	ItemAmount = 0;
	for (int n = 0; n < MaxItemAmount; n++) {
		int on = ScaledMouseX() > x && ScaledMouseX() < x + w && ScaledMouseY() > y && ScaledMouseY() < y + h;
		Items *it = Inventory[n];
		if (it) wornframe(it, x, y, w, h);
		if (on) { slot = n; bbColor(255, 0, 0); bbRect(x - 1, y - 1, w + 2, h + 2, 1); }
		bbColor(255, 255, 255);
		DrawFrame(x, y, w, h, x % 64, x % 64);
		if (it && (SelectedItem != it || on)) bbDrawImage(it->invimg, x + w / 2 - 32, y + h / 2 - 32, 0);
		if (it && SelectedItem != it) {
			if (on && !SelectedItem) {
				if (MouseHit1) {
					SelectedItem = it;
					MouseHit1 = 0;
					if (DoubleClick) { // use it
						if (WearingHazmat > 0 && !bbInstr(SelectedItem->itemtemplate->tempname, "hazmatsuit", 1)) {
							say("You cannot use any items while wearing a hazmat suit.");
							SelectedItem = NULL;
							return 1;
						}
						if (it->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[it->itemtemplate->sound]);
						InvOpen = 0;
						DoubleClick = 0;
					}
				}
				AASetFont(Font1);
				bbColor(0, 0, 0); AAText(x + w / 2 + 1, y + h + sp - 15 + 1, it->name, 1, 0);
				bbColor(255, 255, 255); AAText(x + w / 2, y + h + sp - 15, it->name, 1, 0);
			}
			ItemAmount++;
		} else if (on && MouseHit1) { unslot(SelectedItem); Inventory[n] = SelectedItem; }
		x += w + sp;
		if (n == 4) { y += h * 2; x = x0; }
	}
	if (SelectedItem) {
		if (MouseDown1) { // dragging
			if (slot == 66 || SelectedItem != Inventory[slot])
				bbDrawImage(SelectedItem->invimg, ScaledMouseX() - bbImageWidth(SelectedItem->itemtemplate->invimg) / 2, ScaledMouseY() - bbImageHeight(SelectedItem->itemtemplate->invimg) / 2, 0);
		} else release(slot);
	}
	if (Fullscreen) bbDrawImage(CursorIMG, ScaledMouseX(), ScaledMouseY(), 0);
	if (!InvOpen) { ResumeSounds(); flushmouse(); }
	return 0;
}

// esc: toggles the pause menu, puts down a half-worn vest/suit
static void esc(void) {
	if (!bbKeyHit(1) || EndingTimer != 0) return; // TODO Using294
	if (MenuOpen || InvOpen) { ResumeSounds(); if (OptionsMenu) SaveOptionsINI(); flushmouse(); }
	else PauseSounds();
	MenuOpen = !MenuOpen;
	AchievementsMenu = OptionsMenu = QuitMSG = 0;
	SelectedDoor = NULL; SelectedScreen = NULL; SelectedMonitor = NULL;
	if (SelectedItem && (bbInstr(SelectedItem->itemtemplate->tempname, "vest", 1) || bbInstr(SelectedItem->itemtemplate->tempname, "hazmatsuit", 1))) {
		if (!WearingVest && !WearingHazmat) DropItem(SelectedItem, 1);
		SelectedItem = NULL;
	}
}

// the end of DrawGUI: esc, then the sub-inventory / inventory / item in hand, then housekeeping
void DrawInventory(int prevInvOpen) {
	esc();
	if (OtherOpen) { /* TODO the clipboard/wallet screen (Main.bb 5030-5243) */ OtherOpen = NULL; }
	else if (InvOpen) { if (screen()) return; }
	else if (SelectedItem && UseItem()) return;
	if (!SelectedItem) for (int i = 0; i <= 6; i++) if (RadioCHN[i] && bbChannelPlaying(RadioCHN[i])) bbPauseChannel(RadioCHN[i]);
	each(Items, it) {
		if (it == SelectedItem) continue;
		static const char *reset[] = {"firstaid", "finefirstaid", "firstaid2", "vest", "finevest", "hazmatsuit", "hazmatsuit2", "hazmatsuit3", "scp1499", "super1499"};
		for (int k = 0; k < 10; k++) if (is(it, reset[k])) { it->state = 0; break; }
	}
	if (prevInvOpen && !InvOpen) bbMoveMouse(viewport_center_x, viewport_center_y);
}
