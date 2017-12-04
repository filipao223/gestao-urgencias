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
  sem_destroy(&globalVars.semLog);
  sem_destroy(&globalVars.semMQ);
  shmdt(&globalVars.dadosPartilhados);
  shmctl(globalVars.shmid, IPC_RMID, NULL);
  munmap(globalVars.log_ptr, getpagesize());
  exit(0);
}
