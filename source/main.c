#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <signal.h>

#include "structs.h"
#include "ficheiro.h"
#include "triagem.h"
#include "global.h"
#include "doutor.h"
#include "sinais.h"

#define MEM_SIZE 2048
#define BUF_SIZE 2000

//struct sigaction sa;

//void signal_handler(int signal);
int k=0, maxNewDoctors = 7;

Globals globalVars;

int main(int argc, char** argv){
  int cont=0;
  Dados dados;
  FILE *fileptr = fopen("registo.txt", "r");

  //Signal handler
  //sa.sa_handler = &signal_handler;
  //sa.sa_flags = SA_RESTART;

  //sigfillset(&sa.sa_mask); //Bloqueia outros sinais enquanto estiver a tratar um

  //Lê dados do ficheiro
  dados = readFile(fileptr);
  globalVars.TRIAGE=dados.triage;
  globalVars.DOCTORS=dados.doctors;
  globalVars.SHIFT_LENGTH=dados.shift_length;
  globalVars.MQ_MAX=dados.mq_max;

  pthread_t threads[globalVars.TRIAGE];
  long ids[globalVars.TRIAGE];

  srand(time(NULL));

  //Cria as zonas de memoria partilhada
  globalVars.shmid = shmget(IPC_PRIVATE, MEM_SIZE, 0666 | IPC_CREAT);
  globalVars.dadosPartilhados = shmat(globalVars.shmid, NULL, 0);

  //Inicializa dados partilhados
  globalVars.numDadosPartilhados = 5;
  for(int i=0; i<globalVars.numDadosPartilhados; i++){
    globalVars.dadosPartilhados[i] = 0;
  }

  //Inicializa semaphore
  sem_init(&globalVars.semPacAten, 1, 1);
  sem_init(&globalVars.semPacTri, 1, 1);
  sem_init(&globalVars.semPacTempo, 1, 1);

  //Cria as threads de triagem
  for(int i=0; i<globalVars.TRIAGE; i++){
    if(pthread_create(&threads[i], NULL, createTriage, &ids[i]) != 0)
      printf("Erro ao criar thread!\n");
  }

  sleep(5);
  //Trata os dados relativos a tempos de Espera

  //Cria os processos doctor iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }
  }

  while(1){

    //Verifica SIGINT
    //if(sigaction(SIGINT, &sa, NULL) == -1) printf("Erro ao receber sinal.\n");

    //Quando um processo acabar cria outro
    wait(NULL);
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }

    cont++;

    //Para acabar o programa antes de implementar signal handler
    if(cont==maxNewDoctors) break;
  }

  //Espera que todas as threads acabem
  for(int i=0; i<globalVars.TRIAGE; i++){
    pthread_join(threads[i], NULL);
  }

  //Espera que todos os processos acabem
  for(int i=0; i<globalVars.DOCTORS+5; i++){
    wait(NULL);
  }

  //Calcula media de tempo triado
  float media = globalVars.dadosPartilhados[2]/(float)globalVars.dadosPartilhados[0];

  printf("Numero total de pacientes triados: %d\n", globalVars.dadosPartilhados[0]);
  printf("Numero total de pacientes atendidos: %d\n", globalVars.dadosPartilhados[1]);
  printf("Media de tempo a ser triado: %.1fs\n", media);

  //Limpa recursos
  sem_destroy(&globalVars.semPacTri);
  sem_destroy(&globalVars.semPacAten);
  sem_destroy(&globalVars.semPacTempo);
  shmdt(&(globalVars.dadosPartilhados));
  shmctl(globalVars.shmid, IPC_RMID, NULL);

  return 1;
}

/*void signal_handler(int signal){

  fileno(stdout);
  sem_t* mutex;
  int* dadosPartilhados;
  int DOCTORS, shmid;

  if(signal != SIGINT){
    char *buf = "Sinal errado!";
    write(STDOUT_FILENO, buf, BUF_SIZE);
  }
  else{
    char *buf = "SIGINT recebido!";
    write(STDOUT_FILENO, buf, BUF_SIZE);

    //Espera que todos os processos acabem
    for(int i=0; i<DOCTORS+10; i++){
      wait(NULL);
    }

    //Apresenta no ecra informaçao partilhada
    //sprintf(buf, "Numero total de pacientes atendidos: %d\n", *dadosPartilhados+1);
    //write(STDOUT_FILENO, buf, BUF_SIZE);

    //Limpa recursos
    sem_destroy(mutex);
    shmdt(&dadosPartilhados);
    shmctl(shmid, IPC_RMID, NULL);

    exit(0);
  }
}*/
