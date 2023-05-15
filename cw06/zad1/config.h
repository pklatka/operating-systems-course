#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/wait.h>

#define SERVER_KEY 1234
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 512

typedef struct msgbuf
{
    long mtype;
    char mtext[MAX_MSG_SIZE];
    int id;
    int receiver_id;
    key_t queue_id;
    time_t send_time;
} msgbuf;

typedef enum msgtype
{
    INIT = 1,
    LIST = 2,
    _2ALL = 3,
    _2ONE = 4,
    STOP = 5,
    SRV_STOP = 6
} msgtype;

const size_t MSG_SIZE = sizeof(msgbuf) - sizeof(long);
