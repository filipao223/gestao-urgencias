/*Autores:
  João Filipe Sequeira Montenegro Nº 2016228672
  João Miguel Rainho Mendes Nº 2016230975
*/
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

  char* time_buffer = malloc(MAX_TIME_STRING);
  return_time_format(time_buffer);
  printf("%s%s", time_buffer, message);

  if(sem_wait(globalVars.semLog) != 0){
    perror("Erro ao decrementar semLog em write_to_log\n");
    cleanup(2);
  }
  //Escreve no endereço de memoria mapeado
  for(int i=0; i<strlen(time_buffer); i++){
    globalVars.log_ptr[globalVars.ptr_pos] = time_buffer[i];
    globalVars.ptr_pos++;
  }
  for(int i=0; i<strlen(message); i++){
    globalVars.log_ptr[globalVars.ptr_pos] = message[i];
    globalVars.ptr_pos++;
  }

  globalVars.ptr_pos+=strlen(message);
  if(sem_post(globalVars.semLog) != 0){
    perror("Erro ao incrementar semLog em write_to_log\n");
    cleanup(2);
  }
}

void write_to_log_no_sems(char* message){

  char* time_buffer = malloc(MAX_TIME_STRING);
  return_time_format(time_buffer);
  printf("%s%s", time_buffer, message);

  //Escreve no endereço de memoria mapeado
  for(int i=0; i<strlen(time_buffer); i++){
    globalVars.log_ptr[globalVars.ptr_pos] = time_buffer[i];
  }
  for(int i=0; i<strlen(message); i++){
    globalVars.log_ptr[globalVars.ptr_pos] = message[i];
  }

}

void return_time_format(char* time_buffer){
  time_t timenow = time(NULL);

  struct tm* timeformatted;
  timeformatted = localtime(&timenow);

  sprintf(time_buffer, "[%d:%d:%d]", timeformatted->tm_hour, timeformatted->tm_min, timeformatted->tm_sec);
}
