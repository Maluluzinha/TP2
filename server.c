#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <asm-generic/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

//Comando pra branch -> git checkout --track origin/teste_p2p

#define BUFSZ 1024
#define MAX_CLIENTS 10
#define MIN_CLIENTS 2
#define MAX_PEER_CONNECTIONS 2

/* ---------------------- Configuração do P2P ---------------------- */
int peer_connections_count = 0;
int enable = 1;
char resposta_Server[BUFSZ];
char resposta_Peer[BUFSZ];

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

typedef struct {
    int id;
    int potencia;
    int eficiencia;
} Sensor;

//int tabelaSensor[MAX_SENSORS][DADOS_SENSOR]; //Tabela
Sensor tabelaSensor[MAX_SENSORS];

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
    struct sockaddr_in *p2p_addr = (struct sockaddr_in *)(&p2p_storage);; //Essa parte aqui que ta dando problema <<<--------- tava
    
     if (0 != server_sockaddr_init(argv[1], argv[2], &p2p_storage)) {
        usage(argc, argv, 0);
    }

    int p2p_socket = socket(AF_INET, SOCK_STREAM, 0); //Socket servidor 1 - Mi
    if (p2p_socket == -1) {
        logexit("P2P socket");
    }

    

    /*---------------------- SERVER PASSIVO -----------------------*/
    if (0 != connect(p2p_socket, (struct sockaddr *)p2p_addr, sizeof(p2p_storage))) {
        // Se falhar, significa que não há conexão ativa, então imprime a mensagem e abre uma conexão passiva


    if (0 != setsockopt(p2p_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR , &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    if (0 != bind(p2p_socket, (struct sockaddr *)p2p_addr, sizeof(p2p_storage))) {
        logexit("bind");
    }
    
    // Colocar o socket em modo de escuta para o Servidor 1 - como ele é o ativo, acho que não precisa agora
     if (0 != listen(p2p_socket, 10)) {
        logexit("listen");
    }
    //listen(p2p_socket, 5);
    printf("No peer found, starting to listen..\n");
    //printf("Servidor P2P aguardando conexão na porta 1\n");

    //TENTANDO CONECTAR:
        while(1){
            struct sockaddr_storage cstorage;
            struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
            socklen_t caddrlen = sizeof(cstorage);
            
            int csock = accept(p2p_socket, caddr, &caddrlen);
            if (csock == -1) {
                logexit("accept_P2P_out");
            }

            //CODE INIT HERE
            //Enviar a mensagem REQ_ADDPEER para o servidor Mj
            // snprintf(resposta_Server, sizeof(resposta_Server), "REQ_ADDPEER");
            // send(csock, resposta_Server, strlen(resposta_Server), 0);
            // if(send(csock, resposta_Server, strlen(resposta_Server), 0) == -1) {
            //     logexit("Mensage Lost...");
            // }
            // printf("Sent REQ_ADDPEER message to peer Mj\n");

            //  // Receber dados do peer
            // if(recv(csock, resposta_Peer, sizeof(resposta_Peer), 0) == -1) {
            // //    logexit("No mensage received from peer..");
            // }

            // printf("Peer diz: %s", resposta_Peer);
        }

    
    }

    /* ---------------------- SERVER ATIVO, TENTA CONECTAR AO PASSIVO ---------------------- */
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
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR , &enable, sizeof(int))) {
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
    //printf("Server waiting for connections...\n");

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
    //printf("bound to %s, waiting connections\n", addrstr_p2p);

    printf("Conexão P2P estabelecida com sucesso.\n");

    // Lógica do peer

    // char buffer[1024];
    // memset(buffer, 0, sizeof(buffer));

    // //Receber resposta do servidor P2P
    // if (recv(s, buffer, sizeof(buffer), 0) == -1) {
    // //    logexit("No mensage from passive..");
    // }

    // printf("Servidor P2P diz: %s\n", buffer);

    // char mensagem[] = "Olá, servidor! Esta é uma mensagem do peer.";
    // send(s, mensagem, sizeof(mensagem), 0);

    while (1) {

        /* --------------------------- Teste do Select() -------------------------- */

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            logexit("select");
        }


        if (FD_ISSET(p2p_socket, &read_fds)) {
            
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
            
            //Inicializa a tabela de sensor com valores aleatórios
            //O número de sensores também é aleatório
            int numSensores = rand() % (MAX_SENSORS - 2) + 3;

            for (int i = 0; i < numSensores; i++) {
            tabelaSensor[i].id = i;
            tabelaSensor[i].potencia = rand() % 2001; // Valor entre 0 e 2000 W
            tabelaSensor[i].eficiencia = rand() % 101; // Valor entre 0 e 100%
            }
            
            //void imprimirDadosSensores();
            printf("Tabela de Sensores:\n");
            printf("ID\tPotência\tEficiência\n");

            for (int i = 0; i < MAX_SENSORS; i++) {
            printf("%d\t%d W\t\t%d%%\n", tabelaSensor[i].id, tabelaSensor[i].potencia, tabelaSensor[i].eficiencia);
            }

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
                //int id_Teste = clients_List[contador_Cliente].id;

                if(localiza_Cliente(id_Teste) == 1){

                    // Remova o cliente da lista
                    clients_List[id_Teste].id = 0;
                    clients_List[id_Teste].csock = -1;
                    printf("Client %d removed\n", id_Teste);
                    contador_Cliente--;

                    //Removido com sucesso
                    memcpy(resposta_Server, "OK_01", sizeof("OK_01")); //Funciona mas ta bugado <---------------
                    send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente
                    //close(csock);
                    //PROVAVELMENTE FALTA UM CLOSE CSOCK

                } else{

                    memcpy(resposta_Server, "ERROR_04", sizeof("ERROR_04")); //Funciona mas ta bugado <---------------
                    send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente
                    printf("Errou o número ae mano \n"); //APAGAR DEPOIS
                
                }
            }
            /* ------------------------------- Calcula o sensor com maior potência útil ------------------------------ */
            else if (0 == strncmp(buf, "REQ_LS", 7)) {
            Sensor sensorMaiorPotencia;
            int max_pot;
            int pot_atual;

            for (int i = 0; i < MAX_SENSORS; i++) {
                pot_atual = (tabelaSensor[i].potencia * tabelaSensor[i].eficiencia)/100;
                if (pot_atual > max_pot) {
                    max_pot = pot_atual;
                    sensorMaiorPotencia = tabelaSensor[i];
                    }
            }

            printf("local 1 sensor %d: %d (%d %d)\n", sensorMaiorPotencia.id,
                                                                    max_pot,
                                                                    sensorMaiorPotencia.potencia,
                                                                    sensorMaiorPotencia.eficiencia); //APAGAR DEPOIS
            //Manda os dados pro cliente
            sprintf(resposta_Server, "RES_ES local 1 sensor %d: %d (%d %d)\n", sensorMaiorPotencia.id,
                                                                    max_pot,
                                                                    sensorMaiorPotencia.potencia,
                                                                    sensorMaiorPotencia.eficiencia);
            send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente



            }
            /* ------------------------------- Calcula a potência útil total LOCAL ------------------------------ */
            else if (0 == strncmp(buf, "REQ_LP", 7)) {
            int pot_atual;
            int pot_util_total;

            for (int i = 0; i < MAX_SENSORS; i++) {
                pot_atual = (tabelaSensor[i].potencia * tabelaSensor[i].eficiencia)/100;
                pot_util_total += pot_atual;
            }

            printf("local 1 potency: %d\n", pot_util_total); //APAGAR DEPOIS

            sprintf(resposta_Server, "RES_LP local 1 potency: %d\n", pot_util_total);
            send(csock, resposta_Server, strlen(resposta_Server) + 1, 0); // Manda o dado pro cliente

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
    
    free(clients_List);

    exit(EXIT_SUCCESS);
}