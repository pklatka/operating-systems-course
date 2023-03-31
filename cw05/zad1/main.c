#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    FILE *file;

    if (argc == 2)
    {
        char *command, line[LINE_MAX];
        if (strncmp(argv[1], "nadawca", 7) == 0)
        {
            command = "echo | mail -H | sort -k 2";
        }
        else if (strncmp(argv[1], "data", 4) == 0)
        {
            command = "echo | mail -H | sort -k 3 --reverse";
        }
        else
        {
            fprintf(stderr, "Wrong argument\n");
            return 1;
        }

        if ((file = popen(command, "r")) == NULL)
        {
            fprintf(stderr, "Error while opening pipe\n");
            return 1;
        }

        while (fgets(line, LINE_MAX, file) != NULL)
        {
            fprintf(stdout, "%s", line);
        }

        pclose(file);
    }
    else if (argc == 4)
    {
        char command[LINE_MAX], line[LINE_MAX];
        sprintf(command, "echo %s | mail -s %s %s", argv[3], argv[2], argv[1]);

        if ((file = popen(command, "r")) == NULL)
        {
            fprintf(stderr, "Error while opening pipe\n");
            return 1;
        }

        while (fgets(line, LINE_MAX, file) != NULL)
        {
            fprintf(stdout, "%s", line);
        }

        fprintf(stdout, "Message sent\n");
        pclose(file);
    }
    else
    {
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "./main [nadawca|data]\n");
        fprintf(stderr, "./main [recipient] [title] [message]\n");
        return 1;
    }

    return 0;
}