CFLAGS = -c -g -Wall -D_REENTRANT -pthread
CFLAGSF = -g -Wall -D_REENTRANT -pthread
CC = gcc

final: ficheiro.o main.o triagem.o doutor.o log.o
	$(CC) ficheiro.o main.o triagem.o doutor.o log.o -o final $(CFLAGSF)
main.o: main.c ficheiro.h triagem.h global.h doutor.h log.h
	$(CC) main.c $(CFLAGS)
ficheiro.o: ficheiro.c global.h
	$(CC) ficheiro.c $(CFLAGS)
triagem.o: triagem.c global.h
	$(CC) triagem.c $(CFLAGS)
doutor.o:	doutor.c global.h
	$(CC) doutor.c $(CFLAGS)
log.o: log.c global.h
	$(CC) log.c $(CFLAGS)
clean:
	rm *.o final named_pipe
