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
#define max_Clients 10

//COMANDO PRA COMENTAR: CTRL K CTRL C NESSA ORDEM
/////////////NÃO MEXER AINDA//////////////

// void usage(int argc, char **argv) {
//     printf("usage: %s <v4|v6> <server port>\n", argv[0]);
//     printf("example: %s v4 51511\n", argv[0]);
//     exit(EXIT_FAILURE);
// }

//Usage para server IP Port
// void usage(int argc, char **argv) {
//     printf("usage: %s <server port> <peer IP> <peer port>\n", argv[0]);
//     printf("example: %s 51511 127.0.0.1 51512\n", argv[0]);
//     exit(EXIT_FAILURE);
// }


int main(int argc, char **argv) {
    //if (argc < 3) {
    if (argc < 4) {
        usage(argc, argv, 1);
    }

    struct sockaddr_storage storage;
    struct sockaddr_storage p2p_Storage; //storage para o p2p também

    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv, 1);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
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

    /////////////NÃO MEXER AINDA//////////////


    //USANDO SELECT
    // Encaminhar a mensagem para o peer
    int p2p_sock = socket(p2p_Storage.ss_family, SOCK_STREAM, 0); //Socket pro servidor
    if (p2p_sock == -1) {
        logexit("socket");
    }
    struct sockaddr *p2p_addr = (struct sockaddr *)(&p2p_Storage);
    
    //Socket para comunicar com o cliente
    int cliente_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cliente_socket == -1) {
        logexit("cliente socket not ok");
    }

    //Comandos select:
    fd_set read_fds;
    int max_fd;

    FD_ZERO(&read_fds); //Limpa fd_set
    FD_SET(p2p_sock, &read_fds); //Adiciona socket ao fd_set
    //FD_SET(passive_socket, &read_fds);
    //int activity = select( max_Clients , &read_fds , NULL , NULL , NULL);

    max_fd = 10;
    printf("Server waiting for connections...\n");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        //Monitoramento do socket
        fd_set temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) { //Essa função recebe uma lista de sockets e os monitora
            logexit("select");
        }

for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &temp_fds)) {
                if (fd == cliente_socket) {
                    // New client connection
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_socket = accept(cliente_socket, (struct sockaddr*)&client_addr, &client_len);
                    if (client_socket == -1) {
                        logexit("accept");
                    }

                    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    FD_SET(client_socket, &read_fds);
                    if (client_socket > max_fd) {
                        max_fd = client_socket;
                    }
                } else if (fd == p2p_sock) {
                    // Handle P2P connection
                    char buf[BUFSZ];
                    memset(buf, 0, BUFSZ);

                    ssize_t count = recv(fd, buf, BUFSZ - 1, 0);
                    if (count <= 0) {
                        // P2P connection closed
                        printf("P2P connection closed.\n");
                        close(p2p_sock);
                        exit(EXIT_SUCCESS);
                    } else {
                        // Process received data from P2P connection
                        printf("Received from P2P: %s\n", buf);
                        // Add your processing logic here
                    }
                }
            }
        }
        ////

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

        //////////////////////////////////////
        //Essa parte recebe a mensagem do cliente:
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = send(csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
        //CODE INIT HERE:
        //Receiving message from SERVER
        int server_sock = accept(p2p_sock, caddr, &caddrlen); //Mudar o nome do socket do qual ele recebe
        if (server_sock == -1) {
            logexit("accept");
        }
        char buf_server[BUFSZ];
        memset(buf_server, 0, BUFSZ);
        size_t count_server = recv(server_sock, buf_server, BUFSZ - 1, 0);  //Mensagem que chega do servidor
        printf("%s\n", buf_server); //Print a mensagem do server
        
        //P2P socket:
        if (0 != connect(p2p_sock, p2p_addr, sizeof(p2p_Storage))) {
            logexit("connect to peer");
        }

        count = send(p2p_sock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send to peer");
        }

        close(p2p_sock);

        close(csock);
    }

    exit(EXIT_SUCCESS);
}