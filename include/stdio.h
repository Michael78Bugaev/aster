#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>

/* File structure */
typedef struct _FILE {
    uint32_t curr_cluster;
    uint32_t file_size;
    uint32_t fptr;
    uint32_t buffptr;
    uint8_t currbuf[512];
} FILE;

/* File operations */
FILE* fopen(const char* pathname, const char* mode);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

/* Directory operations */
typedef struct _DIR {
    struct dir_entry* entries;
    size_t num_entries;
    size_t index;
} DIR;

DIR* opendir(const char* name);
struct dir_entry* readdir(DIR* dirp);
int closedir(DIR* dirp);

/* Standard I/O */
void printf(const char* format, ...);

/* Error handling */
#define EOF (-1)

/* Buffer size for standard streams */
#define BUFSIZ 512

/* Seek origin */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* File access modes */
#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */

#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */

/* File buffering options */
#define _IOFBF 0x0000
#define _IOLBF 0x0040
#define _IONBF 0x0004

#endif /* STDIO_H */