#pragma once
#include <stdbool.h>
#include "quest_resolution.h"

enum { QOPTIONS_PAGE_GRAPHICS=1,QOPTIONS_PAGE_COUNT=QOPTIONS_PAGE_GRAPHICS+1 };
enum { QOPTIONS_GUN_PITCH_MIN=-60,QOPTIONS_GUN_PITCH_MAX=60 };
enum { QOPTIONS_WORLD_SCALE_MIN=25,QOPTIONS_WORLD_SCALE_MAX=400,
       QOPTIONS_WORLD_SCALE_DEFAULT=100,QOPTIONS_WORLD_SCALE_STEP=5 };
enum { QOPTIONS_VIEW_DISTANCE_MIN=0,QOPTIONS_VIEW_DISTANCE_MAX=100,
       QOPTIONS_VIEW_DISTANCE_DEFAULT=40,QOPTIONS_VIEW_DISTANCE_STEP=5 };
typedef struct QOptions { bool laser_enabled,physical_crouch,left_handed; int gun_pitch,world_scale,view_distance; bool texture_filtering; int resolution; } QOptions;
void qoptions_load(const char *path,QOptions *options);
bool qoptions_save(const char *path,const QOptions *options);
