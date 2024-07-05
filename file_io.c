#include <stdio.h>
#include <stdlib.h>

#include "file_io.h"
#include "log.h"

char* file_io_read(const char* file_path, size_ty* size) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        log_error("File I/O: Failed to open file \"%s\"\n", file_path);
        return NULL;
    }
    
    if (fseek(file, 0L, SEEK_END) != 0) {
        log_error("File I/O: Failed to determine file size: error while navigating to end of file.\n");
        fclose(file);
        return NULL;
    }
    long file_position = ftell(file);
    if (file_position == -1L) {
        log_error("File I/O: Failed to determine file size: Error while trying to get current position in file.\n");
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        log_error("File I/O: Failed to determine file size: Error while navigating to start of file buffer.\n");
        fclose(file);
        return NULL;
    }

    char* buffer = malloc(sizeof(char) * (size_t)file_position);
    size_t buffer_size = fread(buffer, sizeof(char), (size_t)file_position, file);
    if (ferror(file) != 0) {
        log_error("File I/O: Failed to read data from file: Error while reading file contents in to buffer.\n");
        fclose(file);
        free(buffer);
        return NULL;
    }
    buffer[buffer_size++] = '\0';

    fclose(file);

    *size = buffer_size;
    return buffer;
}