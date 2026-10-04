#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

int tc_game_main(int,char **);
/* SDL supplies the discrete-GPU selection exports for hybrid laptops. */

int main(int argc,char **argv){
    SDL_SetMainReady();
    int headless=0,no_dialog=0,replay=0;
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--headless"))headless=1;
        if(!strcmp(argv[i],"--no-dialog"))no_dialog=1;
        if(!strcmp(argv[i],"--replay"))replay=1;
    }
    char *base=SDL_GetBasePath();if(base){_chdir(base);SDL_free(base);}
    remove("timecris-previous.log");rename("timecris.log","timecris-previous.log");
    /* The engine uses CRT getenv; SDL_setenv only updates Win32's table. */
    setenv("ENG_HUD_CENTER","1",1);
    setenv("ENG_OUTPUT_GAIN","2.4",1);
    setenv("ENG_TEX_BUDGET","250000:350000",0);
    if(!replay){remove("previous-session.inputs");rename("last-session.inputs","previous-session.inputs");}
    char **args=calloc((size_t)argc+6,sizeof *args);if(!args)return 2;
    int n=0;args[n++]=argv[0];args[n++]="roms";if(!headless)args[n++]="--window";
    if(!replay){args[n++]="--record";args[n++]="last-session.inputs";}
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--desktop"))setenv("TCVR_DESKTOP","1",1);
        else if(!strcmp(argv[i],"--headless")||!strcmp(argv[i],"--no-dialog"))continue;
        else args[n++]=argv[i];
    }
    int result=tc_game_main(n,args);free(args);
    if(result&&!headless&&!no_dialog)SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Time Crisis VR",
        "Startup failed. See timecris.log next to the executable.\nFor VR, connect your headset and select an OpenXR runtime with OpenGL support (such as SteamVR).\nUse Play Desktop.cmd to play without a headset.",NULL);
    return result;
}
