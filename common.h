#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

void logexit(const char *msg);
//void usage(int argc, char **argv);
void usage(int argc, char **argv, int n);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

// int server_sockaddr_init(const char *proto, const char *portstr,
//                          struct sockaddr_storage *storage);
int server_sockaddr_init(const char *id_server, const char *portstr, 
                                                const char *portp2p, 
                                                struct sockaddr_storage *storage);

void quebraString(const char *entrada, char *info[], int maxPedacos);
int quantosDados(const char *dados[]);

//Para testar: Terminal 1: ./server 127.0.0.1 90900 90100 ./server 127.0.0.1 90900 90200 ./client 127.0.0.1 90100 ./client 127.0.0.1 90200

