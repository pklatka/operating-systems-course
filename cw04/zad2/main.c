#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void siginfo_handler(int signum, siginfo_t *info, void *ucontext)
{
    fprintf(stdout, "Signal number: %d\n", signum);
    fprintf(stdout, "Sending process ID: %d\n", info->si_pid);
    fprintf(stdout, "Real user ID of sending process: %d\n", info->si_uid);
    fprintf(stdout, "System time consumed: %ld\n", info->si_stime);
    fprintf(stdout, "User time consumed: %ld\n----------------------\n", info->si_utime);
}

void test_siginfo(int skip_no_sigaction)
{
    fprintf(stdout, "-----Testing SA_SIGINFO-----\n");
    struct sigaction sig;
    sig.sa_sigaction = siginfo_handler;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);

    if (!skip_no_sigaction)
    {

        if (sigaction(SIGINT, &sig, NULL) < 0)
        {
            perror("Sigaction error");
            exit(1);
        }

        fprintf(stdout, "Sending SIGINT to %d\n", getpid());
        fprintf(stdout, "WARNING: this can throw segfault. If it does, change method flag.\n");
        raise(SIGINT);
    }

    fprintf(stdout, "Sending SIGINT to %d after setting SA_SIGINFO\n", getpid());
    sig.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &sig, NULL) < 0)
    {
        perror("Sigaction error");
        exit(1);
    }

    raise(SIGINT);
}

void resethand_handler(int signum, siginfo_t *info, void *ucontext)
{
    fprintf(stdout, "I'm a resethand handler!\n");
}

void test_resethand()
{
    fprintf(stdout, "-----Testing SA_RESETHAND-----\n");

    struct sigaction sig;
    sig.sa_sigaction = resethand_handler;
    sig.sa_flags = SA_RESETHAND;
    sigemptyset(&sig.sa_mask);
    if (sigaction(SIGURG, &sig, NULL) < 0)
    {
        perror("Sigaction error");
        exit(1);
    }

    fprintf(stdout, "Expecting handler execution\n");
    raise(SIGURG);

    fprintf(stdout, "Expecting SIGURG default action (ignored)\n");
    raise(SIGURG);
}

int call_depth = 0;

void nodefer_handler(int sig, siginfo_t *info, void *ucontext)
{
    printf("Start call depth: %d\n", call_depth);

    call_depth++;
    if (call_depth < 10)
    {
        raise(SIGUSR1);
    }
    call_depth--;

    printf("End call depth: %d\n", call_depth);
}

void test_nodefer()
{
    fprintf(stdout, "-----Testing SA_NODEFER-----\n");
    struct sigaction sig;
    sig.sa_sigaction = nodefer_handler;
    sig.sa_flags = SA_NODEFER;
    sigemptyset(&sig.sa_mask);
    if (sigaction(SIGUSR1, &sig, NULL) < 0)
    {
        perror("Sigaction error");
        exit(1);
    }

    raise(SIGUSR1);
}

int main(int argc, char **argv)
{

    test_siginfo(0);
    // test_siginfo(1); // Uncomment if above function throws segfault

    test_nodefer();

    test_resethand();

    return 0;
}