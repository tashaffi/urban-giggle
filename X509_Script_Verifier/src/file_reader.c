#include <stdio.h>
#include <stdlib.h>
#include "../include/file_reader.h"

char* read_file_into_buffer(const char* filename) {
    // Open the script file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
        return NULL;
    }

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate a buffer large enough to hold the entire file
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        fclose(file);
        return NULL;
    }

    // Read the entire file into the buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read < file_size) {
        perror("Failed to read entire file");
        free(buffer); 
        fclose(file); 
        return NULL;
    }

    fclose(file);
    // Null-terminate the buffer
    buffer[bytes_read] = '\0';
    return buffer;
}