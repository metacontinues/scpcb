// props.h: every baked model the world draws, loaded once by id and shared
#pragma once
#include "model.h"

Mdl *prop_get(const char *id); // NULL if it isn't baked
void prop_clear(void);
