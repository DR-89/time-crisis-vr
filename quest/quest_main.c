#include <SDL2/SDL.h>
#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int tc_game_main(int,char**);
int SDL_main(int argc,char **argv) {
    (void)argc;(void)argv;
    const char *dir=SDL_AndroidGetInternalStoragePath();
    if(!dir||chdir(dir))return 2;
    remove("timecris-vr-previous.log");rename("timecris-vr.log","timecris-vr-previous.log");
    remove("previous-session.inputs");rename("last-session.inputs","previous-session.inputs");
    freopen("timecris-vr.log","w",stderr);setvbuf(stderr,NULL,_IOLBF,0);
    setenv("SDL_ACCELEROMETER_AS_JOYSTICK","0",1);
    setenv("ENG_HUD_CENTER","1",1);
    setenv("ENG_OUTPUT_GAIN","2.4",1);
    /* Fine-grained engine timers call the clock several times per polygon.
     * Keep them opt-in; the lightweight XR frame/stage counters stay enabled. */
    if(!access("profile.request",F_OK))setenv("ENG_FTIME","1",1);else unsetenv("ENG_FTIME");
    /* Bound cold texture work per original game frame. Upstream refines coarse
     * entries over subsequent frames; both stereo eyes share the same budget. */
    setenv("ENG_TEX_BUDGET","100000:100000",1);
    /* The engine records controller-derived arcade inputs, not video or audio.
     * This permits exact replay of a reported crash, even after restarting once. */
    char *args[]={"TimeCrisisVR","roms","--window","--record","last-session.inputs",NULL};
    int result=tc_game_main(5,args);
    __android_log_print(ANDROID_LOG_ERROR,"TCVR","Game ended: %d; see files/timecris-vr.log",result);
    if(result) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Time Crisis VR","Startup failed. Log: files/timecris-vr.log (adb run-as).",NULL);
    return result;
}
