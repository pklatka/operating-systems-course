#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/moja_rura"

double f(double x)
{
    return 4 / (x * x + 1);
}

double a = 0;
double b = 1;

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "%d\n", argc);

        fprintf(stderr, "Not enough arguments\n");
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

    int i = (int)strtol(argv[3], (char **)NULL, 10);

    if (i < 0)
    {
        fprintf(stderr, "Number of process must be positive\n");
        return 1;
    }

    // Child
    double length = (b - a) / n;
    double a = length * (i - 1);
    double b = length * i;
    double sum = 0;

    for (double x = a; x < b; x += h)
    {
        sum += f(x) * h;
    }

    // Write to pipe
    int fd = open(PIPE_NAME, O_WRONLY);
    write(fd, &sum, sizeof(sum));
    close(fd);

    return 0;
}