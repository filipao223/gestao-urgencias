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
#define LOG_SIZE 1048576
#define MEM_SIZE 40
#define MAX_BUFFER 2000
#define PIPE_NAME "input_pipe"
#define MAX_NOME_PACIENTE 256
#define MTYPE 1
//#define DEBUG

//Struct das variavies globais
typedef struct{
  //Constantes iniciais
  unsigned long MQ_MAX;
  int TRIAGE, DOCTORS, SHIFT_LENGTH;
  //ID's threads, processos, memorias
  int shmid, mq_id_thread, mq_id_doctor, numDadosPartilhados, ptr_pos;
  int64_t* dadosPartilhados;
  int64_t *n_pacientes_triados, *n_pacientes_atendidos, *total_before_triage, *total_before_atend, *total_time;
  char* log_ptr;
  //Descriptores de ficheiro
  int log_fd, named_fd;
  //Semaforos, mutexes, variaveis condiçao
  sem_t* semLog, *semMQ, *semSHM;
  pthread_cond_t cond_var_doctor;
  pthread_mutex_t mutex_doctor;
  //Outros
  int checkRequestedDoctor;
  pid_t pid;
  pthread_t* thread_triage, *new_thread_triage;
  pthread_t thread_doctors, temp_doctor_thread;
  int newTriage;
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
  //Tempos totais de atendimento e triagem
  suseconds_t before_triage, before_atend, total_time;
}Paciente;

#endif //GLOBAL_H
