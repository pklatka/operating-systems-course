#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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

typedef struct
{
    int client_id;
    int client_time;
} client;

int main(void)
{
    char line[LINE_MAX];
    int barber_has_been_used[BARBER_NUMBER] = {0};

    struct sembuf smbuf;
    smbuf.sem_num = 0;
    smbuf.sem_flg = IPC_NOWAIT;
    smbuf.sem_op = -1;

    // Initialize barbers
    key_t barber_key = ftok("main.c", PROJECT_ID);

    if (barber_key < 0)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int barber_semid = semget(barber_key, BARBER_NUMBER, IPC_CREAT | 0666);
    if (barber_semid < 0)
    {
        perror("semget barber");
        printf("%d\n", errno);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < BARBER_NUMBER; i++)
    {
        if (semctl(barber_semid, i, SETVAL, 1) < 0)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize chairs
    key_t chair_key = ftok("main.c", PROJECT_ID + 1);
    int chair_semid = semget(chair_key, CHAIR_NUMBER, IPC_CREAT | 0666);
    for (int i = 0; i < CHAIR_NUMBER; i++)
    {
        if (semctl(chair_semid, i, SETVAL, 1) < 0)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize queue (as pipe but with semaphores)
    int fd[2];
    if (pipe(fd) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    key_t queue_key = ftok("main.c", PROJECT_ID + 2);
    int queue_semid = semget(queue_key, 2, IPC_CREAT | 0666);

    if (semctl(queue_semid, 0, SETVAL, WAITING_ROOM_SIZE) < 0)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    if (semctl(queue_semid, 1, SETVAL, 0) < 0)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

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
                for (int i = 0; i < WAITING_ROOM_SIZE; i++)
                {
                    smbuf.sem_num = i;
                    // If there is a free chair, sit on it
                    if (semop(chair_semid, &smbuf, 1) >= 0)
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
                    smbuf.sem_num = i;

                    if (barber_has_been_used[i] == 0)
                    {
                        all_barbers_are_busy = 0;
                    }

                    // If there is a free barber, wake him up
                    if (barber_has_been_used[i] == 0 && semop(barber_semid, &smbuf, 1) >= 0)
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

            if (fork() == 0)
            {
                // Update queue counter
                smbuf.sem_num = 0;
                smbuf.sem_op = 1;
                semop(queue_semid, &smbuf, 1);

                // Do work for client
                sleep(client_time);

                // Unblock semaphores
                smbuf.sem_num = free_barber;
                smbuf.sem_op = 1;
                semop(barber_semid, &smbuf, 1);
                smbuf.sem_num = free_chair;
                semop(chair_semid, &smbuf, 1);

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
                if (semctl(queue_semid, 0, GETVAL) == 0)
                {
                    fprintf(stdout, "Client %d has left the barbershop\n", client_number);
                    continue;
                }

                smbuf.sem_num = 0;
                smbuf.sem_op = -1;
                semop(queue_semid, &smbuf, 1);

                struct sembuf smbuf2;
                smbuf2.sem_num = 1;
                smbuf2.sem_op = 0;
                smbuf2.sem_flg = 0;

                if (semop(queue_semid, &smbuf2, 1) < 0)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

                smbuf2.sem_op = 1;

                if (semop(queue_semid, &smbuf2, 1) < 0)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

                write(fd[1], &c, sizeof(client));

                smbuf2.sem_op = -1;

                if (semop(queue_semid, &smbuf2, 1) < 0)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    // Close semaphores
    semctl(barber_semid, 0, IPC_RMID);
    semctl(chair_semid, 0, IPC_RMID);
    semctl(queue_semid, 0, IPC_RMID);

    return 0;
}