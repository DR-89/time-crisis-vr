#pragma once
#include <stdbool.h>
enum { QOPTIONS_GUN_PITCH_MIN=-60,QOPTIONS_GUN_PITCH_MAX=60 };
typedef struct QOptions { bool laser_enabled,physical_crouch,left_handed; int gun_pitch; } QOptions;
void qoptions_load(const char *path,QOptions *options);
bool qoptions_save(const char *path,const QOptions *options);
