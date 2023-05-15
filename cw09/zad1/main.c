#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

#define ELVES_NUMBER 10
#define REINDEERS_NUMBER 9
#define REQUIRED_ELVES_NUMBER 3
#define DELIVERIES_NUMBER 3

pthread_mutex_t elves_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
int elves_counter = 0;
pthread_mutex_t elves_wakeup_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t elves_wakeup_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t reindeers_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
int reindeers_counter = 0;
pthread_mutex_t reindeers_wakeup_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reindeers_wakeup_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
bool is_santa_sleeping = true;
pthread_cond_t santa_sleep_cond = PTHREAD_COND_INITIALIZER;

bool is_santa_dead = false;

int elves_ids[ELVES_NUMBER] = {0};

pthread_t elves[ELVES_NUMBER];
pthread_t reindeers[REINDEERS_NUMBER];
pthread_t santa_thread;

int random_number(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void *reindeer(void *arg)
{
    int id = *(int *)arg;

    while (true)
    {
        if (is_santa_dead)
            pthread_exit((void *)0);

        // Work
        sleep(random_number(5, 10));

        // Go to santa, else solve problem alone
        pthread_mutex_lock(&reindeers_counter_mutex);

        if (reindeers_counter == REINDEERS_NUMBER - 1)
        {
            printf("Renifer: czeka %d reniferow na Mikołaja, %d\n", reindeers_counter + 1, id);
            pthread_mutex_unlock(&reindeers_counter_mutex);

            pthread_mutex_lock(&santa_mutex);

            // Update counter when santa is sleeping
            pthread_mutex_lock(&reindeers_counter_mutex);
            reindeers_counter++;
            pthread_mutex_unlock(&reindeers_counter_mutex);

            printf("Renifer: wybudzam Mikołaja, %d\n", id);
            is_santa_sleeping = false;
            pthread_cond_broadcast(&santa_sleep_cond);
            pthread_mutex_unlock(&santa_mutex);
        }
        else
        {
            reindeers_counter++;
            printf("Renifer: czeka %d reniferow na Mikołaja, %d\n", reindeers_counter, id);
            pthread_mutex_unlock(&reindeers_counter_mutex);
        }

        pthread_mutex_lock(&reindeers_wakeup_mutex);
        pthread_cond_wait(&reindeers_wakeup_cond, &reindeers_wakeup_mutex);
        pthread_mutex_unlock(&reindeers_wakeup_mutex);
    }
}

void *elf(void *arg)
{
    int id = *(int *)arg;

    while (true)
    {
        if (is_santa_dead)
            pthread_exit((void *)0);

        // Work
        sleep(random_number(2, 5));

        // Go to santa, else solve problem alone
        pthread_mutex_lock(&elves_counter_mutex);

        if (elves_counter >= REQUIRED_ELVES_NUMBER)
        {
            printf("Elf: samodzielnie rozwiązuję swój problem, %d\n", id);
            pthread_mutex_unlock(&elves_counter_mutex);
        }
        else if (elves_counter == REQUIRED_ELVES_NUMBER - 1)
        {
            elves_ids[elves_counter] = id;
            elves_counter++;
            printf("Elf: czeka %d elfow na Mikołaja, %d\n", elves_counter, id);
            pthread_mutex_unlock(&elves_counter_mutex);

            pthread_mutex_lock(&santa_mutex);
            printf("Elf: wybudzam Mikołaja, %d\n", id);

            is_santa_sleeping = false;
            pthread_cond_broadcast(&santa_sleep_cond);
            pthread_mutex_unlock(&santa_mutex);

            pthread_mutex_lock(&elves_wakeup_mutex);
            pthread_cond_wait(&elves_wakeup_cond, &elves_wakeup_mutex);
            pthread_mutex_unlock(&elves_wakeup_mutex);
        }
        else
        {
            elves_ids[elves_counter] = id;
            elves_counter++;
            printf("Elf: czeka %d elfow na Mikołaja, %d\n", elves_counter, id);
            pthread_mutex_unlock(&elves_counter_mutex);

            pthread_mutex_lock(&elves_wakeup_mutex);
            pthread_cond_wait(&elves_wakeup_cond, &elves_wakeup_mutex);
            pthread_mutex_unlock(&elves_wakeup_mutex);
        }
    }
}

void *santa(void *arg)
{
    int deliveries = 0;

    while (true)
    {
        pthread_mutex_lock(&santa_mutex);
        while (is_santa_sleeping)
        {
            pthread_cond_wait(&santa_sleep_cond, &santa_mutex);
        }

        printf("Mikołaj: budzę się\n");

        pthread_mutex_lock(&elves_counter_mutex);
        if (elves_counter == REQUIRED_ELVES_NUMBER)
        {
            printf("Mikołaj: rozwiązuje problemy elfów %d %d %d\n", elves_ids[0], elves_ids[1], elves_ids[2]);
            sleep(random_number(1, 2));
            elves_counter = 0;
            printf("Mikołaj: rozwiązałem problemy elfów\n");
            pthread_mutex_unlock(&elves_counter_mutex);

            pthread_mutex_lock(&elves_wakeup_mutex);
            pthread_cond_broadcast(&elves_wakeup_cond);
            pthread_mutex_unlock(&elves_wakeup_mutex);
        }
        else
        {
            pthread_mutex_unlock(&elves_counter_mutex);
        }

        pthread_mutex_lock(&reindeers_counter_mutex);
        if (reindeers_counter == REINDEERS_NUMBER)
        {
            printf("Mikołaj: dostarczam zabawki\n");
            sleep(random_number(2, 4));
            deliveries++;
            reindeers_counter = 0;
            printf("Mikołaj: dostarczyłem zabawki\n");
            pthread_mutex_unlock(&reindeers_counter_mutex);

            pthread_mutex_lock(&reindeers_wakeup_mutex);
            pthread_cond_broadcast(&reindeers_wakeup_cond);
            pthread_mutex_unlock(&reindeers_wakeup_mutex);
        }
        else
        {
            pthread_mutex_unlock(&reindeers_counter_mutex);
        }

        is_santa_sleeping = true;
        pthread_mutex_unlock(&santa_mutex);

        if (deliveries == DELIVERIES_NUMBER)
        {
            is_santa_dead = true;

            printf("==========KONIEC==========\n");

            for (int i = 0; i < ELVES_NUMBER; i++)
            {
                pthread_kill(elves[i], SIGKILL);
            }

            for (int i = 0; i < REINDEERS_NUMBER; i++)
            {
                pthread_kill(reindeers[i], SIGKILL);
            }

            pthread_mutex_destroy(&santa_mutex);
            pthread_mutex_destroy(&elves_counter_mutex);
            pthread_mutex_destroy(&reindeers_counter_mutex);
            pthread_cond_destroy(&elves_wakeup_cond);
            pthread_cond_destroy(&reindeers_wakeup_cond);
            pthread_cond_destroy(&reindeers_wakeup_cond);

            exit(0);
        }

        printf("Mikołaj: idę spać, żegnam\n");
    }
}

int main()
{
    srand(time(NULL));

    pthread_create(&santa_thread, NULL, santa, NULL);

    for (int i = 0; i < ELVES_NUMBER; i++)
    {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&elves[i], NULL, elf, (void *)id);
    }

    for (int i = 0; i < REINDEERS_NUMBER; i++)
    {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&reindeers[i], NULL, reindeer, (void *)id);
    }

    pthread_join(santa_thread, NULL);

    return 0;
}