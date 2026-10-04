#pragma once
#include "vr_math.h"

/* Asset coordinates are metres, +Y up, muzzle facing -Z; origin is the aim pose. */
bool qgun_init(const char *path);
void qgun_draw(const float view[16],const float projection[16],V3 position,Q4 rotation,float recoil);
V3 qgun_muzzle(V3 position,Q4 rotation);
void qgun_shutdown(void);
