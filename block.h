#ifndef __BLOCK_H
#define __BLOCK_H

#include "common.h"

#define PLAYER_STATUS_MATCH                 (0x1 << 0)
#define PLAYER_STATUS_WAIT                  ~(PLAYER_STATUS_MATCH)
#define PLAYER_READY                        (0x1 << 1)
#define PLAYER_STATUS_LOSE                  (0x1 << 2)
#define PLAYER_STATUS_WIN                   ~(PLAYER_STATUS_WIN)

#define STATUS_RUN                          (0x1 << 0)
#define STATUS_EXIT                         (0x1 << 1)
#define STATUS_ADD_FINISH                   (0x1 << 2)

#define GAMETYPE_ALONE_MATCH                1
#define GAMETYPE_ALONE_SINGLE               2
#define GAMETYPE_COMPUTER_MATCH             3
#define GAMETYPE_ONLINE_SERVER              4
#define GAMETYPE_ONLINE_CLIENT              5

#define BLOCK_MAP_W         12
#define BLOCK_MAP_H         20
#define BLOCK_SIZE          20
#define SOLIDIFY_TIME       400
#define BLOCK_GRADE         5
#define BLOCK_INIT_SPEED    500


#define     EVENT_NUM               6
#define     EVENT_LEFT              0
#define     EVENT_RIGHT             1
#define     EVENT_DOWN              2
#define     EVENT_UP                3
#define     EVENT_SPACE             4
#define     EVENT_ESCAPE            5

#define PLAYER_JOIN         0

typedef struct _Player          Player;
typedef struct _RBOPS           RBOPS;
typedef struct _RussiaBlock     RussiaBlock;

typedef Uchar Matrix[4][4];

typedef struct {
    int x, y;
} Block;

typedef struct {
    Uchar matrix[4][4];
    int w;
} Shape;

struct _Player {
    int type;
    int status;

    int x, y;
    int direct;

    int grade, old_grade;
    int level;

    int solidify;

    Shape * cur_shape;
    Shape * next_shape;

    Mtime time;
    Mtime speed;
    Mtime slice;

    Block blocks[4];
    Uchar next_matrix[4][4];
    Uchar block_map[BLOCK_MAP_H][BLOCK_MAP_W];

    DataBuffer * event_buf;
};

typedef struct _InOut {
    int (* init)(Player * player);
    void (* out_grade)(Player * player);
    void (* out_next_block)(Player * player);
    void (* out_win_map)(Player * player, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W]);
    void (* refresh_screen)(Mtime time);
} IOOPS;

struct _RBOPS {
    int (* init)(RussiaBlock * rb, void * data);
    int (* native_init)(RussiaBlock * rb);
    Player * (* add_player)(RussiaBlock * rb);
    void (* add_match)(RussiaBlock * rb);
    void (* del_match)(RussiaBlock * rb, Player * player);
    int (* recv_player_data)(RussiaBlock * rb);
    void (* send_player_data)(RussiaBlock * rb);
    int (* exec_misc_cmd)(RussiaBlock * rb);
};

struct _RussiaBlock {
    int type;
    int status;

    Player * match[2];

    DataBuffer * cmd_buf;

    IOOPS * io_ops;
    RBOPS * rb_ops;

    void * type_data;
};

extern Shape shapes[7][4];
void cp_next_shape(Player * player);
void add_player(RussiaBlock * rb);
RussiaBlock * new_game(int type, void * data);
void game_frame(RussiaBlock * rb, Mtime time);
void init_player(RussiaBlock * rb, Player * player);
int ready_event(Player * player);
#endif
