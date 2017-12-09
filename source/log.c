#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>

#include "global.h"
#include "sinais.h"

Globals globalVars;

void write_to_log(char* message){
  char toWrite[MAX_LOG_MESSAGE];
  strcpy(toWrite, message);
  #ifdef DEBUG
  printf("A entrar em write_to_log...\n");
  #endif
  if(sem_wait(globalVars.semLog) != 0){
    perror("Erro ao decrementar semLog em write_to_log\n");
    cleanup(2);
  }
  #ifdef DEBUG
  printf("A escrever no log...\n");
  #endif
  memcpy(globalVars.log_ptr+globalVars.ptr_pos, toWrite, strlen(toWrite));
  #ifdef DEBUG
  printf("Passou do memcpy...\n");
  #endif
  globalVars.ptr_pos+=strlen(toWrite);
  if(sem_post(globalVars.semLog) != 0){
    perror("Erro ao incrementar semLog em write_to_log\n");
    cleanup(2);
  }
}
