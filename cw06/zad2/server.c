#include "config.h"

mqd_t server_queue;
char *client_queue_name[MAX_CLIENTS];

void save_to_file(msgbuf msg)
{
    FILE *file = fopen("log.txt", "a");
    if (file == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Client id: %d, message type: %ld, message: %s, send time: %ld\n", msg.id, msg.mtype, msg.mtext, msg.send_time);

    fclose(file);
}

void handle_init(msgbuf msg)
{
    msgbuf response;
    response.mtype = INIT;
    response.id = -1;
    response.send_time = time(NULL);

    // Save client queue name
    int i = 0;
    while (client_queue_name[i] != NULL)
    {
        i++;
    }
    mqd_t client_queue = mq_open(msg.queue_id, O_RDWR);

    if (i == MAX_CLIENTS)
    {
        mq_send(client_queue, (char *)&response, MSG_SIZE, 0);
        return;
    }

    client_queue_name[i] = calloc(MAX_QUEUE_NAME, sizeof(char));
    strcpy(client_queue_name[i], msg.queue_id);
    response.id = i;

    if (mq_send(client_queue, (char *)&response, MSG_SIZE, 0) < 0)
    {
        perror("Error while sending INIT response");
        exit(1);
    }
    mq_close(client_queue);
    printf("Client connected with id %d \n", i);
}

void handle_list(msgbuf msg)
{
    msgbuf response;
    response.mtype = LIST;
    response.id = msg.id;
    response.send_time = time(NULL);

    if (client_queue_name[msg.id] == NULL)
    {
        return;
    }

    mqd_t client_queue = mq_open(client_queue_name[msg.id], O_RDWR);

    if (client_queue == -1)
    {
        perror("Error while opening client queue");
        exit(1);
    }

    int i = 0;
    char *tmp_text = calloc(MSG_SIZE, sizeof(char));
    strcat(tmp_text, "\nConnected clients: \n");
    while (client_queue_name[i] != NULL)
    {
        char tmp[MSG_SIZE];
        sprintf(tmp, "Client %d: %s \n", i, client_queue_name[i]);
        strcat(tmp_text, tmp);
        i++;
    }

    strcpy(response.mtext, tmp_text);
    free(tmp_text);

    if (mq_send(client_queue, (char *)&response, MSG_SIZE, 0) < 0)
    {
        perror("Error while sending LIST response");
        exit(1);
    }

    mq_close(client_queue);
}

void handle_2all(msgbuf msg)
{
    msgbuf response;
    response.mtype = _2ALL;
    response.id = msg.id;
    strcpy(response.mtext, msg.mtext);
    response.send_time = time(NULL);

    if (client_queue_name[msg.id] == NULL)
    {
        return;
    }

    mqd_t client_queue = mq_open(client_queue_name[msg.id], O_RDWR);

    if (client_queue == -1)
    {
        perror("Error while opening client queue");
        exit(1);
    }

    int i = 0;
    while (i < MAX_CLIENTS)
    {
        if (i != msg.id && client_queue_name[i] != NULL)
        {
            mqd_t tmp_client_queue = mq_open(client_queue_name[i], O_RDWR);
            if (tmp_client_queue == -1)
            {
                perror("Error while opening client queue");
                continue;
            }

            response.id = i;
            if (mq_send(tmp_client_queue, (char *)&response, MSG_SIZE, 0) < 0)
            {
                perror("Error while sending _2ALL response");
                exit(1);
            }
            mq_close(tmp_client_queue);
        }
        i++;
    }

    // response.id = msg.id;
    // if (mq_send(client_queue, (char *)&response, MSG_SIZE, 0) < 0)
    // {
    //     perror("Error while sending _2ALL response");
    //     exit(1);
    // }

    mq_close(client_queue);
}

void handle_2one(msgbuf msg)
{
    msgbuf response;
    response.mtype = _2ONE;
    response.id = msg.id;
    strcpy(response.mtext, msg.mtext);
    response.send_time = time(NULL);

    if (client_queue_name[msg.id] == NULL)
    {
        return;
    }

    mqd_t client_queue = mq_open(client_queue_name[msg.id], O_RDWR);

    if (client_queue == -1)
    {
        perror("Error while opening client queue");
        exit(1);
    }

    int i = 0;
    while (i < MAX_CLIENTS)
    {
        if (i == msg.receiver_id && client_queue_name[i] != NULL)
        {
            mqd_t tmp_client_queue = mq_open(client_queue_name[i], O_RDWR);
            if (tmp_client_queue == -1)
            {
                perror("Error while opening client queue");
                continue;
            }

            response.id = i;
            if (mq_send(tmp_client_queue, (char *)&response, MSG_SIZE, 0) < 0)
            {
                perror("Error while sending _2ALL response");
                exit(1);
            }
            mq_close(tmp_client_queue);
            break;
        }
        i++;
    }

    // response.id = msg.id;
    // if (mq_send(client_queue, (char *)&response, MSG_SIZE, 0) < 0)
    // {
    //     perror("Error while sending _2ALL response");
    //     exit(1);
    // }

    mq_close(client_queue);
}

void handle_stop(msgbuf msg)
{
    if (client_queue_name[msg.id] == NULL)
    {
        return;
    }

    mqd_t client_queue = mq_open(client_queue_name[msg.id], O_RDWR);

    if (client_queue == -1)
    {
        perror("Error while opening client queue");
        exit(1);
    }

    free(client_queue_name[msg.id]);
    client_queue_name[msg.id] = NULL;

    printf("Client %d disconnected\n", msg.id);

    msg.id = -1;

    if (mq_send(client_queue, (char *)&msg, MSG_SIZE, 0) < 0)
    {
        perror("Error while sending STOP response");
        exit(1);
    }

    mq_close(client_queue);
}

void handle_sigint()
{
    // Send STOP to all clients
    msgbuf msg;
    msg.mtype = STOP;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_queue_name[i] != NULL)
        {
            mqd_t client_queue = mq_open(client_queue_name[i], O_RDWR);

            if (client_queue == -1)
            {
                perror("Error while opening client queue");
                exit(1);
            }

            if (mq_send(client_queue, (char *)&msg, MSG_SIZE, 0) < 0)
            {
                perror("Error while sending STOP response");
                exit(1);
            }

            mq_close(client_queue);

            free(client_queue_name[i]);
        }
    }

    mq_close(server_queue);

    exit(0);
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_queue_name[i] = NULL;
    }

    mq_unlink(SQUEUE);
    server_queue = create_queue(SQUEUE);

    msgbuf msg;

    signal(SIGINT, handle_sigint);

    printf("Server started \n");

    while (1)
    {
        mq_receive(server_queue, (char *)&msg, MSG_SIZE, NULL);

        switch (msg.mtype)
        {
        case INIT:
            printf("INIT\n");
            handle_init(msg);
            break;
        case LIST:
            printf("LIST\n");
            handle_list(msg);
            break;
        case _2ALL:
            printf("_2ALL\n");
            handle_2all(msg);
            break;
        case _2ONE:
            printf("_2ONE\n");
            handle_2one(msg);
            break;
        case STOP:
            printf("STOP\n");
            handle_stop(msg);
            break;
        default:
            printf("Unknown message type\n");
            break;
        }

        save_to_file(msg);
    }

    return 0;
}