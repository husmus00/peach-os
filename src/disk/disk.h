#if !defined(DISK_H)
#define DISK_H

#include "fs/file.h"

typedef unsigned int PEACHOS_DISK_TYPE;

// Represents a real physical hard disk
#define PEACHOS_DISK_TYPE_REAL 0

// The fs_resolve() function will bind the correct filesystem to the disk
struct disk {
    PEACHOS_DISK_TYPE type;
    int sector_size;

    int id;

    struct filesystem* filesystem;

    // The private data of our filesystem
    // Should be stored here rather than the filesystem struct because a filesystem may be binded to multiple disks
    void* fs_private;
};

// int disk_read_sector(int lba, int total, void* buf); // Should only be used internally by disk.c code
void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);

#endif // DISK_H
