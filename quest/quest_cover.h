#pragma once
#include <stdbool.h>
typedef struct QCover { float upright_y;bool calibrated,ducked; } QCover;
void qcover_calibrate(QCover *cover,float head_y);
/* True is the original arcade pedal: leave cover. Invalid tracking releases it. */
bool qcover_pedal(QCover *cover,float head_y,bool tracked);
