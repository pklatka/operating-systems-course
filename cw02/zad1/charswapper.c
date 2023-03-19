#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHOW_TIMES 1

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
    if (argc != 5)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    // Check if arguments are correct
    for (int i = 1; i < argc; i++)
    {
        if (i == 1 || i == 2)
        {
            // Check if argument is a ascii char
            if (strlen(argv[i]) != 1)
            {
                fprintf(stderr, "Argument %d is not a ascii char\n", i);
                return 1;
            }
        }
    }

    char char1 = argv[1][0];
    char char2 = argv[2][0];

    start_clock();

#if USE_FILE_SYSTEM_FUNCTIONS == 1
    fprintf(stdout, "Using functions from standard library\n");

    // Open first file (read)
    FILE *file1 = fopen(argv[3], "r");

    if (file1 == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[3]);
        return 1;
    }

    // Open second file (write)
    FILE *file2 = fopen(argv[4], "w");

    if (file2 == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[4]);
        return 1;
    }

    // Get length of first file
    fseek(file1, 0, SEEK_END);
    long file1_length = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    // Allocate memory for file1
    char *c = malloc(file1_length * sizeof(char));

    // Read file1 to memory
    fread(c, sizeof(char), file1_length, file1);

    // Swap chars
    for (int i = 0; i < file1_length; i++)
    {
        if (c[i] == char1)
        {
            c[i] = char2;
        }
    }

    // Write to file2
    fwrite(c, sizeof(char), file1_length, file2);
    free(c);

    end_clock();
#else
    fprintf(stdout, "Using system functions\n");

    // Open first file (read)
    int file1 = open(argv[3], O_RDONLY);

    if (file1 == -1)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[3]);
        return 1;
    }

    // Open second file (write)
    int file2 = open(argv[4], O_WRONLY | O_CREAT, 0666); // 0666 (octal) - read and write for everyone

    if (file2 == -1)
    {
        fprintf(stderr, "Cannot open file %s\n", argv[4]);
        return 1;
    }

    // Get length of first file
    long file1_length = lseek(file1, 0, SEEK_END);
    lseek(file1, 0, SEEK_SET);

    // Allocate memory for file1
    char *c = malloc(file1_length * sizeof(char));

    // Read file1 to memory
    read(file1, c, file1_length);

    // Swap chars
    for (int i = 0; i < file1_length; i++)
    {
        if (c[i] == char1)
        {
            c[i] = char2;
        }
    }

    // Write to file2
    write(file2, c, file1_length);
    free(c);

    end_clock();
#endif
}