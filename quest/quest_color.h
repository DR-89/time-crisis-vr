#pragma once
#include "quest_gpu.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
#endif

static inline bool qcolor_srgb_write_control(void){
#ifdef TCVR_PC
    return true; /* Desktop OpenGL 4.3 exposes GL_FRAMEBUFFER_SRGB in core. */
#else
    GLint count=0;glGetIntegerv(GL_NUM_EXTENSIONS,&count);
    for(GLint i=0;i<count;i++){
        const char *extension=(const char*)glGetStringi(GL_EXTENSIONS,(GLuint)i);
        if(extension&&!strcmp(extension,"GL_EXT_sRGB_write_control"))return true;
    }
    return false;
#endif
}

static inline int64_t qcolor_swapchain_format(const int64_t *formats,uint32_t count,bool write_control){
    int64_t selected=0;
    for(uint32_t i=0;i<count;i++)if(formats[i]==GL_RGBA8)selected=GL_RGBA8;
    if(write_control)
        for(uint32_t i=0;i<count;i++)if(formats[i]==GL_SRGB8_ALPHA8)selected=GL_SRGB8_ALPHA8;
    return selected;
}

static inline void qcolor_raw_output(int64_t format){
    /* The renderer and arcade LUT already produce display-encoded values.
     * Mark them as sRGB for OpenXR, but store them without another encoding.
     * Call only with a format negotiated through qcolor_swapchain_format. */
    if(format==GL_SRGB8_ALPHA8)glDisable(GL_FRAMEBUFFER_SRGB_EXT);
}
