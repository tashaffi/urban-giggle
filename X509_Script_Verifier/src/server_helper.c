#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/file_reader.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024


int setup_server_socket(struct sockaddr_in *address){
    int server_fd, new_socket;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}


void handle_client(int new_socket, char* filename) {
    sprintf(filename, "received_file_%d.sh", getpid());  // Create a unique filename using the PID
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    char complete_data[MAX_BUFFER_SIZE * 10];  // Adjust size as needed
    memset(complete_data, 0, sizeof(complete_data));

    while (1) {
        ssize_t bytes_received = recv(new_socket, buffer, MAX_BUFFER_SIZE - 1, 0);
        // printf("Received %ld bytes\n", bytes_received);
        if (bytes_received <= 0) {
            // Handle error
            break;
        }
        buffer[bytes_received] = '\0';  // Null-terminate the received data
        strcat(complete_data, buffer);  // Concatenate the received data to complete_data

        if (strstr(buffer, "EOF") != NULL) {
            // Remove the EOF marker from the buffer before writing to the file
            char *eof_marker = strstr(complete_data, "EOF");
            *eof_marker = '\0';
            fputs(complete_data, file);
            break;  // Break the loop when the EOF marker is received
        }
        memset(buffer, 0, sizeof(buffer));
    }

    fclose(file);
}