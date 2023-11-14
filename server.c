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
#define min_Clients 2

//COMANDO PRA COMENTAR: CTRL K CTRL C NESSA ORDEM
/////////////NÃO MEXER AINDA//////////////

// void usage(int argc, char **argv) {
//     printf("usage: %s <v4> <server port>\n", argv[0]);
//     printf("example: %s v4 51511\n", argv[0]);
//     exit(EXIT_FAILURE);
// }

int main(int argc, char **argv) {
    if (argc < 3) { //Se = 3, funciona ./server v4 90900, se = 4, funciona ./server v4 90900 90100
        usage(argc, argv, 1);
    }

    //CODE INIT HERE
    // Setup P2P socket
    //CUIDADO COM AS VARIAVEIS NO SOCKADDR_IN
    struct sockaddr_storage p2p_storage; //storage para o p2p também
     if (0 != server_sockaddr_init(argv[1], argv[2], argv[3], &p2p_storage)) {
        usage(argc, argv, 1);
    }

    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0); //Socket servidor 1
    if (p2p_socket == -1) {
        logexit("P2P socket");
    }

    int enable = 1;
    if (0 != setsockopt(p2p_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    //struct sockaddr_storage p2p_storage; //storage para o p2p também

    struct sockaddr_in *p2p_addr = (struct sockaddr_in *)(&p2p_storage);; //Essa parte aqui que ta dando problema <<<---------
    // memset(p2p_addr, 0, sizeof(*p2p_addr));
    // p2p_addr->sin_family = AF_INET;
    // p2p_addr->sin_port = htons(atoi(argv[2]));
    // p2p_addr->sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(p2p_socket, (struct sockaddr *)p2p_addr, sizeof(* p2p_addr))) {
        logexit("bind");
    }
    
    // Colocar o socket em modo de escuta para o Servidor 1
     if (0 != listen(p2p_socket, 10)) {
        logexit("listen");
    }
    //listen(p2p_socket, 5);

    printf("Servidor 1 aguardando conexão na porta primaria\n");

    //Até aqui "foi" M - Servidor 1 aguardando conexão na porta primaria - connect to peer: Transport endpoint is already connected

    
    //struct sockaddr_in server_addr;
    // memset(&p2p_addr, 0, sizeof(p2p_addr));
    // p2p_addr.sin_family = AF_INET;
    // p2p_addr.sin_addr.s_addr = inet_addr(argv[2]);
    // p2p_addr.sin_port = htons(atoi(argv[3]));

    //Necessário modificar o init
    //if (connect(p2p_socket, (struct sockaddr*)&p2p_addr, sizeof(p2p_addr)) == -1) {
    //    logexit("connect to peer"); //->>>>>dando problema aqui -> connect to peer: Transport endpoint is already connected
    //}
    //CODE END HERE

    struct sockaddr_storage storage;

    if (0 != server_sockaddr_init(argv[1], argv[2], argv[3], &storage)) {
        usage(argc, argv, 1);
    }

    int s; //passive socket provavelmente
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

    /////////////NÃO MEXER AINDA//////////////

    //CODE RESUME HERE
 // Criar o socket para o Servidor 2
    // int server_socket2 = socket(AF_INET, SOCK_STREAM, 0);
    // struct sockaddr_in p2p_addr_server2;

    // // Configurar a estrutura sockaddr_in para o Servidor 2
    // p2p_addr_server2.sin_family = AF_INET;
    // p2p_addr_server2.sin_port = htons(90100);
    // p2p_addr_server2.sin_addr.s_addr = INADDR_ANY;

    // // Associar o socket ao endereço e à porta para o Servidor 2
    // bind(server_socket2, (struct sockaddr*)&p2p_addr_server2, sizeof(p2p_addr_server2));

    // // Colocar o socket em modo de escuta para o Servidor 2
    // listen(server_socket2, 5);

    // printf("Servidor 2 aguardando conexão na porta secundária\n");

    // // Aceitar a conexão para o Servidor 1
    // addr_size = sizeof(client_addr);
    // client_socket1 = accept(server_socket1, (struct sockaddr*)&client_addr, &addr_size);

    // // Lógica para o Servidor 1
    // handle_client(client_socket1);

    // // Aceitar a conexão para o Servidor 2
    // addr_size = sizeof(client_addr);
    // client_socket2 = accept(server_socket2, (struct sockaddr*)&client_addr, &addr_size);

    // // Lógica para o Servidor 2
    // handle_client(client_socket2);

    //CODE END HERE

    //USANDO SELECT
    // // Encaminhar a mensagem para o peer
    // int p2p_sock;
    // p2p_sock = socket(p2p_storage.ss_family, SOCK_STREAM, 0); //Socket pro servidor
    // if (p2p_sock == -1) {
    // //     logexit("socket"); //TA DANDO ERRO AQUI O -> socket: Address family not supported by protocol
    //  }
    // //struct sockaddr *p2p_addr = (struct sockaddr *)(&p2p_storage);
    
    // //Socket para comunicar com o cliente
    // int cliente_socket = socket(AF_INET, SOCK_STREAM, 0);
    // if (cliente_socket == -1) {
    //     logexit("cliente socket not ok");
    // }

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

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {

        //Config fd_set//
        fd_set temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) {
            logexit("select");
        }
        //////////////

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

        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = send(csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
        close(csock);
    }

    exit(EXIT_SUCCESS);
}