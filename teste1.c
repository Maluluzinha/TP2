// #include "common.h"

// #define BUFSZ 1024

//  struct sockaddr_storage storage; //Instanciavel

// // void usage(int argc, char **argv){ //Função em caso de erro na chamada
// //     printf("usage: %s <v4|v6> <server port>\n", argv[0]);
// //     printf("example: %s v4 51511\n", argv[0]);
// //     exit(EXIT_FAILURE);

// // }

// int main(int argc, char **argv){
//     /*----------------SOCKET PRINCIPAL--------------*/
//     if (argc < 3){          //Verifica se chamou o programa corretamente, senão retorna pro usage() acima. Primeiro verificador de erro
//       usage(argc, argv, 0);    //Ver se os parâmetros tão ok
//     }

//     struct sockaddr_storage storage; //Instanciavel
//      if(0 != server_sockaddr_init(argv[1], argv[2], &storage)){ //Recebe o tipo (v4 ou v6), o port, e o ponteiro pro storage. ERRO
//         usage(argc, argv, 0);
//     }

//     int socket_passivo; //Passivo, principal
//     socket_passivo = socket(storage.ss_family, SOCK_STREAM, 0); //Socket que recebe a conexão
//     if(socket_passivo = -1){ //Primeira verificação de erro
//         logexit("Socket \n");
//     }

//     struct sockaddr *addr = (struct sockaddr *)(&storage); //Servidor não tem send
//     if (0 != bind(socket_passivo, addr, sizeof(storage))) { //Inicializa com o socket e a struct, e o tamanho
//         logexit("Bind \n");
//     }

//     if (0 != listen(socket_passivo, 10)) { //Numero que vai no listen numero de conexoes que podem estar pendentes para tratamento
//     logexit("listen");
//     }

//     char addrstr[BUFSZ];
//     addrtostr(addr, addrstr, BUFSZ);
//     printf("Bound to %s, waiting connections \n", addrstr);

//     if (connect(socket_passivo, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//         perror("Erro ao conectar ao servidor central");
//         exit(EXIT_FAILURE);
//     }

//     int socket_ativo; //Passivo, principal
//     socket_ativo = socket(storage.ss_family, SOCK_STREAM, 0); //Socket que recebe a conexão
//     if(socket_ativo = -1){ //Primeira verificação de erro
//         logexit("Socket 2 \n");
//     }


//     //Servidor inicializado, agora chama o connect
//     while(1){
//         //Função accept retorna uma novo soquete. Recebe o socket, o socaddr do cliente que conectou
//     struct sockaddr_storage cstorage;
//     struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
//     socklen_t caddrlen = sizeof(cstorage);

//     int csock = accept(socket_passivo, caddr, &caddrlen); //Socket q conversa com o cliente
//     if (csock == -1) { //Erro
//         logexit("Accept");
//         }

//     char caddrstr[BUFSZ]; //Endereço do cliente
//     addrtostr(addr, caddrstr, BUFSZ);
//     printf("[log] connection from %s\n", caddrstr);

//     //Agora para receber o dado/ler a mensagem que o cliente enviou
//     // Receiving message from client
//     char buf[BUFSZ];
//     memset(buf, 0, BUFSZ);
//     size_t count = recv(csock, buf, BUFSZ - 1, 0); //Mensagem que chega do cliente
//     printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf); //Print a mensagem do cliente
    
//     sprintf(buf, "remote endpoint: %.1000s \n", caddrstr);
//     send(csock, buf, strlen(buf)+1, 0); //Mand ao dado pro cliente
//     if(count != strlen(buf)+1){ //Erro envio
//        logexit("Send");
//     }
//     close(csock); //Fecha e volta pro inicio do loop

//     }
    
// }