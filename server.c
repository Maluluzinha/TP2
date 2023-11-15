#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFSZ 1024
#define MAX_CLIENTS 10
#define MIN_CLIENTS 2

typedef struct client_data {
  int csock;
  struct sockaddr_in addr;
  int id;
} client_data;

//COMANDO PRA COMENTAR: CTRL K CTRL C NESSA ORDEM

int main(int argc, char **argv) {

    /* ---------------------- 1 - Servidor Mi, socket que realiza o p2p ---------------------- */
    //CODE INIT HERE
    // Setup P2P socket
    if (argc < 3) { //Se = 3, funciona ./server v4 90900, se = 4, funciona ./server v4 90900 90100
        usage(argc, argv, 1);
    }

    printf("Parâmetros: ID: %s, Port 1: %s, Port 2: %s \n", argv[1],argv[2],argv[3]);
    
    struct sockaddr_storage p2p_storage; //storage para o p2p também
     if (0 != server_sockaddr_init(argv[1], argv[2], argv[3], &p2p_storage)) {
        usage(argc, argv, 0);
    }

    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0); //Socket servidor 1 - Mi
    if (p2p_socket == -1) {
        logexit("P2P socket");
    }

    int enable = 1;
    if (0 != setsockopt(p2p_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr_in *p2p_addr = (struct sockaddr_in *)(&p2p_storage);; //Essa parte aqui que ta dando problema <<<--------- tava
    if (0 != bind(p2p_socket, (struct sockaddr *)p2p_addr, sizeof(* p2p_addr))) {
        logexit("bind");
    }
    
    // Colocar o socket em modo de escuta para o Servidor 1 - como ele é o ativo, acho que não precisa agora
     if (0 != listen(p2p_socket, 10)) {
        logexit("listen");
    }
    //listen(p2p_socket, 5);

    printf("Servidor 1 aguardando conexão na porta 1\n");

    //Até aqui "foi" M - Servidor 1 aguardando conexão na porta primaria - connect to peer: Transport endpoint is already connected
    //CODE END HERE

    /* ---------------------- 2 - Servidor Mj, servidor passivo, que é aberto depois ---------------------- */
    //CODE INIT HERE
    // Setup P2P socket
    struct sockaddr_storage storage;

    if (0 != server_sockaddr_init(argv[1], argv[2], argv[3], &storage)) {
        usage(argc, argv, 1);
    }

    int s; //socket pro cliente
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    //int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
      logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    /* --------------------------- 3 - Configuração do select -------------------------- */
    //Comandos select:
    fd_set read_fds;
    int max_fd;

    FD_ZERO(&read_fds); //Limpa fd_set
    FD_SET(p2p_socket, &read_fds); //Adiciona socket ao fd_set, o declarado bem acima
    FD_SET(s, &read_fds);
    //int activity = select( max_Clients , &read_fds , NULL , NULL , NULL);

    //max_fd = 10;
    max_fd = (p2p_socket > s) ? p2p_socket : s;
    printf("Server waiting for connections...\n");


    /* --------------------------- 4 - Configuração de buffer para receber a mensagem -------------------------- */
    //Mj
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    //Mi
    char addrstr_p2p[BUFSZ];
    addrtostr( (struct sockaddr *)p2p_addr, addrstr_p2p, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr_p2p);
    
	if (0 != connect(s, addr, sizeof(storage))) {
	 	logexit("connect");
	}
    
    //Configuração para receber um comando:
    char dadosDigitados[BUFSZ]; //Entrada
	memset(dadosDigitados, 0, BUFSZ); //Aloca memória
    printf("Digite uma string: ");
    fgets(dadosDigitados, sizeof(dadosDigitados), stdin);
    dadosDigitados[strcspn(dadosDigitados, "\n")] = '\0'; // Remover o caractere de nova linha

	//char addrstr[BUFSZ];
	//addrtostr(addr, addrstr, BUFSZ);

	//printf("connected to %s\n", addrstr);

    while (1) {

        //Config fd_set//
        fd_set temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) {
            logexit("select");
        }

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        /*--------------------------------- PRIMEIRO COMANDO PARA CONECTAR --------------------------------------------*/
         if (0 == strncmp(dadosDigitados, "REQ_ADDPEER", sizeof(dadosDigitados))) {
            if("peer compare"){
                printf("Peer found, verifing limit..");
            }
            else{
                printf("No peer found, starting to listen..");
            }
         }

        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = send(csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
        close(csock);
    }

    exit(EXIT_SUCCESS);
}