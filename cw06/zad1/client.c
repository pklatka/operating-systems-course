#include "config.h"

int client_queue_id;
int server_queue_id;
key_t client_key;
int client_id;
pid_t child_pid;
pid_t parent_pid;

void send_msg(msgtype, char *, int);
void initialize_client(void);
void handle_server_messages(void);
void close_connection(void);

void handle_sigint()
{
    if (getpid() == parent_pid)
    {
        exit(0);
    }

    send_msg(STOP, "", -1);
}

int main(int argc, char *argv[])
{
    // Initialize queue
    char *home = getenv("HOME");
    if (home == NULL)
    {
        perror("getenv");
        exit(EXIT_FAILURE);
    }

    key_t key;
    if ((key = ftok(home, SERVER_KEY)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if ((server_queue_id = msgget(key, 0)) == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((client_key = ftok(home, getpid())) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if ((client_queue_id = msgget(client_key, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    initialize_client();

    // Read command from stdin
    char line[MAX_MSG_SIZE];

    parent_pid = getpid();
    child_pid = fork();

    signal(SIGINT, handle_sigint);

    if (child_pid == 0)
    {
        handle_server_messages();
    }

    while (fgets(line, MAX_MSG_SIZE, stdin))
    {

        if (strncmp(line, "LIST", 4) == 0)
        {
            send_msg(LIST, "", -1);
        }
        else if (strncmp(line, "2ALL", 4) == 0)
        {
            // Split and get message
            char *msg = strtok(line, " ");
            msg = strtok(NULL, "\0");
            send_msg(_2ALL, msg, -1);
        }
        else if (strncmp(line, "2ONE", 4) == 0)
        {
            // Split and get message
            char *msg = strtok(line, " ");
            char *id = strtok(NULL, " ");
            msg = strtok(NULL, " \t\n\0");
            send_msg(_2ONE, msg, atoi(id));
        }
        else if (strncmp(line, "STOP", 4) == 0)
        {
            send_msg(STOP, "", -1);
        }
    }

    exit(EXIT_SUCCESS);
}

void handle_server_messages()
{
    msgbuf req_msg;

    while (msgrcv(client_queue_id, &req_msg, MSG_SIZE, -6, 0) >= 0)
    {

        if (req_msg.mtype == STOP)
        {
            printf("Received stop message, leaving..\n");
            close_connection();
        }
        else if (req_msg.mtype == SRV_STOP)
        {
            kill(parent_pid, SIGKILL);

            printf("Received server stop message, leaving..\n");

            send_msg(SRV_STOP, "", -1);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("%s\n", req_msg.mtext);

            // Print time from req_msg.send_time
            struct tm tm = *localtime(&req_msg.send_time);
            printf("Message time: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
    }
}

void send_msg(msgtype type, char *msg, int receiver_id)
{
    msgbuf req_msg;
    req_msg.mtype = type;
    req_msg.id = client_id;
    strcpy(req_msg.mtext, msg);

    if (receiver_id >= 0)
    {
        req_msg.receiver_id = receiver_id;
    }

    if (msgsnd(server_queue_id, (void *)&req_msg, MSG_SIZE, 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void initialize_client()
{
    msgbuf req_msg;
    req_msg.queue_id = client_key;
    req_msg.mtype = INIT;
    req_msg.id = -1;
    strcpy(req_msg.mtext, "");

    if (msgsnd(server_queue_id, (void *)&req_msg, MSG_SIZE, 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    msgbuf res_msg;

    if (msgrcv(client_queue_id, (void *)&res_msg, MSG_SIZE, INIT, 0) == -1)
    {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", res_msg.mtext);

    if (res_msg.id == -1)
    {
        close_connection();
        exit(EXIT_FAILURE);
    }

    client_id = res_msg.id;
}

void close_connection()
{
    printf("Closing connection..\n");
    if (msgctl(client_queue_id, IPC_RMID, NULL) < 0)
    {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    kill(parent_pid, SIGKILL);
    waitpid(child_pid, NULL, 0);
    exit(EXIT_SUCCESS);
}
