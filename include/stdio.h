#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>

typedef struct FILE {
    uint32_t curr_cluster;
    uint32_t file_size; // total file size
    uint32_t fptr; // index into the file
    uint32_t buffptr; // index into currbuf
    uint8_t currbuf[]; // flexible member for current cluster
} FILE;

typedef struct DIR {
    struct dir_entry *entries; // Array of directory entries
    size_t num_entries; // Number of entries in the directory
    size_t index; // Current index in the directory entries
} DIR;

// File operations
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

// Directory operations
DIR *opendir(const char *name);
struct dir_entry *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif // STDIO_H