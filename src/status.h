#if !defined(STATUS_H)
#define STATUS_H

// #define PEACHOS_ALL_OK 0
// #define EIO 1       // IO error
// #define EINVARG 2   // Invalid argument error
// #define ENOMEM 3    // No memory error
// #define EBADPATH 4  // Bath path error
// #define EFSNOTUS 5  // Wrong filesystem
// #define ERDONLY 6   // File is read only error
// #define EUNIMP 7    // Unimplemented functionality
// #define EISTKN 8    // Slot is taken (e.g., process slot)
// #define EINFORMAT 9 // Invalid file format (e.g., when loading elf file)

enum peachos_status {
    PEACHOS_ALL_OK = 0,
    EIO = 1,       // IO error
    EINVARG = 2,   // Invalid argument error
    ENOMEM = 3,    // No memory error
    EBADPATH = 4,  // Bath path error
    EFSNOTUS = 5,  // Wrong filesystem
    ERDONLY = 6,   // File is read only error
    EUNIMP = 7,    // Unimplemented functionality
    EISTKN = 8,    // Slot is taken (e.g., process slot)
    EINFORMAT = 9, // Invalid file format (e.g., when loading elf file)
};

#endif // STATUS_H
