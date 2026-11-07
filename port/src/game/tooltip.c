// tooltip.c: Menu.bb's tooltips + GetLineAmount
#include "g.h"

#define M MenuScale
#define Q "\""

// RowText's wrapping, only counting lines
int GetLineAmount(const char *a, int w, int h, float leading) {
	if (h < 1) h = 2048;
	int shown = 0, lh = iround(AAStringHeight(a) + leading);
	str A = s_dup(a), b = "";
	while (s_len(A) > 0) {
		int sp = s_instr(A, " ", 1);
		if (!sp) sp = s_len(A);
		str t = s_left(A, sp), tr = s_trim(t);
		int extra = 0;
		if (AAStringWidth(cat(b, t)) > w && AAStringWidth(cat(b, tr)) <= w) { t = tr; extra = 1; }
		if (AAStringWidth(cat(b, t)) > w) { shown++; b = ""; }
		else { b = cat(b, t); A = s_right(A, s_len(A) - (s_len(t) + extra)); }
		if ((shown + 1) * lh > h) break;
	}
	return shown + 1;
}

void DrawTooltip(const char *msg) {
	float sc = GraphicHeight / 768.0f;
	int w = iround(AAStringWidth(msg) + 20 * M), mx = ScaledMouseX(), my = ScaledMouseY();
	set_color(25, 25, 25); draw_rect(mx + 20, my, w, iround(19 * sc), 1);
	set_color(150, 150, 150); draw_rect(mx + 20, my, w, iround(19 * sc), 0);
	AASetFont(Font1);
	AAText(iround(mx + 20 * M + w / 2), iround(my + 12 * M), msg, 1, 1);
}

static str pct(const char *pre, float v, const char *def) { return cat(pre, s_str(iround(v * 100)), "% (default is ", def, ")"); }

void DrawOptionsTooltip(int x, int y, int w, int h, const char *option, float value, int ingame) {
	float fx = x + 6 * M, fy = y + 6 * M, fw = w - 12 * M, fh = h - 12 * M;
	str txt = "", txt2 = "", o = s_lower(option);
	int R = 0, G_ = 0, B = 0;
	AASetFont(Font1);
	set_color(255, 255, 255);
#define IS(s) s_eq(o, s)
	if (IS("bump")) { txt = Q "Bump mapping" Q " is used to simulate bumps and dents by distorting the lightmaps."; txt2 = "This option cannot be changed in-game."; R = 255; }
	else if (IS("vsync")) txt = Q "Vertical sync" Q " waits for the display to finish its current refresh cycle before calculating the next frame, preventing issues such as "
		"screen tearing. This ties the game's frame rate to your display's refresh rate and may cause some input lag.";
	else if (IS("antialias")) { txt = Q "Anti-Aliasing" Q " is used to smooth the rendered image before displaying in order to reduce aliasing around the edges of models."; txt2 = "This option only takes effect in fullscreen."; R = 255; }
	else if (IS("roomlights")) txt = "Toggles the artificial lens flare effect generated over specific light sources.";
	else if (IS("gamma")) {
		txt = Q "Gamma correction" Q " is used to achieve a good brightness factor to balance out your display's gamma if the game appears either too dark or bright. "
			"Setting it too high or low can cause the graphics to look less detailed.";
		R = G_ = B = 255; txt2 = pct("Current value: ", value, "100%");
	}
	else if (IS("texquality")) txt = Q "Texture LOD Bias" Q " affects the distance at which texture detail will change to prevent aliasing. Change this option if textures flicker or look too blurry.";
	else if (IS("particleamount")) {
		txt = "Determines the amount of particles that can be rendered per tick.";
		int v = iround(value);
		if (v == 0) { R = 255; txt2 = "Only smoke emitters will produce particles."; }
		else if (v == 1) { R = G_ = 255; txt2 = "Only a few particles will be rendered per tick."; }
		else if (v == 2) { G_ = 255; txt2 = "All particles are rendered."; }
	}
	else if (IS("vram")) { txt = "Textures that are stored in the Video-RAM will load faster, but this also has negative effects on the texture quality as well."; txt2 = "This option cannot be changed in-game."; R = 255; }
	else if (IS("musicvol")) { txt = "Adjusts the volume of background music. Sliding the bar fully to the left will mute all music."; R = G_ = B = 255; txt2 = pct("Current value: ", value, "50%"); }
	else if (IS("soundvol")) { txt = "Adjusts the volume of sound effects. Sliding the bar fully to the left will mute all sounds."; R = G_ = B = 255; txt2 = pct("Current value: ", value, "100%"); }
	else if (IS("sfxautorelease")) { txt = Q "Sound auto-release" Q " will free a sound from memory if it not used after 5 seconds. Prevents memory allocation issues."; R = 255; txt2 = "This option cannot be changed in-game."; }
	else if (IS("usertrack")) {
		txt = "Toggles the ability to play custom tracks over channel 1 of the radio. These tracks are loaded from the " Q "SFX\\Radio\\UserTracks\\" Q
			" directory. Press " Q "1" Q " when the radio is selected to change track.";
		R = 255; txt2 = "This option cannot be changed in-game.";
	}
	else if (IS("usertrackmode")) {
		txt = "Sets the playing mode for the custom tracks. " Q "Repeat" Q " plays every file in alphabetical order. " Q "Random" Q " chooses the next track at random.";
		R = G_ = 255; txt2 = "Note that the random mode does not prevent previously played tracks from repeating.";
	}
	else if (IS("usertrackscan")) txt = "Re-checks the user tracks directory for any new or removed sound files.";
	else if (IS("mousesensitivity")) { txt = "Adjusts the speed of the mouse pointer."; R = G_ = B = 255; txt2 = pct("Current value: ", .5f + value, "50%"); }
	else if (IS("mouseinvert")) txt = Q "Invert mouse Y-axis" Q " is self-explanatory.";
	else if (IS("mousesmoothing")) { txt = "Adjusts the amount of smoothing of the mouse pointer."; R = G_ = B = 255; txt2 = pct("Current value: ", value, "100%"); }
	else if (IS("controls")) txt = "Configure the in-game control scheme.";
	else if (IS("hud")) txt = "Display the blink and stamina meters.";
	else if (IS("consoleenable")) txt = cat("Toggles the use of the developer console. Can be used in-game by pressing ", KeyName[KEY_CONSOLE], ".");
	else if (IS("consoleerror")) txt = Q "Open console on error" Q " is self-explanatory.";
	else if (IS("achpopup")) txt = "Displays a pop-up notification when an achievement is unlocked.";
	else if (IS("showfps")) txt = "Displays the frames per second counter at the top left-hand corner.";
	else if (IS("framelimit")) {
		txt = "Limits the frame rate that the game can run at to a desired value.";
		if (value > 0 && value < 60) { R = G_ = 255; txt2 = "Usually, 60 FPS or higher is preferred. If you are noticing excessive stuttering at this setting, try lowering it to make your framerate more consistent."; }
	}
	else if (IS("antialiastext")) txt = Q "Antialiased text" Q " smooths out the text before displaying. Makes text easier to read at high resolutions.";
#undef IS
	int lines = GetLineAmount(txt, iround(fw), iround(fh), 1), sh = AAStringHeight(txt);
	float fh1 = sh * lines + (10 + lines) * M;
	if (!*txt2) DrawFrame(x, y, w, iround(fh1), 0, 0);
	else {
		int l2 = GetLineAmount(txt2, iround(fw), iround(fh), 1);
		DrawFrame(x, y, w, iround(fh1 + AAStringHeight(txt2) * l2 + (10 + l2) * M), 0, 0);
	}
	RowText(txt, iround(fx), iround(fy), iround(fw), iround(fh), 0, 1);
	if (*txt2) {
		set_color(R, G_, B);
		RowText(txt2, iround(fx), iround(fy + sh * lines + (5 + lines) * M), iround(fw), iround(fh), 0, 1);
	}
}

void DrawMapCreatorTooltip(int x, int y, int w, int h, const char *map) {
	float fx = x + 6 * M, fy = y + 6 * M, fw = w - 12 * M, fh = h - 12 * M;
	AASetFont(Font1);
	set_color(255, 255, 255);
	str t[6], author = "[Unknown]", descr = "[No description]";
	int ramount = 0, forest = 0, mt = 0;
	if (s_eq(s_right(map, 6), "cbmap2")) {
		t[0] = s_left(map, s_len(map) - 7);
		FILE *f = open_file(cat("Map Creator\\Maps\\", map));
		if (f) {
			author = read_line(f); descr = read_line(f);
			read_byte(f); read_byte(f);
			ramount = read_int(f);
			forest = read_int(f) > 0; mt = read_int(f) > 0;
			close_file(f);
		}
	} else t[0] = s_left(map, s_len(map) - 6);
	t[1] = cat("Made by: ", author);
	t[2] = cat("Description: ", descr);
	t[3] = ramount > 0 ? cat("Room amount: ", s_str(ramount)) : "Room amount: [Unknown]";
	t[4] = forest ? "Has custom forest: Yes" : "Has custom forest: No";
	t[5] = mt ? "Has custom maintenance tunnel: Yes" : "Has custom maintenance tunnel: No";
	int lines = GetLineAmount(t[2], iround(fw), iround(fh), 1), h0 = AAStringHeight(t[0]), h2 = AAStringHeight(t[2]);
	DrawFrame(x, y, w, iround(h0 * 6 + h2 * lines + 5 * M), 0, 0);
	set_color(255, 255, 255);
	AAText(iround(fx), iround(fy), t[0], 0, 0);
	AAText(iround(fx), iround(fy + h0), t[1], 0, 0);
	RowText(t[2], iround(fx), iround(fy + h0 * 2), iround(fw), iround(fh), 0, 1);
	for (int i = 3; i < 6; i++) AAText(iround(fx), iround(fy + h0 * (i - 1) + h2 * lines + 5 * M), t[i], 0, 0);
}
