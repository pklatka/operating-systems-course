#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#define MAX_LINE_SIZE 255

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
    if (argc != 3)
    {
        fprintf(stderr, "Wrong argument number!\n");
        return 1;
    }

    // Check if size of argv[1] is lower than PATH_MAX bytes (from limits.h)
    if (strlen(argv[1]) + 1 > PATH_MAX)
    {
        fprintf(stderr, "First argument is too long!\n");
        return 1;
    }

    // Check if size of argv[2] is lower than 255 bytes
    if (strlen(argv[2]) > MAX_LINE_SIZE)
    {
        fprintf(stderr, "Second argument is too long!\n");
        return 1;
    }

    DIR *dir;

    if (!(dir = opendir(argv[1])))
    {
        perror("Directory argument error");
        return 1;
    }

    struct dirent *entry = malloc(sizeof(struct dirent));
    struct stat *info = malloc(sizeof(struct stat));

    int line_length = strlen(argv[2]);

    while ((entry = readdir(dir)) != NULL)
    {
        // Concatenate path for file
        char *path = malloc(strlen(argv[1]) + strlen(entry->d_name) + 2);
        snprintf(path, strlen(argv[1]) + strlen(entry->d_name) + 2, "%s/%s", argv[1], entry->d_name);

        // Get file info
        int result = stat(path, info);

        if (result == -1)
        {
            fprintf(stderr, "Cannot get file size for %s \n", entry->d_name);
            perror("Reason");
            free(path);
            continue;
        }

        // Check if file is directory
        if (S_ISDIR(info->st_mode))
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                free(path);
                continue;
            }

            int child_pid = fork();

            if (child_pid == 0)
            {
                // exec* function will replace memory of the old process completely with the new program
                int error = execl(argv[0], argv[0], path, argv[2], NULL);
                if (error == -1)
                {
                    fprintf(stderr, "Cannot execute program for path %s \n", path);
                    perror("Reason");
                    continue;
                }
            }
        }
        else
        {
            // Read first MAX_LINE_SIZE bytes from file
            int file = open(path, O_RDONLY);

            if (file == -1)
            {
                fprintf(stderr, "Cannot open file %s\n", path);
                perror("Reason");
                continue;
            }

            char *buffer = malloc(MAX_LINE_SIZE);

            read(file, buffer, MAX_LINE_SIZE);

            if (strncmp(buffer, argv[2], line_length) == 0)
            {
                // fprintf(stdout, "%s %d\n", argv[1], file);
                fprintf(stdout, "%s %ld\n", argv[1], info->st_ino);
            }

            free(buffer);
            close(file);
        }

        free(path);
    }

    free(entry);
    free(info);
    return 0;
}
