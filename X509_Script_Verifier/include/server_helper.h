#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <netinet/in.h>

#define PORT 8080

int setup_server_socket(struct sockaddr_in *address);
void handle_client(int new_socket, char* filename);

#endif // SERVER_SOCKET_H