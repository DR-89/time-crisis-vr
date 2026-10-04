/* Included by sprite_hw.c after tile_dst. Palette effects are constant for a
 * tile: evaluate them once per pen, retaining the reference's integer rounding. */
static void qvr_sprite_tile(const tile_dst *dst,const uint8_t *pal,int pal_base,
                           const uint8_t *tilep,int flipx,int flipy,int x0,int y0,
                           int ssw,int ssh,const tile_ctx *c)
{
    int left=x0,top=y0,right=x0+ssw,bottom=y0+ssh;
    if(left<0)left=0;if(top<0)top=0;
    if(right>SPR_W)right=SPR_W;if(bottom>SPR_H)bottom=SPR_H;
    if(left<c->cxn)left=c->cxn;if(top<c->cyn)top=c->cyn;
    if(right>c->cxx+1)right=c->cxx+1;if(bottom>c->cyx+1)bottom=c->cyx+1;
    if(left<dst->ox)left=dst->ox;if(top<dst->oy)top=dst->oy;
    if(right>dst->ox+dst->w)right=dst->ox+dst->w;
    if(bottom>dst->oy+dst->h)bottom=dst->oy+dst->h;
    if(left>=right||top>=bottom)return;
    uint8_t colors[256][4],xs[SPR_W];
    for(int pen=0;pen<255;pen++){
        int idx=(pal_base+pen)&0x7FFF;
        uint8_t *rgb=colors[pen];rgb[0]=pal[idx];rgb[1]=pal[0x8000+idx];rgb[2]=pal[0x10000+idx];
        if(c->fogfactor)blend3(rgb,c->fog_rgb,0xFF-c->fogfactor);
        if(c->fadefactor)blend3(rgb,c->fade_rgb,0xFF-c->fadefactor);
        rgb[3]=(c->alphafactor!=0xFF&&(c->alpha_enabled||pen==c->alpha_pen))?(uint8_t)c->alphafactor:255;
    }
    for(int x=left;x<right;x++){
        int u=(int)((x-x0+0.5)*32.0/ssw);xs[x-left]=(uint8_t)(flipx?31-u:u);
    }
    for(int y=top;y<bottom;y++){
        int v=(int)((y-y0+0.5)*32.0/ssh);
        const uint8_t *row=tilep+(flipy?31-v:v)*32;
        size_t offset=(size_t)(y-dst->oy)*dst->w+left-dst->ox;
        uint8_t *out=dst->rgba+offset*4;
        for(int x=left;x<right;x++,out+=4,offset++){
            uint8_t pen=row[xs[x-left]];if(pen==255)continue;
            memcpy(out,colors[pen],4);
            if(dst->prio)dst->prio[offset]=(uint8_t)(2|c->prioverchar);
        }
    }
}
