#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

// void usage(int argc, char **argv) {
// 	printf("usage: %s <server IP> <server port>\n", argv[0]);
// 	printf("example: %s 127.0.0.1 51511\n", argv[0]);
// 	exit(EXIT_FAILURE);
// }

#define BUFSZ 1024

char sensor[5][BUFSZ];
char buf[BUFSZ];
char bufInfo[BUFSZ];

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv, 1);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv, 1);
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
	
	while(1){
	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	/*--------------------------------- Buffers para alocar informações --------------------------------------------*/
	//CODE INIT HERE
	char dadosDigitados[BUFSZ]; //Entrada
	memset(dadosDigitados, 0, BUFSZ); //Aloca memória
    printf("Digite uma string: ");
    fgets(dadosDigitados, sizeof(dadosDigitados), stdin);

    dadosDigitados[strcspn(dadosDigitados, "\n")] = '\0'; // Remover o caractere de nova linha

    char *dados[20]; //Limite de 20 palavras
	char *dadosDoServer[20]; //Limite de 20 palavras
	memset(dados, 0, BUFSZ); //Limpa os dados para validar o tamanho deles
	memset(dadosDoServer, 0, BUFSZ);
    quebraString(dadosDigitados, dados, 20); //Chama a função para quebrar os dados da string	

	if (dados[0] != NULL) {
		
		/*--------------------------------- Mensagem de requisição de saída de cliente na rede -------------------------------------*/
		if(strcmp(dados[0], "kill") == 0){
			//char *msg_req = "REQ_DC";
      		char buf_to_send[BUFSZ];	   //Uma unica string para enviar
      	    memset(buf_to_send, 0, BUFSZ); //Memoria pra string
            strcat(buf_to_send, "REQ_DC");
			size_t count = send(s, buf_to_send, strlen(buf_to_send) + 1, 0);
			if (count != strlen(buf_to_send) + 1) {
        		logexit("send");
      		}

		}
		/*--------------------------------- Consultar Max Potência Local -------------------------------------*/
		else if(strcmp(dados[0], "show") == 0){
			if(strcmp(dados[1], "localmaxsensor") == 0){
				
			char buf_to_send[BUFSZ];	   
      	    memset(buf_to_send, 0, BUFSZ); 
            strcat(buf_to_send, "REQ_LS");
			size_t count = send(s, buf_to_send, strlen(buf_to_send) + 1, 0);
				if (count != strlen(buf_to_send) + 1) {
        			logexit("send");
      			}
			}
		/*--------------------------------- Consultar Max Potência Externa --------------------------------*/
			else if(strcmp(dados[1], "externalmaxsensor") == 0){

			char buf_to_send[BUFSZ];	   
      	    memset(buf_to_send, 0, BUFSZ); 
            strcat(buf_to_send, "REQ_ES");
			size_t count = send(s, buf_to_send, strlen(buf_to_send) + 1, 0);
				if (count != strlen(buf_to_send) + 1) {
        			logexit("send");
      			}
			
			}
		}

	size_t count = send(s, dadosDigitados, strlen(dadosDigitados)+1, 0);
	if (count != strlen(dadosDigitados) + 1) {
	logexit("send");
	}

	// memset(buf, 0, BUFSZ);
	// unsigned total = 0;
	// while(1) {
	// 	count = recv(s, dadosDigitados + total, BUFSZ - total, 0);
	// 	if (count == 0) {
	// 		// Connection terminated.
	// 		break;
	// 	}
	// 	total += count;
	// }
	// //close(s);
	
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	recv(s, buf, BUFSZ - 1, 0);
	buf[strcspn(buf, "\n")] = '\0'; // Remover o caractere de nova linha
	quebraString(buf, dadosDoServer, 20);

	//Recebe a string e imprime a mensagem
	if (dadosDoServer[0] != NULL) {
		printf("Recebido: %s \n", dadosDoServer[0]);
    	if (strcmp(dadosDoServer[0], "RES_ADD(") == 0) {	//Funciona, mas com o espaço quando o servidor envia a msg
			int Id;											//Depois apagar a parte do terminal que imprime o RES_ADD( 1 ) no cliente
            sscanf(buf, "RES_ADD(%d)", &Id);
			printf("New ID: %d\n", Id);
		}
		else if (strcmp(dadosDoServer[0], "ERROR_01") == 0) {
        	printf("Client limit exceeded\n");
		}
		else if (strcmp(dadosDoServer[0], "ERROR_04") == 0) {
        	printf("Client not found\n");
		}
		else if (strcmp(dadosDoServer[0], "OK_01") == 0) {
        	printf("Successful disconnect\n");
		}
	}
	
	//printf("received %u bytes\n", total);
	//puts(dadosDigitados);

	exit(EXIT_SUCCESS);
	}

	}
}