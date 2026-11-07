// hud.c: the parts of Main.bb's DrawGUI that exist so far (hand icons, door buttons, blink/stamina meters)
// + the message text from the main loop. the inventory part is inv.c/use.c. TODO 294, keypad, debug hud
#include "g.h"

// screen pos for a hand icon pointing at yaw/pitch offsets, clamped to the screen edge like blitz
static float clampv(float v) { if (v > 90 && v <= 180) return 90; if (v > 180 && v < 270) return 270; return v; }
static void handat(int img, float yaw, float pitch) {
	draw_image(img, iround(GraphicWidth / 2 + sinf(clampv(yaw) * DTOR) * (GraphicWidth / 3) - 32),
		iround(GraphicHeight / 2 - sinf(clampv(pitch) * DTOR) * (GraphicHeight / 3) - 32), 0);
}

static void meter(int x, int y, float frac, int img) {
	int w = 204, h = 20;
	set_color(255, 255, 255);
	draw_rect(x, y, w, h, 0);
	for (int i = 1; i <= iround((w - 2) * frac / 10); i++) draw_image(img, x + 3 + 10 * (i - 1), y + 3, 0);
	set_color(0, 0, 0);
	draw_rect(x - 50, y, 30, 30, 1);
}

void DrawGUI(void) {
	if (MenuOpen || ConsoleOpen || SelectedDoor || InvOpen || OtherOpen || EndingTimer < 0) show_pointer(); else hide_pointer();
	PocketDimensionFaces();
	if (ClosestButton && !SelectedDoor && !InvOpen && !MenuOpen && !OtherOpen) {
		int t = create_pivot(0);
		position_entity(t, entity_x(Camera, 0), entity_y(Camera, 0), entity_z(Camera, 0), 0);
		point_entity(t, ClosestButton, 0);
		float yaw = WrapAngle(entity_yaw(Camera, 0) - entity_yaw(t, 0)), pitch = WrapAngle(entity_pitch(Camera, 0) - entity_pitch(t, 0));
		free_entity(t);
		handat(HandIcon, yaw, pitch);
		if (MouseUp1) {
			MouseUp1 = 0;
			if (ClosestDoor) {
				if (s_len(ClosestDoor->Code)) SelectedDoor = ClosestDoor;
				else if (Playable) { PlaySound2(ButtonSFX, Camera, ClosestButton, 10, 1); UseDoor(ClosestDoor, 1, 1); }
			}
		}
	}
	if (ClosestItem) handat(HandIcon2, -delta_yaw(Camera, ClosestItem->collider), -delta_pitch(Camera, ClosestItem->collider));
	if (DrawHandIcon) draw_image(HandIcon, GraphicWidth / 2 - 32, GraphicHeight / 2 - 32, 0);
	for (int i = 0; i <= 3; i++) {
		if (!DrawArrowIcon[i]) continue;
		int x = GraphicWidth / 2 - 32, y = GraphicHeight / 2 - 32;
		if (i == 0) y -= 69; else if (i == 1) x += 69; else if (i == 2) y += 69; else x -= 69;
		draw_image(HandIcon, x, y, 0);
		set_color(0, 0, 0); draw_rect(x + 4, y + 4, 56, 56, 1);
		draw_image(ArrowIMG[i], x + 21, y + 21, 0);
		DrawArrowIcon[i] = 0;
	}
	if (HUDenabled) {
		int x = 80, y = GraphicHeight - 95;
		meter(x, y, BlinkTimer / BLINKFREQ, BlinkMeterIMG);
		if (EyeIrritation > 0) { set_color(200, 0, 0); draw_rect(x - 53, y - 3, 36, 36, 1); }
		set_color(255, 255, 255); draw_rect(x - 51, y - 1, 32, 32, 0);
		draw_image(BlinkIcon, x - 50, y, 0);
		y = GraphicHeight - 55;
		meter(x, y, Stamina / 100, StaminaMeterIMG);
		set_color(255, 255, 255); draw_rect(x - 51, y - 1, 32, 32, 0);
		draw_image(Crouch ? CrouchIcon : SprintIcon, x - 50, y, 0);
	}
	// TODO SelectedScreen, the keypad (SelectedDoor)
	DrawInventory(InvOpen);
}

// main loop's Msg text (fades w/ MsgTimer)
void DrawMessages(void) {
	if (MsgTimer <= 0) return;
	int paper = !InvOpen && SelectedItem && (s_eq(SelectedItem->itemtemplate->tempname, "paper") || s_eq(SelectedItem->itemtemplate->tempname, "oldpaper"));
	float a = Min(MsgTimer / 2, 255) / 255;
	int y = paper ? iround(GraphicHeight * .94f) : GraphicHeight / 2 + 200;
	set_color(0, 0, 0);
	AATextA(GraphicWidth / 2 + 1, y + 1, Msg, 1, 0, a);
	set_color(255, 255, 255);
	if (s_eq(s_left(Msg, 14), "Blitz3D Error!")) set_color(255, 0, 0);
	AATextA(GraphicWidth / 2, y, Msg, 1, 0, a);
	MsgTimer -= FPSfactor2;
}
