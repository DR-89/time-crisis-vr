#include <GLES3/gl3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quest_gun.h"

typedef struct { float position[3],normal[3],uv[2]; } GunVertex;
static GLuint program,vao,vbo,ibo,texture;
static GLsizei index_count;
static GLint u_view,u_projection,u_model,u_albedo;
static V3 muzzle;

static GLuint compile(GLenum type,const char *source){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&source,NULL);glCompileShader(s);
    GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[2048];glGetShaderInfoLog(s,sizeof log,NULL,log);fprintf(stderr,"[GUN] shader: %s\n",log);glDeleteShader(s);return 0;}return s;
}
bool qgun_init(const char *path){
    FILE *f=fopen(path,"rb");if(!f){fprintf(stderr,"[GUN] missing model: %s\n",path);return false;}
    char magic[8];uint32_t n[4];float tip[3];
    GunVertex *vertices=NULL;uint32_t *indices=NULL;uint8_t *pixels=NULL;bool ok=false;
    if(fread(magic,1,8,f)!=8||memcmp(magic,"TCGUN001",8)||fread(n,4,4,f)!=4||fread(tip,4,3,f)!=3)goto done;
    if(!n[0]||n[0]>100000||!n[1]||n[1]>300000||n[1]%3||!n[2]||n[2]>2048||!n[3]||n[3]>2048)goto done;
    vertices=malloc((size_t)n[0]*sizeof *vertices);indices=malloc((size_t)n[1]*4);pixels=malloc((size_t)n[2]*n[3]*4);
    if(!vertices||!indices||!pixels)goto done;
    if(fread(vertices,sizeof *vertices,n[0],f)!=n[0]||fread(indices,4,n[1],f)!=n[1]||fread(pixels,4,(size_t)n[2]*n[3],f)!=(size_t)n[2]*n[3]||fgetc(f)!=EOF)goto done;
    for(uint32_t i=0;i<n[1];i++)if(indices[i]>=n[0])goto done;
    for(uint32_t i=0;i<n[0];i++)for(int j=0;j<8;j++)if(!isfinite(((float*)&vertices[i])[j]))goto done;
    for(int i=0;i<3;i++)if(!isfinite(tip[i]))goto done;
    GLuint vs=compile(GL_VERTEX_SHADER,
        "#version 300 es\nprecision highp float;layout(location=0) in vec3 position;layout(location=1) in vec3 normal;layout(location=2) in vec2 texcoord;uniform mat4 view,projection,model;out vec3 n,p;out vec2 uv;void main(){mat4 mv=view*model;vec4 v=mv*vec4(position,1);p=v.xyz;n=mat3(mv)*normal;uv=texcoord;gl_Position=projection*v;}");
    GLuint fs=compile(GL_FRAGMENT_SHADER,
        "#version 300 es\nprecision highp float;in vec3 n,p;in vec2 uv;uniform sampler2D albedo;out vec4 frag;void main(){vec3 N=normalize(n);if(!gl_FrontFacing)N=-N;vec3 L=normalize(vec3(-0.4,0.8,0.6));vec3 V=normalize(-p);vec3 H=normalize(L+V);vec3 base=texture(albedo,uv).rgb;float diffuse=0.48+0.52*max(dot(N,L),0.0);float spec=0.16*pow(max(dot(N,H),0.0),40.0);frag=vec4(pow(clamp(base*diffuse+spec,0.0,1.0),vec3(1.0/2.2)),1);}");
    if(!vs||!fs){if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);goto done;}
    program=glCreateProgram();glAttachShader(program,vs);glAttachShader(program,fs);glLinkProgram(program);glDeleteShader(vs);glDeleteShader(fs);
    GLint linked;glGetProgramiv(program,GL_LINK_STATUS,&linked);if(!linked)goto done;
    u_view=glGetUniformLocation(program,"view");u_projection=glGetUniformLocation(program,"projection");u_model=glGetUniformLocation(program,"model");u_albedo=glGetUniformLocation(program,"albedo");
    GLint binding;glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
    glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,GL_SRGB8_ALPHA8,n[2],n[3],0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,binding);
    glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenBuffers(1,&vbo);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)n[0]*sizeof *vertices,vertices,GL_STATIC_DRAW);
    glGenBuffers(1,&ibo);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)n[1]*4,indices,GL_STATIC_DRAW);
    for(int i=0;i<3;i++){glEnableVertexAttribArray(i);glVertexAttribPointer(i,i==2?2:3,GL_FLOAT,GL_FALSE,sizeof(GunVertex),(void*)(size_t)(i==0?0:i==1?12:24));}
    index_count=(GLsizei)n[1];muzzle=v3(tip[0],tip[1],tip[2]);
    ok=glGetError()==GL_NO_ERROR;
    if(ok)fprintf(stderr,"[GUN] Tripo model loaded: %u vertices, %u triangles, %ux%u albedo\n",n[0],n[1]/3,n[2],n[3]);
done:
    fclose(f);free(vertices);free(indices);free(pixels);
    if(!ok){fprintf(stderr,"[GUN] model initialization failed\n");qgun_shutdown();}return ok;
}
V3 qgun_muzzle(V3 position,Q4 rotation){return add(position,rotate(rotation,muzzle));}
void qgun_draw(const float view[16],const float projection[16],V3 position,Q4 rotation,float recoil){
    if(!program)return;
    /* Recoil is visual only: it cannot change the game's shot coordinates. */
    float kick=fminf(1,fmaxf(0,recoil));
    position=add(position,rotate(rotation,v3(0,0,.014f*kick)));
    rotation=product(rotation,(Q4){sinf(.045f*kick),0,0,cosf(.045f*kick)});
    V3 x=rotate(rotation,v3(1,0,0)),y=rotate(rotation,v3(0,1,0)),z=rotate(rotation,v3(0,0,1));
    float model[16]={x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,position.x,position.y,position.z,1};
    GLint binding,active_texture;glGetIntegerv(GL_ACTIVE_TEXTURE,&active_texture);glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
    glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);
    /* The arcade has painter-sorted polygons. This foreground pass uses its own
     * cleared depth buffer for the weapon's self-occlusion. */
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(GL_TRUE);glClearDepthf(1);glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);glUniformMatrix4fv(u_view,1,GL_FALSE,view);glUniformMatrix4fv(u_projection,1,GL_FALSE,projection);glUniformMatrix4fv(u_model,1,GL_FALSE,model);glUniform1i(u_albedo,0);
    glBindTexture(GL_TEXTURE_2D,texture);glBindVertexArray(vao);glDrawElements(GL_TRIANGLES,index_count,GL_UNSIGNED_INT,0);
    glDisable(GL_DEPTH_TEST);glBindTexture(GL_TEXTURE_2D,binding);glActiveTexture(active_texture);
}
void qgun_shutdown(void){
    glDeleteProgram(program);glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);glDeleteBuffers(1,&ibo);glDeleteTextures(1,&texture);
    program=vao=vbo=ibo=texture=0;index_count=0;
}
