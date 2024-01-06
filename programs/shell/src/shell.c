#include "shell.h"
#include "stdio.h"
#include "peachos.h"
#include "string.h"

int main(int argc, char** argv) {
    println("Shell loaded");

    char* current_path = (char*)peachos_malloc(PEACHOS_MAX_PATH);
    const char* default_path = "0:/";  
    strncpy(current_path, default_path, strlen(default_path) + 1);

    while(1) 
    {
        printf("%s> ", current_path);
        char buf[1024];
        int path_length = strlen(current_path);
        strncpy(buf, current_path, strlen(current_path) + 1);
        peachos_terminal_readline(buf + path_length, (sizeof(buf) - path_length), 1); // This is a blocking funtion
        // println(buf);
        println("");
        peachos_system_run(buf);
        println("");
    }
    return 0;
}