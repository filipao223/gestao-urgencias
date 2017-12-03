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

//Struct das variavies globais
typedef struct{

  int TRIAGE, DOCTORS, SHIFT_LENGTH, MQ_MAX;
  int shmid, numDadosPartilhados;
  int* dadosPartilhados;
  //int* pacienteTriados, pacienteAtend, avgTempoAntesT, avgTempoDepoisT, avgTempoTotal; //Dados partilhados
  sem_t semPacAten, semPacTri, semPacTempo;
}Globals;

extern Globals globalVars;

#endif //GLOBAL_H
