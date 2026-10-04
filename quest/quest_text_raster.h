/* Included by text_hw.c. Same text palette/spot/fade arithmetic as the original
 * non-gate path, hoisted out of the pixel loop. Walk each tile row in spans. */
static void qvr_text_render(const text_state *st,const fog_state *fog,uint8_t *rgba)
{
    uint8_t colors[256][4]={{0}};
    int base=(fog->text_palbase<<8)&0x7F00;
    int fade=((fog->mixer_flags&2)!=0)&&fog->screen_fade_factor;
    for(int i=0;i<256;i++){
        if((i&15)==15)continue;
        int pen=(base|i)&0x7FFF,p8=pen&255,dark=-1;
        if(g_spot_on){
            int sp=g_spotram[((i<<2)|((base>>8)&3))&0x3FF]&255;p8=sp;
            if(sp<128)pen=(base|sp)&0x7FFF;
            else dark=(fog->spot_factor*(sp&127))>>7;
        }
        uint8_t *c=colors[i];
        if(dark>=0){c[3]=(uint8_t)(dark>255?255:dark);continue;}
        int r=st->pal[pen],g=st->pal[pen+0x8000],b=st->pal[pen+0x10000];
        int f=fog->screen_fade_factor;
        if(fade){r=(r*(255-f)+fog->screen_fade[0]*f)/255;g=(g*(255-f)+fog->screen_fade[1]*f)/255;b=(b*(255-f)+fog->screen_fade[2]*f)/255;}
        c[0]=(uint8_t)r;c[1]=(uint8_t)g;c[2]=(uint8_t)b;c[3]=255;
        if(fog->text_alpha&&((p8&15)==(fog->text_alpha_mask&15)||(fog->text_alpha_lo<=p8&&p8<=fog->text_alpha_hi)))c[3]=(uint8_t)(255-fog->text_alpha);
    }
    int sx=(st->attr[0]-0x35C)&0x3FF,sy=st->attr[1]&0x3FF;
    const uint8_t *tram=st->cgram+0x1E000;
    for(int y=0;y<TH;y++){
        int ty=(y+sy)&1023;
        for(int x=0;x<TW;){
            int tx=(x+sx)&1023,ti=((ty>>4)*64+(tx>>4))*2;
            uint16_t d=(uint16_t)((tram[ti]<<8)|tram[ti+1]);
            int cy=(d&0x800)?15-(ty&15):ty&15;
            const uint8_t *row=st->cgram+(d&1023)*128+cy*8;
            int n=16-(tx&15);if(n>TW-x)n=TW-x;
            uint8_t *out=rgba+((size_t)y*TW+x)*4;
            for(int k=0;k<n;k++,out+=4){
                int cx=(tx+k)&15;if(d&0x400)cx=15-cx;
                int byte=row[cx>>1],pix=(cx&1)?byte&15:byte>>4;
                if(pix!=15)memcpy(out,colors[(d>>12)*16+pix],4);
            }
            x+=n;
        }
    }
}
