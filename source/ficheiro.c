#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ficheiro.h"
#include "structs.h"

#define MAX_BUFFER 2000

Dados readFile(FILE* fileptr){

  Dados dados; //struct a ser devolvido
  char buf[MAX_BUFFER];
  char* token;

  //Ficheiro no formato:
  //TRIAGE=X
  //DOCTORS=X
  //SHIFT_LENGTH=X
  //MQ_MAX=X

  fgets(buf, MAX_BUFFER, fileptr); //recebe a linha
  token = strtok(buf, "="); //separa a linha de acordo com o delim '=', token=triage
  token = strtok(NULL, "="); //avança para a proxima token, o numero, token=5
  dados.triage = atoi(token); //converte de char para inteiro, e guarda no struct

  fgets(buf, MAX_BUFFER, fileptr); //proxima linha
  token = strtok(buf, "=");
  token = strtok(NULL, "=");
  dados.doctors = atoi(token);

  fgets(buf, MAX_BUFFER, fileptr);
  token = strtok(buf, "=");
  token = strtok(NULL, "=");
  dados.shift_length = atoi(token);

  fgets(buf, MAX_BUFFER, fileptr);
  token = strtok(buf, "=");
  token = strtok(NULL, "=");
  dados.mq_max = atoi(token);

  return dados;
}