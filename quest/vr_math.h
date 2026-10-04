#pragma once
#include <math.h>
#include <stdbool.h>
typedef struct { float x,y,z; } V3;
typedef struct { float x,y,z,w; } Q4;
static inline V3 v3(float x,float y,float z) { return (V3){x,y,z}; }
static inline V3 add(V3 a,V3 b) { return v3(a.x+b.x,a.y+b.y,a.z+b.z); }
static inline V3 sub(V3 a,V3 b) { return v3(a.x-b.x,a.y-b.y,a.z-b.z); }
static inline V3 mul(V3 a,float s) { return v3(a.x*s,a.y*s,a.z*s); }
static inline float dot(V3 a,V3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static inline V3 cross(V3 a,V3 b) { return v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
static inline Q4 conjugate(Q4 q) { return (Q4){-q.x,-q.y,-q.z,q.w}; }
static inline Q4 product(Q4 a,Q4 b) {
    V3 v=add(add(mul(v3(b.x,b.y,b.z),a.w),mul(v3(a.x,a.y,a.z),b.w)),cross(v3(a.x,a.y,a.z),v3(b.x,b.y,b.z)));
    return (Q4){v.x,v.y,v.z,a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z};
}
static inline V3 rotate(Q4 q,V3 p) {
    V3 t=mul(cross(v3(q.x,q.y,q.z),p),2); return add(p,add(mul(t,q.w),cross(v3(q.x,q.y,q.z),t)));
}
static inline Q4 yaw_only(Q4 q) {
    V3 f=rotate(q,v3(0,0,-1)); float y=atan2f(-f.x,-f.z)*0.5f;
    return (Q4){0,sinf(y),0,cosf(y)};
}
static inline bool ray_triangle(V3 o,V3 d,V3 a,V3 b,V3 c,float *distance) {
    V3 e1=sub(b,a),e2=sub(c,a),p=cross(d,e2); float det=dot(e1,p);
    if (fabsf(det)<1e-7f) return false;
    float inv=1/det; V3 t=sub(o,a); float u=dot(t,p)*inv;
    if (u<0 || u>1) return false;
    V3 q=cross(t,e1); float v=dot(d,q)*inv;
    if (v<0 || u+v>1) return false;
    float hit=dot(e2,q)*inv;
    if (hit<0.02f || hit>=*distance) return false;
    *distance=hit; return true;
}
static inline void view_matrix(float *m,V3 p,Q4 orientation) {
    Q4 q=conjugate(orientation);
    V3 x=rotate(q,v3(1,0,0)),y=rotate(q,v3(0,1,0)),z=rotate(q,v3(0,0,1)),t=rotate(q,mul(p,-1));
    float a[16]={x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,t.x,t.y,t.z,1};
    for (int i=0;i<16;i++) m[i]=a[i];
}
static inline void projection(float *m,float left,float right,float down,float up) {
    float l=tanf(left),r=tanf(right),b=tanf(down),t=tanf(up),n=.02f,f=2000;
    for(int i=0;i<16;i++)m[i]=0;
    m[0]=2/(r-l);m[5]=2/(t-b);m[8]=(r+l)/(r-l);m[9]=(t+b)/(t-b);
    m[10]=-(f+n)/(f-n);m[11]=-1;m[14]=-2*f*n/(f-n);
}
