#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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
 * Main function
 *
 * @param argc
 * @param argv
 *
 * @return int
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Wrong argument number!\n");
        return 1;
    }

    if (!is_str_num(argv[1]))
    {
        printf("Argument is not a number!\n");
        return 1;
    }

    int process_number = (int)strtol(argv[1], (char **)NULL, 10);
    int child_pid = 1;

    for (int i = 0; i < process_number; i++)
    {
        child_pid = fork();

        if (child_pid == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (child_pid == 0)
        {
            // Child process
            fprintf(stdout, "Hi, I'm child %d, my parent is %d\n", getpid(), getppid());
            break;
        }
        else
        {
            // Parent process
            wait(NULL);
        }
    }

    if (child_pid > 0)
        fprintf(stdout, "%s\n", argv[1]);

    return 0;
}
