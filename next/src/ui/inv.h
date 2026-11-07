// inv.h: the inventory screen (DrawGUI's InvOpen part) + the item in your
// hand (its SelectedItem part). both draw inside Begin/EndDrawing
#pragma once
#include "play/item.h"

// slots, drag + drop, double click to use. eye/yaw/pitch: where a dropped
// item goes
void inv_screen(World *w, Inv *v, Vector3 eye, float yaw, float pitch);
// documents, cards & co held up; right click puts it away
void inv_hand(World *w, Inv *v);
