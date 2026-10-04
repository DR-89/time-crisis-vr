#pragma once
#include "geo_hw.h"
#include "vr_math.h"
extern float qvr_units_per_meter;
void qvr_vertex(const geo_quad *q,float x,float y,float z,float out[4]);
void qvr_scene_begin(void);
void qvr_scene_quad(const geo_quad *q);
bool qvr_aim(V3 origin,V3 direction,float *nx,float *ny,V3 *hit);
