#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>

#include "doutor.h"
#include "global.h"

Globals globalVars;

void trataPaciente_tempDoctor();

void* createDoctors(){
  pid_t temp_doctor;

  //Cria os doutores iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }
  }

  printf("Thread [%ld] criou doutores iniciais.\n", pthread_self());

  //Vai esperar que seja acordada para criar mais um processo temporario
  while(1){
    pthread_cond_wait(&globalVars.cond_var_doctor, &globalVars.mutex_doctor);
    //Foi chamada, cria um processo doutor adicional
    printf("\n\n\n\nCreating temporary doctor\n\n\n\n\n");
    if((temp_doctor = fork()) == 0){
      trataPaciente_tempDoctor();
      exit(0);
    }
    //Espera que ele acabe
    waitpid(temp_doctor, NULL, WNOHANG);
  }
}

void trataPaciente(){
  Paciente paciente;
  struct msqid_ds* info_mq = malloc(sizeof(struct msqid_ds));

  while(1){
    //Verifica o estado da message queue
    //Verifica se ja foi pedido doutor adicional
    sem_wait(&globalVars.semMQ);
    msgctl(globalVars.mq_id_doctor, IPC_STAT, info_mq);
    printf("Numero de mensagens: %ld\n", info_mq->msg_qnum);
    if(info_mq->msg_qnum > globalVars.MQ_MAX){
      printf("numM: %ld\n", info_mq->msg_qnum);
      //Atingiu mais de 100% de lotaçao, faz signal a thread para fazer mais um processo (por fazer)
      printf("Requesting temporary doctor\n");
      pthread_mutex_lock(&globalVars.mutex_doctor);
      pthread_cond_signal(&globalVars.cond_var_doctor);
      pthread_mutex_unlock(&globalVars.mutex_doctor);
    }
    sem_post(&globalVars.semMQ);

    //Recebe paciente da queue
    msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0);
    printf("Doctor [%d] recebeu paciente %s\n", getpid(), paciente.nome);
    //Escreve nas estatisticas (por fazer)
    //Espera tempo de atendimento
    usleep(paciente.atend_time);
    printf("Doctor [%d] atendeu paciente %s\n", getpid(), paciente.nome);
  }
}

//Função para o doutor temporario
void trataPaciente_tempDoctor(){
  Paciente paciente;
  struct msqid_ds* info_mq = malloc(sizeof(struct msqid_ds));
  int check_exit = 0;

  while(1){
    //Verifica o estado da message queue
    sem_wait(&globalVars.semMQ);
    msgctl(globalVars.mq_id_doctor, IPC_STAT, info_mq);
    if(info_mq->msg_qnum < ((globalVars.MQ_MAX*80*100)/100.0)){
      //Voltou a menos de 80% de lotação, acaba este processo nesta iteraçao
      check_exit = 1;
    }
    sem_post(&globalVars.semMQ);

    //Recebe paciente da queue
    msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0);
    printf("Doctor [%d] recebeu paciente %s\n", getpid(), paciente.nome);
    //Escreve nas estatisticas (por fazer)
    //Espera tempo de atendimento
    usleep(paciente.atend_time);
    printf("Doctor [%d] atendeu paciente %s\n", getpid(), paciente.nome);

    if(check_exit) break;
  }

  printf("Temporary doctor ending.\n");
}
