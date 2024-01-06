#include "stdlib.h"
#include "stdio.h"
#include "peachos.h"
#include "string.h"

int main(int argc, char** argv) {
    // println("Now in blank.c");
    
    // void* ptr = malloc(512);
    // free(ptr);

    // print(itoa(50));

    // putchar('C');
    // println("");

    // printf("Testing %s", "Hello\n");
    // println("Hey");


    // // Testing tokenisation
    // char words[] = "hello how are you";
    // const char* token = strtok(words, " ");
    // while(token) {
    //     println(token);
    //     // println(token);
    //     token = strtok(NULL, " ");
    // }

    // // Testing allocations
    // char* ptr = malloc(20);
    // strcpy(ptr, "Hello, test!");
    // print(ptr);
    // free(ptr);
    // ptr[0] = 'B';
    // print("abc");

    // // Testing parsing a command
    // char str[] = "Hello world";
    // struct command_argument* root_command =  peachos_parse_command(str, sizeof(str));
    // println(root_command->argument);
    // println(root_command->next->argument);

    // // Testing passing arguments
    // struct process_arguments arguments;
    // peachos_process_get_arguments(&arguments);
    // printf("%i: %s", arguments.argc, arguments.argv[0]);

    // Printing all arguments
    // printf("Number of arguments: %i\n", argc);
    // for (int i = 0; i < argc; i++) {
    //     printf("%i. %s\n", i + 1, argv[i]);
    // }

    // int zero = tonumericdigit(*argv[1]);
    // int crash = 10 / zero;
    // print(itoa(crash));

    // char* ptr = (char*)0x00;
    // *ptr = "Hello";

    // for (int i = 0; i < 25; i++) {
    //     println(itoa(i));
    // }

    // while(1) {
    //     int x = peachos_getkeyblock();
    //     // putchar(x);
    //     println("");
    // }

    // peachos_exit(-1);

    int counter = 0;

    while (true) {
        println(argv[0]);
        while (counter < 20000000) {
            counter++;
        }
        counter = 0;
    }
    // println(argv[0]);

    return 0;
}