#include "common.h"

struct pollfd fds[2 + MAX_CLIENTS];
char *port;
char *path;
char *clients[MAX_CLIENTS];

pthread_t ping_thread;
pthread_mutex_t fds_mutex = PTHREAD_MUTEX_INITIALIZER;

int init_network_socket(char *port)
{
    int network_socket;
    struct sockaddr_in server_address;

    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket == -1)
    {
        perror("socket");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port));
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(network_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(network_socket, MAX_BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    return network_socket;
}

int init_local_socket(char *path)
{
    if (strlen(path) > UNIX_PATH_MAX)
    {
        printf("Path is too long.\n");
        exit(1);
    }

    int local_socket;
    struct sockaddr_un server_address;

    local_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_socket == -1)
    {
        perror("socket");
        exit(1);
    }

    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, path);

    if (bind(local_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(local_socket, MAX_BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    return local_socket;
}

void kill_server(int signo)
{
    char *msg = "STOPServer has been stopped.";
    // Close all connections
    pthread_mutex_lock(&fds_mutex);
    for (int i = 2; i < 2 + MAX_CLIENTS; i++)
    {
        if (fds[i].fd != -1 && send(fds[i].fd, msg, strlen(msg) + 1, 0) == -1)
        {
            perror("send");
        }
    }

    shutdown(fds[0].fd, SHUT_RDWR);
    close(fds[0].fd);
    shutdown(fds[1].fd, SHUT_RDWR);
    close(fds[1].fd);

    // Remove local socket
    unlink(path);

    pthread_mutex_unlock(&fds_mutex);
    fprintf(stdout, "\nServer is shutting down...\n");
    exit(0);
}

void *ping_clients()
{
    while (1)
    {
        sleep(PING_TIMEOUT);
        char *msg = "PING";

        pthread_mutex_lock(&fds_mutex);
        for (int i = 2; i < 2 + MAX_CLIENTS; i++)
        {
            // Check if connection still exists
            if (fds[i].fd != -1)
            {
                // Check if fd is not broken
                if (send(fds[i].fd, msg, strlen(msg) + 1, MSG_NOSIGNAL) == -1)
                {
                    // Client disconnected
                    fprintf(stdout, "Client %s disconnected\n", clients[i - 2]);
                    fds[i].fd = -1;
                    clients[i - 2] = NULL;
                    continue;
                }

                // Receive response
                char buffer[LINE_MAX];
                int res = recv(fds[i].fd, buffer, sizeof(buffer), MSG_DONTWAIT);

                if (res == 0 || (res == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    // Client disconnected
                    fprintf(stdout, "Client %s disconnected\n", clients[i - 2]);
                    fds[i].fd = -1;
                    clients[i - 2] = NULL;
                }
            }
        }
        pthread_mutex_unlock(&fds_mutex);
    }
}

void handle_message(char *buffer, int client_id)
{
    // Get current date and time as string
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time_str[20];
    sprintf(time_str, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Save request to log file
    FILE *log_file = fopen("serverlog.txt", "a");
    fprintf(log_file, "%s Client: %s (id: %d) Request: %s\n", time_str, clients[client_id], client_id, buffer);
    fclose(log_file);

    if (strncmp(buffer, "PONG", 4) == 0)
    {
        return;
    }
    else if (strncmp(buffer, "LIST", 4) == 0)
    {
        // Print all clients and save the list to buffer
        char msg[LINE_MAX];
        sprintf(msg, "LISTClient list:");
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] != NULL)
            {
                sprintf(msg + strlen(msg), "\n%s", clients[i]);
            }
        }

        // Send list to client
        if (send(fds[client_id + 2].fd, msg, strlen(msg) + 1, 0) == -1)
        {
            perror("send");
        }

        // Print list to server
        fprintf(stdout, "%s\n", msg + 4);
    }
    else if (strncmp(buffer, "2ALL", 4) == 0)
    {
        char msg[LINE_MAX + 100];
        sprintf(msg, "2ALLMessage from %s: %s\nTime: %s", clients[client_id], buffer + 4, time_str);

        // Send message to all clients
        for (int i = 2; i < 2 + MAX_CLIENTS; i++)
        {
            if (fds[i].fd != -1 && i - 2 != client_id)
            {
                if (send(fds[i].fd, msg, strlen(msg) + 1, 0) == -1)
                {
                    perror("send");
                }
            }
        }
    }
    else if (strncmp(buffer, "2ONE", 4) == 0)
    {
        char *name = strtok(buffer + 4, " ");
        char *msg = strtok(NULL, "\n");

        char message_to_send[LINE_MAX + 100];
        sprintf(message_to_send, "2ONEMessage from %s: %s\nTime: %s", clients[client_id], msg, time_str);

        // Send message to specified client

        int found = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] != NULL && strcmp(clients[i], name) == 0)
            {
                if (send(fds[i + 2].fd, message_to_send, strlen(message_to_send) + 1, 0) == -1)
                {
                    perror("send");
                }
                found = 1;
                break;
            }
        }

        if (!found)
        {
            char *msg = "ERRORClient not found";
            send(fds[client_id + 2].fd, msg, strlen(msg) + 1, 0);
            fprintf(stdout, "Client %s not found\n", name);
        }
    }
    else if (strncmp(buffer, "STOP", 4) == 0)
    {
        // Remove client
        fprintf(stdout, "Client %s disconnected\n", clients[client_id]);
        fds[client_id + 2].fd = -1;
        clients[client_id] = NULL;
    }
    else
    {
        fprintf(stdout, "Unknown command\n");

        // Send back to client
        if (send(fds[client_id + 2].fd, "Unknown command", strlen("Unknown command") + 1, 0) == -1)
        {
            perror("send");
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <path>\n", argv[0]);
        exit(1);
    }

    port = argv[1];
    path = argv[2];

    // Register signal handlers
    struct sigaction act;
    act.sa_handler = kill_server;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    int network_socket = init_network_socket(port);
    int local_socket = init_local_socket(path);

    // Initialize fds
    fds[0].fd = network_socket;
    fds[0].events = POLLIN;
    fds[1].fd = local_socket;
    fds[1].events = POLLIN;

    // Initialize other fds to -1
    for (int i = 2; i < 2 + MAX_CLIENTS; i++)
    {
        fds[i].fd = -1;
    }

    fprintf(stdout, "Server is running!\n");

    // Initialize ping thread
    pthread_create(&ping_thread, NULL, ping_clients, NULL);

    while (1)
    {
        int res = poll(fds, 2 + MAX_CLIENTS, -1);
        if (res == -1)
        {
            perror("poll");
        }

        pthread_mutex_lock(&fds_mutex);

        // Handle new connections
        for (int i = 0; i < 2; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                int client_socket = accept(fds[i].fd, NULL, NULL);
                if (client_socket == -1)
                {
                    perror("accept");
                    exit(1);
                }

                // Receive config message
                char buffer[LINE_MAX];
                int res = recv(client_socket, buffer, sizeof(buffer), 0);
                if (res == -1)
                {
                    perror("recv");
                    exit(1);
                }

                int j = 2;
                int same_name_exists = 0;
                while (j < 2 + MAX_CLIENTS)
                {
                    if (clients[j - 2] != NULL && strcmp(clients[j - 2], buffer) == 0)
                    {
                        same_name_exists = 1;
                        break;
                    }
                    j++;
                }

                if (same_name_exists)
                {
                    // Send response that client already exists
                    char *response = "STOPClient with the same name already exists!";
                    if (send(client_socket, response, strlen(response) + 1, 0) == -1)
                    {
                        perror("send same name");
                        exit(1);
                    }
                    break;
                }

                j = 2;
                while (j < 2 + MAX_CLIENTS && fds[j].fd != -1)
                {
                    j++;
                }

                if (j == 2 + MAX_CLIENTS)
                {
                    // Send response that server is full
                    char *response = "STOPServer is full! Try again later.";
                    if (send(client_socket, response, strlen(response) + 1, 0) == -1)
                    {
                        perror("send maxclients");
                    }
                }
                else
                {
                    fds[j].fd = client_socket;
                    fds[j].events = POLLIN;
                    clients[j - 2] = malloc(strlen(buffer) + 1);
                    strcpy(clients[j - 2], buffer);

                    // Send response that client was added
                    char *response = "OKClient added successfully";
                    if (send(client_socket, response, strlen(response) + 1, 0) == -1)
                    {
                        perror("send confirm");
                    }

                    printf("Client %s connected!\n", clients[j - 2]);
                }

                fds[i].revents = 0;
            }
        }

        // Handle messages
        for (int i = 2; i < 2 + MAX_CLIENTS; i++)
        {
            if (fds[i].revents & POLLIN && fds[i].fd != -1)
            {
                char buffer[LINE_MAX];
                int res = recv(fds[i].fd, buffer, sizeof(buffer), MSG_DONTWAIT);

                if (res == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        continue;
                    }
                    perror("recv handler");
                }

                else if (res == 0)
                {
                    // shutdown(fds[i].fd, SHUT_RDWR);
                    // close(fds[i].fd);
                    // fds[i].fd = -1;
                    // fprintf(stdout, "Client %s disconnected\n", clients[i - 2]);
                    // clients[i - 2] = NULL;

                    // Or wait until PING is not received...
                }
                else
                {
                    handle_message(buffer, i - 2);
                }

                fds[i].revents = 0;
            }
        }

        pthread_mutex_unlock(&fds_mutex);
    }

    return 0;
}