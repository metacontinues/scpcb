// blur.c: Dreamfilter.bb (the motion blur overlay) + Main.bb's RenderWorld2
#include "g.h"

void CreateBlurImage(void) {
	int cam = ark_blur_cam = create_camera(0);
	camera_proj_mode(cam, 2);
	camera_zoom(cam, .1f);
	camera_cls_mode(cam, 0, 0);
	camera_range(cam, .1f, 1.5f);
	move_entity(cam, 0, 0, 10000);
	ark_sw = GraphicWidth; ark_sh = GraphicHeight;
	camera_viewport(cam, 0, 0, ark_sw, ark_sh);
	int spr = create_mesh(cam);
	surf *sf = create_surface(spr, 0);
	add_vertex(sf, -1, 1, 0, 0, 0, 0); add_vertex(sf, 1, 1, 0, 1, 0, 0);
	add_vertex(sf, -1, -1, 0, 0, 1, 0); add_vertex(sf, 1, -1, 0, 1, 1, 0);
	add_triangle(sf, 0, 1, 2); add_triangle(sf, 3, 2, 1);
	entity_fx(spr, 17);
	scale_entity(spr, 2048.0f / ark_sw, 2048.0f / ark_sw, 1, 0);
	position_entity(spr, 0, 0, 1.0001f, 0);
	entity_order(spr, -100000);
	entity_blend(spr, 1);
	ark_blur_image = spr;
	ark_blur_texture = create_texture(2048, 2048, 256, 1);
	entity_texture(spr, ark_blur_texture, 0, 0);
}

// last frame stays on the overlay at `power` alpha. TODO the copy is a cpu readback, glCopyTexSubImage2D would do
void UpdateBlur(float power) {
	entity_alpha(ark_blur_image, power);
	copy_rect(0, 0, GraphicWidth, GraphicHeight, iround(1024.0f - GraphicWidth / 2), iround(1024.0f - GraphicHeight / 2), back_buffer(), texture_buffer(ark_blur_texture, 0));
}

void RenderWorld2(void) {
	camera_proj_mode(ark_blur_cam, 0);
	camera_proj_mode(Camera, 1);
	if (WearingNightVision > 0 && WearingNightVision < 3) { float b = Min(Brightness * 2, 255); ambient_light(b, b, b); }
	else if (WearingNightVision == 3) ambient_light(255, 255, 255);
	else if (PlayerRoom) {
		str n = PlayerRoom->RoomTemplate->Name;
		if (!s_eq(n, "173") && !s_eq(n, "exit1") && !s_eq(n, "gatea")) ambient_light(Brightness, Brightness, Brightness);
	}
	IsNVGBlinking = 0;
	hide_entity(NVBlink);
	camera_viewport(Camera, 0, 0, GraphicWidth, GraphicHeight);
	int hasBattery = 2, power = 0;
	if (WearingNightVision == 1 || WearingNightVision == 2) {
		for (int i = 0; i < MaxItemAmount; i++) {
			Items *it = Inventory[i];
			if (!it) continue;
			str t = it->itemtemplate->tempname;
			if (!((WearingNightVision == 1 && s_eq(t, "nvgoggles")) || (WearingNightVision == 2 && s_eq(t, "supernv")))) continue;
			it->state -= FPSfactor * (.02f * WearingNightVision);
			power = iround(it->state);
			if (it->state <= 0) {
				hasBattery = 0;
				Msg = "The batteries in these night vision goggles died.";
				BlinkTimer = -1; MsgTimer = 350;
				break;
			} else if (it->state <= 100) hasBattery = 1;
		}
		if (hasBattery) render_world(1);
	} else render_world(1);
	CurrTrisAmount = tris_rendered();
	if (hasBattery == 0 && WearingNightVision != 3) { IsNVGBlinking = 1; show_entity(NVBlink); }
	if (BlinkTimer < -16 || BlinkTimer > -6) {
		// TODO WearingNightVision == 2: the "REFRESHING DATA" npc readout (needs NPCs)
		if ((WearingNightVision == 1 || WearingNightVision == 2) && hasBattery) {
			if (WearingNightVision == 2) set_color(0, 0, 55); else set_color(0, 55, 0);
			for (int k = 0; k <= 10; k++) draw_rect(45, iround(GraphicHeight * .5f - k * 20), 54, 10, 1);
			if (WearingNightVision == 2) set_color(0, 0, 255); else set_color(0, 255, 0);
			for (int l = 0; l <= (int)floorf((power + 50) * .01f); l++) draw_rect(45, iround(GraphicHeight * .5f - l * 20), 54, 10, 1);
			draw_image(NVGImages, 40, iround(GraphicHeight * .5f + 30), WearingNightVision == 2);
			set_color(255, 255, 255);
		}
	}
	camera_proj_mode(ark_blur_cam, 2);
	camera_proj_mode(Camera, 0);
	render_world(1);
	camera_proj_mode(ark_blur_cam, 0);
	if ((BlinkTimer < -16 || BlinkTimer > -6) && (WearingNightVision == 1 || WearingNightVision == 2) && hasBattery == 1 && MilliSecs2() % 800 < 400) {
		set_color(255, 0, 0);
		AASetFont(Font3);
		AAText(GraphicWidth / 2, iround(20 * MenuScale), "WARNING: LOW BATTERY", 1, 0);
		set_color(255, 255, 255);
	}
}
