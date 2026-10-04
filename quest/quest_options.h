#pragma once
#include <stdbool.h>
typedef struct QOptions { bool laser_enabled,physical_crouch; } QOptions;
void qoptions_load(const char *path,QOptions *options);
bool qoptions_save(const char *path,const QOptions *options);
