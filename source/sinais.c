#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "global.h"
#include "sinais.h"

Globals globalVars;

void foo(int signo){

  printf("Valor de SHIFT_LENGTH em sinais.c: %d\n", globalVars.SHIFT_LENGTH);
}
