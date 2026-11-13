#pragma once
#include <stdbool.h>
#include "quest_resolution.h"

enum { QOPTIONS_PAGE_MAIN=0,QOPTIONS_PAGE_GRAPHICS=1,QOPTIONS_PAGE_GUN=2,QOPTIONS_PAGE_COUNT=QOPTIONS_PAGE_GUN+1 };
enum { QOPTIONS_GUN_PITCH_MIN=-60,QOPTIONS_GUN_PITCH_MAX=60 };
enum { QOPTIONS_WORLD_SCALE_MIN=25,QOPTIONS_WORLD_SCALE_MAX=400,
       QOPTIONS_WORLD_SCALE_DEFAULT=100,QOPTIONS_WORLD_SCALE_STEP=5 };
enum { QOPTIONS_VIEW_DISTANCE_MIN=0,QOPTIONS_VIEW_DISTANCE_MAX=100,
       QOPTIONS_VIEW_DISTANCE_DEFAULT=40,QOPTIONS_VIEW_DISTANCE_STEP=5 };
/* Gun model offset from the controller, in cm along the angled gun: a controller
 * mounted on top of a physical gun (e.g. WonderFitter) sits above and ahead of it. */
enum { QOPTIONS_GUN_DROP_MIN=0,QOPTIONS_GUN_DROP_MAX=40,QOPTIONS_GUN_BACK_MIN=-30,QOPTIONS_GUN_BACK_MAX=40 };
typedef struct QOptions { bool laser_enabled,physical_crouch,left_handed; int gun_pitch,world_scale,view_distance; bool texture_filtering; int resolution; int gun_drop,gun_back; } QOptions;
void qoptions_load(const char *path,QOptions *options);
bool qoptions_save(const char *path,const QOptions *options);
