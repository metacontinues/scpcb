// keyname.c: KeyName.bb, directinput scancode -> label
#include "g.h"

void InitKeyName(void) {
	static const char *t[212] = {
		[1] = "Esc", [11] = "0", [12] = "-", [13] = "=", [14] = "Backspace", [15] = "TAB", [16] = "Q",
		[17] = "W", [18] = "E", [19] = "R", [20] = "T", [21] = "Y", [22] = "U", [23] = "I", [24] = "O",
		[25] = "P", [26] = "[", [27] = "]", [28] = "Enter", [29] = "Left Ctrl", [30] = "A", [31] = "S",
		[32] = "D", [33] = "F", [34] = "G", [35] = "H", [36] = "J", [37] = "K", [38] = "L", [39] = ";",
		[40] = "'", [42] = "Left Shift", [43] = "\\", [44] = "Z", [45] = "X", [46] = "C", [47] = "V", [48]
		= "B", [49] = "N", [50] = "M", [51] = ",", [52] = ".", [54] = "Right Shift", [56] = "Left Alt",
		[57] = "Space", [58] = "Caps Lock", [59] = "F1", [60] = "F2", [61] = "F3", [62] = "F4", [63] =
		"F5", [64] = "F6", [65] = "F7", [66] = "F8", [67] = "F9", [68] = "F10", [157] = "Right Control",
		[184] = "Right Alt", [200] = "Up", [203] = "Left", [205] = "Right", [208] = "Down",
	};
	for (int i = 0; i < 212; i++) KeyName[i] = (str)(t[i] ? t[i] : "");
	for (int i = 2; i <= 10; i++) KeyName[i] = s_str(i - 1);
}
