#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int request_counter = 0;
/**
 * Check if string is a number
 *
 * @param str
 * @return int - 1 if string is a number, 0 otherwise
 */
int is_str_num(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * Handler for SIGUSR1
 *
 * @param sig
 */
void handler(int sig)
{
    fprintf(stdout, "Signal received. Total no. signals received: %d\n", ++request_counter);
}

/**
 * Main function
 *
 * @param argc
 * @param argv
 *
 * @return int
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pid> [...args]\n", argv[0]);
        return 1;
    }

    // Check if arguments are numbers
    for (int i = 1; i < argc; i++)
    {
        if (!is_str_num(argv[i]))
        {
            fprintf(stderr, "Argument is not a number!\n");
            return 1;
        }
    }

    sigset_t blocking_mask;
    sigset_t allowing_mask;
    struct sigaction sa;
    union sigval value;

    // Set blocking mask for SIGUSR1
    sigemptyset(&blocking_mask);
    sigaddset(&blocking_mask, SIGUSR1);

    // Set allowing mask
    sigemptyset(&allowing_mask);

    // Set handler for SIGUSR1
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sa, NULL) < 0)
    {
        perror("Sigaction error");
        return 1;
    }

    int pid = (int)strtol(argv[1], (char **)NULL, 10);
    int queue = 2;

    // Block SIGUSR1 to make sure that signal will be put into pending state
    if (sigprocmask(SIG_BLOCK, &blocking_mask, NULL) < 0)
    {
        perror("Sigprocmask error");
        return 1;
    }

    while (queue < argc)
    {
        value.sival_int = (int)strtol(argv[queue], (char **)NULL, 10);
        sigqueue(pid, SIGUSR1, value); // Send signal

        // This code will change mask to accept SIGUSR1, especially when it is pending
        sigsuspend(&allowing_mask); // Wait for confirmation

        queue += 1;
    }

    // Unblock SIGUSR1
    if (sigprocmask(SIG_UNBLOCK, &blocking_mask, NULL) < 0)
    {
        perror("Sigprocmask error");
        return 1;
    }

    return 0;
}
