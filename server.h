#ifndef __NET_SERVER_H
#define __NET_SERVER_H

#include "common.h"
#include "block.h"

#define NETBUF_SIZE         1024
#define NETNAME_LEN         20

#define MSGSIZE_WINMAP      (((BLOCK_MAP_W + 7) / 8) * BLOCK_MAP_H)
#define MSGSIZE_GRADE       (sizeof(int))
#define MSGSIZE_MATRIX      2
#define MSGSIZE_LOSE        0
#define MSGSIZE_READY       0
#define MSG_MAXSIZE         (MSGSIZE_WINMAP + sizeof(int) * 2)

#define MSGTYPE_EVENT       1
#define MSGTYPE_GRADE       2
#define MSGTYPE_MATRIX      3
#define MSGTYPE_WINMAP      4
#define MSGTYPE_MATCH       5
#define MSGTYPE_READY       7
#define MSGTYPE_LOSE        6
#define MSGTYPE_WIN         7

#define FIRST_PLAYER        0
#define SECOND_PLAYER       1

#define NET_MAX_CONNECT     10

typedef struct _ServerRB ServerRB;
typedef struct {
    Player player;

    int sockfd;
    int id;
    DataBuffer * send_buf;
    DataBuffer * recv_buf;
    Uchar old_map[BLOCK_MAP_H][BLOCK_MAP_W];

    ServerRB * host;
    list_t w_list;
    list_t p_list;
} NetPlayer;

struct _ServerRB {
    int listenfd;
    DataBuffer * send_buf;
    list_t player_list;
    list_t wait_list;
};

extern IOOPS server_io_ops;
extern RBOPS server_rb_ops;

#endif
