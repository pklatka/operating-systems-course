#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PROJECT_ID 1
#define BARBER_NUMBER 5
#define CHAIR_NUMBER 3
#define WAITING_ROOM_SIZE 5

long client_id = 0;
sem_t *barber_semaphores[BARBER_NUMBER];
sem_t *chair_semaphores[CHAIR_NUMBER];
sem_t *waiting_room_semaphore;
sem_t *queue_semaphore;

typedef struct
{
    int client_id;
    int client_time;
} client;

int main(void)
{
    int barber_has_been_used[BARBER_NUMBER] = {0};

    // Initialize barbers
    for (int i = 0; i < BARBER_NUMBER; i++)
    {
        char name[LINE_MAX];
        sprintf(name, "/barber%d", i);
        if ((barber_semaphores[i] = sem_open(name, O_CREAT, 0666, 1)) == SEM_FAILED)
        {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize chairs
    for (int i = 0; i < CHAIR_NUMBER; i++)
    {
        char name[LINE_MAX];
        sprintf(name, "/chair%d", i);
        if ((chair_semaphores[i] = sem_open(name, O_CREAT, 0666, 1)) == SEM_FAILED)
        {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize waiting room
    if ((waiting_room_semaphore = sem_open("/waiting-room", O_CREAT, 0666, WAITING_ROOM_SIZE)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Initialize queue (as pipe but with semaphores)
    int fd[2];
    if (pipe(fd) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((queue_semaphore = sem_open("/queue", O_CREAT, 0666, 0)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    char line[LINE_MAX];

    pid_t child_pid = fork();

    if (child_pid == 0)
    {
        close(fd[1]);
        // Handle clients
        while (1)
        {
            // In production, there is one "dispatcher" process which handles clients
            // That's why we don't need to block semaphore

            // Get client's number and time
            client client;
            if (read(fd[0], &client, sizeof(client)) < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            int client_number = client.client_id;
            int client_time = client.client_time;

            // Check if there is a free chair
            int free_chair = -1;
            while (free_chair == -1)
            {
                for (int i = 0; i < CHAIR_NUMBER; i++)
                {
                    // If there is a free chair, sit on it
                    if (sem_trywait(chair_semaphores[i]) >= 0)
                    {
                        free_chair = i;
                        break;
                    }
                }
            }

            // If there is a free chair, get free barber
            int free_barber = -1;
            while (free_barber == -1)
            {
                int all_barbers_are_busy = 1;

                for (int i = 0; i < BARBER_NUMBER; i++)
                {
                    if (barber_has_been_used[i] == 0)
                    {
                        all_barbers_are_busy = 0;
                    }
                    // If there is a free barber, sit on it
                    if (barber_has_been_used[i] == 0 && sem_trywait(barber_semaphores[i]) >= 0)
                    {
                        barber_has_been_used[i] = 1;
                        free_barber = i;
                        break;
                    }
                }

                if (free_barber == -1 && all_barbers_are_busy)
                {
                    // Reset all barbers
                    for (int i = 0; i < BARBER_NUMBER; i++)
                    {
                        barber_has_been_used[i] = 0;
                    }
                }
            }

            // printf("Client %d has sat on chair %d and is handled by barber %d\n", client_number, free_chair, free_barber);

            if (fork() == 0)
            {
                // Update queue counter
                if (sem_post(waiting_room_semaphore) < 0)
                {
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }

                // Do work for client
                sleep(client_time);

                // Unblock semaphores

                // Free barber
                if (sem_post(barber_semaphores[free_barber]) < 0)
                {
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }

                // Free chair
                if (sem_post(chair_semaphores[free_chair]) < 0)
                {
                    perror("sem_post");
                    exit(EXIT_FAILURE);
                }

                fprintf(stdout, "Client %d has been served by barber %d\n", client_number, free_barber);
                return 0;
            }
        }
    }
    else
    {
        close(fd[0]);
        while (fgets(line, LINE_MAX, stdin) != NULL)
        {
            if (strncmp(line, "END", 3) == 0)
            {
                kill(child_pid, SIGKILL);
                break;
            }
            else
            {
                int client_number = client_id++;
                int client_time = atoi(line);

                client c = {client_number, client_time};

                // Check if queue is full
                if (sem_trywait(waiting_room_semaphore) < 0)
                {
                    fprintf(stdout, "Client %d has left the barbershop\n", client_number);
                    continue;
                }

                write(fd[1], &c, sizeof(client));
            }
        }
    }

    // Close semaphores
    for (int i = 0; i < BARBER_NUMBER; i++)
    {
        if (sem_close(barber_semaphores[i]) == -1)
        {
            perror("sem_close");
            exit(EXIT_FAILURE);
        }

        char name[LINE_MAX];
        sprintf(name, "/barber%d", i);

        if (sem_unlink(name) == -1)
        {
            perror("sem_unlink");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < CHAIR_NUMBER; i++)
    {
        if (sem_close(chair_semaphores[i]) == -1)
        {
            perror("sem_close");
            exit(EXIT_FAILURE);
        }

        char name[LINE_MAX];
        sprintf(name, "/chair%d", i);

        if (sem_unlink(name) == -1)
        {
            perror("sem_unlink");
            exit(EXIT_FAILURE);
        }
    }

    if (sem_close(waiting_room_semaphore) == -1)
    {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/waiting-room") == -1)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_close(queue_semaphore) == -1)
    {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/queue") == -1)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}