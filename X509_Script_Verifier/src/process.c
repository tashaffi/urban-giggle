#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>

#include "../include/file_reader.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <script_name>\n", argv[0]);
        return 1;
    }

    char *script_name = argv[1];
    // Create a buffer to hold the script path
    char script_path[MAX_BUFFER_SIZE];
    snprintf(script_path, sizeof(script_path), "../scripts/%s", script_name);

    // Check if the file exists
    if (access(script_path, F_OK) == -1) {
        fprintf(stderr, "File %s not found\n", script_path);
        return 1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //I assume that the total size (including "EOF" and the null terminator)
    // will not exceed 1024 bytes, so I don't check for buffer overflow here.
    // Read the file
    char* buffer = read_file_into_buffer(script_path);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to read file into buffer\n");
        return -1;
    }
    strcat(buffer, "EOF");

    // Send the buffer over the socket
    ssize_t bytes_sent = send(sock, buffer, strlen(buffer), 0);
    if (bytes_sent < strlen(buffer)) {
        perror("Failed to send entire buffer");
        free(buffer);
        return -1;
    }
   
    buffer[0] = '\0';  // Clear the buffer

    // Read the message from the servers
    ssize_t bytes_read = read(sock, buffer, 1024);
    if (bytes_read < 0) {
        perror("Failed to read message from server");
        free(buffer);
        return -1;
    }
    
    printf("%s\n", buffer);

    free(buffer);
    close(sock);

    return 0;
}