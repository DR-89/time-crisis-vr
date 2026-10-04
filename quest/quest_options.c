#include "quest_options.h"
#include <stdio.h>

void qoptions_load(const char *path,QOptions *options){
    /* Existing installations retain trigger control; laser remains opt-in. */
    *options=(QOptions){0};
    FILE *f=fopen(path,"r");if(!f)return;
    char line[128],extra;int value;
    while(fgets(line,sizeof line,f)){
        if(sscanf(line,"laser_enabled=%d %c",&value,&extra)==1&&(value==0||value==1))options->laser_enabled=value==1;
        if(sscanf(line,"physical_crouch=%d %c",&value,&extra)==1&&(value==0||value==1))options->physical_crouch=value==1;
    }
    fclose(f);
}
bool qoptions_save(const char *path,const QOptions *options){
    FILE *f=fopen(path,"w");if(!f)return false;
    bool ok=fprintf(f,"laser_enabled=%d\nphysical_crouch=%d\n",options->laser_enabled?1:0,options->physical_crouch?1:0)>0;
    if(fclose(f))ok=false;
    return ok;
}
