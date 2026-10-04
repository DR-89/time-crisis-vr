/* Compile twice, with and without TCVR, to compare the exact engine kernels. */
#include RASTER_SOURCE
static uint32_t seed;
static uint32_t rng(void){seed=seed*1664525u+1013904223u;return seed;}
static void noise(uint8_t *p,size_t n){for(size_t i=0;i<n;i++)p[i]=(uint8_t)(rng()>>24);}
__declspec(dllexport) void raster_fixture(unsigned variant,uint8_t *out){
    seed=variant+100;
    static uint8_t pal[0x18000];noise(pal,sizeof pal);
#ifdef TEST_TEXT
    static uint8_t cg[0x20000];static uint16_t spot[0x1000];noise(cg,sizeof cg);
    for(int i=0;i<0x1000;i++)spot[i]=(uint16_t)rng();
    text_state st={0};st.cgram=cg;st.pal=pal;st.valid=1;
    st.attr[0]=(uint16_t)rng();st.attr[1]=(uint16_t)rng();
    fog_state fog={0};noise(fog.screen_fade,3);
    fog.screen_fade_factor=(uint8_t)rng();fog.mixer_flags=(uint8_t)variant;
    fog.text_palbase=(uint8_t)rng();fog.text_alpha=(uint8_t)rng();
    fog.text_alpha_lo=(uint8_t)rng();fog.text_alpha_hi=(uint8_t)rng();fog.text_alpha_mask=(uint8_t)rng();
    fog.spot_factor=(int)(variant%4)*128;
    fog.have_gamma=1;noise((uint8_t*)fog.gamma,sizeof fog.gamma);noise(fog.bg,3);
    if(variant%7==0)fog.text_alpha=fog.screen_fade_factor=0;
    text_set_spot(spot,variant%3!=0);
    memset(out,73,640*480*5);text_render(&st,&fog,out,variant%11==0);
#else
    static uint8_t tiles[4096];noise(tiles,sizeof tiles);g_sprite_tiles=tiles;g_sprite_tiles_size=sizeof tiles;
    noise(out,640*480*5);
    uint8_t fog[3],fade[3];noise(fog,3);noise(fade,3);
    tile_ctx c={0};c.fog_rgb=fog;c.fade_rgb=fade;c.compose_alpha=variant%11!=0;
    c.fogfactor=variant%3?(int)(rng()%256):0;c.fadefactor=variant%4?(int)(rng()%256):0;
    c.alphafactor=variant%5?(int)(rng()%256):255;c.alpha_enabled=variant%2;c.alpha_pen=(int)(rng()%255);c.prioverchar=variant%2;
    c.cxn=(int)(rng()%80)-20;c.cyn=(int)(rng()%60)-20;c.cxx=600+(int)(rng()%80);c.cyx=420+(int)(rng()%80);
    int ox=(int)(rng()%30),oy=(int)(rng()%30);
    tile_dst dst={out,out+640*480*4,ox,oy,640-ox,480-oy};
    for(int i=0;i<12;i++){
        int w=(int)(rng()%800),h=(int)(rng()%600),x=(int)(rng()%1000)-200,y=(int)(rng()%800)-200;
        if(i&1)w=-w;if(i&2)h=-h;
        draw_tile_dst(&dst,pal,(int)(rng()%0x8000),rng()%5,i&1,(i>>1)&1,x,y,w,h,&c);
    }
#endif
}
