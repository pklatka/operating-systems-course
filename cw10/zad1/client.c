#include "common.h"

int socket_fd;
char *ip = NULL;
char *port = NULL;
char *unixpath = NULL;
int is_local = 0;

int trim(char *str)
{
    int i = strlen(str) - 1;
    while (i >= 0 && isspace(str[i]))
    {
        str[i] = '\0';
        i--;
    }

    i = 0;
    while (i < strlen(str) && isspace(str[i]))
    {
        i++;
    }

    // Modify string
    int j = 0;
    while (i < strlen(str))
    {
        str[j] = str[i];
        i++;
        j++;
    }

    str[j] = '\0';

    return i;
}

void handle_response(char *response)
{
    if (strncmp(response, "OK", 2) == 0)
    {
        fprintf(stdout, "Response: %s\n", response + 2);
    }
    else if (strncmp(response, "2ALL", 4) == 0 || strncmp(response, "2ONE", 4) == 0 || strncmp(response, "LIST", 4) == 0)
    {
        fprintf(stdout, "%s\n", response + 4);
    }
    else if (strncmp(response, "PING", 4) == 0)
    {
        if (write(socket_fd, "PONG", 4) == -1)
        {
            perror("write");
            exit(1);
        }
    }
    else if (strncmp(response, "STOP", 4) == 0)
    {
        printf("Response: %s\n", response + 4);
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);

        if (is_local)
            free(unixpath);
        else
        {
            free(ip);
            free(port);
        }

        kill(getppid(), SIGINT);
        exit(0);
    }
    else if (strncmp(response, "ERROR", 5) == 0)
    {
        printf("Response: %s\n", response + 5);
    }
    else
    {
        printf("Response: Unknown response\n");
    }
}

void exit_handler()
{
    if (write(socket_fd, "STOP", 4) == -1)
    {
        perror("write");
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    if (is_local)
    {
        free(unixpath);
    }
    else
    {
        free(ip);
        free(port);
    }

    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s <name> <method> <ip:port|localname>\n", argv[0]);
        exit(1);
    }

    // Check what method is used to connect
    if (strcmp(argv[2], "local") == 0)
        is_local = 1;
    else if (strcmp(argv[2], "network") != 0)
    {
        printf("Method must be either 'local' or 'network'\n");
        exit(1);
    }

    // Parse ip and port or unixpath
    if (is_local)
    {
        unixpath = malloc(strlen(argv[3]) + 1);
        strcpy(unixpath, argv[3]);

        if (strlen(unixpath) > UNIX_PATH_MAX)
        {
            printf("Unixpath too long\n");
            exit(1);
        }
    }
    else
    {
        char *colon = strchr(argv[3], ':');
        if (colon == NULL)
        {
            printf("Invalid ip:port\n");
            exit(1);
        }

        ip = malloc(colon - argv[3] + 1);
        strncpy(ip, argv[3], colon - argv[3]);
        ip[colon - argv[3]] = '\0';

        port = malloc(strlen(colon + 1) + 1);
        strcpy(port, colon + 1);
    }

    // Create socket
    if (is_local)
        socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    else
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1)
    {
        printf("Error while creating socket\n");
        exit(1);
    }

    // Connect to server
    if (is_local)
    {
        struct sockaddr_un server_address;
        server_address.sun_family = AF_UNIX;
        strcpy(server_address.sun_path, unixpath);

        if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        {
            perror("connect");
            printf("Error while connecting\n");
            exit(1);
        }
    }
    else
    {
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(atoi(port));
        inet_pton(AF_INET, ip, &server_address.sin_addr);

        if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        {
            printf("Error while connecting\n");
            exit(1);
        }
    }

    signal(SIGINT, exit_handler);

    // Send name
    send(socket_fd, argv[1], strlen(argv[1]) + 1, 0);

    char response[LINE_MAX];

    // Start listening for responses
    if (fork() == 0)
    {
        while (1)
        {
            // Read response
            if (recv(socket_fd, response, LINE_MAX, 0) == -1)
            {
                printf("Error while receiving response\n");
                exit(1);
            }

            handle_response(response);
        }

        return 0;
    }

    // Read commands
    char command[LINE_MAX];
    while (fgets(command, LINE_MAX, stdin) != NULL)
    {
        // Remove newline
        command[strlen(command) - 1] = '\0';

        // Check if command is valid
        if (strlen(command) == 0)
            continue;

        if (strncmp(command, "STOP", 4) == 0)
        {
            if (send(socket_fd, command, strlen(command) + 1, 0) == -1)
            {
                printf("Error while sending command\n");
                exit(1);
            }

            break;
        }

        // Get command and message separated by space
        char *type = strtok(command, " ");
        char *message = strtok(NULL, "\n");

        trim(type);

        if (message == NULL)
            message = "";
        else
            trim(message);

        if (strcmp(type, "2ONE") == 0)
        {
            char *name = strtok(message, " ");
            char *msg = strtok(NULL, "\n");
            if (msg == NULL)
                msg = "";
            else
                trim(msg);
            sprintf(message, "%s %s", name, msg);
        }

        // Create string concatenated from type and message
        sprintf(command, "%s%s", type, message);

        // Send command
        if (send(socket_fd, command, strlen(command) + 1, 0) == -1)
        {
            printf("Error while sending command\n");
            exit(1);
        }
    }

    // Close socket
    if (close(socket_fd) == -1)
    {
        printf("Error while closing socket\n");
        exit(1);
    }

    // Free memory
    if (is_local)
        free(unixpath);
    else
    {
        free(ip);
        free(port);
    }

    return 0;
}
