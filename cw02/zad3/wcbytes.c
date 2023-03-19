#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *entry = malloc(sizeof(struct dirent));
    struct stat *info = malloc(sizeof(struct stat));
    char *path = ".";

    if (!(dir = opendir(path)))
    {
        fprintf(stderr, "Cannot open directory %s \n", path);

        free(entry);
        free(info);

        return 1;
    }

    long long total_size = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        // Get file size
        int result = stat(entry->d_name, info);
        if (result == -1)
        {
            fprintf(stderr, "Cannot get file size for %s \n", entry->d_name);
            continue;
        }

        if (S_ISDIR(info->st_mode))
            continue; // Skip directories

        // Add file size to total
        total_size += info->st_size;

        // Print file name and size
        fprintf(stdout, "%5ld %s\n", info->st_size, entry->d_name);
    }

    // Print total size
    fprintf(stdout, "%5lld total\n", total_size);
    closedir(dir);

    free(entry);
    free(info);

    return 0;
}
