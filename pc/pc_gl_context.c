#include <SDL2/SDL.h>
#include "quest_gpu.h"

/* Used by the upstream diagnostic render-dump path. */
SDL_GLContext eng_gl_create_win(SDL_Window **window,const char **missing){
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GLContext context=SDL_GL_CreateContext(*window);
    if(!context||!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)){
        if(missing)*missing="OpenGL 4.3";
        if(context)SDL_GL_DeleteContext(context);
        return NULL;
    }
    return context;
}
