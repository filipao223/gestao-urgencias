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
  close(globalVars.named_fd);

  msgctl(globalVars.mq_id_thread, IPC_RMID, 0);
  msgctl(globalVars.mq_id_doctor, IPC_RMID, 0);

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
  printf("SIGUSR1 received! Showing stats\n");
  fflush(stdout);

  if(sem_wait(globalVars.semSHM) != 0){
    perror("");
  }

  printf("Numero de pacientes atendidos: %d\n", *(globalVars.n_pacientes_atendidos));
  printf("Numero de pacientes triados: %d\n", *(globalVars.n_pacientes_triados));

  if(sem_post(globalVars.semSHM) != 0){
    perror("");
  }
}
