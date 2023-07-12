#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include "../include/parse_cert.h"
#include "../include/file_reader.h"
#include "../include/check_sig.h"
#include "../include/server_helper.h"

#define MAX_BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    server_fd = setup_server_socket(&address);

    while(1){
         // Accept a connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char filename[64];
        handle_client(new_socket, filename);

        //I assume that the total size (including "EOF" and the null terminator)
        // will not exceed 1024 bytes, so I don't check for buffer overflow here.
        // Read the file
        // Read the file
        char* script_buff = read_file_into_buffer(filename);
        if (script_buff == NULL) {
            fprintf(stderr, "Failed to read file\n");
            return 0;
        }

        // Extract the signature
        char *signature_str = strtok(script_buff, "\n");
        // printf("Signature: %s\n", signature_str);
        // Advance script_buff past the signature
        char *script_start = signature_str + strlen(signature_str) + 1;
        printf("Script received from Process ID: %d\n", getpid());
        // printf("%s\n", script_start);

        
        int status_code = check_signature_with_all_certs(script_start, signature_str, "../certs");
        if (status_code == 1) {
            printf("Congratulations! The script is safe to execute!\n");
           
            pid_t pid = fork();  // Create a new process
            if (pid < 0) {
                // Handle error
                perror("fork failed");
            } else if (pid == 0) {
                //fork a child process to execute the script to keep the server running
                char *args[] = {"/bin/bash", "-c", script_start, NULL};
                execvp(args[0], args);
                exit(EXIT_SUCCESS);
            } else {
                // This block will be executed by the parent process
                // The parent can continue to run while the child executes the script
            }

        } else {
            printf("NO valid certificate found for this script! Do not execute!\n");
        }

        //cleanup
        free(script_buff);
        close(new_socket);

        if (remove(filename) != 0) {
            perror("Error deleting file");
        } else {
            // printf("File successfully deleted\n");
        }
    }

    return 0;
}