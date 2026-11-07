// items.c: Items.bb. templates, CreateItem, RemoveItem (UpdateItems/PickItem/DropItem come w/ the player)
#include "g.h"

TLIST(ItemTemplates); TLIST(Items);

ItemTemplates *CreateItemTemplate(const char *name, const char *tempname, const char *objpath, const char *invimgpath,
	const char *imgpath, float scale, const char *texturepath, const char *invimgpath2, int anim, int texflags) {
	ItemTemplates *it = lnew(ItemTemplates);
	// items sharing a model copy it
	each(ItemTemplates, it2) if (s_eq(it2->objpath, objpath) && it2->obj) { it->obj = copy_entity(it2->obj, 0); sset(&it->parentobjpath, it2->objpath); break; }
	if (!it->obj) {
		it->isAnim = anim != 0;
		it->obj = anim ? LoadAnimMesh_Strict(objpath, 0) : LoadMesh_Strict(objpath, 0);
	}
	sset(&it->objpath, s_dup(objpath));
	if (s_len(texturepath)) {
		int tex = 0;
		each(ItemTemplates, it2) if (s_eq(it2->texpath, texturepath) && it2->tex) { tex = it2->tex; break; }
		if (!tex) { tex = LoadTexture_Strict(texturepath, texflags); sset(&it->texpath, s_dup(texturepath)); }
		entity_texture(it->obj, tex, 0, 0);
		it->tex = tex;
	}
	it->scale = scale;
	scale_entity(it->obj, scale, scale, scale, 1);
	each(ItemTemplates, it2) if (s_eq(it2->invimgpath, invimgpath) && it2->invimg) {
		it->invimg = it2->invimg;
		if (it2->invimg2) it->invimg2 = it2->invimg2;
		break;
	}
	if (!it->invimg) {
		it->invimg = LoadImage_Strict(invimgpath);
		sset(&it->invimgpath, s_dup(invimgpath));
		mask_image(it->invimg, 255, 0, 255);
	}
	if (s_len(invimgpath2)) {
		if (!it->invimg2) { it->invimg2 = LoadImage_Strict(invimgpath2); mask_image(it->invimg2, 255, 0, 255); }
	} else it->invimg2 = 0;
	sset(&it->imgpath, s_dup(imgpath));
	sset(&it->tempname, s_dup(tempname));
	sset(&it->name, s_dup(name));
	it->sound = 1;
	hide_entity(it->obj);
	return it;
}

// generated from Items.bb's InitItemTemplates (one line per template, same order)
void InitItemTemplates(void) {
	ItemTemplates *it;
	it = CreateItemTemplate("Some SCP-420-J", "420", "GFX\\items\\420.x", "GFX\\items\\INV420.jpg", "", 0.0005, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Level 1 Key Card", "key1", "GFX\\items\\keycard.x", "GFX\\items\\INVkey1.jpg", "", 0.0004, "GFX\\items\\keycard1.jpg", "", 0, 9);
	it = CreateItemTemplate("Level 2 Key Card", "key2", "GFX\\items\\keycard.x", "GFX\\items\\INVkey2.jpg", "", 0.0004, "GFX\\items\\keycard2.jpg", "", 0, 9);
	it = CreateItemTemplate("Level 3 Key Card", "key3", "GFX\\items\\keycard.x", "GFX\\items\\INVkey3.jpg", "", 0.0004, "GFX\\items\\keycard3.jpg", "", 0, 9);
	it = CreateItemTemplate("Level 4 Key Card", "key4", "GFX\\items\\keycard.x", "GFX\\items\\INVkey4.jpg", "", 0.0004, "GFX\\items\\keycard4.jpg", "", 0, 9);
	it = CreateItemTemplate("Level 5 Key Card", "key5", "GFX\\items\\keycard.x", "GFX\\items\\INVkey5.jpg", "", 0.0004, "GFX\\items\\keycard5.jpg", "", 0, 9);
	it = CreateItemTemplate("Playing Card", "misc", "GFX\\items\\keycard.x", "GFX\\items\\INVcard.jpg", "", 0.0004, "GFX\\items\\card.jpg", "", 0, 9);
	it = CreateItemTemplate("Mastercard", "misc", "GFX\\items\\keycard.x", "GFX\\items\\INVmastercard.jpg", "", 0.0004, "GFX\\items\\mastercard.jpg", "", 0, 9);
	it = CreateItemTemplate("Key Card Omni", "key6", "GFX\\items\\keycard.x", "GFX\\items\\INVkeyomni.jpg", "", 0.0004, "GFX\\items\\keycardomni.jpg", "", 0, 9);
	it = CreateItemTemplate("SCP-860", "scp860", "GFX\\items\\key.b3d", "GFX\\items\\INVkey.jpg", "", 0.001, "", "", 0, 9); it->sound = 3;
	it = CreateItemTemplate("Document SCP-079", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc079.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-895", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc895.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-860", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc860.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-860-1", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc8601.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("SCP-093 Recovered Materials", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc093rm.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-106", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc106.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Dr. Allok's Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc106_2.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Recall Protocol RP-106-N", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docRP.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-682", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc682.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-173", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc173.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-372", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc372.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-049", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc049.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-096", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc096.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-008", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc008.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-012", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc012.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-500", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc500.png", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-714", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc714.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-513", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc513.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-035", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc035.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("SCP-035 Addendum", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc035ad.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-939", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc939.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-966", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc966.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-970", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc970.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-1048", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1048.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-1123", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1123.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-1162", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1162.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-1499", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1499.png", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Incident Report SCP-1048-A", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1048a.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Drawing", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc1048.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Leaflet", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\leaflet.jpg", 0.003, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Dr. L's Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docL1.jpg", 0.0025, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Dr L's Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docL2.jpg", 0.0025, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Blood-stained Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docL3.jpg", 0.0025, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Dr. L's Burnt Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVbn.jpg", "GFX\\items\\docL4.jpg", 0.0025, "GFX\\items\\BurntNoteTexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Dr L's Burnt Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVbn.jpg", "GFX\\items\\docL5.jpg", 0.0025, "GFX\\items\\BurntNoteTexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Scorched Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVbn.jpg", "GFX\\items\\docL6.jpg", 0.0025, "GFX\\items\\BurntNoteTexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Journal Page", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docGonzales.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Log #1", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\f4.jpg", 0.004, "GFX\\items\\f4.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Log #2", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\f5.jpg", 0.004, "GFX\\items\\f4.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Log #3", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\f6.jpg", 0.004, "GFX\\items\\f4.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Strange Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docStrange.jpg", 0.0025, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Nuclear Device Document", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docNDP.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Class D Orientation Leaflet", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docORI.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Note from Daniel", "paper", "GFX\\items\\note.x", "GFX\\items\\INVnote2.jpg", "GFX\\items\\docdan.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Burnt Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVbn.jpg", "GFX\\items\\bn.it", 0.003, "GFX\\items\\BurntNoteTexture.jpg", "", 0, 9); it->img = BurntNote; it->sound = 0;
	it = CreateItemTemplate("Mysterious Note", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\sn.it", 0.003, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Mobile Task Forces", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docMTF.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Security Clearance Levels", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docSC.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Object Classes", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docOBJC.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docRAND3.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Addendum: 5/14 Test Log", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docRAND2.jpg", 0.003, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Notification", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVnote.jpg", "GFX\\items\\docRAND1.jpg", 0.003, "GFX\\items\\notetexture.jpg", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Incident Report SCP-106-0204", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docIR106.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Ballistic Vest", "vest", "GFX\\items\\vest.x", "GFX\\items\\INVvest.jpg", "", 0.02, "GFX\\items\\Vest.png", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Heavy Ballistic Vest", "finevest", "GFX\\items\\vest.x", "GFX\\items\\INVvest.jpg", "", 0.022, "GFX\\items\\Vest.png", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Bulky Ballistic Vest", "veryfinevest", "GFX\\items\\vest.x", "GFX\\items\\INVvest.jpg", "", 0.025, "GFX\\items\\Vest.png", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Hazmat Suit", "hazmatsuit", "GFX\\items\\hazmat.b3d", "GFX\\items\\INVhazmat.jpg", "", 0.013, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Hazmat Suit", "hazmatsuit2", "GFX\\items\\hazmat.b3d", "GFX\\items\\INVhazmat.jpg", "", 0.013, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Heavy Hazmat Suit", "hazmatsuit3", "GFX\\items\\hazmat.b3d", "GFX\\items\\INVhazmat.jpg", "", 0.013, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("cup", "cup", "GFX\\items\\cup.x", "GFX\\items\\INVcup.jpg", "", 0.04, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Empty Cup", "emptycup", "GFX\\items\\cup.x", "GFX\\items\\INVcup.jpg", "", 0.04, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("SCP-500-01", "scp500", "GFX\\items\\pill.b3d", "GFX\\items\\INVpill.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; entity_color(it->obj, 255, 0, 0);
	it = CreateItemTemplate("First Aid Kit", "firstaid", "GFX\\items\\firstaid.x", "GFX\\items\\INVfirstaid.jpg", "", 0.05, "", "", 0, 9);
	it = CreateItemTemplate("Small First Aid Kit", "finefirstaid", "GFX\\items\\firstaid.x", "GFX\\items\\INVfirstaid.jpg", "", 0.03, "", "", 0, 9);
	it = CreateItemTemplate("Blue First Aid Kit", "firstaid2", "GFX\\items\\firstaid.x", "GFX\\items\\INVfirstaid2.jpg", "", 0.03, "GFX\\items\\firstaidkit2.jpg", "", 0, 9);
	it = CreateItemTemplate("Strange Bottle", "veryfinefirstaid", "GFX\\items\\eyedrops.b3d", "GFX\\items\\INVbottle.jpg", "", 0.002, "GFX\\items\\bottle.jpg", "", 0, 9);
	it = CreateItemTemplate("Gas Mask", "gasmask", "GFX\\items\\gasmask.b3d", "GFX\\items\\INVgasmask.jpg", "", 0.02, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Gas Mask", "supergasmask", "GFX\\items\\gasmask.b3d", "GFX\\items\\INVgasmask.jpg", "", 0.021, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Heavy Gas Mask", "gasmask3", "GFX\\items\\gasmask.b3d", "GFX\\items\\INVgasmask.jpg", "", 0.021, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Origami", "misc", "GFX\\items\\origami.b3d", "GFX\\items\\INVorigami.jpg", "", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Electronical components", "misc", "GFX\\items\\electronics.x", "GFX\\items\\INVelectronics.jpg", "", 0.0011, "", "", 0, 9);
	it = CreateItemTemplate("Metal Panel", "scp148", "GFX\\items\\metalpanel.x", "GFX\\items\\INVmetalpanel.jpg", "", RoomScale, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("SCP-148 Ingot", "scp148ingot", "GFX\\items\\scp148.x", "GFX\\items\\INVscp148.jpg", "", RoomScale, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("S-NAV 300 Navigator", "nav", "GFX\\items\\navigator.x", "GFX\\items\\INVnavigator.jpg", "GFX\\items\\navigator.png", 0.0008, "", "", 0, 9);
	it = CreateItemTemplate("S-NAV Navigator", "nav", "GFX\\items\\navigator.x", "GFX\\items\\INVnavigator.jpg", "GFX\\items\\navigator.png", 0.0008, "", "", 0, 9);
	it = CreateItemTemplate("S-NAV Navigator Ultimate", "nav", "GFX\\items\\navigator.x", "GFX\\items\\INVnavigator.jpg", "GFX\\items\\navigator.png", 0.0008, "", "", 0, 9);
	it = CreateItemTemplate("S-NAV 310 Navigator", "nav", "GFX\\items\\navigator.x", "GFX\\items\\INVnavigator.jpg", "GFX\\items\\navigator.png", 0.0008, "", "", 0, 9);
	it = CreateItemTemplate("Radio Transceiver", "radio", "GFX\\items\\radio.x", "GFX\\items\\INVradio.jpg", "GFX\\items\\radioHUD.png", 1.0, "", "", 0, 9);
	it = CreateItemTemplate("Radio Transceiver", "fineradio", "GFX\\items\\radio.x", "GFX\\items\\INVradio.jpg", "GFX\\items\\radioHUD.png", 1.0, "", "", 0, 9);
	it = CreateItemTemplate("Radio Transceiver", "veryfineradio", "GFX\\items\\radio.x", "GFX\\items\\INVradio.jpg", "GFX\\items\\radioHUD.png", 1.0, "", "", 0, 9);
	it = CreateItemTemplate("Radio Transceiver", "18vradio", "GFX\\items\\radio.x", "GFX\\items\\INVradio.jpg", "GFX\\items\\radioHUD.png", 1.02, "", "", 0, 9);
	it = CreateItemTemplate("Cigarette", "cigarette", "GFX\\items\\420.x", "GFX\\items\\INV420.jpg", "", 0.0004, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Joint", "420s", "GFX\\items\\420.x", "GFX\\items\\INV420.jpg", "", 0.0004, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Smelly Joint", "420s", "GFX\\items\\420.x", "GFX\\items\\INV420.jpg", "", 0.0004, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Severed Hand", "hand", "GFX\\items\\severedhand.b3d", "GFX\\items\\INVhand.jpg", "", 0.04, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Black Severed Hand", "hand2", "GFX\\items\\severedhand.b3d", "GFX\\items\\INVhand2.jpg", "", 0.04, "GFX\\items\\shand2.png", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("9V Battery", "bat", "GFX\\items\\Battery\\Battery.x", "GFX\\items\\Battery\\INVbattery9v.jpg", "", 0.008, "", "", 0, 9);
	it = CreateItemTemplate("18V Battery", "18vbat", "GFX\\items\\Battery\\Battery.x", "GFX\\items\\Battery\\INVbattery18v.jpg", "", 0.01, "GFX\\items\\Battery\\Battery 18V.jpg", "", 0, 9);
	it = CreateItemTemplate("Strange Battery", "killbat", "GFX\\items\\Battery\\Battery.x", "GFX\\items\\Battery\\INVbattery22900.jpg", "", 0.01, "GFX\\items\\Battery\\Strange Battery.jpg", "", 0, 9);
	it = CreateItemTemplate("Eyedrops", "fineeyedrops", "GFX\\items\\eyedrops.b3d", "GFX\\items\\INVeyedrops.jpg", "", 0.0012, "GFX\\items\\eyedrops.jpg", "", 0, 9);
	it = CreateItemTemplate("Eyedrops", "supereyedrops", "GFX\\items\\eyedrops.b3d", "GFX\\items\\INVeyedrops.jpg", "", 0.0012, "GFX\\items\\eyedrops.jpg", "", 0, 9);
	it = CreateItemTemplate("ReVision Eyedrops", "eyedrops", "GFX\\items\\eyedrops.b3d", "GFX\\items\\INVeyedrops.jpg", "", 0.0012, "GFX\\items\\eyedrops.jpg", "", 0, 9);
	it = CreateItemTemplate("RedVision Eyedrops", "eyedrops", "GFX\\items\\eyedrops.b3d", "GFX\\items\\INVeyedropsred.jpg", "", 0.0012, "GFX\\items\\eyedropsred.jpg", "", 0, 9);
	it = CreateItemTemplate("SCP-714", "scp714", "GFX\\items\\scp714.b3d", "GFX\\items\\INV714.jpg", "", 0.3, "", "", 0, 9); it->sound = 3;
	it = CreateItemTemplate("SCP-1025", "scp1025", "GFX\\items\\scp1025.b3d", "GFX\\items\\INV1025.jpg", "", 0.1, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("SCP-513", "scp513", "GFX\\items\\513.x", "GFX\\items\\INV513.jpg", "", 0.1, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Clipboard", "clipboard", "GFX\\items\\clipboard.b3d", "GFX\\items\\INVclipboard.jpg", "", 0.003, "", "GFX\\items\\INVclipboard2.jpg", 1, 9);
	it = CreateItemTemplate("SCP-1123", "1123", "GFX\\items\\HGIB_Skull1.b3d", "GFX\\items\\inv1123.jpg", "", 0.015, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Night Vision Goggles", "supernv", "GFX\\items\\NVG.b3d", "GFX\\items\\INVsupernightvision.jpg", "", 0.02, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Night Vision Goggles", "nvgoggles", "GFX\\items\\NVG.b3d", "GFX\\items\\INVnightvision.jpg", "", 0.02, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Night Vision Goggles", "finenvgoggles", "GFX\\items\\NVG.b3d", "GFX\\items\\INVveryfinenightvision.jpg", "", 0.02, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Syringe", "syringe", "GFX\\items\\Syringe\\syringe.b3d", "GFX\\items\\Syringe\\inv.png", "", 0.005, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Syringe", "finesyringe", "GFX\\items\\Syringe\\syringe.b3d", "GFX\\items\\Syringe\\inv.png", "", 0.005, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Syringe", "veryfinesyringe", "GFX\\items\\Syringe\\syringe.b3d", "GFX\\items\\Syringe\\inv.png", "", 0.005, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("SCP-1499", "scp1499", "GFX\\items\\SCP-1499.b3d", "GFX\\items\\INVscp1499.jpg", "", 0.023, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("SCP-1499", "super1499", "GFX\\items\\SCP-1499.b3d", "GFX\\items\\INVscp1499.jpg", "", 0.023, "", "", 0, 9); it->sound = 2;
	it = CreateItemTemplate("Emily Ross' Badge", "badge", "GFX\\items\\badge.x", "GFX\\items\\INVbadge.jpg", "GFX\\items\\badge1.jpg", 0.0001, "GFX\\items\\badge1_tex.jpg", "", 0, 9);
	it = CreateItemTemplate("Lost Key", "key", "GFX\\items\\key.b3d", "GFX\\items\\INV1162_1.jpg", "", 0.001, "GFX\\items\\key2.png", "", 0, 1+2+8); it->sound = 3;
	it = CreateItemTemplate("Disciplinary Hearing DH-S-4137-17092", "oldpaper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\dh.s", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Coin", "coin", "GFX\\items\\key.b3d", "GFX\\items\\INVcoin.jpg", "", 0.0005, "GFX\\items\\coin.png", "", 0, 1+2+8); it->sound = 3;
	it = CreateItemTemplate("Movie Ticket", "ticket", "GFX\\items\\key.b3d", "GFX\\items\\INVticket.jpg", "GFX\\items\\ticket.png", 0.002, "GFX\\items\\tickettexture.png", "", 0, 1+2+8); it->sound = 0;
	it = CreateItemTemplate("Old Badge", "badge", "GFX\\items\\badge.x", "GFX\\items\\INVoldbadge.jpg", "GFX\\items\\badge2.png", 0.0001, "GFX\\items\\badge2_tex.png", "", 0, 1+2+8);
	it = CreateItemTemplate("Quarter", "25ct", "GFX\\items\\key.b3d", "GFX\\items\\INVcoin.jpg", "", 0.0005, "GFX\\items\\coin.png", "", 0, 1+2+8); it->sound = 3;
	it = CreateItemTemplate("Wallet", "wallet", "GFX\\items\\wallet.b3d", "GFX\\items\\INVwallet.jpg", "", 0.0005, "", "", 1, 9); it->sound = 2;
	it = CreateItemTemplate("SCP-427", "scp427", "GFX\\items\\427.b3d", "GFX\\items\\INVscp427.jpg", "", 0.001, "", "", 0, 9);
	it = CreateItemTemplate("Upgraded pill", "scp500death", "GFX\\items\\pill.b3d", "GFX\\items\\INVpill.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; entity_color(it->obj, 255, 0, 0);
	it = CreateItemTemplate("Pill", "pill", "GFX\\items\\pill.b3d", "GFX\\items\\INVpillwhite.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; entity_color(it->obj, 255, 255, 255);
	it = CreateItemTemplate("Sticky Note", "paper", "GFX\\items\\note.x", "GFX\\items\\INVnote2.jpg", "GFX\\items\\note682.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("The Modular Site Project", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docMSP.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Research Sector-02 Scheme", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docmap.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-427", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc427.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	(void)it;
	// the textures were only needed to paint the models, free each one once
	each(ItemTemplates, t) {
		if (!t->tex) continue;
		if (s_len(t->texpath)) each(ItemTemplates, t2) if (t2 != t && t2->tex == t->tex) t2->tex = 0;
		free_texture(t->tex); t->tex = 0;
	}
}

Items *CreateItem(const char *name, const char *tempname, float x, float y, float z, int r, int g, int b, float a, int invSlots) {
	Items *i = lnew(Items);
	str n = s_lower(name), tn = s_lower(tempname);
	each(ItemTemplates, it) if (s_eq(s_lower(it->name), n) && s_eq(s_lower(it->tempname), tn)) {
		i->itemtemplate = it;
		i->collider = create_pivot(0);
		entity_radius(i->collider, .01f, .01f);
		entity_pick_mode(i->collider, 1, 0);
		i->model = copy_entity(it->obj, i->collider);
		sset(&i->name, it->name);
		show_entity(i->collider);
		show_entity(i->model);
	}
	if (!i->itemtemplate) RuntimeError(cat("Item template not found (", n, ", ", tn, ")"));
	reset_entity(i->collider);
	position_entity(i->collider, x, y, z, 1);
	rotate_entity(i->collider, 0, irand(1, 360), 0, 0);
	i->dist = entity_distance(Collider, i->collider);
	if (s_eq(tn, "cup")) {
		i->r = r; i->g = g; i->b = b; i->a = a;
		float sc = i->itemtemplate->scale;
		int liquid = copy_entity(LiquidObj, 0);
		scale_entity(liquid, sc, sc, sc, 1);
		position_entity(liquid, entity_x(i->collider, 1), entity_y(i->collider, 1), entity_z(i->collider, 1), 0);
		entity_parent(liquid, i->model, 1);
		entity_color(liquid, r, g, b);
		if (a < 0) entity_fx(liquid, 1);
		entity_alpha(liquid, fabsf(a));
		entity_shininess(liquid, 1);
	}
	i->invimg = i->itemtemplate->invimg;
	if (s_eq(tn, "clipboard") && !invSlots) { invSlots = 10; set_anim_time(i->model, 17, 0); i->invimg = i->itemtemplate->invimg2; }
	else if (s_eq(tn, "wallet") && !invSlots) { invSlots = 10; set_anim_time(i->model, 0, 0); }
	i->invSlots = invSlots;
	i->ID = ++LastItemID;
	return i;
}

void RemoveItem(Items *i) {
	free_entity(i->model); free_entity(i->collider); i->collider = 0;
	for (int n = 0; n < MaxItemAmount; n++) if (Inventory[n] == i) { Inventory[n] = NULL; ItemAmount--; break; }
	if (SelectedItem == i) {
		str t = i->itemtemplate->tempname;
		if (s_eq(t, "nvgoggles") || s_eq(t, "supernv")) WearingNightVision = 0;
		else if (s_eq(t, "gasmask") || s_eq(t, "supergasmask") || s_eq(t, "gasmask2") || s_eq(t, "gasmask3")) WearingGasMask = 0;
		else if (s_eq(t, "vest") || s_eq(t, "finevest") || s_eq(t, "veryfinevest")) WearingVest = 0;
		else if (s_eq(t, "hazmatsuit") || s_eq(t, "hazmatsuit2") || s_eq(t, "hazmatsuit3")) WearingHazmat = 0;
		else if (s_eq(t, "scp714")) Wearing714 = 0;
		else if (s_eq(t, "scp1499") || s_eq(t, "super1499")) Wearing1499 = 0;
		else if (s_eq(t, "scp427")) I_427->Using = 0;
		SelectedItem = NULL;
	}
	if (i->itemtemplate->img) { free_image(i->itemtemplate->img); i->itemtemplate->img = 0; }
	ldel(i);
}

// ---- Items.bb: per frame (UpdateItems), PickItem, DropItem

static int istemp(Items *it, const char *a) { return s_eq(it->itemtemplate->tempname, a); }
static int ishazmat(Items *it) { return istemp(it, "hazmatsuit") || istemp(it, "hazmatsuit2") || istemp(it, "hazmatsuit3"); }
static int isvest(Items *it) { return istemp(it, "vest") || istemp(it, "finevest"); }

void UpdateItems(void) {
	int hd = iround(HideDistance * .5f); // blitz Local w/o suffix: int
	ClosestItem = NULL;
	each(Items, i) {
		i->Dropped = 0;
		if (i->Picked) { i->DropSpeed = 0; i->xspeed = i->zspeed = 0; continue; }
		if (i->disttimer < MilliSecs2()) {
			i->dist = entity_distance(Camera, i->collider);
			i->disttimer = MilliSecs2() + 700;
			if (i->dist < hd) show_entity(i->collider);
		}
		if (i->dist >= hd) { hide_entity(i->collider); continue; }
		show_entity(i->collider);
		if (i->dist < 1.2f && (!ClosestItem || ClosestItem == i || i->dist < entity_distance(Camera, ClosestItem->collider))
			&& entity_in_view(i->model, Camera) && entity_visible(i->collider, Camera)) ClosestItem = i;
		if (entity_collided(i->collider, HIT_MAP)) { i->DropSpeed = 0; i->xspeed = i->zspeed = 0; }
		else if (ShouldEntitiesFall && line_pick(entity_x(i->collider, 0), entity_y(i->collider, 0), entity_z(i->collider, 0), 0, -10, 0, 0)) {
			i->DropSpeed -= .0004f * FPSfactor;
			translate_entity(i->collider, i->xspeed * FPSfactor, i->DropSpeed * FPSfactor, i->zspeed * FPSfactor, 0);
			if (i->WontColl) reset_entity(i->collider);
		} else { i->DropSpeed = 0; i->xspeed = i->zspeed = 0; }
		if (i->dist < hd * .2f) each(Items, i2) { // push items apart that sit on top of each other
			if (i == i2 || i2->Picked || i2->dist >= hd * .2f) continue;
			float xt = entity_x(i2->collider, 1) - entity_x(i->collider, 1), yt = entity_y(i2->collider, 1) - entity_y(i->collider, 1);
			float zt = entity_z(i2->collider, 1) - entity_z(i->collider, 1), ed = xt * xt + zt * zt;
			if (!(ed < .07f && fabsf(yt) < .25f) || s_eq(PlayerRoom->RoomTemplate->Name, "room2storage")) continue;
			xt *= .07f - ed; zt *= .07f - ed;
			while (fabsf(xt) + fabsf(zt) < .001f) { xt += frnd(-.002f, .002f); zt += frnd(-.002f, .002f); }
			translate_entity(i2->collider, xt, 0, zt, 0);
			translate_entity(i->collider, -xt, 0, -zt, 0);
		}
		if (entity_y(i->collider, 0) < -35) RemoveItem(i);
	}
	if (ClosestItem && MouseHit1) PickItem(ClosestItem);
}

// 0 can pick, 1 already wearing the same kind, 2 wearing the other kind (vest vs hazmat)
static int wearclash(int (*same)(Items *), int (*other)(Items *)) {
	for (int z = 0; z < MaxItemAmount; z++) {
		Items *v = Inventory[z];
		if (!v) continue;
		if (same(v)) return 1;
		if (other(v)) return 2;
	}
	return 0;
}

void PickItem(Items *item) {
	int full = 1;
	for (int n = 0; n < MaxItemAmount; n++) if (!Inventory[n]) { full = 0; break; }
	if (WearingHazmat > 0) { sset(&Msg, "You cannot pick up any items while wearing a hazmat suit."); MsgTimer = 70 * 5; return; }
	if (full) { sset(&Msg, "You cannot carry any more items."); MsgTimer = 70 * 5; return; }
	int n = 0;
	while (Inventory[n]) n++;
	str t = item->itemtemplate->tempname;
	if (s_eq(t, "1123")) {
		if (Wearing714 == 1) return;
		if (!s_eq(PlayerRoom->RoomTemplate->Name, "room1123")) {
			show_entity(Light); LightFlash = 7;
			PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg"));
			sset(&DeathMSG, "Subject D-9341 was shot dead after attempting to attack a member of Nine-Tailed Fox. Surveillance tapes show that the subject had been "
				"wandering around the site approximately 9 minutes prior, shouting the phrase \"get rid of the four pests\""
				" in chinese. SCP-1123 was found in [REDACTED] nearby, suggesting the subject had come into physical contact with it. How "
				"exactly SCP-1123 was removed from its containment chamber is still unknown.");
			Kill();
		}
		Events *e = FindEvent("room1123");
		if (e) {
			if (e->EventState == 0) { show_entity(Light); LightFlash = 3; PlaySound_Strict(LoadTempSound("SFX\\SCP\\1123\\Touch.ogg")); }
			e->EventState = Max(1, e->EventState);
		}
		return;
	} else if (s_eq(t, "killbat")) {
		show_entity(Light); LightFlash = 1;
		PlaySound_Strict(IntroSFX[11]);
		sset(&DeathMSG, "Subject D-9341 found dead inside SCP-914's output booth next to what appears to be an ordinary nine-volt battery. The subject is covered in severe "
			"electrical burns, and assumed to be killed via an electrical shock caused by the battery. The battery has been stored for further study.");
		Kill();
	} else if (s_eq(t, "scp148")) GiveAchievement(Achv148, 1);
	else if (s_eq(t, "scp513")) GiveAchievement(Achv513, 1);
	else if (s_eq(t, "scp860")) GiveAchievement(Achv860, 1);
	else if (s_eq(t, "key6")) GiveAchievement(AchvOmni, 1);
	else if (s_eq(t, "veryfinevest")) { sset(&Msg, "The vest is too heavy to pick up."); MsgTimer = 70 * 6; return; }
	else if (s_eq(t, "firstaid") || s_eq(t, "finefirstaid") || s_eq(t, "veryfinefirstaid") || s_eq(t, "firstaid2")) item->state = 0;
	else if (s_eq(t, "navigator") || s_eq(t, "nav")) { if (s_eq(item->itemtemplate->name, "S-NAV Navigator Ultimate")) GiveAchievement(AchvSNAV, 1); }
	else if (ishazmat(item)) {
		int c = wearclash(ishazmat, isvest);
		if (c == 1) { sset(&Msg, "You are not able to wear two hazmat suits at the same time."); MsgTimer = 70 * 5; return; }
		if (c == 2) { sset(&Msg, "You are not able to wear a vest and a hazmat suit at the same time."); MsgTimer = 70 * 5; return; }
		SelectedItem = item;
	} else if (isvest(item)) {
		int c = wearclash(isvest, ishazmat);
		if (c == 1) { sset(&Msg, "You are not able to wear two vests at the same time."); MsgTimer = 70 * 5; return; }
		if (c == 2) { sset(&Msg, "You are not able to wear a vest and a hazmat suit at the same time."); MsgTimer = 70 * 5; return; }
		SelectedItem = item;
	}
	if (item->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[item->itemtemplate->sound]);
	item->Picked = 1; item->Dropped = -1;
	item->itemtemplate->found = 1;
	ItemAmount++;
	Inventory[n] = item;
	hide_entity(item->collider);
}

void DropItem(Items *item, int playdropsound) {
	if (WearingHazmat > 0) { sset(&Msg, "You cannot drop any items while wearing a hazmat suit."); MsgTimer = 70 * 5; return; }
	if (playdropsound && item->itemtemplate->sound != 66) PlaySound_Strict(PickSFX[item->itemtemplate->sound]);
	item->Dropped = 1;
	show_entity(item->collider);
	position_entity(item->collider, entity_x(Camera, 0), entity_y(Camera, 0), entity_z(Camera, 0), 0);
	rotate_entity(item->collider, entity_pitch(Camera, 0), entity_yaw(Camera, 0) + frnd(-20, 20), 0, 0);
	move_entity(item->collider, 0, -.1f, .1f);
	rotate_entity(item->collider, 0, entity_yaw(Camera, 0) + frnd(-110, 110), 0, 0);
	reset_entity(item->collider);
	item->Picked = 0;
	for (int z = 0; z < MaxItemAmount; z++) if (Inventory[z] == item) Inventory[z] = NULL;
	str t = item->itemtemplate->tempname;
	if (s_eq(t, "gasmask") || s_eq(t, "supergasmask") || s_eq(t, "gasmask3")) WearingGasMask = 0;
	else if (ishazmat(item)) WearingHazmat = 0;
	else if (isvest(item)) WearingVest = 0;
	else if (s_eq(t, "nvgoggles")) { if (WearingNightVision == 1) { CameraFogFar = StoredCameraFogFar; WearingNightVision = 0; } }
	else if (s_eq(t, "supernv")) { if (WearingNightVision == 2) { CameraFogFar = StoredCameraFogFar; WearingNightVision = 0; } }
	else if (s_eq(t, "finenvgoggles")) { if (WearingNightVision == 3) { CameraFogFar = StoredCameraFogFar; WearingNightVision = 0; } }
	else if (s_eq(t, "scp714")) Wearing714 = 0;
	else if (s_eq(t, "scp1499") || s_eq(t, "super1499")) Wearing1499 = 0;
	else if (s_eq(t, "scp427")) I_427->Using = 0;
}
