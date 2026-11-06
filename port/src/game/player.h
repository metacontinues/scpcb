// player.h: the player + per-frame game state (Main.bb). included from g.h
#pragma once

typedef struct SCP427 { lnode n; int Using; float Timer; int Sound[2], SoundCHN[2]; } SCP427;
TLIST_DECL(SCP427);
G(SCP427 *, I_427);
G(Rooms *, PlayerRoom);
G(int, ark_blur_image, ark_blur_texture, ark_sw, ark_sh, ark_blur_cam);

float CurveValue(float number, float old, float smooth);
float CurveAngle(float val, float old, float smooth);
float Distance(float x1, float y1, float x2, float y2);
void Kill(void);
void MovePlayer(void);
void MouseLook(void);
void AmbientLightRooms(int value);
void UpdateRooms(void);
int IsRoomAdjacent(Rooms *a, Rooms *b);
void UpdateDoors(void);
void UseDoor(Doors *d, int showmsg, int playsfx);
void RemoveDoor(Doors *d);
int ValidRoom2slCamRoom(Rooms *r);
void CreateBlurImage(void);
void UpdateBlur(float power);
void RenderWorld2(void);
void InitNewGame(void);
void DrawGUI(void);
void DrawMessages(void);
void UpdateGame(void);
