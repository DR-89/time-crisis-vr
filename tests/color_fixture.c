#include <assert.h>
#include <stdio.h>
#include "quest_gpu.h"

static const char *extensions[4];
static GLint extension_count;
static unsigned queries,disable_calls;
static GLenum disabled;
static void test_get_integer(GLenum name,GLint *value){assert(name==GL_NUM_EXTENSIONS);*value=extension_count;queries++;}
static const GLubyte *test_get_string(GLenum name,GLuint index){assert(name==GL_EXTENSIONS&&index<(GLuint)extension_count);queries++;return (const GLubyte*)extensions[index];}
static void test_disable(GLenum cap){disabled=cap;disable_calls++;}
#undef glGetIntegerv
#undef glGetStringi
#undef glDisable
#define glGetIntegerv test_get_integer
#define glGetStringi test_get_string
#define glDisable test_disable
#include "quest_color.h"

int main(void){
    const int64_t rgba_first[]={GL_RGBA8,GL_SRGB8_ALPHA8};
    const int64_t srgb_first[]={GL_SRGB8_ALPHA8,GL_RGBA8};
    const int64_t srgb_only[]={GL_SRGB8_ALPHA8};
    const int64_t rgba_only[]={GL_RGBA8};
    const int64_t unsupported[]={GL_RGBA16F,GL_RGB8};
    assert(qcolor_swapchain_format(rgba_first,2,true)==GL_SRGB8_ALPHA8);
    assert(qcolor_swapchain_format(srgb_first,2,true)==GL_SRGB8_ALPHA8);
    assert(qcolor_swapchain_format(srgb_only,1,true)==GL_SRGB8_ALPHA8);
    assert(qcolor_swapchain_format(rgba_only,1,true)==GL_RGBA8);
    assert(qcolor_swapchain_format(rgba_first,2,false)==GL_RGBA8);
    assert(qcolor_swapchain_format(srgb_first,2,false)==GL_RGBA8);
    assert(qcolor_swapchain_format(srgb_only,1,false)==0);
    assert(qcolor_swapchain_format(unsupported,2,true)==0);
    assert(qcolor_swapchain_format(NULL,0,true)==0);
#ifdef TCVR_PC
    assert(qcolor_srgb_write_control()&&queries==0);
#else
    assert(!qcolor_srgb_write_control()&&queries>0);
    extensions[0]="GL_EXT_sRGB";extensions[1]="GL_EXT_sRGB_write_control_extra";
    extensions[2]=NULL;extension_count=3;
    assert(!qcolor_srgb_write_control());
    int64_t fallback=qcolor_swapchain_format(rgba_first,2,qcolor_srgb_write_control());
    qcolor_raw_output(fallback);assert(disable_calls==0);
    extensions[3]="GL_EXT_sRGB_write_control";extension_count=4;
    assert(qcolor_srgb_write_control());
#endif
    qcolor_raw_output(GL_RGBA8);assert(disable_calls==0);
    int64_t selected=qcolor_swapchain_format(rgba_first,2,qcolor_srgb_write_control());
    qcolor_raw_output(selected);assert(disable_calls==1&&disabled==GL_FRAMEBUFFER_SRGB_EXT);
    puts("PASS: sRGB preference in either runtime order, guarded RGBA8 fallback, unsupported formats, capability detection and raw write state");
}
