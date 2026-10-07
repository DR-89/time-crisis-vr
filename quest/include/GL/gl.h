/* Minimal fixed-function adapter for the engine's actual GL calls, backed by GLES 3. */
#pragma once
#include "quest_gpu.h"
#ifndef APIENTRY
#define APIENTRY GL_APIENTRY
#endif
#define GL_VERSION_1_3 1
#define GL_QUADS 0x0007
#define GL_ALPHA_TEST 0x0BC0
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_MODULATE 0x2100
#define GL_REPLACE 0x1E01
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_RGB_SCALE 0x8573
#define GL_INTERPOLATE 0x8575
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_VERTEX_ARRAY 0x8074
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
void qglEnable(GLenum);void qglDisable(GLenum);void qglActiveTexture(GLenum);
void qglAlphaFunc(GLenum,float);void qglBegin(GLenum);void qglEnd(void);
void qglColor4f(float,float,float,float);void qglTexCoord2f(float,float);void qglTexCoord4f(float,float,float,float);void qglVertex2f(float,float);
void qglVertex4f(float,float,float,float);
void qglEnableClientState(GLenum);void qglDisableClientState(GLenum);
void qglVertexPointer(GLint,GLenum,GLsizei,const void*);void qglColorPointer(GLint,GLenum,GLsizei,const void*);void qglTexCoordPointer(GLint,GLenum,GLsizei,const void*);
void qglDrawArrays(GLenum,GLint,GLsizei);
void qglBindTexture(GLenum,GLuint);void qglBlendFunc(GLenum,GLenum);void qglColorMask(GLboolean,GLboolean,GLboolean,GLboolean);void qglClear(GLbitfield);
void qglTexImage2D(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*);
void qglTexSubImage2D(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void*);
void qglTexParameteri(GLenum,GLenum,GLint);void qglDeleteTextures(GLsizei,const GLuint*);
void qglTexEnvi(GLenum,GLenum,GLint);void qglTexEnvf(GLenum,GLenum,GLfloat);void qglTexEnvfv(GLenum,GLenum,const GLfloat*);
void qglMatrixMode(GLenum);void qglLoadIdentity(void);void qglOrtho(double,double,double,double,double,double);void qglScissor(GLint,GLint,GLsizei,GLsizei);
#ifndef QGL_IMPLEMENTATION
#undef glEnable
#define glEnable qglEnable
#undef glDisable
#define glDisable qglDisable
#undef glActiveTexture
#define glActiveTexture qglActiveTexture
#undef glAlphaFunc
#define glAlphaFunc qglAlphaFunc
#undef glBegin
#define glBegin qglBegin
#undef glEnd
#define glEnd qglEnd
#undef glColor4f
#define glColor4f qglColor4f
#undef glTexCoord2f
#define glTexCoord2f qglTexCoord2f
#undef glTexCoord4f
#define glTexCoord4f qglTexCoord4f
#undef glVertex2f
#define glVertex2f qglVertex2f
#undef glVertex4f
#define glVertex4f qglVertex4f
#undef glEnableClientState
#define glEnableClientState qglEnableClientState
#undef glDisableClientState
#define glDisableClientState qglDisableClientState
#undef glVertexPointer
#define glVertexPointer qglVertexPointer
#undef glColorPointer
#define glColorPointer qglColorPointer
#undef glTexCoordPointer
#define glTexCoordPointer qglTexCoordPointer
#undef glDrawArrays
#define glDrawArrays qglDrawArrays
#undef glBindTexture
#define glBindTexture qglBindTexture
#undef glBlendFunc
#define glBlendFunc qglBlendFunc
#undef glColorMask
#define glColorMask qglColorMask
#undef glClear
#define glClear qglClear
#undef glTexImage2D
#define glTexImage2D qglTexImage2D
#undef glTexSubImage2D
#define glTexSubImage2D qglTexSubImage2D
#undef glTexParameteri
#define glTexParameteri qglTexParameteri
#undef glDeleteTextures
#define glDeleteTextures qglDeleteTextures
#undef glTexEnvi
#define glTexEnvi qglTexEnvi
#undef glTexEnvf
#define glTexEnvf qglTexEnvf
#undef glTexEnvfv
#define glTexEnvfv qglTexEnvfv
#undef glMatrixMode
#define glMatrixMode qglMatrixMode
#undef glLoadIdentity
#define glLoadIdentity qglLoadIdentity
#undef glOrtho
#define glOrtho qglOrtho
#undef glScissor
#define glScissor qglScissor
#endif
