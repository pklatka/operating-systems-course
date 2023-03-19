#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ftw.h>

#define MAX_DESCRIPTORS 20

long long total_size = 0;

int display_info(const char *fpath, const struct stat *sb, int tflag)
{
    if (!S_ISDIR(sb->st_mode))
    {
        fprintf(stdout, "%5ld %s\n", sb->st_size, fpath);
        total_size += sb->st_size;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Wrong number of arguments. Usage: %s <path>\n", argv[0]);
        return 1;
    }

    if (ftw(argv[1], display_info, MAX_DESCRIPTORS) == -1)
    {
        perror("ftw()");
        return 1;
    }

    fprintf(stdout, "%5lld total\n", total_size);
    return 0;
}
