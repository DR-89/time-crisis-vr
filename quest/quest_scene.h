#pragma once
/* Flat desktop view uses the original arcade projection and mouse coordinates. */
extern int qvr_flat_view;
#include "geo_hw.h"
#include "sprite_hw.h"
#include "vr_math.h"
extern float qvr_units_per_meter;
/* Size relative to the calibrated world; geometry caches remain in base metres. */
extern float qvr_world_scale;
/* Extra real-metre distance from the arcade camera, world geometry only. */
extern float qvr_view_distance;
void qvr_vertex(const geo_quad *q,float x,float y,float z,float out[4]);
void qvr_scene_begin(void);
void qvr_scene_quad(const geo_quad *q);
/* Time Crisis shot-mark animation. These marks have sort key 0 like the HUD,
 * so they are recognised by tile and placed on the nearest surface behind them. */
#define QVR_IMPACT_FIRST 9368
#define QVR_IMPACT_LAST 9373
/* Every other sprite is HUD art (timer, life, ammo, banners, score) or a world
 * effect (explosions, fire, sparks, debris, gun flashes). The game says which:
 * HUD art has depth 0 and Time Crisis's own HUD description (upstream
 * timecris/src/tc_game.c, eng_hud_cfg.sprite_zmax) treats anything at or below
 * this depth as HUD. Deeper sprites carry the same 21-bit view-space depth as
 * polygons, so they are placed in the world at that depth with the same screen
 * footprint they have in the arcade, and HUD art stays on the flat plane. */
#define QVR_HUD_ZMAX 0x10
void qvr_sprites_prepare(const sprite_item *items,int n);
bool qvr_sprite_corners(const sprite_item *item,int slot,float out[4][4]);
bool qvr_flat_camera(float *cx,float *cy,float *focal);
bool qvr_aim(V3 origin,V3 direction,float *nx,float *ny,V3 *hit);
