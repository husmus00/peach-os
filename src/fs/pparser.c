// Path parser

#include "pparser.h"
#include "kernel.h"
#include "status.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/heap/kernelheap.h"
#include "config.h"

static int pathparser_path_valid_format(const char* filename) {
    int len = strnlen(filename, PEACHOS_MAX_PATH);
    // File path must be at least 3 chars, must start with a digit followed by :/
    return (len >= 3 && is_digit(filename[0]) && memcmp((void*)&filename[1],  ":/", 2) == 0);
}

// Get the drive portion from a path
// E.g., get '0' from '0:/path/file.txt'
// The double pointer is passed so we can change the *pointer* itself, rather than what is points to
static int pathparser_get_drive_by_path(const char** path) {
    if (!pathparser_path_valid_format(*path)) {
        return -EBADPATH;
    }

    int drive_no = to_numeric_digit(*path[0]);

    *path += 3; // Skip the 0:/ part of the path

    return drive_no;
}

// Extract the next path part from the provided string
// E.g., get 'path1' from 'path1/path2/file.txt'
// The result of this function is what gets assigned as the 'part' field of a path_part struct
static const char* pathparser_get_path_part(const char** path) {
    char* result_path_part = kzalloc(PEACHOS_MAX_PATH);
    int i = 0;
    while(**path != '/' && **path != 0x00) {
        result_path_part[i] = **path;
        *path += 1; // Changes the path for the caller too.
        i++;
    }

    if (**path == '/') {
        // Skip the forward slash to avoid problems
        *path += 1;
    }

    if (i == 0) {
        // Nothing was resolved
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part; // This is the 'part' field for a path_part struct
}

// Convert a path section into a path_part struct, assigning the appropriate fields 
// + assigning the field for the pevious part of the path, if applicable
struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path) {
    const char* path_part_str = pathparser_get_path_part(path);
    if (!path_part_str) {
        return 0;
    }

    struct path_part* new_part = kzalloc(sizeof(struct path_part));
    new_part->part = path_part_str;
    new_part->next = 0x00;

    if (last_part) {
        last_part->next = new_part;
    }

    return new_part;
}

static struct path_root* pathparser_create_root(int drive_number) {
    struct path_root* path_r = kzalloc(sizeof(struct path_root));

    path_r->drive_no = drive_number;
    path_r->first = 0;
    return path_r;
}

// The public function that other parts of the kernel can call
// This function returns the root which will contain any subsequent path parts (recursive)
struct path_root* pathparser_parse(const char* path, const char* current_directory_path) {
    // current_directory_path variable currently not used (will be later)

    int res = 0;
    const char* tmp_path = path; // initial path since we'll change this pointer and we want to preserve the 'path' variable
    struct path_root* path_root = 0;

    if (strlen(path) > PEACHOS_MAX_PATH) {
        goto out;
    }

    res = pathparser_get_drive_by_path(&tmp_path);
    if (res < 0) {
        goto out;
    }

    path_root = pathparser_create_root(res);
    if (!path_root) {
        goto out;
    }

    struct path_part* first_part = pathparser_parse_path_part(NULL, &tmp_path);
    if (!first_part) {
        goto out;
    }

    path_root->first = first_part;

    struct path_part* next_part = pathparser_parse_path_part(first_part, &tmp_path);
    while (next_part) {
        next_part = pathparser_parse_path_part(next_part, &tmp_path);
    }

out:
    return path_root;
}

// Iteratively frees both the part field and the path_part struct of an entire path (including root)
// The only other pparser function publicly available
void pathparser_free(struct path_root* root) {
    struct path_part* current_part = root->first;
    
    while(current_part) { // While a part still exists
        struct path_part* next_part = current_part->next;
        kfree((void*)current_part->part);
        kfree(current_part);
        current_part = next_part;
    }

    kfree(root);
}