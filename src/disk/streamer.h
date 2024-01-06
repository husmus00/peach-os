#if !defined(DISKSTREAMER_H)
#define DISKSTREAMER_H

#include "disk.h"

struct disk_stream {
    int pos; // byte position
    struct disk* disk;
};

struct disk_stream* disk_streamer_new(int disk_id);
int diskstreamer_seek(struct disk_stream* stream, int pos);
int diskstreamer_read(struct disk_stream* stream, void* out, int total);
void diskstreamer_close(struct disk_stream* stream);

#endif // DISKSTREAMER_H
