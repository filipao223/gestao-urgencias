#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>

#include "global.h"

Globals globalVars;

void write_to_log(char* message){
  int i;
  sem_wait(globalVars.semLog);
  for(i=0; i<strlen(message); i++){
    globalVars.log_ptr[globalVars.ptr_pos] = message[i];
  }
  globalVars.ptr_pos+=(i-1);
  sem_post(globalVars.semLog);
}
