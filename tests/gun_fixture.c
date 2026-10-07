#include <GLES3/gl3.h>
#include <stdint.h>
#include "quest_gun.h"

__declspec(dllexport) int gun_fixture(const char *asset,int variant,int w,int h,uint8_t *out){
    if(!qgun_init(asset))return 0;
    /* ANGLE pbuffer already provides depth via the EGL config. */
    float v[16],p[16];
    view_matrix(v,v3((variant==2||variant==3)?.032f:variant==1?-.032f:0,0,0),(Q4){0,0,0,1});projection(p,-.4f,.4f,-.4f,.4f);
    glViewport(0,0,w,h);glClearColor(.10f,.13f,.17f,1);glColorMask(1,1,1,1);glClear(GL_COLOR_BUFFER_BIT);
    bool side=variant==0||variant>=5;
    Q4 rotation=side?(Q4){0,sinf(.785398f),0,cosf(.785398f)}:variant==4?(Q4){0,1,0,0}:product((Q4){0,sinf(.10f),0,cosf(.10f)},(Q4){sinf(-.10f),0,0,cosf(-.10f)});
    V3 tip=qgun_muzzle(v3(0,0,0),(Q4){0,0,0,1});
    if(fabsf(tip.x)>.0001f||fabsf(tip.y)>.0001f||fabsf(tip.z+.150763f)>.0001f)return -1;
    if(qgun_recoil(0)!=0||qgun_recoil(150)!=0||qgun_recoil(18)<.999f)return -2;
    for(int ms=0;ms<180;ms++)if(qgun_recoil(ms)<0||qgun_recoil(ms)>1)return -3;
    float recoil=variant==3||variant==5?qgun_recoil(18):variant==6?qgun_recoil(150):0;
    qgun_draw(v,p,v3(side?.055f:0,.045f,variant==4?-.6f:-.37f),rotation,recoil,variant==7?1:0);
    V3 after=qgun_muzzle(v3(0,0,0),(Q4){0,0,0,1});
    if(tip.x!=after.x||tip.y!=after.y||tip.z!=after.z)return -4;
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,out);GLenum error=glGetError();
    qgun_shutdown();return error?-(int)error:1;
}
