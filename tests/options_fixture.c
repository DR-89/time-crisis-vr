#include <GLES3/gl3.h>
#include <stdint.h>
#include "quest_ui.h"
__declspec(dllexport) int options_fixture(int variant,int w,int h,uint8_t *pixels){
    if(!qui_init())return 0;
    float view[16],proj[16];view_matrix(view,v3(variant==3?.032f:-.032f,0,0),(Q4){0,0,0,1});projection(proj,-.65f,.65f,-.65f,.65f);
    glViewport(0,0,w,h);glClearColor(.08f,.12f,.16f,1);glClear(GL_COLOR_BUFFER_BIT);
    qui_draw(view,proj,v3(0,0,0),(Q4){0,0,0,1},variant>=1,variant>=2,variant!=4);
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);GLenum error=glGetError();qui_shutdown();return error?-(int)error:1;
}
