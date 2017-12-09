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
#include "log.h"

//Operaçoes triagem

Globals globalVars;

void* triaPaciente(void* t){
  Paciente paciente;
  printf("Thread %ld criada\n", pthread_self());
  /*char message[MAX_LOG_MESSAGE];
  sprintf(message, "Thread %ld criada\n", pthread_self());
  write_to_log(message);*/

  while(1){
    if(msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("Erro ao receber da queue em triaPaciente\n");
    }

    //Pára o contador do tempo antes da triagem e Calcula o tempo
    struct timeval cont_tempo;
    gettimeofday(&cont_tempo, NULL);
    suseconds_t temp = cont_tempo.tv_usec;
    temp-=paciente.before_triage;

    //Coloca o tempo no total de microsegundos
    if(sem_wait(globalVars.semSHM) != 0){
      perror("Erro ao decrementar semSHM em triaPaciente\n");
    }
    (*globalVars.total_before_triage)+=temp;
    if(sem_post(globalVars.semSHM) != 0){
      perror("Erro ao incrementar semSHM em triaPaciente\n");
    }

    printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);

    //Espera pelo tempo de triagem
    usleep(paciente.triage_time);

    //Inicia o contador de tempo entre a triagem e ser atendido
    struct timeval cont_tempo2;
    gettimeofday(&cont_tempo2, NULL);
    paciente.before_atend = cont_tempo2.tv_usec;

    if(msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0) < 0){
      perror("Erro ao enviar para a queue em triaPaciente\n");
    }
    else{
      printf("Thread [%ld] enviou paciente %s\n", pthread_self(), paciente.nome);
      //aumenta o numero de pacientes triados
      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em triaPaciente (2)\n");
      }
      (*globalVars.n_pacientes_triados)++;

      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em triaPaciente (2)\n");
      }
    }
  }
}
