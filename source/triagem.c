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
struct timeval stop_atend, start_atend,stop_triagem, start_triage;
int tempo_atend = 0;
int tempo_triagem = 0;

void* triaPaciente(void* t){
  Paciente paciente;
  while(1){

    gettimeofday(&start_atend,NULL);
    if(msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("");
    }
    gettimeofday(&stop_atend,NULL);
    tempo_atend += stop_atend.tv_usec - start_atend.tv_usec;
    paciente.atend_time = tempo_atend;
    else{
      printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);
      gettimeofday(&start_triage,NULL);
      //Escreve as estatisticas em memoria partilhada (por fazer)
      //Espera pelo tempo de triagem
      usleep(paciente.triage_time);

      if(msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0) < 0){
        perror("");
      }
      else{
        gettimeofday(&stop_triage,NULL);
        tempo_triagem += stop_triage.tv_usec - start_triage.tv_usec;
        paciente.triage_time = tempo_triagem;
        printf("Thread [%ld] enviou paciente %s\n", pthread_self(), paciente.nome);
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
