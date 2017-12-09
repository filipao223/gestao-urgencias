#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/mman.h>

#include "global.h"
#include "sinais.h"

Globals globalVars;

void cleanup(int signum){
  printf("\n\n^C pressionado. A sair...\n\n");

  //Pára os subprocessos
  if(globalVars.pid == 0){
    exit(0);
  }

  close(globalVars.named_fd);

  msgctl(globalVars.mq_id_thread, IPC_RMID, 0);
  msgctl(globalVars.mq_id_doctor, IPC_RMID, 0);

  sem_unlink("SemLog");
  sem_unlink("SemMQ");
  sem_unlink("SemSHM");

  sem_close(globalVars.semLog);
  sem_close(globalVars.semMQ);
  sem_close(globalVars.semSHM);

  shmdt(&globalVars.dadosPartilhados);
  shmctl(globalVars.shmid, IPC_RMID, NULL);

  msync(globalVars.log_ptr, LOG_SIZE, MS_SYNC);
  munmap(globalVars.log_ptr, getpagesize());
  close(globalVars.log_fd);

  exit(0);
}

void show_stats(int signum){
  printf("\n\nSIGUSR1 received! Showing stats\n\n");
  fflush(stdout);

  if(sem_wait(globalVars.semSHM) != 0){
    perror("Erro ao decrementar semSHM em show_stats\n");
  }
  printf("Numero de pacientes atendidos: %ld\n", *(globalVars.n_pacientes_atendidos));
  printf("Numero de pacientes triados: %ld\n", *(globalVars.n_pacientes_triados));
  printf("Media de tempo ate ser triado (em microsegundos): %.2lf\n", *(globalVars.total_before_triage)/(double)*(globalVars.n_pacientes_triados));
  printf("Media de tempo ate ser atendido (em microsegundos): %.2lf\n", *(globalVars.total_before_atend)/(double)*(globalVars.n_pacientes_atendidos));
  if(sem_post(globalVars.semSHM) != 0){
    perror("Erro ao incrementar semSHM em show_stats\n");
  }
}

void ignore_signal(int signum){
  printf("\nRecebeu sinal %d!\n", signum);
}
