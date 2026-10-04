#include "quest_cover.h"
#include <math.h>
void qcover_calibrate(QCover *cover,float head_y){
    if(!isfinite(head_y))return;
    cover->upright_y=head_y;cover->calibrated=true;cover->ducked=false;
}
bool qcover_pedal(QCover *cover,float head_y,bool tracked){
    if(!tracked||!cover->calibrated||!isfinite(head_y))return false;
    float drop=cover->upright_y-head_y;
    /* 8 cm hysteresis avoids switching on small tracking/body movements. */
    if(drop>=.20f)cover->ducked=true;
    else if(drop<=.12f)cover->ducked=false;
    return !cover->ducked;
}
