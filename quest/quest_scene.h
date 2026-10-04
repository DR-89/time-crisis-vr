#pragma once
/* Flat desktop view uses the original arcade projection and mouse coordinates. */
extern int qvr_flat_view;
#include "geo_hw.h"
#include "sprite_hw.h"
#include "vr_math.h"
extern float qvr_units_per_meter;
void qvr_vertex(const geo_quad *q,float x,float y,float z,float out[4]);
void qvr_scene_begin(void);
void qvr_scene_quad(const geo_quad *q);
/* Time Crisis shot-mark animation; other HUD art stays flat. */
#define QVR_IMPACT_FIRST 9368
#define QVR_IMPACT_LAST 9373
void qvr_sprites_prepare(const sprite_item *items,int n);
bool qvr_sprite_corners(const sprite_item *item,int slot,float out[4][4]);
bool qvr_flat_camera(float *cx,float *cy,float *focal);
bool qvr_aim(V3 origin,V3 direction,float *nx,float *ny,V3 *hit);
