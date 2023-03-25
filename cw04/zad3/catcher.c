#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int request_counter = 0;
int show_time = 0;

/**
 * Print current time
 */
void print_current_time()
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    fprintf(stdout, "Current local time and date: %s", asctime(timeinfo));
}

/**
 * Handler for SIGUSR1
 *
 * @param sig
 * @param info
 * @param context
 */
void handler(int sig, siginfo_t *info, void *context)
{
    fprintf(stdout, "Signal received from %d\n", info->si_pid);

    // Get action
    if (info->si_value.sival_int == 1)
    {
        show_time = 0;

        for (int i = 1; i < 101; i++)
        {
            fprintf(stdout, "%d ", i);
        }
        fprintf(stdout, "\n");
    }
    else if (info->si_value.sival_int == 2)
    {
        show_time = 0;
        print_current_time();
    }
    else if (info->si_value.sival_int == 3)
    {
        show_time = 0;
        fprintf(stdout, "Number of requests: %d\n", request_counter);
    }
    else if (info->si_value.sival_int == 4)
    {
        fprintf(stdout, "Starting to show time...\n");
        show_time = 1;
    }
    else if (info->si_value.sival_int == 5)
    {
        fprintf(stdout, "Exiting...\n");
        kill(info->si_pid, SIGUSR1);
        exit(0);
    }
    else
    {
        fprintf(stdout, "Unknown action! (action id: %d)\n", info->si_value.sival_int);
    }

    request_counter += 1;
    kill(info->si_pid, SIGUSR1);
}

/**
 * Main function
 *
 * @return int
 */
int main()
{
    // Get PID of the process
    fprintf(stdout, "Catcher PID: %d\n", getpid());

    // Set handler for SIGUSR1
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &sa, NULL) < 0)
    {
        perror("Sigaction error");
        exit(1);
    }

    // Wait for signal
    while (1)
    {
        if (show_time)
        {
            print_current_time();
        }
        sleep(1);
    }

    return 0;
}