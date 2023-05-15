#include "config.h"

int client_queue_ids[MAX_CLIENTS] = {0};
int server_queue_id;

void handle_exit()
{
    // Send exit to all clients
    int reciever_id;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_queue_ids[i] != -1)
        {
            msgbuf msg;
            msg.mtype = SRV_STOP;
            sprintf(msg.mtext, "Server is shutting down!\n");
            msg.id = i;
            msg.send_time = time(NULL);

            if ((reciever_id = msgget(client_queue_ids[i], 0)) == -1)
            {
                perror("msgget");
                exit(EXIT_FAILURE);
            }

            if (msgsnd(reciever_id, (void *)&msg, MSG_SIZE, 0) == -1)
            {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }

            if (msgrcv(server_queue_id, (void *)&msg, MSG_SIZE, SRV_STOP, 0) == -1)
            {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (msgctl(server_queue_id, IPC_RMID, NULL) < 0)
    {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

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

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_exit);

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

    if ((server_queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // TODO: better initialization
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_queue_ids[i] = -1;
    }

    // Main loop
    msgbuf req_msg;

    fprintf(stdout, "Listening for messages...\n");

    while (1)
    {
        if (msgrcv(server_queue_id, (void *)&req_msg, MSG_SIZE, -6, 0) == -1)
        {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        save_to_file(req_msg);

        msgbuf res_msg;
        int reciever_id;
        res_msg.send_time = time(NULL);

        switch (req_msg.mtype)
        {
        case INIT:
            res_msg.mtype = req_msg.mtype;
            // Create message
            // Find available_id
            int available_id = 0;
            while (client_queue_ids[available_id] != -1 && available_id < MAX_CLIENTS)
            {
                available_id++;
            }

            if (available_id == MAX_CLIENTS)
            {
                sprintf(res_msg.mtext, "Hello from server! Unfortunately, there is no space for you! Please try again later.\n");
                res_msg.id = -1;
            }
            else
            {
                sprintf(res_msg.mtext, "Hello from server! Your id is %d\n", available_id);
                client_queue_ids[available_id] = req_msg.queue_id;
                res_msg.id = available_id;
            }

            if ((reciever_id = msgget(req_msg.queue_id, 0)) == -1)
            {
                perror("msgget");
                exit(EXIT_FAILURE);
            }

            if (msgsnd(reciever_id, (void *)&res_msg, MSG_SIZE, 0) == -1)
            {
                perror("msgsnd");
                break;
            }
            break;
        case LIST:
            res_msg.mtype = req_msg.mtype;

            char *msg_to_send = calloc(MAX_MSG_SIZE, sizeof(char));
            strcat(msg_to_send, "List of clients:\n");
            char *text = calloc(MAX_MSG_SIZE, sizeof(char));
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_queue_ids[i] != -1)
                {
                    sprintf(text, "id: %d, queue_id: %d\n", i, client_queue_ids[i]);
                    strcat(msg_to_send, text);
                }
            }

            free(text);
            strcpy(res_msg.mtext, msg_to_send);
            free(msg_to_send);

            if ((reciever_id = msgget(client_queue_ids[req_msg.id], 0)) == -1)
            {
                perror("msgget");
                exit(EXIT_FAILURE);
            }

            if (msgsnd(reciever_id, (void *)&res_msg, MSG_SIZE, 0) == -1)
            {
                perror("msgsnd");
                break;
            }
            break;
        case _2ALL:
            res_msg.mtype = req_msg.mtype;
            // Create message
            char *message_from_client = calloc(MAX_MSG_SIZE, sizeof(char));
            sprintf(message_from_client, "Message from client %d: %s", req_msg.id, req_msg.mtext);
            strcpy(res_msg.mtext, message_from_client);
            free(message_from_client);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_queue_ids[i] == -1 || i == req_msg.id)
                {
                    continue;
                }

                res_msg.id = i;
                res_msg.send_time = time(NULL);

                if ((reciever_id = msgget(client_queue_ids[i], 0)) == -1)
                {
                    perror("msgget");
                    exit(EXIT_FAILURE);
                }

                if (msgsnd(reciever_id, (void *)&res_msg, MSG_SIZE, 0) == -1)
                {
                    perror("msgsnd");
                    break;
                }
            }
            break;
        case _2ONE:
            res_msg.mtype = req_msg.mtype;
            // Create message
            message_from_client = calloc(MAX_MSG_SIZE, sizeof(char));
            sprintf(message_from_client, "Message from client %d: %s", req_msg.id, req_msg.mtext);
            strcpy(res_msg.mtext, message_from_client);
            free(message_from_client);

            res_msg.id = req_msg.receiver_id;
            res_msg.send_time = time(NULL);

            if ((reciever_id = msgget(client_queue_ids[req_msg.receiver_id], 0)) == -1)
            {
                perror("msgget");
                exit(EXIT_FAILURE);
            }

            if (msgsnd(reciever_id, (void *)&res_msg, MSG_SIZE, 0) == -1)
            {
                perror("msgsnd");
                break;
            }

            break;
        case STOP:
            res_msg.mtype = req_msg.mtype;
            // Create message

            sprintf(res_msg.mtext, "See you later, alligator!\n");

            if ((reciever_id = msgget(client_queue_ids[req_msg.id], 0)) == -1)
            {
                perror("msgget");
                exit(EXIT_FAILURE);
            }

            // Remove id
            client_queue_ids[req_msg.id] = -1;

            if (msgsnd(reciever_id, (void *)&res_msg, MSG_SIZE, 0) == -1)
            {
                perror("msgsnd");
                break;
            }
            break;
        default:
            printf("Unknown message type: %ld\n", req_msg.mtype);
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
