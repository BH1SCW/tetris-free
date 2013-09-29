/*网络客户端输入输出代码*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "client.h"
#include "xvid.h"
#include "block.h"
#include "server.h"
#include "native.h"
#include "digit.h"

static int client_init(RussiaBlock * rb, void * data);
static Player * client_add_player(RussiaBlock * rb);
static int client_recv_player_data(RussiaBlock * rb);
static void client_send_player_data(RussiaBlock * rb);
static int client_exec_misc_cmd(RussiaBlock * rb);

static int key_code[6] = {K_LEFT, K_RIGHT, K_DOWN, K_UP, K_SPACE, K_ESCAPE};

RBOPS client_rb_ops = {
init:                   client_init,
add_player:             client_add_player,
add_match:              NULL,
del_match:              NULL,
recv_player_data:       client_recv_player_data,
send_player_data:       client_send_player_data,
exec_misc_cmd:          client_exec_misc_cmd,
};

/*返回网络消息的字节大小*/
static int msg_sizeof(int msg)
{
    int size = -1;

    switch (msg) {
    case MSGTYPE_GRADE:
        size = MSGSIZE_GRADE;
    break;

    case MSGTYPE_MATRIX:
        size = MSGSIZE_MATRIX;
    break;

    case MSGTYPE_WINMAP:
        size = MSGSIZE_WINMAP;
    break;

    case MSGTYPE_MATCH:
        size = MSGSIZE_READY;
    break;

    case MSGTYPE_LOSE:
        size = MSGSIZE_LOSE;
    break;

    case MSGTYPE_WIN:
        size = MSGSIZE_LOSE;
    break;
    }

    return size;
}

/*解压下一个方块的位数据*/
static void bits_to_matrix(char * buf, Uchar matrix[4][4])
{
    unsigned short * s = (unsigned short *)buf;
    int x, y;

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            if (*s & 0x1)
                matrix[y][x] = 1;
            else
                matrix[y][x] = 0;
            *s >>= 1;
        }
    }
}

/*解压位数据到输出位图*/
static void bits_to_win_map(char * buf, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W])
{
    char * t = buf;
    int x, y;
    
    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
            if (x % 8 == 0 && x != 0)
                t++;
            if (*t & 0x1)
                map[y][x] = 1;
            else
                map[y][x] = 0;
            *t >>= 1;
        }
        t++;
    }
}

static int get_msg_from_buf(DataBuffer * buf, int * msg, int * id, char * arg)
{
    int len;

    if (BUFEMPTY(buf))
        return 0;

    if (read_data_from_buf(buf, (char *)msg, sizeof(int)) < sizeof(int))
        return 0;

    if ((len = msg_sizeof(*msg)) < 0) {
        CLEARBUF(buf);
        return 0;
    }

    if (read_data_from_buf(buf, (char *)id, sizeof(int)) < sizeof(int))
        return 0;

    if (len > 0 && read_data_from_buf(buf, arg, len) < len)
		return 0;

    return 1;
}

static int client_exec_misc_cmd(RussiaBlock * rb)
{
    CliRB * rb_c = (CliRB *)rb->type_data;
    IOOPS * io_ops = rb->io_ops;
    char buf[MSG_MAXSIZE];
    int msg, id;

    while (get_msg_from_buf(rb_c->cmd_buf, &msg, &id, buf)) {
        Player * player = 0;

        if (id == 0)
            player = rb->match[0];
        else if (id == 1)
            player = rb->match[1];

        if (!player)
            continue;

        if (msg == MSGTYPE_WINMAP) {
            bits_to_win_map(buf, player->block_map);
            io_ops->out_win_map(player, player->block_map); 
        }
        else if (msg == MSGTYPE_GRADE) {
            player->grade = *(int *)buf;
            io_ops->out_grade(player);
        }
        else if (msg == MSGTYPE_MATRIX) {
            bits_to_matrix(buf, player->next_matrix);
            io_ops->out_next_block(player);
        }
        else if (msg == MSGTYPE_LOSE) {
            rb_c->status &= ~CLI_STATUS_MATCH;
            rb_c->status &= ~CLI_STATUS_READY;
        }
        else if (msg == MSGTYPE_WIN) {
            rb_c->status &= ~CLI_STATUS_READY;
        }
        else if (msg == MSGTYPE_MATCH) {
            rb_c->status |= CLI_STATUS_MATCH;
        }
    }

    return 0;
}

static int recv_data_from_socket(int sockfd, char * buf, int len)
{
    int size;
    if ((size = read(sockfd, buf, len)) < 0)
        return 0;
    return size;
}

static int recv_server_data(RussiaBlock * rb)
{
    CliRB * rb_c = (CliRB *)rb->type_data;
    char buf[NETBUF_SIZE], * b = buf;
    int cmd_size, len;

    len = recv_data_from_socket(rb_c->connfd, buf, NETBUF_SIZE);

    while (len >= sizeof(int) * 2
            && (cmd_size = msg_sizeof(*b)) >= 0
            && len - sizeof(int) * 2 >= cmd_size) {
        int size;
        char * t;
        int l = sizeof(int) * 2 + cmd_size;

        t = b;
        len -= l;
        b += l;
        while ((size = write_data_to_buf(rb_c->cmd_buf, t, l)) < l) {
            if (!rb->rb_ops->exec_misc_cmd(rb))
                return 0;
            l -= size;
            t += size;
        }
    }

    return 1;
}

static int recv_key_event(CliRB * rb_c)
{
    int cmd[2] = {MSGTYPE_EVENT};
    int key;

    while ((key = x_key_event()) >= 0) {
        if (rb_c->status & CLI_STATUS_READY) {
            int i;
            for (i = 0; i < EVENT_NUM; i++) {
                if (key == rb_c->event[i]) {
                    cmd[1] = i;
                    write_data_to_buf(rb_c->send_buf, (char *)cmd, sizeof(cmd));
                }
            }
        }
        else if (key == K_SPACE) {
            int t = MSGTYPE_READY;
            write_data_to_buf(rb_c->send_buf, (char *)&t, sizeof(t));
            rb_c->status |= CLI_STATUS_READY;
        }
    }

    return 0;
}

static int client_recv_player_data(RussiaBlock * rb)
{
    CliRB * rb_c = (CliRB *)rb->type_data;

    recv_server_data(rb);

    if (rb_c->status & CLI_STATUS_MATCH)
        recv_key_event(rb_c);

    return 1;
}

static void client_send_player_data(RussiaBlock * rb)
{
    CliRB * rb_c = (CliRB *)rb->type_data;
    char buf[NETBUF_SIZE];
    int len;

    if ((len = read_data_from_buf(rb_c->send_buf, buf, NETBUF_SIZE)) > 0)
        if (write(rb_c->connfd, buf, len) != len)
            fprintf(stderr, "send data fail.\n");
}

static Player * client_add_player(RussiaBlock * rb)
{
    CliRB * rb_c = (CliRB *)rb->type_data;
    NatPlayer * player;

    if (!rb->match[0]) {
        player = (NatPlayer *)malloc(sizeof(NatPlayer));
        player->io_handle = &io_handle_1;

        rb->match[0] = (Player *)player;
        rb->match[0]->status |= PLAYER_STATUS_WAIT;
    }
    else if (!rb->match[1]) {
        player = (NatPlayer *)malloc(sizeof(NatPlayer));
        player->io_handle = &io_handle_2;

        rb->match[1] = (Player *)player;
        rb->match[1]->status |= PLAYER_STATUS_WAIT;
    }
    else {
        int status = PLAYER_JOIN;
        player = 0;
        sleep(1);
        write(rb_c->connfd, (char *)&status, sizeof(status));
        rb->status |= STATUS_ADD_FINISH;
    }

    return (Player *)player;
}

static int client_init(RussiaBlock * rb, void * data)
{
    char * ip_addr = (char *)data;
    struct sockaddr_in servaddr = {0};
    int sockfd;
    CliRB * rb_c;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client");
        return 0;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9877);
    servaddr.sin_addr.s_addr = inet_addr(ip_addr);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("client");
        goto close_fd;
    }
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    rb_c = (CliRB *)malloc(sizeof(CliRB));
    rb_c->connfd = sockfd;
    rb_c->cmd_buf = (DataBuffer *)malloc_data_buf(CMDBUFSIZE);
    rb_c->send_buf = (DataBuffer *)malloc_data_buf(SENDBUFSIZE);
    rb_c->event = key_code;
    rb->type_data = rb_c;

    if (x_init_display(SCREEN_W, SCREEN_H) < 0) {
        fprintf(stderr, "init display fail.\n");
        return 0;
    }

    if (init_digit() < 0) {
        fprintf(stderr, "Cannt init digit.\n");
        return 0;
    }
    return 1;

close_fd:
    close(sockfd);
    return 0;
}
