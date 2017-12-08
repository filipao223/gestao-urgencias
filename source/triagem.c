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
int n_pacientes_triados = 0;
int tempo_antes = 0;
int tempo_antes_med;

void* triaPaciente(void* t){
  Paciente paciente;
  time_t start_time = time(NULL), end_time;
  while(1){
    
    if(msgrcv(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("");
    }
    else{
      printf("Thread [%ld] recebeu paciente %s\n", pthread_self(), paciente.nome);
      //Escreve as estatisticas em memoria partilhada (por fazer)
      //Espera pelo tempo de triagem
      usleep(paciente.triage_time);
      n_pacientes_triados += 1;
      end_time = time(NULL);
      tempo_antes += start_time - end_time;
      tempo_antes_med = tempo_antes/n_pacientes_triados;
      msgsnd(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), 0);
      printf("Thread [%ld] enviou paciente %s\n", pthread_self(), paciente.nome);
    }
  }
}
