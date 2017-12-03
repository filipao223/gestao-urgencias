#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>

#include "global.h"

Globals globalVars;

void write_to_log(char* message){
  /*sem_wait(&globalVars.semLog);
  strcpy((globalVars.log_ptr+globalVars.log_ptr_offset), message);
  globalVars.log_ptr_offset += sizeof(message);
  sem_post(&globalVars.semLog);*/
}
