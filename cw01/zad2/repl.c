#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include "../zad1/wc.h"

#ifdef DYNAMIC
#include <dlfcn.h>
#endif

#define BUFFER_SIZE 1000
#define SHOW_TIMES 1

wc_data wc;
short initialized = 0;
clock_t st_time;
clock_t en_time;
struct tms st_cpu;
struct tms en_cpu;
clock_t prog_st_time;
struct tms prog_st_cpu;
int tics_per_second;

// Time functions docs available at:
// https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-times-get-process-child-process-times

/**
 * Start counting time
 *
 * @return void
 */
void start_clock() {
    st_time = times(&st_cpu);
}

/**
 * Stop counting time and print results
 *
 * @return void
 */
void end_clock() {
#ifdef SHOW_TIMES
    en_time = times(&en_cpu);
    fprintf(stdout, "============= TIMES ===============\n");
    fprintf(stdout, "Real time: %f\n", (double) (en_time - st_time) / tics_per_second);
    fprintf(stdout, "User time: %f\n", (double) (en_cpu.tms_utime - st_cpu.tms_utime) / tics_per_second);
    fprintf(stdout, "System time: %f\n\n", (double) (en_cpu.tms_stime - st_cpu.tms_stime) / tics_per_second);
#endif
}

/**
 * Check if string is a number
 *
 * @param str
 * @return int - 1 if string is a number, 0 otherwise
 */
int is_str_num(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * Show help
 *
 * @return void
 */
void show_help() {
    fprintf(stdout, "Available commands:\n");
    fprintf(stdout, "init <size> - initialize structure with size <size>\n");
    fprintf(stdout, "count <path> - count words in file <path>\n");
    fprintf(stdout, "show <index> - show result of counting words at index <index>\n");
    fprintf(stdout, "delete <index> - delete result of counting words at index <index>\n");
    fprintf(stdout, "destroy - destroys initialized structure\n");
    fprintf(stdout, "exit - close REPL\n");
    fprintf(stdout, "help - show available commands\n");
}

/**
 * REPL main function
 *
 * @return void
 */
int main() {
    // Get number of clock ticks per second
    tics_per_second = sysconf(_SC_CLK_TCK);

#ifdef DYNAMIC
    void *handle = dlopen("./libwc.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Cannot read library file");
        exit(1);
    }

    wc_data (*wc_init)(int);
    int (*wc_start)(struct wc_data *, char *);
    char *(*wc_get)(struct wc_data *, int);
    int (*wc_free)(struct wc_data *, int);
    int (*wc_destroy)(struct wc_data *);

    wc_init = (wc_data (*)(int)) dlsym(handle, "wc_init");
    wc_start = (int (*)(struct wc_data *, char *)) dlsym(handle, "wc_start");
    wc_get = (char *(*)(struct wc_data *, int)) dlsym(handle, "wc_get");
    wc_free = (int (*)(struct wc_data *, int)) dlsym(handle, "wc_free");
    wc_destroy = (int (*)(struct wc_data *)) dlsym(handle, "wc_destroy");
    if (dlerror() != NULL) {
        fprintf(stderr, "Cannot load lib file");
        exit(1);
    }
#endif

    char *command = (char *) calloc(BUFFER_SIZE, sizeof(char));

    char delim_for_args[] = "\n\t"; // Delimiters for arguments, ex. when folder has a space in its name
    char delim[] = " \t\r\n\v\f";

    printf("Welcome to the REPL\n");
    show_help();

#ifdef SHOW_TIMES
    // Start counting program execution time (from the beginning of the program after initialization)
    prog_st_time = times(&prog_st_cpu);
#endif

    while (fgets(command, BUFFER_SIZE, stdin) != NULL) {
#ifdef SHOW_TIMES
        // Start counting time (counting time for the whole loop, not specific commands)
        start_clock();
#endif

        // If enter was pressed with no command, repeat loop
        if (command[0] == '\n') {
            continue;
        }

        // Get command
        char *token = strtok(command, delim);

        // COMMAND: init <size>
        if (!strcmp(token, "init")) {
            if (initialized) {
                fprintf(stderr, "Structure is already initialized\n");
                continue;
            }

            // Get size of array
            token = strtok(NULL, delim);
            if (token == NULL) {
                fprintf(stderr, "Missing second argument\n");
                continue;
            }

            if (!is_str_num(token)) {
                fprintf(stderr, "Argument is not a number\n");
                continue;
            }

            int size = (int) strtol(token, NULL, 10);

            wc = wc_init(size);

            if (wc.data == NULL && wc.max_block_size == 1 && wc.current_block_size == 0) {
                fprintf(stderr, "Maximum block size must be greater than 0\n");
                continue;
            }

            if (wc.data == NULL && wc.max_block_size == 0 && wc.current_block_size == 0) {
                fprintf(stderr, "Failed to initialize structure\n");
                continue;
            }

            initialized = 1;
            fprintf(stdout, "Initialized structure with size %d\n", size);
        }
            // COMMAND: count <path>
        else if (!strcmp(token, "count")) {
            if (!initialized) {
                fprintf(stderr, "Structure is not initialized\n");
                continue;
            }

            // Get path
            token = strtok(NULL, delim_for_args);
            if (token == NULL) {
                fprintf(stderr, "Missing second argument\n");
                continue;
            }

            int index = wc_start(&wc, token);
            if (index < 0) {
                switch (index) {
                    case -1:
                        fprintf(stderr, "No space left in structure\n");
                        break;

                    case -2:
                        fprintf(stderr, "Failed to run wc command\n");
                        break;

                    case -3:
                        fprintf(stderr, "Failed to open file with temporary results\n");
                        break;

                    case -4:
                        fprintf(stderr, "Failed to read file\n");
                        break;

                    case -5:
                        fprintf(stderr, "Failed to remove file with temporary results\n");
                        break;

                    default:
                        fprintf(stderr, "Unknown error\n");
                        break;
                }

                continue;
            }

            fprintf(stdout, "Counting process completed. Record saved at index %d\n", index);
        }
            // COMMAND: show <index>
        else if (!strcmp(token, "show")) {
            if (!initialized) {
                fprintf(stderr, "Structure is not initialized\n");
                continue;
            }

            token = strtok(NULL, delim);
            if (token == NULL) {
                fprintf(stderr, "Missing second argument\n");
                continue;
            }

            if (!is_str_num(token)) {
                fprintf(stderr, "Argument is not a number\n");
                continue;
            }

            int index = (int) strtol(token, NULL, 10);

            char *result = wc_get(&wc, index);
            if (result == NULL) {
                fprintf(stderr, "No record at index %d or index is out of range\n", index);
                continue;
            }

            // Print result
            fprintf(stdout, "%s\n", result);
        }
            // COMMAND: delete <index>
        else if (!strcmp(token, "delete")) {
            if (!initialized) {
                fprintf(stderr, "Structure is not initialized\n");
                continue;
            }

            token = strtok(NULL, delim);
            if (token == NULL) {
                fprintf(stderr, "Missing second argument\n");
                continue;
            }

            if (!is_str_num(token)) {
                fprintf(stderr, "Argument is not a number\n");
                continue;
            }

            int index = (int) strtol(token, NULL, 10);

            int result = wc_free(&wc, index);

            if (result != 0) {
                switch (result) {
                    case 1:
                        fprintf(stderr, "Index out of range\n");
                        break;

                    case 2:
                        fprintf(stderr, "No record at given index\n");
                        break;

                    default:
                        fprintf(stderr, "Unknown error\n");
                        break;
                }

                continue;
            }


            fprintf(stdout, "Deleted element at index %d\n", index);
        }
            // COMMAND: destroy
        else if (!strcmp(token, "destroy")) {
            if (!initialized) {
                fprintf(stderr, "Structure not initialized\n");
                continue;
            }

            int result = wc_destroy(&wc);
            if (result == 1) {
                fprintf(stderr, "No data to destroy\n");
                continue;
            }

            initialized = 0;
            fprintf(stdout, "Structure destroyed\n");
        }
            // COMMAND: exit
        else if (!strcmp(token, "exit")) {
            if (initialized) {
                wc_destroy(&wc);
            }
            break;

        }
        // COMMAND: help
        else if (!strcmp(token, "help")) {
            show_help();
            continue;
        } else {
            fprintf(stderr, "Unknown command\n");
            continue;
        }

#ifdef SHOW_TIMES
        // End counting time
        end_clock();
#endif

    }

#ifdef SHOW_TIMES
    // End counting time for the whole program
    en_time = times(&en_cpu);
    printf("========== PROGRAM RUNNING TIME=========\n");
    printf("Real time: %f\n", (double) (en_time - prog_st_time) / tics_per_second);
    printf("User time: %f\n", (double) (en_cpu.tms_utime - prog_st_cpu.tms_utime) / tics_per_second);
    printf("System time: %f\n", (double) (en_cpu.tms_stime - prog_st_cpu.tms_stime) / tics_per_second);
#endif

#ifdef DYNAMIC
    dlclose(handle);
#endif

    return 0;
}