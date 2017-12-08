#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>

#include "triagem.h"
#include "global.h"

//Operaçoes triagem

Globals globalVars;

void* triaPaciente(void* t){
  Paciente paciente;
  while(1){

    if(msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("");
    }
    else{
      printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);
      fflush(stdout);
      //Escreve as estatisticas em memoria partilhada (por fazer)
      //Espera pelo tempo de triagem
      usleep(paciente.triage_time);

      if(msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0) < 0){
        perror("");
      }
      else{
        printf("Thread [%ld] enviou paciente %s\n", pthread_self(), paciente.nome);
        fflush(stdout);
        //estatisticas
        if(sem_wait(globalVars.semSHM) != 0){
          perror("");
        }
        (*globalVars.n_pacientes_triados)++;

        if(sem_post(globalVars.semSHM) != 0){
          perror("");
        }
      }
    }
  }
}
