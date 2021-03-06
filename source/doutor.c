/*Autores:
  João Filipe Sequeira Montenegro Nº 2016228672
  João Miguel Rainho Mendes Nº 2016230975
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <string.h>
#include <sys/mman.h>

#include "doutor.h"
#include "global.h"
#include "sinais.h"
#include "log.h"

Globals globalVars;

void trataPaciente_tempDoctor();
void* createTempDoctor();

void ignore_signal(int signum);

void* createDoctors(){
  //Cria os doutores iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if((globalVars.pid = fork()) == 0){
      signal(SIGINT, cleanup);

      //Ignora os outros sinais
      for(int i=1; i<31; i++){
        if((i!=2) && (i!=10) && (i!=11)){
          signal(i, ignore_signal);
        }
      }

      trataPaciente();
      exit(0);
    }
    else if(globalVars.pid < 0){
      perror("Erro ao criar doutor\n");
    }
  }

  printf("Thread [%ld] criou doutores iniciais.\n", pthread_self());

  //Quando um acabar, começa outro
  while(1){
    wait(NULL);
    if((globalVars.pid = fork())==0){
      trataPaciente();
      exit(0);
    }
    else if(globalVars.pid < 0){
      perror("Erro ao criar novo doutor\n");
    }
  }

  pthread_exit(NULL);
}

void trataPaciente(){
  Paciente paciente;
  struct msqid_ds* info_mq = malloc(sizeof(struct msqid_ds));
  time_t start_time = time(NULL), end_time = 0;

  printf("Doutor [%d] começou o seu turno.\n", getpid());
  //Escreve no log
  char* message_buffer = malloc(MAX_LOG_MESSAGE);
  sprintf(message_buffer, "Doutor [%d] começou o seu turno.\n",getpid());
  write_to_log(message_buffer); free(message_buffer);

  while((end_time-start_time) < globalVars.SHIFT_LENGTH){
    //Verifica o estado da message queue
    if(sem_wait(globalVars.semMQ) != 0){
      perror("Erro ao decrementar semMQ em trataPaciente\n");
    }

    //Verifica se ja foi pedido doutor adicional
    if(globalVars.requestDoctor == 0){ //Ainda n foi pedido, pode ser pedido
      msgctl(globalVars.mq_id_doctor, IPC_STAT, info_mq);

      //printf("Numero de mensagens: %ld\n", info_mq->msg_qnum);
      if(info_mq->msg_qnum > globalVars.MQ_MAX){
        //Atingiu mais de 100% de lotaçao, faz signal a thread para fazer mais um processo (por fazer)
        printf("Requesting temporary doctor\n");
        globalVars.requestDoctor = 1;

        //BLoqueia o mutex e faz sinal à thread para criar um doutor temporario
        if(pthread_mutex_lock(&globalVars.mutex_doctor) != 0){
          perror("Erro ao bloquear mutex_doctor em trataPaciente\n");
        }
        if(pthread_cond_signal(&globalVars.cond_var_doctor) != 0){
          perror("Erro ao fazer signal de cond_var_doctor em trataPaciente\n");
        }
        if(pthread_mutex_unlock(&globalVars.mutex_doctor) != 0){
          perror("Erro ao desbloquear mutex_doctor em trataPaciente\n");
        }
      }
    }
    //Já foi pedido, não pode ser pedido outra vez

    if(sem_post(globalVars.semMQ) !=0 ){
      perror("Erro ao incrementar semMQ em trataPaciente\n");
    }

    //Recebe paciente da queue
    if(msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("Erro ao receber da message queue doutor\n");
    }
    else{
      //Pára o contador do tempo entre a triagem e o atendimento
      struct timespec cont_tempo;
      clock_gettime(CLOCK_REALTIME, &cont_tempo);
      int64_t temp = cont_tempo.tv_nsec;
      temp-=paciente.before_atend;

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente\n");
      }
      (*globalVars.total_time_before_atend)+=temp; //Adiciona ao total do tempo
      #ifdef DEBUG
      printf("Total ms depois triagem e antes atend: %ld\n", *globalVars.total_time_before_atend);
      fflush(stdout);
      #endif
      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente\n");
      }

      printf("Doutor [%d] recebeu paciente %s\n", getpid(), paciente.nome);

      //Espera tempo de atendimento
      usleep(paciente.atend_time*1000); //Converte para milisegundos
      end_time = time(NULL);

      //Pára o contador de tempo total do paciente
      struct timespec cont_tempo2;
      clock_gettime(CLOCK_REALTIME, &cont_tempo2);
      temp = cont_tempo2.tv_nsec;
      temp-=paciente.total_time;

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente\n");
      }
      (*globalVars.total_time)+=temp; //Adiciona ao total do tempo
      #ifdef DEBUG
      printf("Total ms: %ld\n", *globalVars.total_time);
      fflush(stdout);
      #endif
      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente\n");
      }

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente (2)\n");
      }

      (*globalVars.n_pacientes_atendidos)++; //incrementa o numero de pacientes atendidos

      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente (2)\n");
      }
      printf("Doutor [%d] atendeu paciente %s\n", getpid(), paciente.nome);
    }

  }
  printf("Doutor [%d] acabou o seu turno\n", getpid());
  //Escreve no log
  message_buffer = malloc(MAX_LOG_MESSAGE);
  sprintf(message_buffer, "Doutor [%d] acabou o seu turno.\n",getpid());
  write_to_log(message_buffer); free(message_buffer);
}

void* createTempDoctor(){
  pid_t temp_doctor;
  //Vai esperar que seja acordada para criar mais um processo temporario
  while(1){
    if(pthread_mutex_lock(&globalVars.mutex_doctor) != 0){
      perror("Erro ao bloquear mutex_doctor em createTempDoctor\n");
    }

    while(!globalVars.requestDoctor){
      pthread_cond_wait(&globalVars.cond_var_doctor, &globalVars.mutex_doctor); //Espera por pthread_cond_signal
    }

    //Doutor temporario pedido
    printf("\n\nTemporary doctor requested\n\n");
    if(pthread_mutex_unlock(&globalVars.mutex_doctor)!=0){
      perror("Erro ao desbloquear mutex_doctor em createTempDoctor\n");
    }

    //Cria um doutor
    if((temp_doctor = fork()) == 0){
      signal(SIGINT, cleanup);

      //Ignora os outros sinais
      for(int i=1; i<31; i++){
        if((i!=2) && (i!=10) && (i!=11)){
          signal(i, ignore_signal);
        }
      }
      trataPaciente_tempDoctor();
      exit(0);
    }
    else if(temp_doctor < 0){
      perror("Erro na criacao do doutor temporario\n");
    }

    //Espera que ele acabe
    waitpid(temp_doctor, NULL, WNOHANG);
    //Acabou, volta a colocar a condiçao a 0
    if(pthread_mutex_lock(&globalVars.mutex_doctor)!=0){
      perror("Erro ao bloquear mutex_doctor em createTempDoctor (2)\n");
    }
    globalVars.requestDoctor = 0;
    if(pthread_mutex_unlock(&globalVars.mutex_doctor)!=0){
      perror("Erro ao desbloquear mutex_doctor em createTempDoctor (2)\n");
    }
  }
  pthread_exit(NULL);
}

//Função para o doutor temporario
void trataPaciente_tempDoctor(){
  Paciente paciente;
  struct msqid_ds* info_mq = malloc(sizeof(struct msqid_ds));
  int check_exit = 0;

  printf("Doutor temporario [%d] começou o seu turno\n", getpid());
  //Escreve no log
  char* message_buffer = malloc(MAX_LOG_MESSAGE);
  sprintf(message_buffer, "Doutor temporario [%d] começou o seu turno.\n",getpid());
  write_to_log(message_buffer); free(message_buffer);

  while(1){
    //Verifica o estado da message queue
    if(sem_wait(globalVars.semMQ)!=0){
      perror("Erro ao decrementar semMQ em trataPaciente_tempDoctor\n");
    }

    msgctl(globalVars.mq_id_doctor, IPC_STAT, info_mq);
    if(info_mq->msg_qnum < ((globalVars.MQ_MAX*80*100)/100.0)){
      //Voltou a menos de 80% de lotação, acaba este processo nesta iteraçao
      check_exit = 1;
    }
    if(sem_post(globalVars.semMQ)!=0){
      perror("Erro ao incrementar semMQ em trataPaciente_tempDoctor\n");
    }

    //Recebe paciente da queue
    if(msgrcv(globalVars.mq_id_doctor, &paciente, sizeof(Paciente)-sizeof(long), MTYPE, 0) < 0){
      perror("Erro ao receber paciente da queue em trataPaciente_tempDoctor\n");
    }
    else{
      printf("Doutor temporario recebeu paciente %s\n", paciente.nome);

      //Pára o contador do tempo entre a triagem e o atendimento
      struct timespec cont_tempo;
      clock_gettime(CLOCK_REALTIME, &cont_tempo);
      int64_t temp = cont_tempo.tv_nsec;
      temp-=paciente.before_atend;

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente\n");
      }
      (*globalVars.total_time_before_atend)+=temp; //Adiciona ao total do tempo
      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente\n");
      }

      //Espera tempo de atendimento
      usleep(paciente.atend_time);
      printf("Doutor temporario atendeu paciente %s\n", paciente.nome);

      //Pára o contador de tempo total do paciente
      struct timespec cont_tempo2;
      clock_gettime(CLOCK_REALTIME, &cont_tempo2);
      temp = cont_tempo2.tv_nsec;
      temp-=paciente.total_time;

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente\n");
      }
      (*globalVars.total_time)+=temp; //Adiciona ao total do tempo
      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente\n");
      }

      if(sem_wait(globalVars.semSHM) != 0){
        perror("Erro ao incrementar semSHM em trataPaciente (2)\n");
      }

      (*globalVars.n_pacientes_atendidos)++; //incrementa o numero de pacientes atendidos

      if(sem_post(globalVars.semSHM) != 0){
        perror("Erro ao decrementar semSHM em trataPaciente (2)\n");
      }
    }

    if(check_exit) break;
  }

  printf("Doutor temporario [%d] acabou o seu turno.\n", getpid());
  //Escreve no log
  message_buffer = malloc(MAX_LOG_MESSAGE);
  sprintf(message_buffer, "Doutor temporario [%d] acabou o seu turno.\n",getpid());
  write_to_log(message_buffer); free(message_buffer);
}
