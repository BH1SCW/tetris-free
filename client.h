#ifndef __NET_CLIENT_H
#define __NET_CLIENT_H

#include "block.h"
#include "common.h"

#define SENDBUFSIZE         100
#define CMDBUFSIZE          512

#define CLI_STATUS_MATCH    (0x1 << 0)
#define CLI_STATUS_READY    (0x1 << 1)

typedef struct {
    int connfd;
    int status;
    DataBuffer * cmd_buf;
    DataBuffer * send_buf;
    int * event;
} CliRB;

extern RBOPS client_rb_ops;
#endif
