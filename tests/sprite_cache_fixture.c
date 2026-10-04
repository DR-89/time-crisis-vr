#include RENDERER_SOURCE
#include "sprite_hw.h"
static sprite_state sst;
fog_state g_fog;
static sprite_item items[1024];
static int ni,frames,render_calls;
static uint8_t prio_mask[SPR_W*SPR_H];static bool prio_any;
static int g_eng_hud_e;
static struct { int sprite_zmax; } *hud_cfg;
static int eng_hud_dx(double x){return 0;}
void sprite_render_item(const sprite_state *s,const fog_state *f,const sprite_item *it,uint8_t *dst,int compose){
    render_calls++;
    for(int y=0;y<it->h;y++)for(int x=0;x<it->w;x++){
        uint8_t *p=dst+((size_t)y*it->w+x)*4;
        p[0]=(uint8_t)(x+frames*37);p[1]=(uint8_t)(y+it->idx*51);p[2]=(uint8_t)(x+y);
        p[3]=((x/7+y/11)%3)?(it->idx?125:255):0;
    }
}
#include CACHE_SOURCE
__declspec(dllexport) int render_fixture(int variant,int w,int h,uint8_t *out){
    if(!qgl_init())return 0;
    render_calls=0;prio_any=false;memset(prio_mask,0,sizeof prio_mask);
    float view[16],proj[16];projection(proj,-.65f,.65f,-.65f,.65f);
    for(frames=1;frames<=4;frames++){
        ni=frames==4?1:2;
        items[0]=(sprite_item){.x0=40,.y0=100,.w=frames==2?107:63,.h=frames==3?29:61,.idx=0,.prioverchar=frames<3};
        items[1]=(sprite_item){.x0=210,.y0=80,.w=133,.h=159,.idx=1,.prioverchar=1};
        for(int pass=0;pass<4;pass++){
#ifdef TCVR
            sprite_cache_begin();
#else
            if(prio_any){memset(prio_mask,0,sizeof prio_mask);prio_any=false;}
#endif
            view_matrix(view,v3((pass&1)?.06f:0,0,0),(Q4){0,0,0,1});qgl_eye(view,proj);
            glViewport(0,0,w,h);glColorMask(1,1,1,1);glClearColor(.2f,.3f,.4f,1);glClear(GL_COLOR_BUFFER_BIT);
            for(int i=0;i<ni;i++)draw_sprite(&items[i]);
            qgl_flush();
            size_t offset=((frames-1)*4+pass)*((size_t)w*h*4+sizeof prio_mask);
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,out+offset);
            memcpy(out+offset+(size_t)w*h*4,prio_mask,sizeof prio_mask);
        }
    }
#ifdef TCVR
    ni=0;frames++;sprite_cache_begin();
    if(spr_cache_bytes)return -2;
    if(!variant&&render_calls!=7)return -3;
    if(variant&&render_calls<=7)return -4; /* forced small budget must exercise fallback */
#else
    if(render_calls!=28)return -5;
#endif
    free(spr_buf);spr_buf=NULL;glDeleteTextures(1,&spr_tex);spr_tex=0;
    GLenum error=glGetError();qgl_shutdown();return error?-(int)error:1;
}
