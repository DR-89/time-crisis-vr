#include "quest_options.h"
#include <stdio.h>

void qoptions_load(const char *path,QOptions *options){
    /* New/missing settings use laser assistance. Explicit saved OFF stays OFF. */
    *options=(QOptions){.laser_enabled=true};
    FILE *f=fopen(path,"r");if(!f)return;
    char line[128],extra;int value;
    while(fgets(line,sizeof line,f)){
        if(sscanf(line,"laser_enabled=%d %c",&value,&extra)==1&&(value==0||value==1))options->laser_enabled=value==1;
        if(sscanf(line,"physical_crouch=%d %c",&value,&extra)==1&&(value==0||value==1))options->physical_crouch=value==1;
        if(sscanf(line,"left_handed=%d %c",&value,&extra)==1&&(value==0||value==1))options->left_handed=value==1;
    }
    fclose(f);
}
bool qoptions_save(const char *path,const QOptions *options){
    FILE *f=fopen(path,"w");if(!f)return false;
    bool ok=fprintf(f,"laser_enabled=%d\nphysical_crouch=%d\nleft_handed=%d\n",options->laser_enabled?1:0,options->physical_crouch?1:0,options->left_handed?1:0)>0;
    if(fclose(f))ok=false;
    return ok;
}
