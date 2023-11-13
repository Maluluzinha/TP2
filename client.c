#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_SENSORS 10
#define MIN_SENSORS 3
char mensage[1024];

//Para teste: ./server 127.0.0.1 90900 90100 ./server 127.0.0.1 90900 90200 ./client 127.0.0.1 90100 ./client 127.0.0.1 90200


 void usage(int argc, char **argv) {
 	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
 	exit(EXIT_FAILURE);
 }

#define BUFSZ 1024

void quebraString(const char *entrada, char *info[], int maxPedacos) {
    char copiaEntrada[strlen(entrada) + 1];
    strcpy(copiaEntrada, entrada);
	char espacoChar[] = " ";

    int contador = 0;
    char *token = strtok(copiaEntrada, espacoChar);

    while (token != NULL && contador < maxPedacos) {
        info[contador] = strdup(token); // Aloca memória para cada pedaço
        token = strtok(NULL, espacoChar);
        contador++;
    }
}

int quantosDados(const char *dados[]) {
    int tamanho = 0;
    while (dados[tamanho] != NULL) {
        tamanho++;
    }
    return tamanho;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	printf("mensagem> ");
	fgets(buf, BUFSZ-1, stdin);
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	memset(buf, 0, BUFSZ);
	unsigned total = 0;
	while(1) {
		count = recv(s, buf + total, BUFSZ - total, 0);
		if (count == 0) {
			// Connection terminated.
			break;
		}
		total += count;
	}
	close(s);

	printf("received %u bytes\n", total);
	puts(buf);

	exit(EXIT_SUCCESS);
}