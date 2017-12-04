#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>

#include "doutor.h"
#include "global.h"

Globals globalVars;

void* createDoctors(){
  //Cria os doutores iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }
  }

  printf("Thread [%ld] criou doutores iniciais.\n", pthread_self());

  //Vai esperar que seja acordada para criar mais um processo temporario
  /*while(1){
    //pthread_cond_wait
  }*/
}

void trataPaciente(){
  Paciente paciente;
  while(1){
    //Recebe paciente da queue
    msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0);
    printf("Doctor [%d] recebeu paciente %s\n", getpid(), paciente.nome);
    //Escreve nas estatisticas (por fazer)
    //Espera tempo de atendimento
    usleep(paciente.atend_time);
    printf("Doctor [%d] atendeu paciente %s\n", getpid(), paciente.nome);
  }
}
