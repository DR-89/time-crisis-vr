#include "quest_scene.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
float qvr_units_per_meter=1000.0f;
int qvr_flat_view;

typedef struct { float focal,cx,cy; } Camera;
typedef struct { V3 a,b,c; Camera camera; } Triangle;
static Triangle triangles[65536];
static unsigned count,frame_number;
static float farthest;
/* Time Crisis's main camera, retained on title/menu frames without polygons. */
static Camera flat_camera={772.5625f,320,240};
static struct { Camera camera;unsigned weight; } cameras[64];
static unsigned camera_count;
static struct { sprite_item item;float depth; } sprites[1024];
static int sprite_count;
/* Cache shot-mark depths once per game frame. The bounded screen grid avoids
 * rescanning every polygon for every animated mark; overflow uses a full scan. */
enum { GRID_W=20,GRID_H=15,GRID_LINKS=131072 };
static int grid[GRID_W*GRID_H];
static struct { unsigned triangle;int next; } links[GRID_LINKS];
static unsigned link_count;
static bool grid_overflow;

static bool camera_valid(Camera c){return isfinite(c.focal)&&c.focal>0&&isfinite(c.cx)&&isfinite(c.cy);}
static Camera quad_camera(const geo_quad *q){return (Camera){q->vr_focal,q->vr_cx,q->vr_cy};}
static V3 unproject(Camera c,float x,float y,float z){return v3((x-c.cx)*z/c.focal,(c.cy-y)*z/c.focal,-z);}
bool qvr_flat_camera(float *cx,float *cy,float *focal){
    Camera c=qvr_flat_view?(Camera){500,320,240}:flat_camera;
    *cx=c.cx;*cy=c.cy;*focal=c.focal;return !qvr_flat_view;
}
void qvr_vertex(const geo_quad *q,float x,float y,float z,float out[4]){
    if(!qvr_flat_view&&!q->direct&&camera_valid(quad_camera(q))&&isfinite(z)&&z>0){
        V3 p=unproject(quad_camera(q),x,y,z/qvr_units_per_meter);
        out[0]=p.x;out[1]=p.y;out[2]=p.z;out[3]=1;
    }else{out[0]=x;out[1]=y;out[2]=0;out[3]=0;}
}
void qvr_scene_begin(void){count=0;camera_count=0;farthest=0;sprite_count=0;frame_number++;}
void qvr_scene_quad(const geo_quad *q){
    Camera camera=quad_camera(q);
    if(q->direct||!camera_valid(camera)||q->nrv<3||q->nrv>10)return;
    V3 v[10];
    for(int i=0;i<q->nrv;i++){
        float z=q->rv[i].z/qvr_units_per_meter;
        if(!isfinite(z)||z<=0)return;
        v[i]=unproject(camera,q->rv[i].sx16/16.f,q->rv[i].sy16/16.f,z);
        if(z>farthest)farthest=z;
    }
    unsigned cam;
    for(cam=0;cam<camera_count;cam++)if(!memcmp(&camera,&cameras[cam].camera,sizeof camera))break;
    if(cam==camera_count&&camera_count<64){cameras[cam].camera=camera;cameras[cam].weight=0;camera_count++;}
    if(cam<camera_count)cameras[cam].weight+=q->nrv-2;
    if(qvr_flat_view)return;
    for(int i=1;i+1<q->nrv&&count<65536;i++)triangles[count++]=(Triangle){v[0],v[i],v[i+1],camera};
}
static bool impact(const sprite_item *it){return it->tile>=QVR_IMPACT_FIRST&&it->tile<=QVR_IMPACT_LAST&&it->z==0;}
static void build_grid(void){
    for(int i=0;i<GRID_W*GRID_H;i++)grid[i]=-1;
    link_count=0;grid_overflow=false;
    for(unsigned i=0;i<count;i++){
        V3 v[3]={triangles[i].a,triangles[i].b,triangles[i].c};
        float x0=FLT_MAX,y0=FLT_MAX,x1=-FLT_MAX,y1=-FLT_MAX;
        for(int j=0;j<3;j++){
            float x=flat_camera.cx+flat_camera.focal*v[j].x/-v[j].z;
            float y=flat_camera.cy-flat_camera.focal*v[j].y/-v[j].z;
            x0=fminf(x0,x);x1=fmaxf(x1,x);y0=fminf(y0,y);y1=fmaxf(y1,y);
        }
        if(x1<0||y1<0||x0>=640||y0>=480)continue;
        int left=(int)(fmaxf(0,x0)/32),right=(int)(fminf(639,x1)/32);
        int top=(int)(fmaxf(0,y0)/32),bottom=(int)(fminf(479,y1)/32);
        for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){
            if(link_count==GRID_LINKS){grid_overflow=true;return;}
            int cell=y*GRID_W+x;links[link_count].triangle=i;links[link_count].next=grid[cell];grid[cell]=(int)link_count++;
        }
    }
}
static float surface_depth(float x,float y){
    V3 ray=unproject(flat_camera,x,y,1);float nearest=FLT_MAX;
    if(!grid_overflow&&x>=0&&x<640&&y>=0&&y<480){
        for(int n=grid[(int)(y/32)*GRID_W+(int)(x/32)];n>=0;n=links[n].next){
            Triangle *t=&triangles[links[n].triangle];ray_triangle(v3(0,0,0),ray,t->a,t->b,t->c,&nearest);
        }
    }else for(unsigned i=0;i<count;i++){Triangle *t=&triangles[i];ray_triangle(v3(0,0,0),ray,t->a,t->b,t->c,&nearest);}
    return nearest==FLT_MAX?0:nearest*.98f;
}
void qvr_sprites_prepare(const sprite_item *items,int n){
    if(camera_count){unsigned best=0;for(unsigned i=1;i<camera_count;i++)if(cameras[i].weight>cameras[best].weight)best=i;flat_camera=cameras[best].camera;}
    bool diagnostic=getenv("TCVR_GEOMETRY_DIAGNOSTICS")!=NULL;
    if(diagnostic){
        fprintf(stderr,"[GEOMETRY] frame %u; cameras %u; main %.6f %.3f %.3f; farthest %.1f m; triangles %u\n",frame_number,camera_count,flat_camera.focal,flat_camera.cx,flat_camera.cy,farthest,count);
        for(unsigned i=0;i<camera_count;i++)fprintf(stderr,"[CAMERA] %.6f %.3f %.3f weight %u\n",cameras[i].camera.focal,cameras[i].camera.cx,cameras[i].camera.cy,cameras[i].weight);
    }
    sprite_count=n<0?0:n>1024?1024:n;bool grid_ready=false;
    for(int i=0;i<sprite_count;i++){
        sprites[i].item=items[i];sprites[i].depth=0;
        if(impact(&items[i])){
            if(!qvr_flat_view){
                if(!grid_ready){build_grid();grid_ready=true;}
                sprites[i].depth=surface_depth(items[i].x0+items[i].w*.5f,items[i].y0+items[i].h*.5f);
            }
            if(diagnostic)fprintf(stderr,"[IMPACT] frame %u tile %d key %u xy %d %d size %d %d depth %.4f\n",frame_number,items[i].tile,items[i].z,items[i].x0,items[i].y0,items[i].w,items[i].h,sprites[i].depth);
        }
    }
}
bool qvr_sprite_corners(const sprite_item *it,int slot,float out[4][4]){
    if(qvr_flat_view||slot<0||slot>=sprite_count||sprites[slot].depth<=0)return false;
    const sprite_item *saved=&sprites[slot].item;
    if(saved->tile!=it->tile||saved->idx!=it->idx||saved->x0!=it->x0||saved->y0!=it->y0||saved->w!=it->w||saved->h!=it->h||saved->z!=it->z)return false;
    const int corners[4][2]={{0,0},{1,0},{1,1},{0,1}};
    for(int j=0;j<4;j++){
        V3 p=unproject(flat_camera,(float)(it->x0+corners[j][0]*it->w),(float)(it->y0+corners[j][1]*it->h),sprites[slot].depth);
        out[j][0]=p.x;out[j][1]=p.y;out[j][2]=p.z;out[j][3]=1;
    }
    return true;
}
bool qvr_aim(V3 o,V3 d,float *nx,float *ny,V3 *hit){
    float nearest=FLT_MAX;int best=-1;
    if(!qvr_flat_view)for(unsigned i=0;i<count;i++)if(ray_triangle(o,d,triangles[i].a,triangles[i].b,triangles[i].c,&nearest))best=(int)i;
    Camera camera;
    if(best>=0){camera=triangles[best].camera;*hit=add(o,mul(d,nearest));if(hit->z>=-.001f)return false;}
    else{
        if(d.z>=-.001f)return false;
        float t=(-2.5f-o.z)/d.z;if(t<0)return false;
        *hit=add(o,mul(d,t));camera=qvr_flat_view?(Camera){500,320,240}:flat_camera;
    }
    *nx=(camera.cx+camera.focal*hit->x/-hit->z)/640;
    *ny=(camera.cy-camera.focal*hit->y/-hit->z)/480;
    return *nx>0&&*nx<1&&*ny>0&&*ny<1;
}
