#include "quest_scene.h"
#include "quest_clock.h"
#include "quest_refresh.h"
#include <assert.h>
#include <stdio.h>
static void near(float a,float b){assert(fabsf(a-b)<.0001f);}
int main(void){
    const float full_rates[]={90,72,120,80},limited_rates[]={72,90,80},compat_rates[]={60,72};
    near(qrefresh_choose(full_rates,4,120),120);near(qrefresh_choose(limited_rates,3,120),90);
    near(qrefresh_choose(full_rates,4,90),90);near(qrefresh_choose(compat_rates,2,120),72);
    near(qrefresh_choose(full_rates,4,NAN),120);near(qrefresh_choose(NULL,0,120),0);
    float cx,cy,focal;assert(qvr_flat_camera(&cx,&cy,&focal));near(focal,772.5625f);
    // Display rates, including compositor half-rate, must preserve arcade time.
    const int rates[]={30,45,60,72,90,120};
    for(unsigned k=0;k<sizeof rates/sizeof rates[0];k++){
        int64_t deadline=0;unsigned ticks=0;
        for(int f=0;f<rates[k]*10;f++){
            int64_t now=1000000000LL+(int64_t)f*1000000000/rates[k];
            while(qclock_take(&deadline,now))ticks++;
        }
        assert(ticks>=598&&ticks<=600);
        assert(qclock_take(&deadline,30000000000LL));
        assert(!qclock_take(&deadline,30000000000LL)); // resume has no large backlog
    }
    float p[16],v[16];projection(p,-.7f,.7f,-.7f,.7f);near(p[8],0);near(p[9],0);
    /* Distant arcade scenery must survive clip-space Z, including >2 km. */
    for(float z=1;z<100000;z*=10){float clip_z=p[10]*-z+p[14],clip_w=z;assert(clip_z>=-clip_w&&clip_z<=clip_w);}
    view_matrix(v,v3(.032f,1,2),(Q4){0,0,0,1});near(v[12],-.032f);near(v[13],-1);near(v[14],-2);
    Q4 turn={0,sinf(.5f),0,cosf(.5f)};V3 back=rotate(conjugate(turn),rotate(turn,v3(1,2,-3)));near(back.x,1);near(back.z,-3);
    float distance=100;assert(ray_triangle(v3(0,0,0),v3(0,0,-1),v3(-1,-1,-3),v3(1,-1,-3),v3(0,1,-3),&distance));near(distance,3);
    distance=100;assert(!ray_triangle(v3(5,0,0),v3(0,0,-1),v3(-1,-1,-3),v3(1,-1,-3),v3(0,1,-3),&distance));
    distance=100;assert(!ray_triangle(v3(0,0,0),v3(0,0,1),v3(-1,-1,-3),v3(1,-1,-3),v3(0,1,-3),&distance));
    geo_quad q={0};q.vr_focal=320;q.vr_cx=320;q.vr_cy=240;q.nrv=4;
    int xy[4][2]={{160,120},{480,120},{480,360},{160,360}};
    for(int i=0;i<4;i++){q.rv[i].sx16=xy[i][0]*16;q.rv[i].sy16=xy[i][1]*16;q.rv[i].z=3000;}
    float vertex[4];qvr_vertex(&q,320,240,3000,vertex);near(vertex[0],0);near(vertex[1],0);near(vertex[2],-3);near(vertex[3],1);
    qvr_scene_begin();qvr_scene_quad(&q);float x,y;V3 hit;
    assert(qvr_aim(v3(0,0,0),v3(0,0,-1),&x,&y,&hit));near(x,.5f);near(y,.5f);near(hit.z,-3);
    assert(qvr_aim(v3(.3f,0,0),v3(0,0,-1),&x,&y,&hit));near(x,.55f);
    qvr_flat_view=1;qvr_vertex(&q,160,120,3000,vertex);near(vertex[0],160);near(vertex[1],120);near(vertex[3],0);qvr_flat_view=0;
    qvr_scene_begin();for(int i=0;i<4;i++)q.rv[i].z=5000000;qvr_scene_quad(&q);
    assert(qvr_aim(v3(0,0,0),v3(0,0,-1),&x,&y,&hit));assert(fabsf(hit.z+5000)<.01f);
    // Nearest surface must win, irrespective of painter order.
    for(int i=0;i<4;i++)q.rv[i].z=1500;qvr_scene_quad(&q);
    assert(qvr_aim(v3(0,0,0),v3(0,0,-1),&x,&y,&hit));near(hit.z,-1.5f);
    q.direct=1;qvr_vertex(&q,10,20,3000,vertex);near(vertex[0],10);near(vertex[1],20);near(vertex[3],0);
    qvr_scene_begin();assert(qvr_aim(v3(0,0,0),v3(0,0,-1),&x,&y,&hit));near(x,.5f);near(y,.5f);near(hit.z,-2.5f);
    assert(!qvr_aim(v3(0,0,0),v3(0,0,1),&x,&y,&hit));
    // IPD yields opposed horizontal disparity and no vertical disparity.
    float left[16],right[16];view_matrix(left,v3(-.032f,0,0),(Q4){0,0,0,1});view_matrix(right,v3(.032f,0,0),(Q4){0,0,0,1});
    assert(left[12]>0&&right[12]<0);near(left[13],right[13]);
    /* Match flat pixels, world pixels and aim at centre AND off-centre. */
    q.direct=0;q.vr_focal=772.6f;q.vr_cx=312;q.vr_cy=236;
    for(int i=0;i<4;i++)q.rv[i].z=30000;
    qvr_scene_begin();qvr_scene_quad(&q);qvr_sprites_prepare(NULL,0);
    assert(qvr_flat_camera(&cx,&cy,&focal));near(cx,312);near(cy,236);near(focal,772.6f);
    for(int px=170;px<=470;px+=30){
        V3 ray=v3((px-cx)/focal,(cy-190)/focal,-1);
        assert(qvr_aim(v3(0,0,0),ray,&x,&y,&hit));near(x,px/640.f);near(y,190/480.f);near(hit.z,-30);
    }
    sprite_item mark={.x0=350,.y0=180,.w=33,.h=33,.tile=QVR_IMPACT_FIRST};
    float corners[4][4];qvr_sprites_prepare(&mark,1);assert(qvr_sprite_corners(&mark,0,corners));near(corners[0][2],-29.4f);
    near(cx+focal*corners[0][0]/-corners[0][2],350);near(cy-focal*corners[0][1]/-corners[0][2],180);
    near(cx+focal*corners[2][0]/-corners[2][2],383);near(cy-focal*corners[2][1]/-corners[2][2],213);
    for(int tile=QVR_IMPACT_FIRST;tile<=QVR_IMPACT_LAST;tile++){mark.tile=tile;qvr_sprites_prepare(&mark,1);assert(qvr_sprite_corners(&mark,0,corners));}
    mark.tile=100;qvr_sprites_prepare(&mark,1);assert(!qvr_sprite_corners(&mark,0,corners));
    mark.tile=QVR_IMPACT_FIRST;mark.z=1;qvr_sprites_prepare(&mark,1);assert(!qvr_sprite_corners(&mark,0,corners));mark.z=0;
    /* Nearest surface wins even if added before distant geometry. */
    qvr_scene_begin();for(int i=0;i<4;i++)q.rv[i].z=20000;qvr_scene_quad(&q);
    for(int i=0;i<4;i++)q.rv[i].z=30000;qvr_scene_quad(&q);qvr_sprites_prepare(&mark,1);
    assert(qvr_sprite_corners(&mark,0,corners));near(corners[0][2],-19.6f);
    /* Clear a scene without losing its camera; misses remain on the HUD plane. */
    qvr_scene_begin();qvr_sprites_prepare(&mark,1);assert(!qvr_sprite_corners(&mark,0,corners));
    assert(qvr_flat_camera(&cx,&cy,&focal));near(focal,772.6f);
    V3 ray=v3((430-cx)/focal,(cy-180)/focal,-1);
    assert(qvr_aim(v3(0,0,0),ray,&x,&y,&hit));near(x,430/640.f);near(y,180/480.f);near(hit.z,-2.5f);
    /* A controller offset must still project its actual hit, not its direction. */
    assert(qvr_aim(v3(.25f,-.12f,0),v3(0,0,-1),&x,&y,&hit));near(x,(312+772.6f*.25f/2.5f)/640);
    qvr_flat_view=1;assert(!qvr_flat_camera(&cx,&cy,&focal));near(focal,500);near(cx,320);
    assert(qvr_aim(v3(.25f,0,0),v3(0,0,-1),&x,&y,&hit));near(x,.5f+.25f/3.2f);
    qvr_scene_begin();qvr_scene_quad(&q);qvr_sprites_prepare(&mark,1);assert(!qvr_sprite_corners(&mark,0,corners));qvr_flat_view=0;
    /* Full-screen triangles exhaust the grid's fixed budget: full scan must win. */
    for(int i=0;i<4;i++){q.rv[i].sx16=(i==1||i==2?640:0)*16;q.rv[i].sy16=(i>=2?480:0)*16;}
    qvr_scene_begin();for(int n=0;n<230;n++)qvr_scene_quad(&q);qvr_sprites_prepare(&mark,1);
    assert(qvr_sprite_corners(&mark,0,corners));near(corners[0][2],-29.4f);
    puts("PASS: VR timing/math, supported refresh selection, camera-aligned HUD/aim, surface shot marks, nearest hit, grid overflow and unchanged desktop projection");return 0;
}
