/* Run the render fixture against the actual Android GLES driver, offscreen. */
#include <EGL/egl.h>
#define __declspec(x)
#define RENDERER_SOURCE "../quest/quest_gl.c"
#define TEST_SCENE_TARGET
#define TEST_ATLAS_ARRAY
#include "render_fixture.c"

int main(int argc,char **argv){
    if(argc!=2)return 2;
    EGLDisplay display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(!eglInitialize(display,NULL,NULL)){fprintf(stderr,"EGL initialize failed: %x\n",eglGetError());return 3;}
    const EGLint attributes[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,0x40,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
    EGLConfig config;EGLint count=0;
    if(!eglChooseConfig(display,attributes,&config,1,&count)||!count)return 4;
    for(int variant=0;variant<4;variant++){
        const EGLint surface_attributes[]={EGL_WIDTH,256,EGL_HEIGHT,256,EGL_NONE};
        const EGLint context_attributes[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
        EGLSurface surface=eglCreatePbufferSurface(display,config,surface_attributes);
        EGLContext context=eglCreateContext(display,config,EGL_NO_CONTEXT,context_attributes);
        if(!surface||!context||!eglMakeCurrent(display,surface,surface,context))return 5;
        if(!variant)fprintf(stderr,"GLES renderer: %s\n",glGetString(GL_RENDERER));
#ifdef TEST_MULTIVIEW
        render_fixture_proc((void*)eglGetProcAddress("glFramebufferTextureMultiviewOVR"));
#endif
        uint8_t *pixels=malloc(256*256*4);if(!pixels)return 6;
        int status=render_fixture(variant,256,256,pixels);
        if(status!=1){fprintf(stderr,"Render variant %d failed: %d\n",variant,status);free(pixels);return 7;}
        char name[512];snprintf(name,sizeof name,"%s/frame-%d.rgba",argv[1],variant);
        FILE *file=fopen(name,"wb");if(!file){free(pixels);return 8;}
        size_t written=fwrite(pixels,1,256*256*4,file);free(pixels);
        int closed=fclose(file);if(written!=256*256*4||closed)return 9;
        eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        eglDestroyContext(display,context);eglDestroySurface(display,surface);
    }
    eglTerminate(display);puts("PASS: four eye/gamma views, no GLES errors");return 0;
}
