#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "doutor.h"
#include "global.h"

Globals globalVars;

void trataPaciente(){

  int timer = 0;

  while(timer<globalVars.SHIFT_LENGTH){

    printf("Novo doutor\n");
    srand(time(NULL)+getpid());

    int sleepTime = rand()%6 + 1;

    printf("Processo %d a dormir por %d segundos\n", getpid(), sleepTime);
    sleep(sleepTime);

    sem_wait(&globalVars.semPacAten);
    printf("Escreve na memoria(proc: %d)\n", getpid());
    globalVars.dadosPartilhados[1]++;

    fflush(stdout);
    sem_post(&globalVars.semPacAten);

    timer+=sleepTime;
  }
}
