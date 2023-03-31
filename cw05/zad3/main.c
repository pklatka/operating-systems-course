#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#define PIPE_NAME "/tmp/moja_rura"

double f(double x)
{
    return 4 / (x * x + 1);
}

double a = 0;
double b = 1;

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
void end_clock(FILE *file)
{
    end_t = times(&en_cpu);
    fprintf(file, "Real time: %f\n", (double)(end_t - start_t) / tics_per_second);
    fprintf(file, "User time: %f\n", (double)(en_cpu.tms_utime - st_cpu.tms_utime) / tics_per_second);
    fprintf(file, "System time: %f\n\n", (double)(en_cpu.tms_stime - st_cpu.tms_stime) / tics_per_second);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Not enough arguments: ./main [h] [n]\n");
        return 1;
    }

    double h = (double)strtod(argv[1], (char **)NULL);

    if (h <= 0)
    {
        fprintf(stderr, "Length of a rectangle must be positive and in double range\n");
        return 1;
    }

    int n = (int)strtol(argv[2], (char **)NULL, 10);

    if (n <= 0)
    {
        fprintf(stderr, "Number of processes must be positive\n");
        return 1;
    }

    tics_per_second = sysconf(_SC_CLK_TCK);
    start_clock();

    // Create pipe
    if (mkfifo(PIPE_NAME, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            return 1;
        }
    }
    double sum = 0;
    int fd;
    for (int i = 1; i <= n; i++)
    {
        // Create process
        pid_t pid = fork();

        if (pid == 0)
        {
            int number_of_digits = (int)log10(i) + 1;
            char *i_str = malloc(number_of_digits);
            sprintf(i_str, "%d", i);

            execl("./solver", "./solver", argv[1], argv[2], i_str, NULL);
        }
        else
        {
            fd = open(PIPE_NAME, O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                return 1;
            }
            double result;
            read(fd, &result, sizeof(result));
            sum += result;
        }
    }
    close(fd);

    // =========================================================
    // This solution is working, but can return wrong results.
    // It's because UNIX won't guarantee the atomicity of read operation.
    //
    // int fd = open(PIPE_NAME, O_RDONLY);
    // if (fd < 0)
    // {
    //     perror("open");
    //     return 1;
    // }

    // // Sum all results
    // double sum = 0;
    // for (int i = 0; i < n; i++)
    // {
    //     double result;
    //     read(fd, &result, sizeof(result));
    //     sum += result;
    // }

    // close(fd);
    // =========================================================

    fprintf(stdout, "Result: %lf\n\n", sum);
    end_clock(stdout);

    // Write times to file
    FILE *file = fopen("raport.txt", "a");
    if (file == NULL)
    {
        perror("fopen");
        return 1;
    }

    fprintf(file, "===================\nParameters: h=%g, n=%d\nResult: %lf\n\n", h, n, sum);
    end_clock(file);
    fclose(file);

    return 0;
}
