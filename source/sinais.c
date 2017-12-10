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

  //Pára as threads
  for(int i=0; i<globalVars.TRIAGE; i++){
    pthread_cancel(globalVars.thread_triage[i]);
    pthread_join(globalVars.thread_triage[i], NULL);
  }
  if(globalVars.newTriage != -1){ //No caso ter criado triagens adicionais
    for(int i=0; i< (sizeof(globalVars.new_thread_triage) / sizeof(pthread_t)); i++){
      pthread_cancel(globalVars.new_thread_triage[i]);
      pthread_join(globalVars.new_thread_triage[i], NULL);
    }
  }
  pthread_cancel(globalVars.thread_doctors);
  pthread_join(globalVars.thread_doctors, NULL);
  pthread_cancel(globalVars.temp_doctor_thread);
  pthread_join(globalVars.temp_doctor_thread, NULL);

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

  pthread_mutex_lock(&globalVars.mutex_doctor);
  printf("Numero de pacientes atendidos: %ld\n", *(globalVars.n_pacientes_atendidos));
  printf("Numero de pacientes triados: %ld\n", *(globalVars.n_pacientes_triados));
  printf("Media de tempo ate ser triado (em milisegundos): %.2lf\n", (*(globalVars.total_before_triage)/(double)*(globalVars.n_pacientes_triados))/1000.0);
  printf("Media de tempo ate ser atendido (em milisegundos): %.2lf\n", (*(globalVars.total_before_atend)/(double)*(globalVars.n_pacientes_atendidos))/1000.0);
  printf("Media de tempo total gasto no sistema (em milisegundos): %.2lf\n", (*(globalVars.total_time)/(double)*(globalVars.n_pacientes_atendidos))/1000.0);
  pthread_mutex_unlock(&globalVars.mutex_doctor);
}

void ignore_signal(int signum){
  if(signum != SIGCHLD) printf("Recebeu sinal %d\n", signum);
}
