#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHOW_TIMES 1
#define BLOCK_SIZE 1024

int tics_per_second;
clock_t start_t, end_t;
struct tms st_cpu, en_cpu;

/**
 * Start counting time
 *
 * @return void
 */
void start_clock()
{
    start_t = times(&st_cpu);
}

/**
 * Stop counting time and print results
 *
 * @return void
 */
void end_clock()
{
#ifdef SHOW_TIMES
    end_t = times(&en_cpu);
    fprintf(stdout, "============= TIMES ===============\n");
    fprintf(stdout, "Real time: %f\n", (double)(end_t - start_t) / tics_per_second);
    fprintf(stdout, "User time: %f\n", (double)(en_cpu.tms_utime - st_cpu.tms_utime) / tics_per_second);
    fprintf(stdout, "System time: %f\n\n", (double)(en_cpu.tms_stime - st_cpu.tms_stime) / tics_per_second);
#endif
}

/**
 * Main function
 *
 * @param argc Number of arguments
 * @param argv Arguments
 * @return int
 */
int main(int argc, char *argv[])
{
    tics_per_second = sysconf(_SC_CLK_TCK);

    // Check number of arguments
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    start_clock();

    // Open first file (read)
    FILE *file1 = fopen(argv[1], "r");

    if (file1 == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[3]);
        return 1;
    }

    // Open second file (write)
    FILE *file2 = fopen(argv[2], "w");

    if (file2 == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[4]);
        return 1;
    }

    // Get length of file1
    fseek(file1, 0, SEEK_END);
    int length = ftell(file1);
    fseek(file1, -1, SEEK_CUR); // Skip EOF

    // Read file1 by one char reversed and save it to file2
#if READ_CHAR_BY_CHAR == 1
    fprintf(stdout, "Reading by one char...\n");
    while (length > 0)
    {
        char c;
        fread(&c, 1, 1, file1);
        fwrite(&c, 1, 1, file2);
        fseek(file1, -2, SEEK_CUR); // fread moved file pointer by 1, so we need to move it back by 2
        length -= 1;
    }
#else
    fprintf(stdout, "Reading by blocks...\n");
    char *c = (char *)malloc(BLOCK_SIZE * sizeof(char));
    fseek(file1, -BLOCK_SIZE + 1, SEEK_CUR); // Add 1 to read char in which file pointer is now

    while (length > 0)
    {
        int read;
        // Move pointer backward
        if (length < BLOCK_SIZE)
        {
            fseek(file1, 0, SEEK_SET);
            read = fread(c, 1, length, file1);
        }
        else
        {
            read = fread(c, 1, BLOCK_SIZE, file1);
        }

        // Reverse c
        for (int i = 0; i < read / 2; i++)
        {
            char tmp = c[i];
            c[i] = c[read - i - 1];
            c[read - i - 1] = tmp;
        }

        fwrite(c, 1, read, file2);
        fseek(file1, -2 * read, SEEK_CUR);
        length -= read;
    }

    free(c);
#endif

    end_clock();
}