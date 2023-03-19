#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

    if (strlen(argv[0]) > 1)
    {
        // Skip "./"
        printf("%s  ", argv[0] + 2);
    }
    else
    {
        printf("%s  ", argv[0]);
    }

    fflush(stdout); // flush stdout

    execl("/bin/ls", "ls", argv[1], NULL);
    return 0;
}
