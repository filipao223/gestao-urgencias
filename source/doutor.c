#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/time.h>

#include "doutor.h"
#include "global.h"

Globals globalVars;

void trataPaciente_tempDoctor();
void* createTempDoctor();

int requestDoctor = 0;
pthread_t temp_doctor_thread;

void* createDoctors(){

  //Cria os doutores iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }
  }
  printf("Thread [%ld] criou doutores iniciais.\n", pthread_self());

  //Thread que vai criar o doutor temporario, se for preciso
  pthread_create(&temp_doctor_thread, 0, createTempDoctor, NULL);

  //Quando um acabar, começa outro
  while(1){
    wait(NULL);

    if(fork()==0){
      trataPaciente();
      exit(0);
    }
  }

  pthread_exit(NULL);
}

void trataPaciente(){
  Paciente paciente;
  struct msqid_ds* info_mq = malloc(sizeof(struct msqid_ds));
  time_t start_time = time(NULL), end_time;

  printf("Doutor [%d] começou o seu turno.\n", getpid());

  while((end_time-start_time) < globalVars.SHIFT_LENGTH){
    //Verifica o estado da message queue
    sem_wait(&globalVars.semMQ);

    //Verifica se ja foi pedido doutor adicional
    if(requestDoctor == 0){ //Ainda n foi pedido, pode ser pedido
      msgctl(globalVars.mq_id_doctor, IPC_STAT, info_mq);
      //printf("Numero de mensagens: %ld\n", info_mq->msg_qnum);
      if(info_mq->msg_qnum > globalVars.MQ_MAX){
        printf("numM: %ld\n", info_mq->msg_qnum);
        //Atingiu mais de 100% de lotaçao, faz signal a thread para fazer mais um processo (por fazer)
        printf("Requesting temporary doctor\n");
        requestDoctor = 1;
        pthread_mutex_lock(&globalVars.mutex_doctor);
        pthread_cond_signal(&globalVars.cond_var_doctor);
        pthread_mutex_unlock(&globalVars.mutex_doctor);
      }
    }
    else{
      //Já foi pedido, n pode ser pedido outra vez
    }

    sem_post(&globalVars.semMQ);

    //Recebe paciente da queue
    if(msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, IPC_NOWAIT) > 0){
      printf("Doutor [%d] recebeu paciente %s\n", getpid(), paciente.nome);
      //Escreve nas estatisticas (por fazer)

      //Espera tempo de atendimento
      usleep(paciente.atend_time*1000); //Converte para milisegundos
      printf("Doutor [%d] atendeu paciente %s\n", getpid(), paciente.nome);
    }
    else{
      usleep(100*1000);
    }
    end_time = time(NULL); //Compara com o num de segundos quando criado
  }
  printf("Doutor [%d] acabou o seu turno\n", getpid());
}

void* createTempDoctor(){
  pid_t temp_doctor;
  //Vai esperar que seja acordada para criar mais um processo temporario
  while(1){
    pthread_mutex_lock(&globalVars.mutex_doctor);

    while(!requestDoctor){
      pthread_cond_wait(&globalVars.cond_var_doctor, &globalVars.mutex_doctor);
    }

    //Doutor temporario pedido
    printf("\n\nTemporary doctor requested\n\n");
    pthread_mutex_unlock(&globalVars.mutex_doctor);

    //Cria um doutor
    if((temp_doctor = fork()) == 0){
      trataPaciente_tempDoctor();
      exit(0);
    }

    //Espera que ele acabe
    waitpid(temp_doctor, NULL, WNOHANG);
    //Acabou, volta a colocar a condiçao a 0
    pthread_mutex_lock(&globalVars.mutex_doctor);
    requestDoctor = 0;
    pthread_mutex_unlock(&globalVars.mutex_doctor);
  }
  pthread_exit(NULL);
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
    printf("Doutor temporario recebeu paciente %s\n", paciente.nome);
    //Escreve nas estatisticas (por fazer)
    //Espera tempo de atendimento
    usleep(paciente.atend_time);
    printf("Doutor temporario atendeu paciente %s\n", paciente.nome);

    if(check_exit) break;
  }

  printf("Temporary doctor ending.\n");
}
