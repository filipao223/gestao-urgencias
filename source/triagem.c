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

void* createTriage(void* t){
  Paciente paciente;
  while(1){
    msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0);
    printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);
    //Escreve as estatisticas em memoria partilhada (por fazer)
    //Espera pelo tempo de triagem
    usleep(paciente.triage_time);
    msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0);
    printf("Thread [%ld] enviou paciente %s\n", pthread_self(), paciente.nome);
  }
}
