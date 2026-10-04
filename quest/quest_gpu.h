#pragma once
#ifdef TCVR_PC
#include <glad/gl.h>
#define GL_APIENTRY GLAD_API_PTR
#include <string.h>
/* Desktop GL 4.3 understands the ES precision qualifiers. Only the version
 * directive differs, keeping the actual rendering math identical. */
static inline void qgpu_shader_source(GLuint shader,const char *source){
    const char *parts[]={"#version 430 core\n",strchr(source,'\n')+1};
    glShaderSource(shader,2,parts,NULL);
}
#else
#include <GLES3/gl3.h>
static inline void qgpu_shader_source(GLuint shader,const char *source){glShaderSource(shader,1,&source,NULL);}
#endif
