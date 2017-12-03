#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "triagem.h"
#include "global.h"

//Operaçoes triagem

Globals globalVars;
int numPacientes=3;

void* createTriage(void* t){
int cont=0;

  srand(time(NULL) + pthread_self());

  while(cont<numPacientes){

    int sleepTime = rand()%5;
    sleep(sleepTime);

    sem_wait(&globalVars.semPacTri);
    printf("Paciente %d triado\n", globalVars.dadosPartilhados[0]+1);
    globalVars.dadosPartilhados[0]++;
    sem_post(&globalVars.semPacTri);

    sem_wait(&globalVars.semPacTempo);
    globalVars.dadosPartilhados[2]+=sleepTime;
    sem_post(&globalVars.semPacTempo);

    cont++;
  }
  pthread_exit(NULL);
}
