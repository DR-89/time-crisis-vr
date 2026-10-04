#pragma once
#include "vr_math.h"
bool qui_init(void);
void qui_draw(const float view[16],const float projection[16],V3 head,Q4 rotation,bool laser,bool physical_crouch,bool saved);
void qui_shutdown(void);
