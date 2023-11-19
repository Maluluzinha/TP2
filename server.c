#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
//#include <sys/syslog.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

//Comando pra branch -> git checkout --track origin/teste_p2p

#define BUFSZ 1024
#define MAX_CLIENTS 10
#define MIN_CLIENTS 2

typedef struct client_data {
  int csock;
  //struct sockaddr_in addr;
  int id;
} client_data;

// typedef struct {
//     int id;
//     int csock;
// } ClientInfo;

// void handle_new_connection(int new_socket, ClientInfo clients[], int *client_count) {
//     if (*client_count >= MAX_CLIENTS) {
//         // Limite de conexões atingido
//         send(new_socket, "ERROR(01)", 10, 0);
//         printf("Error: Maximum connections reached.\n");
//         close(new_socket);
//     } else {
//         // Aceita a conexão
//         clients[*client_count].csock = new_socket;
//         clients[*client_count].id = *client_count + 1; // Simplesmente usa o índice como ID neste exemplo
//         (*client_count)++;

//         // Envia mensagem de sucesso para o cliente
//         char response[20];
//         snprintf(response, sizeof(response), "RES_ADD(%d)", clients[*client_count - 1].id);
//         send(new_socket, response, strlen(response), 0);

//         // Imprime no servidor
//         printf("Client Id%d added\n", clients[*client_count - 1].id);
//     }
// }

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
     if (0 != server_sockaddr_init(argv[1], argv[2], &p2p_storage)) {
        usage(argc, argv, 0);
    }

    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0); //Socket servidor 1 - Mi
    if (p2p_socket == -1) {
        logexit("P2P socket");
    }

    int enable = 1;
    if (0 != setsockopt(p2p_socket, SOL_SOCKET, SO_REUSEADDR , &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr_in *p2p_addr = (struct sockaddr_in *)(&p2p_storage);; //Essa parte aqui que ta dando problema <<<--------- tava
    if (0 != bind(p2p_socket, (struct sockaddr *)p2p_addr, sizeof(p2p_storage))) {
        logexit("bind");
    }
    
    // Colocar o socket em modo de escuta para o Servidor 1 - como ele é o ativo, acho que não precisa agora
     if (0 != listen(p2p_socket, 10)) {
        logexit("listen");
    }
    //listen(p2p_socket, 5);

    printf("Servidor P2P aguardando conexão na porta 1\n");

    //Até aqui "foi" M - Servidor 1 aguardando conexão na porta primaria - connect to peer: Transport endpoint is already connected
    //CODE END HERE

    /* ---------------------- 2 - Servidor Mj, servidor passivo - do cliente, que é aberto depois ---------------------- */
    //CODE INIT HERE
    // Setup P2P socket
    struct sockaddr_storage storage;

    if (0 != server_sockaddr_init(argv[1], argv[3], &storage)) {
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

    /* ---------------------------  Criando a lista de Clientes -------------------------- */
    //client_data *clients = (client_data *)calloc(MAX_CLIENTS, sizeof(client_data));
    client_data clients_List[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
    clients_List[i].id = 0;
    }
    int contador_Cliente = 0;
    int id_Inicial = 0;
    char resposta_Server[BUFSZ];

    /* --------------------------- 4 - Configuração de buffer para receber a mensagem -------------------------- */
    //Mj
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    //Mi
    char addrstr_p2p[BUFSZ];
    addrtostr( (struct sockaddr *)p2p_addr, addrstr_p2p, BUFSZ);
//    printf("bound to %s, waiting connections\n", addrstr_p2p);

    while (1) {

        /* --------------------------- Teste do Select() -------------------------- */

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            logexit("select");
        }

        if (FD_ISSET(p2p_socket, &read_fds)) {
            struct sockaddr_storage cstorage;
            struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
            socklen_t caddrlen = sizeof(cstorage);

            int csock = accept(p2p_socket, caddr, &caddrlen);
            if (csock == -1) {
                logexit("accept");
            }

            char caddrstr[BUFSZ];
            addrtostr(caddr, caddrstr, BUFSZ);
            printf("[log] P2P connection from %s\n", caddrstr);

            close(csock);
        }

        /* ------------------------------- Select() para o cliente ------------------------------ */
        if (FD_ISSET(s, &read_fds)) {
            struct sockaddr_storage cstorage;
            struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
            socklen_t caddrlen = sizeof(cstorage);

            int csock = accept(s, caddr, &caddrlen);
            if (csock == -1) {
                logexit("accept");
            }

        /* ------------------------------- Adiciona o cliente na lista de clientes ------------------------------ */
            //Aqui verifica se a lista está cheia de clientes
            if(contador_Cliente > MAX_CLIENTS){
                memcpy(resposta_Server, "ERROR_01", sizeof("ERROR_01"));
                send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente
        
            }
            else{

            //Se não estiver cheio, adiciona o cliente na lista
            char caddrstr[BUFSZ];
            addrtostr(caddr, caddrstr, BUFSZ);
            printf("[log] connection from server %s\n", caddrstr);

            // Adiciona o novo cliente à lista
            clients_List[contador_Cliente].csock = csock;       //Adiciona socket ao vetor
            clients_List[contador_Cliente].id = id_Inicial + 1; //Atribui ID
            id_Inicial++;

            //Envia resposta ao cliente
            snprintf(resposta_Server, sizeof(resposta_Server), "RES_ADD( %d )", clients_List[contador_Cliente].id);
            //snprintf(resposta_Server, sizeof(resposta_Server), "RES_ADD");
            send(csock, resposta_Server, strlen(resposta_Server), 0);

            // Imprime no servidor
            printf("Client %d added\n", clients_List[contador_Cliente].id);

            contador_Cliente++;
            printf("Conectados: %d", contador_Cliente); //APAGAR DEPOIS

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            size_t count = recv(csock, buf, BUFSZ - 1, 0);
            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf); //APAGAR DEPOIS

            }
            //close(csock);

        }
    }

    // Check for activity on client sockets
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients_List[i].csock > 0 && FD_ISSET(clients_List[i].csock, &read_fds)) {
                char buf[BUFSZ];
                memset(buf, 0, BUFSZ);
                size_t count = recv(clients_List[i].csock, buf, BUFSZ - 1, 0);
                printf("[msg] Client %d, %d bytes: %s\n", clients_List[i].id, (int)count, buf);

                // Handle client message or close the connection as needed

                // For now, just close the connection
                close(clients_List[i].csock);
                clients_List[i].csock = -1;
            }
        }
    

    // Free allocated memory
    free(clients_List);

        // sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        // count = send(csock, buf, strlen(buf) + 1, 0);
        // if (count != strlen(buf) + 1) {
        //     logexit("send");
        // }
        // close(csock);

    exit(EXIT_SUCCESS);
}