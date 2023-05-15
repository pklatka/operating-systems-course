#include "config.h"

mqd_t server_queue;
mqd_t client_queue;
int client_id;

pid_t child_pid;
pid_t parent_pid;

char *client_queue_name;

void generate_random_name()
{
    client_queue_name = malloc(MAX_QUEUE_NAME);
    sprintf(client_queue_name, "/%d", rand());
}

void initialize_client()
{
    msgbuf msg;
    msg.mtype = INIT;
    msg.id = -1;
    strcpy(msg.queue_id, client_queue_name);
    if (mq_send(server_queue, (char *)&msg, MSG_SIZE, 0) < 0)
    {
        perror("Error while sending INIT message");
        exit(1);
    }

    if (mq_receive(client_queue, (char *)&msg, MSG_SIZE, NULL) < 0)
    {
        perror("Error while receiving INIT message");
        exit(1);
    }

    if (msg.id == -1)
    {
        printf("Server is full. Try again later. \n");
        exit(1);
    }

    client_id = msg.id;
    printf("Received INIT message from server with id: %d\n", msg.id);
}

void send_msg(msgtype type, char *msg, int receiver_id)
{
    msgbuf req_msg;
    req_msg.mtype = type;
    req_msg.id = client_id;
    req_msg.receiver_id = receiver_id;
    strcpy(req_msg.mtext, msg);
    if (mq_send(server_queue, (char *)&req_msg, MSG_SIZE, 0) < 0)
    {
        perror("Error while sending message");
        exit(1);
    }

    // msgbuf response_msg;
    // response_msg.mtype = type;
    // if (mq_receive(client_queue, (char *)&response_msg, MSG_SIZE, NULL) < 0)
    // {
    //     perror("Error while receiving message");
    //     exit(1);
    // }

    // printf("Received message from server: %s \n", response_msg.mtext);
}

void handle_stop()
{
    kill(parent_pid, SIGKILL);

    waitpid(parent_pid, NULL, 0);

    if (mq_close(client_queue) < 0)
    {
        perror("Error while closing client queue");
        exit(1);
    }

    if (mq_close(server_queue) < 0)
    {
        perror("Error while closing client queue");
        exit(1);
    }

    if (mq_unlink(client_queue_name) < 0)
    {
        perror("Error while unlinking client queue");
        exit(1);
    }

    exit(0);
}

void handle_server_message()
{
    msgbuf msg;

    while (1)
    {
        if (mq_receive(client_queue, (char *)&msg, MSG_SIZE, NULL) < 0)
        {
            perror("Error while receiving message");
            exit(1);
        }

        switch (msg.mtype)
        {
        case STOP:
            printf("Received STOP message from server. \n");
            handle_stop();
            break;
        case _2ALL:
            printf("Received 2ALL message from server: %s \n", msg.mtext);
            break;
        case _2ONE:
            printf("Received 2ONE message from server: %s \n", msg.mtext);
            break;
        case LIST:
            printf("Received LIST message from server: %s \n", msg.mtext);
            break;
        default:
            break;
        }

        struct tm tm = *localtime(&msg.send_time);
        printf("Message time: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
}

void handle_sigint(int signum)
{
    if (getpid() == parent_pid)
    {
        exit(0);
    }

    send_msg(STOP, "", -1);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    generate_random_name();

    client_queue = create_queue(client_queue_name);

    if (client_queue == -1)
    {
        perror("Error while creating client queue");
        exit(1);
    }

    server_queue = mq_open(SQUEUE, O_RDWR);

    if (server_queue == -1)
    {
        perror("Error while opening server queue");
        exit(1);
    }

    initialize_client();

    parent_pid = getpid();

    child_pid = fork();

    signal(SIGINT, handle_sigint);

    if (child_pid == 0)
    {
        handle_server_message();
    }

    char line[MAX_MSG_SIZE];
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
            // waitpid(child_pid, NULL, 0);
            // exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
