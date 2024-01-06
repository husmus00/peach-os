#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kernelheap.h"
#include "status.h"
#include "string/string.h"
#include "fs/fat/fat16.h"
#include "disk/disk.h"
#include "kernel.h"

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

// Get a free filesystem slot in the array
static struct filesystem** fs_get_free_filesystem() {
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0) {
            return &filesystems[i];
        }
    }
    return 0;
}

// Inserts the supplied filesystem into the 'filesystems' array
void fs_insert_filesystem(struct filesystem* filesystem) {
    struct filesystem** fs;
    if (filesystem == 0) {
        // panic();
    }

    fs = fs_get_free_filesystem();

    if (!fs) {
        panic("Problem inserting filesystem");
    }

    *fs = filesystem;
}

// This function inserts the file systems which are built in to the system (FAT16 in our case)
static void fs_static_load() {
    fs_insert_filesystem(fat16_init()); // fat16_init returns the fat16_fs struct (of type struct filesystem)
}

void fs_load() {
    // Set the array of filesystems to all 0
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static void file_free_descriptor(struct file_descriptor* desc) {
    // We do -1 because descriptors start from 1 not 0
    file_descriptors[desc->index - 1] = 0x00;
    kfree(desc);
}

// Get an empty file descriptor slot and return a new file descriptor
static int file_new_descriptor(struct file_descriptor** desc_out) {
    int res = -ENOMEM;
    for (int i = 0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0) {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }
    return res;
}

static struct file_descriptor* file_get_descriptor(int fd) {
    if (fd <= 0 || fd >= PEACHOS_MAX_FILE_DESCRIPTORS) {
        return 0;
    }

    // Descriptors start at 1
    return file_descriptors[fd - 1];
}

// Returns the correct filesystem for the provided disk
struct filesystem* fs_resolve(struct disk* disk) {
    struct filesystem* fs = 0;
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}

FILE_MODE file_get_mode_by_string(const char* str) {
    FILE_MODE mode = FILE_MODE_INVALID;

    if (strncmp(str, "r", 1) == 0) {
        mode = FILE_MODE_READ;
    }

    if (strncmp(str, "w", 1) == 0) {
        mode = FILE_MODE_WRITE;
    }

    if (strncmp(str, "a", 1) == 0) {
        mode = FILE_MODE_APPEND;
    }

    return mode;
}

// Generic open function which calls the appropriate filesystem function e.g., fat16_open()
int fopen(const char* filename, const char* mode_str) {
    int res = 0;

    struct path_root* root_path = pathparser_parse(filename, NULL);
    if (!root_path) {
        res = -EINVARG;
        goto out;
    }

    if (!root_path->first) {
        // If true, path is only root e.g., 0:/ with no files
        res = -EINVARG;
        goto out;
    }

    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk) {
        // Disk could not be found
        res = -EIO;
        goto out;
    }

    if (!disk->filesystem) {
        // Disk does not have an associated filesystem
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);

    if (mode == FILE_MODE_INVALID) {
        res = -EINVARG;
        goto out;
    }

    // We will get the private data descriptor from the designated filesystem
    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data)) {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }
    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0) {
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // If fopen() fails, it must return NULL, not negative values
    if (res < 0) {
        res = 0;
    }
    return res;
}

// Returns the file status
int fstat(int fd, struct file_stat* stat) {
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc) {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private, stat);

out:
    return res;
}

// Close the file and delete the file descriptor
int fclose(int fd) {
    int res = 0;

    struct file_descriptor* desc = file_get_descriptor(fd); 
    if (!desc) {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
    if (res == PEACHOS_ALL_OK) {
        file_free_descriptor(desc);
    }

out:
    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence) {
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc) {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private, offset, whence);

out: 
    return res;
}

// Read from the file specified by the file descriptor 'fd' (which is essentialy an ID) which was obtained from fopen()
int fread(void* out, uint32_t size, uint32_t no_of_mem_blocks, int fd) {
    int res = 0;
    // Check all parameters are valid
    if (size == 0 || no_of_mem_blocks == 0 || fd < 1) {
        res = -EINVARG;
        goto out;
    }

    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc) {
        res = -EINVARG;
        goto out;
    }

    // Pass the private data obtained from fopen() to the filesystem's implementation of fread()
    res = desc->filesystem->read(desc->disk, desc->private, size, no_of_mem_blocks, (char*) out);
out:
    return res;
}
