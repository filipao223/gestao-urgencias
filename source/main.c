#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "ficheiro.h"
#include "triagem.h"
#include "global.h"
#include "doutor.h"
#include "sinais.h"
#include "log.h"

void signal_handler(int signum){
  close(globalVars.named_fd);
  msgctl(globalVars.mq_id_thread, IPC_RMID, 0);
  sem_destroy(&globalVars.semLog);
  shmdt(&(globalVars.dadosPartilhados));
  shmctl(globalVars.shmid, IPC_RMID, NULL);
  munmap(globalVars.log_ptr, getpagesize());
  exit(0);
}

int check_str_triage(char*);

Globals globalVars;
char buf[MAX_BUFFER];
int contPaciente=1;

int main(int argc, char** argv){
  int cont=0;
  Dados dados;
  FILE *fileptr = fopen("registo.txt", "r");

  signal(SIGINT, signal_handler);

  //Lê dados do ficheiro
  dados = readFile(fileptr);
  globalVars.TRIAGE=dados.triage;
  globalVars.DOCTORS=dados.doctors;
  globalVars.SHIFT_LENGTH=dados.shift_length;
  globalVars.MQ_MAX=dados.mq_max;

  pthread_t thread_doctors, thread_triage[globalVars.TRIAGE];
  long ids[globalVars.TRIAGE];

  srand(time(NULL));

  //Cria as zonas de memoria partilhada
  globalVars.shmid = shmget(IPC_PRIVATE, MEM_SIZE, 0666 | IPC_CREAT);
  globalVars.dadosPartilhados = shmat(globalVars.shmid, NULL, 0);

  //Cria mmf
  globalVars.log_fd = open("log.txt", O_RDWR);
  globalVars.log_ptr = mmap((caddr_t)0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, globalVars.log_fd, getpagesize());

  //Cria e abre named pipe
  if(mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0700) < 0){
    perror("Erro ao criar named pipe");
    exit(0);
  }
  if((globalVars.named_fd = open(PIPE_NAME, O_RDONLY)) < 0){
    perror("Erro ao abrir named pipe");
    exit(0);
  }

  //Cria as filas de mensagens
  if((globalVars.mq_id_thread = msgget(IPC_PRIVATE, O_CREAT|0700)) < 0){
    perror("Erro ao criar message queue");
    exit(0);
  }

  if((globalVars.mq_id_doctor = msgget(IPC_PRIVATE, O_CREAT|0700)) < 0){
    perror("Erro ao criar message queue");
    exit(0);
  }

  //Inicializa dados partilhados
  globalVars.numDadosPartilhados = 5;
  for(int i=0; i<globalVars.numDadosPartilhados; i++){
    globalVars.dadosPartilhados[i] = 0;
  }

  //Inicializa semaphore
  sem_init(&globalVars.semLog, 1, 1);

  //Cria as threads de triagem
  for(int i=0; i<globalVars.TRIAGE; i++){
    if(pthread_create(&thread_triage[i], NULL, createTriage, &ids[i]) != 0)
      printf("Erro ao criar thread!\n");
    /*char message[MAX_LOG_MESSAGE];
    sprintf(message, "Thread %d criada\n", i);
    write_to_log(message);*/
  }

  //Recebe os pacientes pelo named pipe
  while(1){
    Paciente paciente;
    paciente.mtype = MTYPE;
    int temp;
    char* tokens, *ptr, *bufTemp;
    int nread = read(globalVars.named_fd, buf, sizeof(buf));
    buf[nread-1] = '\0'; //\n
    printf("Recebeu: %s\n", buf);

    //Verifica se o formato é num num num num
    if(buf[0] >= '0' && buf[0] <= '9'){
      //Varios pacientes, faz um for com o primeiro token
      bufTemp = strdup(buf);
      tokens = strtok(bufTemp, " ");
      sscanf(tokens, "%d", &temp);
      for(int i=0; i<temp; i++){
        bufTemp = strdup(buf);
        tokens = strtok(bufTemp, " ");
        //Trata os dados do paciente
        sprintf(paciente.nome, "%d", contPaciente);
        paciente.arrival_time = time(NULL);
        tokens = strtok(NULL, " ");
        paciente.triage_time = strtoimax(tokens, &ptr, 10);
        tokens = strtok(NULL, " ");
        paciente.atend_time = strtoimax(tokens, &ptr, 10);
        tokens = strtok(NULL, " ");
        paciente.prioridade = strtoimax(tokens, &ptr, 10);
        contPaciente++;
        //Envia para a message queue
        msgsnd(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), 0);
      }
    }

    //Não é do formato num num num num
    else if((buf[0] >= 'A' && buf[0] <= 'z') && (check_str_triage(buf) != 1)){
      //È do formato nome num num num
      tokens = strtok(buf, " ");
      strcpy(paciente.nome, tokens);
      paciente.arrival_time = time(NULL);
      tokens = strtok(NULL, " ");
      paciente.triage_time = strtoimax(tokens, &ptr, 10);
      tokens = strtok(NULL, " ");
      paciente.atend_time = strtoimax(tokens, &ptr, 10);
      tokens = strtok(NULL, " ");
      paciente.prioridade = strtoimax(tokens, &ptr, 10);
      contPaciente++;
      //Envia para a message queue
      msgsnd(globalVars.mq_id_thread, &paciente, sizeof(Paciente)-sizeof(long), 0);
    }

    //Não é do formato nome num num num
    else if(check_str_triage(buf)){
      //É do formato TRIAGE=??
      tokens = strtok(buf, "="); strtok(NULL, "=");
      int newTriage = strtoimax(tokens, &ptr, 10);
      printf("New triage = %d\n", newTriage);
    }

    //É um formato desconhecido
    else{
      printf("Formato desconhecido!\n");
    }

    globalVars.named_fd = open(PIPE_NAME, O_RDONLY);
  }

  sleep(3);
  //Trata os dados relativos a tempos de Espera

  //Cria os processos doctor iniciais
  for(int i=0; i<globalVars.DOCTORS; i++){
    if(fork() == 0){
      trataPaciente();
      exit(0);
    }
  }

  //Calcula media de tempo triado

  //Limpa recursos
  sem_destroy(&globalVars.semLog);
  shmdt(&(globalVars.dadosPartilhados));
  shmctl(globalVars.shmid, IPC_RMID, NULL);
  munmap(globalVars.log_ptr, getpagesize());

  return 1;
}

int check_str_triage(char* str){
  return
    str[0] == 'T' &&
    str[1] == 'R' &&
    str[2] == 'I' &&
    str[3] == 'A' &&
    str[4] == 'G' &&
    str[5] == 'E';
}
