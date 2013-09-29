/* 网络服务端输入输出代码 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"
#include "block.h"
#include "mtime.h"
#include "common.h"

static int server_io_init(Player * player);
static void server_io_out_grade(Player * player);
static void server_io_out_next_block(Player * player);
static void server_io_out_win_map(Player * player, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W]);

IOOPS server_io_ops = {
init:               server_io_init,
out_grade:          server_io_out_grade,
out_next_block:     server_io_out_next_block,
out_win_map:        server_io_out_win_map,
refresh_screen:         NULL,
};

static int server_init(RussiaBlock * rb, void * data);
static Player * server_add_player(RussiaBlock * rb);
static int server_recv_player_data(RussiaBlock * rb);
static void server_send_player_data(RussiaBlock * rb);
static void server_add_match(RussiaBlock * rb);
static void server_del_match(RussiaBlock * rb, Player * player);

RBOPS server_rb_ops = {
init:                   server_init,
add_player:             server_add_player,
add_match:              server_add_match,
del_match:              server_del_match,
recv_player_data:       server_recv_player_data,
send_player_data:       server_send_player_data,
exec_misc_cmd:          NULL,
};

static inline void add_player_to_wlist(ServerRB * rb_t, NetPlayer * player)
{
    list_add_tail(&player->w_list, &rb_t->wait_list);
}

static inline void add_player_to_plist(ServerRB * rb_t, NetPlayer * player)
{
    list_add_tail(&player->p_list, &rb_t->player_list);
}

static inline void del_player_form_plist(NetPlayer * player)
{
    list_del(&player->p_list);
}

static inline void del_player_form_wlist(NetPlayer * player)
{
    list_del(&player->w_list);
}

static void server_add_match(RussiaBlock * rb)
{
    ServerRB * rb_t = (ServerRB *)rb->type_data;
    list_t * head = &rb_t->wait_list;
    int t[2], i;

    if (rb->match[0] && rb->match[1])
        return;

    for (i = 0; !list_empty(head) && i < 2; i++) {
        NetPlayer * np;

        if (rb->match[i])
            continue;

        np = list_entry(head->next, NetPlayer, w_list);
        del_player_form_wlist(np);

        np->id = i;
        rb->match[i] = (Player *)np;
        rb->io_ops->init((Player *)np);
        ((Player *)np)->status = PLAYER_STATUS_MATCH;
        ((Player *)np)->status &= ~PLAYER_READY;

        t[0] = MSGTYPE_MATCH;
        t[1] = i;
        write_data_to_buf(np->send_buf, (char *)t, sizeof(t));
    }
}

static void server_del_match(RussiaBlock * rb, Player * player)
{
    ServerRB * rb_t = (ServerRB *)rb->type_data;
    NetPlayer * np = (NetPlayer *)player;

    if (player->status & PLAYER_STATUS_LOSE) {
        add_player_to_wlist(rb_t, np);
        rb->match[np->id] = NULL;
        //rb->match[1 - np->id]->status &= ~PLAYER_READY;
    }
    else
        return;

    {
    int t[2];
    t[0] = MSGTYPE_LOSE;
    t[1] = np->id;
    write_data_to_buf(np->send_buf, (char *)t, sizeof(t));
    }
}

/*压缩下一个方块到网络字节位数据*/
static void server_io_out_next_block(Player * player)
{
    NetPlayer * np = (NetPlayer *)player;
    ServerRB * rb_t = np->host;

    char buf[10];
    unsigned short * s = (unsigned short *)(buf + 8);
    int x, y;

    ((int *)buf)[0] = MSGTYPE_MATRIX;
    ((int *)buf)[1] = np->id;
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            *s >>= 1;
            if (player->next_matrix[y][x])
                *s |= 0x8000;
            else
                *s &= 0x7fff;
        }
    }

    write_data_to_buf(rb_t->send_buf, buf, 10);
}

static void server_io_out_grade(Player * player)
{
    NetPlayer * np = (NetPlayer *)player;
    ServerRB * rb_t = np->host;
    int buf[3];

    buf[0] = MSGTYPE_GRADE;
    buf[1] = np->id;
    buf[2] = player->grade;
    write_data_to_buf(rb_t->send_buf, (char *)buf, sizeof(buf));
}

/*把输出位图压缩成字节位数据*/
static void win_map_to_bits(Uchar map[BLOCK_MAP_H][BLOCK_MAP_W], char * buf)
{
    char * t = buf;
    int x, y;

    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
            if (x != 0 && x % 8 == 0)
                t++;
                *t >>= 1;
            if (map[y][x])
                *t |= 0x80; 
            else
                *t &= 0x7f;
        }
        if (BLOCK_MAP_W % 8 > 0)
            *t >>= (8 - BLOCK_MAP_W % 8);
        t++;
    }
}

static void server_io_out_win_map(Player * player, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W])
{
    NetPlayer * np = (NetPlayer *)player;
    ServerRB * rb_t = np->host;
    char buf[sizeof(int) * 2 + MSGSIZE_WINMAP];

    ((int *)buf)[0] = MSGTYPE_WINMAP;
    ((int *)buf)[1] = np->id;
    win_map_to_bits(map, buf + sizeof(int) * 2);

    write_data_to_buf(rb_t->send_buf, (char *)buf, sizeof(int) * 2 + MSGSIZE_WINMAP);
}

static int server_io_init(Player * player)
{
    NetPlayer * np = (NetPlayer *)player;
    int x, y;

    player->grade = 0;
    player->speed = BLOCK_INIT_SPEED;
    player->slice = 0;
    player->cur_shape = 0;
    player->next_shape = shapes[rand_id()];
    cp_next_shape(player);

    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
            player->block_map[y][x] = 0;
            np->old_map[y][x] = 0;
        }
    }

    return 1;
}

static void release_player(NetPlayer * player)
{
    if (player->send_buf)
        release_data_buf(player->send_buf);
    if (player->recv_buf)
        release_data_buf(player->recv_buf);

    if (((Player *)player)->event_buf)
        release_data_buf(((Player *)player)->event_buf);

    close(player->sockfd);

    free((void *)player);
}

static void remove_net_player(RussiaBlock * rb, NetPlayer * player)
{
    if (((Player *)player)->status & PLAYER_STATUS_MATCH) {
        rb->status &= ~STATUS_RUN;
        rb->match[player->id] = NULL;
        del_player_form_plist(player);
    }
    else {
        del_player_form_wlist(player);
        del_player_form_plist(player);
    }

    release_player(player);
}

/*把从网络接收的数据进行分离*/
static int dispatch_data(NetPlayer * net_player, char * data, int len)
{
    Player * player = (Player *)net_player;
    char * buf = data;
    int l = len;
    int msg;

    while (l >= sizeof(int)) {
        msg = ((int *)buf)[0];
        l -= sizeof(int);
        buf += sizeof(int);

        switch (msg) {
        case MSGTYPE_EVENT:
            /*把控制事件写入控制事件buf*/
            if (l >= sizeof(int))
                write_data_to_buf(player->event_buf, buf, sizeof(int));
            l -= sizeof(int);
            buf += sizeof(int);
        break;

        case MSGTYPE_READY:
            player->status |= PLAYER_READY;
        break;

        default:
            write_data_to_buf(net_player->recv_buf, buf, l);
            l = 0;
        }
    }

    return 0;
}

static void recv_player_data(RussiaBlock * rb, NetPlayer * np)
{
    char buf[NETBUF_SIZE];
    ssize_t len;

    if ((len = read(np->sockfd, buf, NETBUF_SIZE)) < 0)
        return;

    if (len == 0) {
        //send_msg_to_all_client(np, NETMSG_DISCONNECT, 0);
        remove_net_player(rb, np);
        return;
    }

    dispatch_data(np, buf, len);
}

static int server_recv_player_data(RussiaBlock * rb)
{
    list_t * pos;
    ServerRB * rb_t = (ServerRB *)rb->type_data;

    list_for_each(pos, &rb_t->player_list) {
        NetPlayer * np;
        np = list_entry(pos, NetPlayer, p_list);
        recv_player_data(rb, np);
    }

    return 1;
}

static void server_send_player_data(RussiaBlock * rb)
{
    ServerRB * rb_t = (ServerRB *)rb->type_data;
    char buf[NETBUF_SIZE], * t = buf;
    ssize_t len;
    list_t * pos;

    len = read_data_from_buf(rb_t->send_buf, buf, NETBUF_SIZE);
    t += len;

    list_for_each(pos, &rb_t->player_list) {
        int l;
        NetPlayer * np = list_entry(pos, NetPlayer, p_list);

        l = read_data_from_buf(np->send_buf, t, NETBUF_SIZE);
        if (l + len > 0)
            write(np->sockfd, buf, l + len);
    }

    return;
}

static int conaccept(int listenfd)
{
    int status;
	int connfd;

    if ((connfd = accept(listenfd, NULL, NULL)) < 0)
        return -1;

    do {
        read(connfd, (char *)&status, sizeof(int));
    } while (status != PLAYER_JOIN);

    fcntl(connfd, F_SETFL, O_NONBLOCK);

    return connfd;
}

static Player * server_add_player(RussiaBlock * rb)
{
    int fd;
    NetPlayer * np;
    ServerRB * rb_t = (ServerRB *)rb->type_data;

    if ((fd = conaccept(rb_t->listenfd)) < 0)
        return 0;

    np = (NetPlayer *)malloc(sizeof(NetPlayer));
    np->sockfd = fd;
    np->send_buf= (DataBuffer *)malloc_data_buf(NETBUF_SIZE);
    np->recv_buf= (DataBuffer *)malloc_data_buf(NETBUF_SIZE);
    np->host = rb_t;

    add_player_to_plist(rb_t, np);
    add_player_to_wlist(rb_t, np);
    ((Player *)np)->status = PLAYER_STATUS_WAIT;

    fprintf(stderr, "%d connect\n", fd);
    return (Player *)np;
}

static int listen_socket(void)
{
    int listenfd;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9877);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        goto close_fd;
    }

    if (listen(listenfd, 1) < 0) {
        perror("listen");
        goto close_fd;
    };
    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    return listenfd;

close_fd:
    close(listenfd);
    return -1;
}

static int server_init(RussiaBlock * rb, void * data)
{
    ServerRB * rb_t;

    rb_t = (ServerRB *)malloc(sizeof(ServerRB));
    if ((rb_t->listenfd = listen_socket()) < 0)
        return 0;

    rb_t->send_buf = (DataBuffer *)malloc_data_buf(NETBUF_SIZE);
    INIT_LIST_HEAD(&rb_t->player_list);
    INIT_LIST_HEAD(&rb_t->wait_list);

    rb->type_data = rb_t;
    return 1;
}

