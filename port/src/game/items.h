// items.h: Items.bb. included from g.h
#pragma once

enum { MaxItemAmount = 10 };

typedef struct ItemTemplates {
	lnode n;
	str name, tempname;
	int sound, found;
	int obj; str objpath, parentobjpath;
	int invimg, invimg2; str invimgpath;
	str imgpath; int img;
	int isAnim;
	float scale;
	int tex; str texpath;
} ItemTemplates;
TLIST_DECL(ItemTemplates);

typedef struct Items Items;
struct Items {
	lnode n;
	str name;
	int collider, model;
	ItemTemplates *itemtemplate;
	float DropSpeed;
	int r, g, b; float a;
	int level, SoundChn;
	float dist, disttimer, state, state2;
	int Picked, Dropped, invimg, WontColl;
	float xspeed, zspeed;
	Items *SecondInv[21];
	int ID, invSlots;
};
TLIST_DECL(Items);

G(int, BurntNote, ItemAmount, InvSelect, LastItemID);
G(Items *, Inventory[MaxItemAmount + 2]);
G(Items *, SelectedItem);
G(Items *, ClosestItem);

ItemTemplates *CreateItemTemplate(const char *name, const char *tempname, const char *objpath, const char *invimgpath,
	const char *imgpath, float scale, const char *texturepath, const char *invimgpath2, int anim, int texflags);
void InitItemTemplates(void);
Items *CreateItem(const char *name, const char *tempname, float x, float y, float z, int r, int g, int b, float a, int invSlots);
void RemoveItem(Items *i);
void UpdateItems(void);
void PickItem(Items *item);
void DropItem(Items *item, int playdropsound);
