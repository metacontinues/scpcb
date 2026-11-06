// npcs.h: NPCs.bb (+ Main.bb's Events type, AnimateNPC & co). included from g.h
#pragma once

enum {
	NPCtype173 = 1, NPCtypeOldMan, NPCtypeGuard, NPCtypeD, NPCtype372 = 6, NPCtypeApache, NPCtypeMTF, NPCtype096,
	NPCtype049, NPCtypeZombie, NPCtype5131, NPCtypeTentacle, NPCtype860, NPCtype939, NPCtype066, NPCtypePdPlane,
	NPCtype966, NPCtype1048a, NPCtype1499, NPCtype008, NPCtypeClerk,
};

typedef struct NPCs NPCs;
struct NPCs {
	lnode n;
	int obj, obj2, obj3, obj4, Collider;
	int NPCtype, ID;
	float DropSpeed; int Gravity;
	float State, State2, State3; int PrevState;
	int MakingNoise;
	float Frame, Angle;
	int Sound, SoundChn; float SoundTimer;
	int Sound2, SoundChn2;
	float Speed, CurrSpeed;
	str texture;
	float Idle, Reload;
	int LastSeen; float LastDist;
	float PrevX, PrevY, PrevZ;
	NPCs *Target; int TargetID;
	float EnemyX, EnemyY, EnemyZ;
	WayPoints *Path[21]; int PathStatus; float PathTimer; int PathLocation;
	float NVX, NVY, NVZ; str NVName;
	float GravityMult, MaxGravity; // blitz defaults 1, .2 (CreateNPC sets them)
	int MTFVariant; NPCs *MTFLeader; int IsDead;
	float BlinkTimer; // = 1
	int IgnorePlayer;
	int ManipulateBone, ManipulationType; str BoneToManipulate; float BonePitch, BoneYaw, BoneRoll; str NPCNameInSection;
	int InFacility, CanUseElevator; // = True, False
	ElevatorObj *CurrElevator;
	int HP;
	float PathX, PathZ;
	str Model; float ModelScaleX, ModelScaleY, ModelScaleZ;
	int HideFromNVG, TextureID; // TextureID = -1
	float CollRadius, IdleTimer;
	int SoundChn_IsStream, SoundChn2_IsStream;
	float FallingPickDistance;
};
TLIST_DECL(NPCs);

// Main.bb's Events (UpdateEvents.bb isn't ported yet, InitEvents is)
typedef struct Events {
	lnode n;
	str EventName; Rooms *room;
	float EventState, EventState2, EventState3;
	int SoundCHN, SoundCHN2, Sound, Sound2, SoundCHN_isStream, SoundCHN2_isStream;
	str EventStr;
	int img;
} Events;
TLIST_DECL(Events);

G(NPCs, *Curr173, *Curr106, *Curr096, *Curr5131);
G(Rooms *, MTFrooms[11]);
G(int, PlayerDetected);

NPCs *CreateNPC(int type, float x, float y, float z);
void RemoveNPC(NPCs *n);
void UpdateNPCs(void);
void Update173(NPCs *n); // npc173.c
void Update106(NPCs *n);
void UpdateGuard(NPCs *n); // npcguard.c
void UpdateClassD(NPCs *n);
void Shoot(float x, float y, float z, float hitProb, int particles, int instaKill);
void PlayMTFSound(int sound, NPCs *n);
void ChangeNPCTextureID(NPCs *n, int id);
void TeleportCloser(NPCs *n);
int OtherNPCSeesMeNPC(NPCs *me, NPCs *other);
int MeNPCSeesPlayer(NPCs *me, int disablesoundoncrouch);
void MoveToPocketDimension(void);
int FindFreeNPCID(void);
void ForceSetNPCID(NPCs *n, int id);
void NPCSpeedChange(NPCs *n);
int PlayerInReachableRoom(int canSpawnIn049Chamber);
int CheckForNPCInFacility(NPCs *n);
void FindNextElevator(NPCs *n);
void GoToElevator(NPCs *n);
void FinishWalking(NPCs *n, float startframe, float endframe, float speed);
void AnimateNPC(NPCs *n, float start, float quit, float speed, int loop);
void SetNPCFrame(NPCs *n, float frame);
float Animate2(int e, float curr, int start, int quit, float speed, int loop);
void TeleportEntity(int e, float x, float y, float z, float customradius, int isglobal, float pickrange, int dir);
// waypoints.c (MapSystem.bb)
void InitWayPoints(int loadingstart);
void RemoveWaypoint(WayPoints *w);
int FindPath(NPCs *n, float x, float y, float z);
// events.c
Events *CreateEvent(const char *eventname, const char *roomname, int id, float prob);
void InitEvents(void);
void RemoveEvent(Events *e);
Events *FindEvent(const char *name); // first event w/ that name, the "For e = Each Events: If name Exit" idiom
void UpdateEvents(void);
typedef struct { const char *name; int (*fn)(Events *); } EvEntry; // event tables end w/ {NULL, NULL}
extern const EvEntry Evs1[]; // ev1.c
int Ev173(Events *e); // ev173.c
int EvAlarm(Events *e);
// evlib.c
G(int, IntercomStreamCHN);
int LoadEventSound(Events *e, const char *file, int num);
float point_direction(float x1, float z1, float x2, float z2);
float point_distance(float x1, float z1, float x2, float z2);
float angleDist(float a0, float a1);
str CheckTriggers(void);
void PlayAnnouncement(const char *file);
void UpdateStreamSounds(void);
int UpdateLever(int obj, int locked);
void UpdateButton(int obj);
float UpdateElevators(float State, Doors *door1, Doors *door2, int room1, int room2, Events *ev, int ignorerot);
void UpdateCheckpointMonitors(int numb);
void TurnCheckpointMonitorsOff(int numb);
