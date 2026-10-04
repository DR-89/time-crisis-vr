#include "quest_scene.h"
#include <stdlib.h>
/* Initial scale estimate; tune on hardware. The arcade has no metre definition. */
float qvr_units_per_meter=1000.0f;
typedef struct { V3 a,b,c; float focal,cx,cy; } Triangle;
static Triangle triangles[65536];
static unsigned count;
void qvr_vertex(const geo_quad *q,float x,float y,float z,float out[4]) {
    if (!q->direct && q->vr_focal>0 && z>0) {
        out[0]=(x-q->vr_cx)/q->vr_focal*z/qvr_units_per_meter;
        out[1]=(q->vr_cy-y)/q->vr_focal*z/qvr_units_per_meter;
        out[2]=-z/qvr_units_per_meter; out[3]=1;
    } else { out[0]=x;out[1]=y;out[2]=0;out[3]=0; }
}
void qvr_scene_begin(void) { count=0; }
void qvr_scene_quad(const geo_quad *q) {
    if(q->direct || q->vr_focal<=0) return;
    V3 v[10];
    for(int i=0;i<q->nrv && i<10;i++) { float a[4];qvr_vertex(q,q->rv[i].sx16/16.f,q->rv[i].sy16/16.f,(float)q->rv[i].z,a);v[i]=v3(a[0],a[1],a[2]); }
    for(int i=1;i+1<q->nrv && i+1<10 && count<65536;i++)
        triangles[count++]=(Triangle){v[0],v[i],v[i+1],q->vr_focal,q->vr_cx,q->vr_cy};
}
bool qvr_aim(V3 o,V3 d,float *nx,float *ny,V3 *hit) {
    float nearest=2000;int best=-1;
    for(unsigned i=0;i<count;i++) if(ray_triangle(o,d,triangles[i].a,triangles[i].b,triangles[i].c,&nearest)) best=(int)i;
    if(best>=0) {
        Triangle *t=&triangles[best]; *hit=add(o,mul(d,nearest));
        if(hit->z>=-.001f)return false;
        *nx=(t->cx+t->focal*hit->x/-hit->z)/640;
        *ny=(t->cy-t->focal*hit->y/-hit->z)/480;
    } else {
        if(d.z>=-.001f)return false;
        float t=(-2.5f-o.z)/d.z;if(t<0)return false;
        *hit=add(o,mul(d,t));
        *nx=.5f+hit->x/3.2f;*ny=.5f-hit->y/2.4f;
    }
    return *nx>0 && *nx<1 && *ny>0 && *ny<1;
}
