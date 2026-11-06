// items.c: Items.bb. templates, CreateItem, RemoveItem (UpdateItems/PickItem/DropItem come w/ the player)
#include "g.h"

TLIST(ItemTemplates); TLIST(Items);

ItemTemplates *CreateItemTemplate(const char *name, const char *tempname, const char *objpath, const char *invimgpath,
	const char *imgpath, float scale, const char *texturepath, const char *invimgpath2, int anim, int texflags) {
	ItemTemplates *it = bbNew(ItemTemplates);
	// items sharing a model copy it
	each(ItemTemplates, it2) if (bbSeq(it2->objpath, objpath) && it2->obj) { it->obj = bbCopyEntity(it2->obj, 0); it->parentobjpath = it2->objpath; break; }
	if (!it->obj) {
		it->isAnim = anim != 0;
		it->obj = anim ? LoadAnimMesh_Strict(objpath, 0) : LoadMesh_Strict(objpath, 0);
	}
	it->objpath = bbSdup(objpath);
	if (bbLen(texturepath)) {
		int tex = 0;
		each(ItemTemplates, it2) if (bbSeq(it2->texpath, texturepath) && it2->tex) { tex = it2->tex; break; }
		if (!tex) { tex = LoadTexture_Strict(texturepath, texflags); it->texpath = bbSdup(texturepath); }
		bbEntityTexture(it->obj, tex, 0, 0);
		it->tex = tex;
	}
	it->scale = scale;
	bbScaleEntity(it->obj, scale, scale, scale, 1);
	each(ItemTemplates, it2) if (bbSeq(it2->invimgpath, invimgpath) && it2->invimg) {
		it->invimg = it2->invimg;
		if (it2->invimg2) it->invimg2 = it2->invimg2;
		break;
	}
	if (!it->invimg) {
		it->invimg = LoadImage_Strict(invimgpath);
		it->invimgpath = bbSdup(invimgpath);
		bbMaskImage(it->invimg, 255, 0, 255);
	}
	if (bbLen(invimgpath2)) {
		if (!it->invimg2) { it->invimg2 = LoadImage_Strict(invimgpath2); bbMaskImage(it->invimg2, 255, 0, 255); }
	} else it->invimg2 = 0;
	it->imgpath = bbSdup(imgpath);
	it->tempname = bbSdup(tempname);
	it->name = bbSdup(name);
	it->sound = 1;
	bbHideEntity(it->obj);
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
	it = CreateItemTemplate("SCP-500-01", "scp500", "GFX\\items\\pill.b3d", "GFX\\items\\INVpill.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; bbEntityColor(it->obj, 255, 0, 0);
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
	it = CreateItemTemplate("Upgraded pill", "scp500death", "GFX\\items\\pill.b3d", "GFX\\items\\INVpill.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; bbEntityColor(it->obj, 255, 0, 0);
	it = CreateItemTemplate("Pill", "pill", "GFX\\items\\pill.b3d", "GFX\\items\\INVpillwhite.jpg", "", 0.0001, "", "", 0, 9); it->sound = 2; bbEntityColor(it->obj, 255, 255, 255);
	it = CreateItemTemplate("Sticky Note", "paper", "GFX\\items\\note.x", "GFX\\items\\INVnote2.jpg", "GFX\\items\\note682.jpg", 0.0025, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("The Modular Site Project", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docMSP.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Research Sector-02 Scheme", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\docmap.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	it = CreateItemTemplate("Document SCP-427", "paper", "GFX\\items\\paper.x", "GFX\\items\\INVpaper.jpg", "GFX\\items\\doc427.jpg", 0.003, "", "", 0, 9); it->sound = 0;
	(void)it;
	// the textures were only needed to paint the models, free each one once
	each(ItemTemplates, t) {
		if (!t->tex) continue;
		if (bbLen(t->texpath)) each(ItemTemplates, t2) if (t2 != t && t2->tex == t->tex) t2->tex = 0;
		bbFreeTexture(t->tex); t->tex = 0;
	}
}

Items *CreateItem(const char *name, const char *tempname, float x, float y, float z, int r, int g, int b, float a, int invSlots) {
	Items *i = bbNew(Items);
	str n = bbLower(name), tn = bbLower(tempname);
	each(ItemTemplates, it) if (bbSeq(bbLower(it->name), n) && bbSeq(bbLower(it->tempname), tn)) {
		i->itemtemplate = it;
		i->collider = bbCreatePivot(0);
		bbEntityRadius(i->collider, .01f, .01f);
		bbEntityPickMode(i->collider, 1, 0);
		i->model = bbCopyEntity(it->obj, i->collider);
		i->name = it->name;
		bbShowEntity(i->collider);
		bbShowEntity(i->model);
	}
	if (!i->itemtemplate) RuntimeError(cat("Item template not found (", n, ", ", tn, ")"));
	bbResetEntity(i->collider);
	bbPositionEntity(i->collider, x, y, z, 1);
	bbRotateEntity(i->collider, 0, bbRand(1, 360), 0, 0);
	i->dist = bbEntityDistance(Collider, i->collider);
	if (bbSeq(tn, "cup")) {
		i->r = r; i->g = g; i->b = b; i->a = a;
		float sc = i->itemtemplate->scale;
		int liquid = bbCopyEntity(LiquidObj, 0);
		bbScaleEntity(liquid, sc, sc, sc, 1);
		bbPositionEntity(liquid, bbEntityX(i->collider, 1), bbEntityY(i->collider, 1), bbEntityZ(i->collider, 1), 0);
		bbEntityParent(liquid, i->model, 1);
		bbEntityColor(liquid, r, g, b);
		if (a < 0) bbEntityFX(liquid, 1);
		bbEntityAlpha(liquid, fabsf(a));
		bbEntityShininess(liquid, 1);
	}
	i->invimg = i->itemtemplate->invimg;
	if (bbSeq(tn, "clipboard") && !invSlots) { invSlots = 10; bbSetAnimTime(i->model, 17, 0); i->invimg = i->itemtemplate->invimg2; }
	else if (bbSeq(tn, "wallet") && !invSlots) { invSlots = 10; bbSetAnimTime(i->model, 0, 0); }
	i->invSlots = invSlots;
	i->ID = ++LastItemID;
	return i;
}

void RemoveItem(Items *i) {
	bbFreeEntity(i->model); bbFreeEntity(i->collider); i->collider = 0;
	for (int n = 0; n < MaxItemAmount; n++) if (Inventory[n] == i) { Inventory[n] = NULL; ItemAmount--; break; }
	if (SelectedItem == i) {
		str t = i->itemtemplate->tempname;
		if (bbSeq(t, "nvgoggles") || bbSeq(t, "supernv")) WearingNightVision = 0;
		else if (bbSeq(t, "gasmask") || bbSeq(t, "supergasmask") || bbSeq(t, "gasmask2") || bbSeq(t, "gasmask3")) WearingGasMask = 0;
		else if (bbSeq(t, "vest") || bbSeq(t, "finevest") || bbSeq(t, "veryfinevest")) WearingVest = 0;
		else if (bbSeq(t, "hazmatsuit") || bbSeq(t, "hazmatsuit2") || bbSeq(t, "hazmatsuit3")) WearingHazmat = 0;
		else if (bbSeq(t, "scp714")) Wearing714 = 0;
		else if (bbSeq(t, "scp1499") || bbSeq(t, "super1499")) Wearing1499 = 0;
		// TODO scp427: I_427\Using = False
		SelectedItem = NULL;
	}
	if (i->itemtemplate->img) { bbFreeImage(i->itemtemplate->img); i->itemtemplate->img = 0; }
	bbDelete(i);
}
