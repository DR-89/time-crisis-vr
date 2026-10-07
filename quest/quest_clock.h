#pragma once
#include <stdint.h>
#include <stdbool.h>

/* The arcade CPU needs ~59.906 ticks/sec even if a PC compositor reprojection
 * mode renders at 45 Hz. Additional simulation ticks reuse the latest input
 * sample and do not submit another XR frame. A long interruption resets time. */
static inline bool qclock_take(int64_t *deadline,int64_t display_time){
    const int64_t tick=16692818;
    if(!*deadline||display_time-*deadline>250000000||*deadline-display_time>250000000)
        *deadline=display_time;
    if(display_time<*deadline)return false;
    *deadline+=tick;return true;
}
