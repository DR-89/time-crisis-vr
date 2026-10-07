#include "quest_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

void qoptions_load(const char *path,QOptions *options){
    /* New/missing settings use laser assistance. Explicit saved OFF stays OFF. */
    *options=(QOptions){.laser_enabled=true};
    FILE *f=fopen(path,"r");if(!f)return;
    char line[128],extra;int value;
    while(fgets(line,sizeof line,f)){
        if(sscanf(line,"laser_enabled=%d %c",&value,&extra)==1&&(value==0||value==1))options->laser_enabled=value==1;
        if(sscanf(line,"physical_crouch=%d %c",&value,&extra)==1&&(value==0||value==1))options->physical_crouch=value==1;
        if(sscanf(line,"left_handed=%d %c",&value,&extra)==1&&(value==0||value==1))options->left_handed=value==1;
        if(!strncmp(line,"gun_pitch=",10)){
            char *end;errno=0;long pitch=strtol(line+10,&end,10);
            if(end!=line+10&&!errno&&pitch>=QOPTIONS_GUN_PITCH_MIN&&pitch<=QOPTIONS_GUN_PITCH_MAX){
                while(isspace((unsigned char)*end))end++;
                if(!*end)options->gun_pitch=(int)pitch;
            }
        }
    }
    fclose(f);
}
bool qoptions_save(const char *path,const QOptions *options){
    FILE *f=fopen(path,"w");if(!f)return false;
    bool ok=fprintf(f,"laser_enabled=%d\nphysical_crouch=%d\nleft_handed=%d\ngun_pitch=%d\n",options->laser_enabled?1:0,options->physical_crouch?1:0,options->left_handed?1:0,options->gun_pitch)>0;
    if(fclose(f))ok=false;
    return ok;
}
