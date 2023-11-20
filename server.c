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

/* ---------------------- Configuração do Cliente ---------------------- */
typedef struct client_data {
  int csock;
  //struct sockaddr_in addr;
  int id;

} client_data;

client_data clients_List[MAX_CLIENTS];
//Status pro RTU
char statusMensagem[BUFSZ];

/* ---------------------- Configuração dos sensores do cliente ---------------------- */
#define MIN_SENSORS 3
#define MAX_SENSORS 10
#define DADOS_SENSOR 100 // Tamanho máximo das informações

int tabelaSensor[MAX_SENSORS][DADOS_SENSOR]; //Tabela

//Inicializar dados aleatórios para os sensores
void inicializa_sensores() {

    // Gere aleatoriamente o número de sensores entre MIN_SENSORS e MAX_SENSORS
    int num_sensors = rand() % (MAX_SENSORS - MIN_SENSORS + 1) + MIN_SENSORS;

    // Adicione sensores à tabela
    for (int i = 0; i < num_sensors; ++i) {
        tabelaSensor[i][0] = i + 1;  // IDs começam de 1
        tabelaSensor[i][1] = rand() % 100 + 1;  // Potência aleatória entre 1 e 100
        tabelaSensor[i][2] = rand() % 100 + 1;  // Eficiência aleatória entre 1 e 100
    }
}

/* ---------------------- Funções auxiliares ---------------------- */

int localiza_Cliente(int client_Line) { // Percorrer lista para verificar a existência do cliente
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_List[i].id == client_Line) {
            return 1;
        }
    }
    return 0;
}

//COMANDO PRA COMENTAR: CTRL K CTRL C NESSA ORDEM || OU CTRL ;

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
            
            void inicializa_sensores();
            printf("Dados gerados: %d %d \n", tabelaSensor[clients_List[contador_Cliente].id][1], tabelaSensor[clients_List[contador_Cliente].id][2]);

            //Envia resposta ao cliente
            snprintf(resposta_Server, sizeof(resposta_Server), "RES_ADD( %d )", clients_List[contador_Cliente].id);
            send(csock, resposta_Server, strlen(resposta_Server), 0);

            // Imprime no servidor
            printf("Client %d added\n", clients_List[contador_Cliente].id);
            contador_Cliente++;
            printf("Conectados: %d \n", contador_Cliente); //APAGAR DEPOIS

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            size_t count = recv(csock, buf, BUFSZ - 1, 0);
            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf); //APAGAR DEPOIS
            
            /* ------------------------------- Remove o cliente na lista de clientes ------------------------------ */
            if (0 == strncmp(buf, "REQ_DC", 7)) {
                int id_Teste = 3;

                if(localiza_Cliente(id_Teste) == 1){

                    // Remova o cliente da lista
                    clients_List[id_Teste].id = 0;
                    clients_List[id_Teste].csock = -1;
                    printf("Client %d removed\n", id_Teste);
                    contador_Cliente--;

                    //Removido com sucesso
                    memcpy(resposta_Server, "OK_01", sizeof("OK_01")); //Funciona mas ta bugado <---------------
                    send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente

                    //PROVAVELMENTE FALTA UM CLOSE CSOCK

                } else{

                    memcpy(resposta_Server, "ERROR_04", sizeof("ERROR_04")); //Funciona mas ta bugado <---------------
                    send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente
                    printf("Errou o número ae mano \n"); //APAGAR DEPOIS
                
                }
            }
            else if (0 == strncmp(buf, "REQ_LS", 7)) {
            //     if (num_sensors > 0) {
            // // Encontre o sensor com maior potência útil
            // int sensor_max_pot = 0;
            // for (int i = 1; i < num_sensors; ++i) {
            //     if ((sensor_table[i].potencia * sensor_table[i].eficiencia_energetica) >
            //         (sensor_table[sensor_max_pot].potencia * sensor_table[sensor_max_pot].eficiencia_energetica)) {
            //         sensor_max_pot = i;
            //     }
            // }

            }
        
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