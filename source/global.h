#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h>

#define MAX_LOG_MESSAGE 1024
#define MEM_SIZE 20
#define MAX_BUFFER 2000
#define PIPE_NAME "named_pipe"
#define MAX_NOME_PACIENTE 256
#define MTYPE 1

//Struct das variavies globais
typedef struct{
  //Constantes iniciais
  int TRIAGE, DOCTORS, SHIFT_LENGTH, MQ_MAX;
  //ID's threads, processos, memorias
  int shmid, mq_id_thread, mq_id_doctor, numDadosPartilhados;
  int* dadosPartilhados;
  char* log_ptr;
  //Descriptores de ficheiro
  int log_fd, named_fd;
  //Semaforos, mutexes
  sem_t semLog;
}Globals;

extern Globals globalVars;

typedef struct dados{
  int triage, doctors, shift_length, mq_max;
}Dados;

typedef struct paciente{
  long mtype;
  time_t arrival_time;
  intmax_t triage_time, atend_time;
  char nome[MAX_NOME_PACIENTE];
  intmax_t prioridade;
}Paciente;

#endif //GLOBAL_H
