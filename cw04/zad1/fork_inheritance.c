#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int signal)
{
    fprintf(stdout, "Received signal %d from %d\n", signal, getpid());
}

int main(int argc, char **argv)
{
    // Array of pointers to strings
    char *allowed_args[] = {"ignore", "handler", "mask", "pending"};

    if (argc != 2)
    {
        fprintf(stderr, "Wrong number of arguments. Usage: ./main [ignore|handler|mask|pending]\n");
        return 1;
    }

    if (strncmp(argv[1], allowed_args[0], 6) == 0)
    {
        // Ignore signal
        signal(SIGUSR1, SIG_IGN); // If the disposition is set to SIG_IGN, then the signal is ignored.

        // Test rasing signal
        pid_t pid = fork();
        if (pid == 0)
        {
            fprintf(stdout, "Raising signal in child %d\n", getpid());
            raise(SIGUSR1);
        }
        else
        {
            fprintf(stdout, "Raising signal in main process %d\n", getpid());
            raise(SIGUSR1);
        }
    }
    else if (strncmp(argv[1], allowed_args[1], 7) == 0)
    {
        // Handle signal
        signal(SIGUSR1, handler);

        // Test rasing signal
        pid_t pid = fork();
        if (pid == 0)
        {
            fprintf(stdout, "Raising signal in child %d\n", getpid());
            raise(SIGUSR1);
        }
        else
        {
            fprintf(stdout, "Raising signal in main process %d\n", getpid());
            raise(SIGUSR1);
        }
    }
    // Check if mask is inherited to child process
    else if (strncmp(argv[1], allowed_args[2], 4) == 0)
    {
        // Mask signal
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        int result = sigprocmask(SIG_BLOCK, &mask, NULL);
        if (result == -1)
        {
            perror("Error while setting mask");
            return 1;
        }

        // Test rasing signal
        fprintf(stdout, "Raising signal in main process %d\n", getpid());
        raise(SIGUSR1);

        sigpending(&mask);
        if (sigismember(&mask, SIGUSR1))
        {
            printf("Signal is pending, %d\n", getpid());
        }
        else
        {
            printf("Signal is not pending, %d\n", getpid());
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            fprintf(stdout, "Raising signal in child %d\n", getpid());
            raise(SIGUSR1);

            sigpending(&mask);
            if (sigismember(&mask, SIGUSR1))
            {
                printf("Signal is pending, %d\n", getpid());
            }
            else
            {
                printf("Signal is not pending, %d\n", getpid());
            }
        }
    }
    else if (strncmp(argv[1], allowed_args[3], 7) == 0)
    {
        // Mask signal
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        int result = sigprocmask(SIG_BLOCK, &mask, NULL);
        if (result == -1)
        {
            perror("Error while setting mask");
            return 1;
        }

        // Test rasing signal
        fprintf(stdout, "Raising signal in main process %d\n", getpid());
        raise(SIGUSR1);

        sigpending(&mask);
        if (sigismember(&mask, SIGUSR1))
        {
            printf("Signal is pending, %d\n", getpid());
        }
        else
        {
            printf("Signal is not pending, %d\n", getpid());
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            sigpending(&mask);
            if (sigismember(&mask, SIGUSR1))
            {
                printf("Signal is pending, %d\n", getpid());
            }
            else
            {
                printf("Signal is not pending, %d\n", getpid());
            }
        }
    }

    wait(NULL);
    return 0;
}