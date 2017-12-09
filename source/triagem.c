#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

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
    if(msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("");
    }

    //Pára o contador do tempo antes da triagem e Calcula o tempo
    struct timeval cont_tempo;
    gettimeofday(&cont_tempo, NULL);
    suseconds_t temp = cont_tempo.tv_usec;
    temp-=paciente.before_triage;

    //Coloca o tempo no total de microsegundos
    if(sem_wait(globalVars.semSHM) != 0){
      perror("");
    }
    (*globalVars.total_before_triage)+=temp;
    if(sem_post(globalVars.semSHM) != 0){
      perror("");
    }

    paciente.atend_time = tempo_atend;
    printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);
    gettimeofday(&start_triage,NULL);
    //Escreve as estatisticas em memoria partilhada (por fazer)
    //Espera pelo tempo de triagem
    usleep(paciente.triage_time);

    //Inicia o contador de tempo entre a triagem e ser atendido
    struct timeval cont_tempo2;
    gettimeofday(&cont_tempo2, NULL);
    paciente.before_atend = cont_tempo2.tv_usec;

    if(msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0) < 0){
      perror("");
    }
    else{
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
