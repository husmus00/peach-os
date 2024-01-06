#include "peachos.h"

extern int main(int argc, char** argv);

// Will be called from the _start assembly function
// so we can handle bootstrapping in C instead of ASM
void c_start() {
    // Get the arguments via kernel syscall
    struct process_arguments arguments;
    peachos_process_get_arguments(&arguments);
    
    // Call the main function!
    int exit_code = main(arguments.argc, arguments.argv);
    if (exit_code) {
        // Handle the exit code?
    }
}