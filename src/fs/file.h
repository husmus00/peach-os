#if !defined(FILE_H)
#define FILE_H

#include "pparser.h"
#include <stdint.h>


typedef unsigned int FILE_SEEK_MODE;
enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

enum {
    FILE_STAT_READ_ONLY = 0b00000001,
};

typedef unsigned int FILE_STAT_FLAGS;

// Represents file status and data
struct file_stat {
    FILE_STAT_FLAGS flags;
    uint32_t file_size;
};

struct disk;
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk); // Resolves whether a disk is a valid filesystem of the caller's type
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t no_of_mem_blocks, char* out); // nmemb = number of memory blocks
typedef int (*FS_CLOSE_FUNCTION)(void* private);
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);

struct filesystem {
    // Filesystem should return 0 from resolve if the provided is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_CLOSE_FUNCTION close;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    char name[20];
};

struct file_descriptor {
    // The descriptor index
    int index;
    struct filesystem* filesystem;

    // Private data for internal file descriptor
    void* private;

    // Disk that the file descriptor should be used on
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
int fclose(int fd);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fread(void* ptr, uint32_t size, uint32_t no_of_mem_blocks, int fd);
int fstat(int fd, struct file_stat* stat);

void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);

#endif // FILE_H
