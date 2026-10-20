/* Exercise the production bake/cache with adjacent coarse atlas slots. */
#include <time.h>
#define CLOCK_MONOTONIC 1
static int clock_gettime(int clock_id,struct timespec *t){(void)clock_id;return timespec_get(t,TIME_UTC)?0:-1;}
#define TCVR
#include "../upstream/engine/tex_bake.c"
uint8_t *g_texture_data,*g_texture_tilemap;
uint8_t direct_palette[ENG_PAL_GROUPS][ENG_PAL_ENTRIES][3];
int direct_palette_loaded=1;
uint16_t g_eng_pal_gen[ENG_PAL_GROUPS];
unsigned g_eng_frame;
__declspec(dllexport) int atlas_filter_fixture(void){
    if(!qgl_init())return 0;
    atlas_dim=64;atlas_maxp=1;g_tex_atlas=1;g_tex_opaque=1;g_tex_pow2sample=g_tex_pow2alloc=0;
    g_texture_data=calloc(1,TEXTURE_TOTAL_SIZE);g_texture_tilemap=calloc(1,TEXTUREMAP_SIZE);
    for(int c=0;c<3;c++)for(int p=0;p<256;p++)direct_palette[c][p][c]=255;
    renderer_texture_init();
    for(int group=0;group<3;group++){
        float su,sv,ou,ov;GLuint tex=bake_impl(0,0,64,64,0,group,0,&su,&sv,&ou,&ov,16,0);
        TexCacheEntry *e=NULL;for(int i=0;i<TEX_CACHE_SIZE;i++)if(tex_cache[i].occupied&&tex_cache[i].pal_group==group)e=&tex_cache[i];
        if(!e||e->alloc_w!=18||e->alloc_h!=18||e->bytes!=18*18*4)return -2;
        if(fabsf(ou-(e->a_x+1)/64.f)>1e-6f||fabsf(ov-(e->a_y+1)/64.f)>1e-6f)return -3;
        for(int y=0;y<e->alloc_h;y++)for(int x=0;x<e->alloc_w;x++){
            const uint8_t *pixel=apages[e->a_page].shadow+((e->a_y+y)*64+e->a_x+x)*4;
            for(int c=0;c<4;c++)if(pixel[c]!=(c==group||c==3?255:0))return -4;
        }
        float cached_u,cached_v;bake_impl(0,0,64,64,0,group,0,&su,&sv,&cached_u,&cached_v,16,0);
        if(cached_u!=ou||cached_v!=ov)return -5;
        tex_bake_commit(tex);
        float view[16],proj[16];view_matrix(view,v3(0,0,0),(Q4){0,0,0,1});projection(proj,-.65f,.65f,-.65f,.65f);
        qgl_eye(view,proj);glViewport(0,0,64,64);glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDisable(GL_ALPHA_TEST);
        glActiveTexture(GL_TEXTURE1);glDisable(GL_TEXTURE_2D);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,tex);glEnable(GL_TEXTURE_2D);
        qgl_set_texture_filter(true);glColor4f(1,1,1,1);glTexCoord4f(ou+.125f/64,ov+.125f/64,0,1);glBegin(GL_QUADS);
        qglVertex4f(-1,1,-2,1);qglVertex4f(1,1,-2,1);qglVertex4f(1,-1,-2,1);qglVertex4f(-1,-1,-2,1);glEnd();qgl_flush();
        uint8_t pixel[4];glReadPixels(32,32,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
        for(int c=0;c<4;c++)if(pixel[c]!=(c==group||c==3?255:0))return -6;
    }
    int ok=glGetError()?0:1;renderer_texture_init();free(g_texture_data);free(g_texture_tilemap);g_texture_data=g_texture_tilemap=NULL;qgl_shutdown();return ok;
}
