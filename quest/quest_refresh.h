#pragma once
#include <math.h>
#include <stdint.h>
/* Request only enumerated rates. Prefer the requested rate, otherwise the
 * highest available rate below it (e.g. 90 when 120 is unavailable). */
static inline float qrefresh_choose(const float *rates,uint32_t n,float wanted){
    if(!isfinite(wanted)||wanted<=0)wanted=120;
    float below=0,lowest=0;
    for(uint32_t i=0;i<n;i++){
        float r=rates[i];if(!isfinite(r)||r<=0)continue;
        if(fabsf(r-wanted)<.1f)return r;
        if(!lowest||r<lowest)lowest=r;
        if(r<wanted&&r>below)below=r;
    }
    return below?below:lowest;
}
