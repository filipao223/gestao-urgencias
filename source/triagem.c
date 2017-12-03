#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "triagem.h"
#include "global.h"

//Operaçoes triagem

Globals globalVars;

void* createTriage(void* t){
  sleep(2);
}
