#include "grid.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define THREADS 20
const int grid_width = 30;
const int grid_height = 30;
char *foreground;
char *background;
pthread_t threads[THREADS + 1];

char *create_grid()
{
    return malloc(sizeof(char) * grid_width * grid_height);
}

void destroy_grid(char *grid)
{
    free(grid);
}

void draw_grid(char *grid)
{
    for (int i = 0; i < grid_height; ++i)
    {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j)
        {
            if (grid[i * grid_width + j])
            {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            }
            else
            {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }

    refresh();
}

void init_grid(char *grid)
{
    for (int i = 0; i < grid_width * grid_height; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid)
{

    int count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
            {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width)
            {
                continue;
            }
            if (grid[grid_width * r + c])
            {
                count++;
            }
        }
    }

    if (grid[row * grid_width + col])
    {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    }
    else
    {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst)
{
    for (int i = 0; i < grid_height; ++i)
    {
        for (int j = 0; j < grid_width; ++j)
        {
            dst[i * grid_width + j] = is_alive(i, j, src);
        }
    }
}

// ====================
typedef struct thread_args
{
    int left_boundary;
    int right_boundary;
    char *src;
    char *dst;
} thread_args;

void *thread_func(void *_args)
{
    thread_args *args = _args;

    while (1)
    {

        for (int i = args->left_boundary; i < args->right_boundary; i++)
        {

            background[i] = is_alive(i / grid_width, i % grid_width, foreground);
        }

        pause();
    }

    free(args);
    return NULL;
}

void catcher(int signum) {}

void init_threads(char *src, char *dst)
{
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = catcher;
    sigaction(SIGALRM, &sigact, NULL);

    int points_per_thread = grid_height * grid_width / THREADS;
    foreground = src;
    background = dst;

    int left_bound = 0;
    for (int i = 0; i < THREADS; i++)
    {
        thread_args *args = malloc(sizeof(thread_args));
        args->src = src;
        args->dst = dst;
        args->left_boundary = left_bound;

        if (left_bound + points_per_thread > grid_height * grid_width)
        {
            args->right_boundary = grid_height * grid_width;
        }
        else
        {
            args->right_boundary = left_bound + points_per_thread;
        }

        left_bound += points_per_thread;

        pthread_create(&threads[i], NULL, thread_func, args);
    }

    if (left_bound < grid_height * grid_width)
    {
        thread_args *args = malloc(sizeof(thread_args));
        args->src = src;
        args->dst = dst;
        args->left_boundary = left_bound;
        args->right_boundary = grid_height * grid_width;
        pthread_create(&threads[THREADS], NULL, thread_func, args);
    }
}

void update_grid_threaded(char *src, char *dst)
{
    foreground = src;
    background = dst;

    for (int i = 0; i < THREADS; i++)
    {
        pthread_kill(threads[i], SIGALRM);
    }
}