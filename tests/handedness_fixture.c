/* Exercise the real host input path against a deterministic OpenXR runtime.
 * Render/startup entry points are excluded; no headset is required. */
#include <assert.h>
#define TCVR_INPUT_TEST
#include "../quest/quest_host.c"

static char paths[40][100];
static unsigned path_count,action_count,space_count,binding_count;
static XrActionSuggestedBinding suggested[16];
static XrActionType types[10];
static bool dual[10],buttons[10][3],connected[2]={true,true},tracked[2]={true,true};
static float triggers[2];
static XrPath last_haptic,last_stop;
static XrResult sync_result=XR_SUCCESS;
static uint16_t arcade_bits;
uint16_t g_ss22_gun_x,g_ss22_gun_y;
bool g_ss22_gun_off;
static unsigned id(XrAction a){return (unsigned)(uintptr_t)a;}
static int hand_index(XrPath p){
    if(p==XR_NULL_PATH)return 2;
    if(p==hand_paths[LEFT_HAND])return LEFT_HAND;
    assert(p==hand_paths[RIGHT_HAND]);return RIGHT_HAND;
}
XRAPI_ATTR XrResult XRAPI_CALL xrStringToPath(XrInstance i,const char *s,XrPath *p){
    (void)i;for(unsigned n=1;n<=path_count;n++)if(!strcmp(paths[n],s)){*p=n;return XR_SUCCESS;}
    assert(path_count+1<40);*p=++path_count;snprintf(paths[*p],100,"%s",s);return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrResultToString(XrInstance i,XrResult r,char buffer[XR_MAX_RESULT_STRING_SIZE]){
    (void)i;snprintf(buffer,XR_MAX_RESULT_STRING_SIZE,"Mock result %d",r);return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSet(XrInstance i,const XrActionSetCreateInfo *c,XrActionSet *out){
    (void)i;(void)c;*out=(XrActionSet)(uintptr_t)1;return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrCreateAction(XrActionSet set,const XrActionCreateInfo *c,XrAction *out){
    (void)set;unsigned n=++action_count;assert(n<10);types[n]=c->actionType;
    dual[n]=c->countSubactionPaths==2;
    if(dual[n]){assert(c->subactionPaths[0]==hand_paths[0]);assert(c->subactionPaths[1]==hand_paths[1]);}
    else assert(c->countSubactionPaths==0);
    *out=(XrAction)(uintptr_t)n;return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrSuggestInteractionProfileBindings(XrInstance i,const XrInteractionProfileSuggestedBinding *s){
    (void)i;assert(!strcmp(paths[s->interactionProfile],"/interaction_profiles/oculus/touch_controller"));
    binding_count=s->countSuggestedBindings;assert(binding_count<=16);
    memcpy(suggested,s->suggestedBindings,binding_count*sizeof suggested[0]);return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrAttachSessionActionSets(XrSession s,const XrSessionActionSetsAttachInfo *a){
    (void)s;assert(a->countActionSets==1&&a->actionSets[0]==action_set);return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSpace(XrSession s,const XrActionSpaceCreateInfo *c,XrSpace *out){
    (void)s;assert(c->action==aim_action&&dual[id(c->action)]);assert(c->poseInActionSpace.orientation.w==1);
    int hand=hand_index(c->subactionPath);assert(hand<2);*out=(XrSpace)(uintptr_t)(hand+1);space_count++;return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrSyncActions(XrSession s,const XrActionsSyncInfo *c){
    (void)s;assert(c->countActiveActionSets==1);return sync_result;
}
XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateBoolean(XrSession s,const XrActionStateGetInfo *get,XrActionStateBoolean *out){
    (void)s;unsigned a=id(get->action);int h=hand_index(get->subactionPath);
    assert(types[a]==XR_ACTION_TYPE_BOOLEAN_INPUT);assert(dual[a]?h<2:h==2);
    out->currentState=buttons[a][h];out->isActive=h==2||connected[h];return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateFloat(XrSession s,const XrActionStateGetInfo *get,XrActionStateFloat *out){
    (void)s;assert(get->action==trigger_action&&dual[id(get->action)]);int h=hand_index(get->subactionPath);assert(h<2);
    out->isActive=connected[h];out->currentState=triggers[h];return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStatePose(XrSession s,const XrActionStateGetInfo *get,XrActionStatePose *out){
    (void)s;assert(get->action==aim_action&&dual[id(get->action)]);int h=hand_index(get->subactionPath);assert(h<2);
    out->isActive=connected[h];return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrLocateSpace(XrSpace space,XrSpace base,XrTime t,XrSpaceLocation *out){
    (void)base;(void)t;int h=(int)(uintptr_t)space-1;assert(h==0||h==1);
    out->locationFlags=tracked[h]?XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_VALID_BIT:0;
    out->pose=(XrPosef){{0,0,0,1},{h==LEFT_HAND?-.25f:.25f,1.4f,-.5f}};return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrApplyHapticFeedback(XrSession s,const XrHapticActionInfo *info,const XrHapticBaseHeader *v){
    (void)s;(void)v;assert(info->action==haptic_action&&dual[id(info->action)]);
    assert(hand_index(info->subactionPath)<2);last_haptic=info->subactionPath;return XR_SUCCESS;
}
XRAPI_ATTR XrResult XRAPI_CALL xrStopHapticFeedback(XrSession s,const XrHapticActionInfo *info){
    (void)s;assert(info->action==haptic_action);last_stop=info->subactionPath;return XR_SUCCESS;
}
void eng_audio_set_volume(int v){(void)v;}
void ss22_snd_inputs(uint16_t bits,unsigned w,unsigned p1,unsigned p2){(void)w;(void)p1;(void)p2;arcade_bits=bits;}
V3 qgun_muzzle(V3 p,Q4 q){return add(p,rotate(q,v3(0,0,-.1f)));}
bool qvr_aim(V3 p,V3 d,float *x,float *y,V3 *hit){*x=p.x+.5f;*y=.5f;*hit=add(p,mul(d,2.f));return true;}
static void tick(void){sync_input(100,true,1.7f);ss22_input_update();}
static void click(XrAction a,int h){buttons[id(a)][h]=true;tick();buttons[id(a)][h]=false;tick();}
static void bound(XrAction a,const char *name){
    for(unsigned i=0;i<binding_count;i++)if(suggested[i].action==a&&!strcmp(paths[suggested[i].binding],name))return;
    fprintf(stderr,"Missing binding: %s\n",name);abort();
}
int main(void){
    options_path="handedness-test.cfg";qoptions_load(options_path,&options);
    assert(!options.left_handed);instance=(XrInstance)(uintptr_t)1;session=(XrSession)(uintptr_t)1;
    running=focused=origin_set=true;assert(actions_init());assert(space_count==2&&binding_count==12);
    for(int h=0;h<2;h++){
        char name[100];const char *prefix=h?"/user/hand/right":"/user/hand/left";
        snprintf(name,sizeof name,"%s/input/aim/pose",prefix);bound(aim_action,name);
        snprintf(name,sizeof name,"%s/input/trigger/value",prefix);bound(trigger_action,name);
        snprintf(name,sizeof name,"%s/input/%s/click",prefix,h?"a":"x");bound(lower_action,name);
        snprintf(name,sizeof name,"%s/input/%s/click",prefix,h?"b":"y");bound(upper_action,name);
        snprintf(name,sizeof name,"%s/output/haptic",prefix);bound(haptic_action,name);
    }
    bound(pause_action,"/user/hand/left/input/menu/click");bound(hand_action,"/user/hand/right/input/thumbstick/click");
    tick();assert(gun_position.x==.25f&&gun_origin.x==.25f&&aim_valid);
    triggers[RIGHT_HAND]=.9f;tick();assert((arcade_bits&0x30)==0x10);
    triggers[LEFT_HAND]=.8f;triggers[RIGHT_HAND]=0;tick();assert((arcade_bits&0x30)==0x20);
    ss22_input_rumble(0,48000,45);assert(last_haptic==hand_paths[RIGHT_HAND]);
    click(hand_action,2);assert(!options.left_handed&&!paused); /* Menu-only shortcut. */
    click(pause_action,2);assert(paused);
    recoil_started=500;click(hand_action,2);
    assert(options.left_handed&&paused&&options_saved&&!recoil_started);
    assert(last_stop==hand_paths[RIGHT_HAND]);assert(gun_position.x==-.25f&&gun_origin.x==-.25f);
    QOptions saved;qoptions_load(options_path,&saved);assert(saved.left_handed&&saved.laser_enabled);
    assert(arcade_bits==0);click(pause_action,2);assert(!paused);
    assert(trigger==0); /* Former cover trigger is still held: must release before firing. */
    triggers[LEFT_HAND]=0;tick();triggers[LEFT_HAND]=.9f;tick();assert((arcade_bits&0x30)==0x10);
    triggers[LEFT_HAND]=0;triggers[RIGHT_HAND]=.8f;tick();assert((arcade_bits&0x30)==0x20);
    ss22_input_rumble(0,48000,45);assert(last_haptic==hand_paths[LEFT_HAND]);
    click(lower_action,LEFT_HAND);assert(coin_frames>0);coin_frames=0;recenter_requested=false;
    click(lower_action,RIGHT_HAND);assert(recenter_requested&&coin_frames==0);
    click(upper_action,LEFT_HAND);assert(!options.laser_enabled&&!paused);
    click(upper_action,RIGHT_HAND);assert(!options.physical_crouch&&!paused);
    click(pause_action,2);click(upper_action,RIGHT_HAND);assert(options.physical_crouch);
    click(pause_action,2);triggers[RIGHT_HAND]=0;tick();assert(pedal==1);
    sync_input(100,true,1.45f);assert(pedal==0);tick();assert(pedal==1);
    sync_input(100,false,1.7f);assert(pedal==0);tick();
    options.physical_crouch=false;triggers[RIGHT_HAND]=.8f;tracked[LEFT_HAND]=false;
    tick();assert(!gun_tracked&&!aim_valid&&trigger==0&&pedal==.8f);
    triggers[LEFT_HAND]=.9f;tracked[LEFT_HAND]=true;tick();assert(trigger==0);
    triggers[LEFT_HAND]=0;tick();triggers[LEFT_HAND]=.9f;tick();assert(trigger==.9f);
    connected[RIGHT_HAND]=false;tick();assert(pedal==0&&gun_tracked);
    connected[RIGHT_HAND]=true;
    focused=false;tick();assert(arcade_bits==0);
    buttons[id(upper_action)][LEFT_HAND]=true;focused=true;tick();
    assert(!options.laser_enabled&&trigger==0); /* Held across focus loss must not toggle/fire. */
    buttons[id(upper_action)][LEFT_HAND]=false;tick();click(upper_action,LEFT_HAND);assert(options.laser_enabled);
    sync_result=XR_ERROR_RUNTIME_FAILURE;tick();assert(trigger==0&&pedal==0);
    buttons[id(upper_action)][LEFT_HAND]=true;sync_result=XR_SUCCESS;tick();assert(options.laser_enabled);
    buttons[id(upper_action)][LEFT_HAND]=false;tick();
    /* Held face buttons must not acquire their new roles when hands switch. */
    click(pause_action,2);buttons[id(lower_action)][LEFT_HAND]=true;buttons[id(upper_action)][LEFT_HAND]=true;tick();
    bool laser=options.laser_enabled;bool physical=options.physical_crouch;recenter_requested=false;
    buttons[id(hand_action)][2]=true;tick();assert(!options.left_handed&&paused);
    for(int n=0;n<6;n++)tick();
    assert(!options.left_handed&&!recenter_requested&&coin_frames==0);
    assert(options.laser_enabled==laser&&options.physical_crouch==physical);
    assert(last_stop==hand_paths[LEFT_HAND]);
    memset(buttons,0,sizeof buttons);triggers[0]=triggers[1]=0;tick();
    click(lower_action,RIGHT_HAND);assert(coin_frames==36);coin_frames=0;
    click(lower_action,LEFT_HAND);assert(recenter_requested);
    click(upper_action,RIGHT_HAND);assert(options.laser_enabled!=laser);
    click(upper_action,LEFT_HAND);assert(options.physical_crouch!=physical);
    puts("PASS: real host OpenXR bindings, both aim poses, fire/cover/credits/recenter/laser, menu-only hand switching, saved hand, haptics, held buttons/triggers, focus and tracking loss");
    return 0;
}
