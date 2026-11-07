// map.h: MapSystem.bb + Main.bb's doors. included from g.h, so G() works here too
#pragma once

enum { MaxRoomLights = 32, MaxRoomEmitters = 8, MaxRoomObjects = 30 };
enum { ROOM1 = 1, ROOM2, ROOM2C, ROOM3, ROOM4 };
enum { ZONEAMOUNT = 3, gridsz = 19 };

struct NPCs; struct Grids; struct DrawPortal; struct Forest; struct Events;
typedef struct Doors Doors;
typedef struct Rooms Rooms;
typedef struct WayPoints WayPoints;

typedef struct Materials { lnode n; str name; int Diff, Bump, StepSound; } Materials;
TLIST_DECL(Materials);

typedef struct RoomTemplates {
	lnode n;
	int obj, id; str objPath;
	int zone[6];
	int TempSoundEmitter[MaxRoomEmitters + 1];
	float TempSoundEmitterX[MaxRoomEmitters + 1], TempSoundEmitterY[MaxRoomEmitters + 1], TempSoundEmitterZ[MaxRoomEmitters + 1];
	float TempSoundEmitterRange[MaxRoomEmitters + 1];
	int Shape; str Name;
	int Commonness, Large, DisableDecals;
	int TempTriggerboxAmount, TempTriggerbox[129]; str TempTriggerboxName[129];
	int UseLightCones, DisableOverlapCheck;
	float MinX, MinY, MinZ, MaxX, MaxY, MaxZ;
} RoomTemplates;
TLIST_DECL(RoomTemplates);

struct Rooms {
	lnode n;
	int zone, found, obj;
	float x, y, z;
	int angle;
	RoomTemplates *RoomTemplate;
	float dist;
	int SoundCHN;
	struct DrawPortal *dp; struct Forest *fr;
	int SoundEmitter[MaxRoomEmitters + 1], SoundEmitterObj[MaxRoomEmitters + 1];
	float SoundEmitterRange[MaxRoomEmitters + 1];
	int SoundEmitterCHN[MaxRoomEmitters + 1];
	int Lights[MaxRoomLights + 1]; float LightIntensity[MaxRoomLights + 1];
	int LightSprites[MaxRoomLights + 1];
	int Objects[MaxRoomObjects + 1], Levers[12];
	Doors *RoomDoors[8];
	struct NPCs *NPC[13];
	struct Grids *grid;
	Rooms *Adjacent[5];
	Doors *AdjDoor[5];
	surf *NonFreeAble[11]; // blitz stored surface handles in an int field
	int Textures[11];
	int MaxLights;
	int LightSpriteHidden[MaxRoomLights + 1], LightSpritesPivot[MaxRoomLights + 1], LightSprites2[MaxRoomLights + 1];
	int LightHidden[MaxRoomLights + 1], LightFlicker[MaxRoomLights + 1];
	int AlarmRotor[2], AlarmRotorLight[2];
	int TriggerboxAmount, Triggerbox[129]; str TriggerboxName[129];
	float MaxWayPointY;
	float LightR[MaxRoomLights + 1], LightG[MaxRoomLights + 1], LightB[MaxRoomLights + 1];
	int LightCone[MaxRoomLights + 1], LightConeSpark[MaxRoomLights + 1];
	float LightConeSparkTimer[MaxRoomLights + 1];
	float MinX, MinY, MinZ, MaxX, MaxY, MaxZ;
};
TLIST_DECL(Rooms);

typedef struct LightTemplates {
	lnode n;
	RoomTemplates *roomtemplate;
	int ltype; float x, y, z, range; int r, g, b;
	float pitch, yaw; int innerconeangle; float outerconeangle;
} LightTemplates;
TLIST_DECL(LightTemplates);

typedef struct TempWayPoints { lnode n; float x, y, z; RoomTemplates *roomtemplate; } TempWayPoints;
TLIST_DECL(TempWayPoints);
struct WayPoints {
	lnode n;
	int obj; Doors *door; Rooms *room; int state;
	WayPoints *connected[6]; float dist[6];
	float Fcost, Gcost, Hcost;
	WayPoints *parent;
};
TLIST_DECL(WayPoints);

typedef struct Screens { lnode n; int obj; str imgpath; int img; Rooms *room; } Screens;
TLIST_DECL(Screens);
typedef struct TempScreens { lnode n; str imgpath; float x, y, z; RoomTemplates *roomtemplate; } TempScreens;
TLIST_DECL(TempScreens);

struct Doors {
	lnode n;
	int obj, obj2, frameobj, buttons[3];
	int locked, open, angle; float openstate; int fastopen;
	int dir, timer; float timerstate;
	int KeyCard;
	Rooms *room;
	int DisableWaypoint;
	float dist;
	int SoundCHN;
	str Code;
	int ID, Level, LevelDest, AutoClose;
	Doors *LinkedDoor;
	int IsElevatorDoor, MTFClose, NPCCalledElevator, DoorHitOBJ;
};
TLIST_DECL(Doors);

enum { gridsize = 10 };
typedef struct Forest { lnode n; int TileMesh[7], DetailMesh[7], TileTexture[11], grid[gridsize * gridsize + 12], TileEntities[gridsize * gridsize + 2], Forest_Pivot, Door[3], DetailEntities[3], ID; } Forest;
TLIST_DECL(Forest);
void GenForestGrid(Forest *fr);
void PlaceForest(Forest *fr, float x, float y, float z, Rooms *r);

typedef struct MapZones { lnode n; int Transition[2], HasCustomForest, HasCustomMT; } MapZones;
TLIST_DECL(MapZones);

// ---- globals
G(int, room2gw_brokendoor);
G(float, room2gw_x, room2gw_z);
G(int, RoomTempID, MapWidth, MapHeight, RoomAmbience[21], Sky, RemoteDoorOn, Contained106);
G(float, RoomScale, HideDistance, SecondaryLightOn, PrevSecondaryLightOn, LightVolume, TempLightVolume);
G(int, *MapTemp, *MapFound, MapRoomID[ROOM4 + 2], MaxRooms_);
G(str, *MapName, *MapRoom); // MapName(x, y), MapRoom(type, i): use the accessors below
G(MapZones *, I_Zone);
G(Doors *, ClosestDoor);
G(Doors *, SelectedDoor);
G(Screens *, SelectedScreen);
G(int, ClosestButton, DoorTempID, BigDoorOBJ[3], HeavyDoorObj[3], OBJTunnel[8]);
G(float, UpdateDoorsTimer);
// Main.bb entity globals (LoadEntities)
G(int, SoundEmitter, AmbientLightRoomTex, AmbientLightRoomVal, Collider, Head, LightSpriteTex[11]);
G(int, DoorOBJ, DoorFrameOBJ, LeverOBJ, LeverBaseOBJ, DoorColl, ButtonOBJ, ButtonKeyOBJ, ButtonCodeOBJ, ButtonScannerOBJ);
G(int, DecalTextures[21], Monitor, MonitorTexture, CamBaseOBJ, CamOBJ, LiquidObj, MTFObj, GuardObj, ClassDObj, ApacheObj, ApacheRotorObj);
G(int, Monitor2, Monitor3, MonitorTexture2, MonitorTexture3, MonitorTexture4, MonitorTextureOff, LightConeModel);
G(int, NPC049OBJ, NPC0492OBJ, ClerkOBJ, GorePics[11], ScreenTexs[3], Room2slCam, DTextures[9]);
G(int, PauseMenuIMG, SprintIcon, BlinkIcon, CrouchIcon, HandIcon, HandIcon2, StaminaMeterIMG, KeypadHUD, Panel294);
G(int, FogTexture, Fog, GasMaskTexture, GasMaskOverlay, InfectTexture, InfectOverlay, DarkTexture, Dark);
G(int, NVTexture, NVOverlay, LightTexture, Light, TeslaTexture, FogNVTexture);

int *MT(int x, int y);  // MapTemp(x, y), bounds checked like blitz
str *MN(int x, int y);  // MapName(x, y)
str *MR(int t, int i);  // MapRoom(t, i)

// ---- rooms.c
void LoadMaterials_(const char *file); // _ : raylib exports a LoadMaterials too
int GetTextureFromCache(const char *name);
int GetBumpFromCache(const char *name);
Materials *GetCache(const char *name);
void AddTextureToCache(int tex);
void ClearTextureCache(void);
void FreeTextureCache(void);
int LoadRMesh(const char *file, RoomTemplates *rt);
str StripPath(const char *file);
str StripFilename(const char *file);
RoomTemplates *CreateRoomTemplate(const char *meshpath);
void LoadRoomTemplates(const char *file);
void LoadRoomMesh(RoomTemplates *rt);
Rooms *CreateRoom(int zone, int shape, float x, float y, float z, const char *name);
void FillRoom(Rooms *r);
typedef struct { const char *name; void (*fn)(Rooms *); } FillEntry;
extern const FillEntry Fills1[], Fills2[], Fills3[]; // fillroom1-3.c
int AddLight(Rooms *room, float x, float y, float z, int ltype, float range, int r, int g, int b);
LightTemplates *AddTempLight(RoomTemplates *rt, float x, float y, float z, int ltype, float range, int r, int g, int b);
WayPoints *CreateWaypoint(float x, float y, float z, Doors *door, Rooms *room);
Screens *CreateScreen(float x, float y, float z, const char *imgpath, Rooms *r);
void CalculateRoomTemplateExtents(RoomTemplates *r);
void CalculateRoomExtents(Rooms *r);
int CheckRoomOverlap(Rooms *a, Rooms *b);
int PreventRoomOverlap(Rooms *r);
void GetMeshExtents(int mesh);
float WrapAngle(float a);
void InitMapGlobals(void);

// ---- mapgen.c
void CreateMap(void);
int SetRoom(const char *name, int type, int pos, int min_pos, int max_pos);
int GetZone(int y);

// ---- doors.c
Doors *CreateDoor(int lvl, float x, float y, float z, float angle, Rooms *room, int dopen, int big, int keycard, const char *code, int useCollisionMesh);
int CreateButton(float x, float y, float z, float pitch, float yaw, float roll);

// ---- entities.c (Main.bb LoadEntities, partial)
void LoadEntities(void);
