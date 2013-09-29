/*主体代码*/

#include <stdio.h>
#include <stdlib.h>

#include "block.h"
#include "common.h"
#include "native.h"
#include "server.h"
#include "client.h"
#include "mtime.h"
#include "common.h"

/*所有形状的方块位图数组*/
Shape shapes[7][4] = {
    {{{{1, 1, 1, 1},
       {0, 0, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 4},
     {{{1, 0, 0, 0},
       {1, 0, 0, 0},
       {1, 0, 0, 0},
       {1, 0, 0, 0}}, 1},
     {{{1, 1, 1, 1},
       {0, 0, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 4},
     {{{1, 0, 0, 0},
       {1, 0, 0, 0},
       {1, 0, 0, 0},
       {1, 0, 0, 0}}, 1}},

    {{{{1, 1, 1, 0},
       {0, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{0, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 1, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{0, 1, 0, 0},
       {1, 1, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 0, 0, 0},
       {1, 1, 0, 0},
       {1, 0, 0, 0},
       {0, 0, 0, 0}}, 2}},

    {{{{1, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{1, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{1, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{1, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 2}},

    {{{{1, 1, 0, 0},
       {0, 1, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{0, 1, 0, 0},
       {1, 1, 0, 0},
       {1, 0, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{1, 1, 0, 0},
       {0, 1, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{0, 1, 0, 0},
       {1, 1, 0, 0},
       {1, 0, 0, 0},
       {0, 0, 0, 0}}, 2}},

    {{{{0, 1, 1, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 0, 0, 0},
       {1, 1, 0, 0},
       {0, 1, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{0, 1, 1, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 0, 0, 0},
       {1, 1, 0, 0},
       {0, 1, 0, 0},
       {0, 0, 0, 0}}, 2}},

    {{{{1, 1, 1, 0},
       {1, 0, 0, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 1, 0, 0},
       {0, 1, 0, 0},
       {0, 1, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{0, 0, 1, 0},
       {1, 1, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 0, 0, 0},
       {1, 0, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0}}, 2}},

    {{{{1, 1, 1, 0},
       {0, 0, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{0, 1, 0, 0},
       {0, 1, 0, 0},
       {1, 1, 0, 0},
       {0, 0, 0, 0}}, 2},
     {{{1, 0, 0, 0},
       {1, 1, 1, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}}, 3},
     {{{1, 1, 0, 0},
       {1, 0, 0, 0},
       {1, 0, 0, 0},
       {0, 0, 0, 0}}, 2}}
};

/* 设置方块落地后的凝固时间*/
static int set_glue_block(Player * player, Mtime time)
{
    int i;
    Block * blocks = player->blocks;

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;
        if (y + 1 == BLOCK_MAP_H || player->block_map[y+1][x]) {
            if (!player->solidify) {
                player->time = time;
                player->solidify = 1;
            }
            return 1;
        }
    }

    player->solidify = 0;
    player->time = 0;
    return 0;
}

/*输出(BLOCK_MAP_H * BLOCK_MAP_W)大小的位图到本地或是网络*/
static void out_win_map(Player * player, IOOPS * io_ops)
{
    int i;
    int x, y;
    Block * blocks = player->blocks;
    Uchar out_map[BLOCK_MAP_H][BLOCK_MAP_W];

    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
            out_map[y][x] = player->block_map[y][x];
        }
    }

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;
        out_map[y][x] = 1;
    }

    io_ops->out_win_map(player, out_map);
}

/*下落方块*/
static int block_fall(Player * player, int count, Mtime time)
{
    Block * blocks = player->blocks;
    int i, step = count;

    for (i = 0; i < 4; i++) {
        int j;
        int x = blocks[i].x;
        int y = blocks[i].y;

        for (j = 1; j <= step; j++) {
            if (y+j == BLOCK_MAP_H || player->block_map[y+j][x]) {
                step = j - 1;
                break;
            }
        }

        if (step == 0)
            break;
    }

    if (step > 0) {
        for (i = 0; i < 4; i++)
            blocks[i].y += step;
        player->y += step;
    }

    set_glue_block(player, time);
    return 1;
}

/*左移方块*/
static int block_left(Player * player)
{
    Block * blocks = player->blocks;
    int i;

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;

        if (x == 0 || player->block_map[y][x-1])
            return 0;
    }

    for (i = 0; i < 4; i++)
        blocks[i].x--;

    player->x--;

    return 1;
}

/*右移方块*/
static int block_right(Player * player)
{
    Block * blocks = player->blocks;
    int i;

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;

        if (x == (BLOCK_MAP_W - 1) || player->block_map[y][x+1])
            return 0;
    }

    for (i = 0; i < 4; i++)
        blocks[i].x++;

    player->x++;

    return 1;
}

/*从方块位图数组中拷贝一个方块位图*/
static void cp_shape_to_block(Block * blocks, int x, int y, Shape * shape)
{
    int i, k = 0;

    for (i = 0; i < 4 && k < 4; i++) {
        int j;
        for (j = 0; j < 4 && k < 4; j++) {
            if (shape->matrix[i][j] && k < 4) {
                blocks[k].x = x + j;
                blocks[k].y = y + i;
                k++;
            }
        }
    }
}

/*旋转方块*/
static int block_turn(Player * player)
{
    int i;
    int direct = (player->direct + 1) % 4;
    Block blocks[4];

    cp_shape_to_block(blocks, player->x, player->y, player->cur_shape + direct);

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;
        if (x >= BLOCK_MAP_W || y >= BLOCK_MAP_H || player->block_map[y][x])
            return 0;
    }
    player->direct = direct;
    for (i = 0; i < 4; i++) {
        player->blocks[i].x = blocks[i].x;
        player->blocks[i].y = blocks[i].y;
    }

    return 1;
}

/*从buf接收方向事件，buf中的方向事件可以从网络和本地获取*/
static int rb_event(Player * player)
{
    int event;

    if (read_data_from_buf(player->event_buf, (char *)&event, sizeof(event)) < sizeof(event))
        return -1;

    return event;
}

/*响应方向事件*/
static void player_response_event(Player * player, IOOPS * io_ops)
{
    int event;

    if ((event = rb_event(player)) < 0)
        return;

    switch (event) {
        case EVENT_UP:
            block_turn(player);
            set_glue_block(player, SOLIDIFY_TIME);
            break;

        case EVENT_DOWN:
            block_fall(player, BLOCK_MAP_H, 0);
            break;

        case EVENT_LEFT:
            block_left(player);
            set_glue_block(player, SOLIDIFY_TIME);
            break;

        case EVENT_RIGHT:
            block_right(player);
            set_glue_block(player, SOLIDIFY_TIME);
            break;

        case EVENT_SPACE:
        case EVENT_ESCAPE:
            break;
    }

    out_win_map(player, io_ops);

    return;
}

static void response_event(RussiaBlock * rb)
{
    int i;
    for (i = 0; i < 2; i++) if (rb->match[i])
        player_response_event(rb->match[i], rb->io_ops);

    return;
}

typedef struct _block_list_t {
    int x, y;
    struct _block_list_t * next;
} block_list_t;

typedef struct {
    int top;
    int bottom;
    block_list_t * head;
} block_group_t;

static void release_group_list(block_group_t * blocks_list)
{
    block_list_t * head = blocks_list->head;

    while (head) {
        block_list_t * tmp = head;
        head = tmp->next;
        free(tmp);
    }
}

/*消除行后用递归标记分隔的块组*/
static void mark_block(Uchar map[BLOCK_MAP_H][BLOCK_MAP_W], 
        block_group_t * blocks_list, int mark, int x, int y, int bottom)
{
    if (x < 0 || x == BLOCK_MAP_W || y < 0 || y == bottom)
        return;

    if (map[y][x] == 1) {
        block_list_t * list;

        list = (block_list_t *)malloc(sizeof(block_list_t));
        list->x = x;
        list->y = y;
        list->next = blocks_list->head;
        blocks_list->head = list;
        if (y < blocks_list->top)
            blocks_list->top = y;
        if (y > blocks_list->bottom)
            blocks_list->bottom = y;

        map[y][x] = mark;
        mark_block(map, blocks_list, mark, x + 1, y, bottom);
        mark_block(map, blocks_list, mark, x, y + 1, bottom);
        mark_block(map, blocks_list, mark, x - 1, y, bottom);
        mark_block(map, blocks_list, mark, x, y - 1, bottom);
    }
}

static int mark_block_group(Player * player, block_group_t * group_list, int * mark_nr)
{
    int y;
    int bottom = 0;
    int mark = 2;

    for (y = BLOCK_MAP_H - 1; y >= 0; y--) {
        int x, xx = 0;

        for (x = 0; x < BLOCK_MAP_W; x++)
            if (player->block_map[y][x] > 0) {
                player->block_map[y][x] = 1;
                xx++;
            }

        if (xx == BLOCK_MAP_W) {
            player->grade++;
            for (x = 0; x < BLOCK_MAP_W; x++)
                player->block_map[y][x] = 0;
            if (bottom == 0)
                bottom = y;
        }
    }

    while (bottom > 0) {
        int left = 0;
        int top = 0;
        for (; top < bottom; top++) {
            for (left = 0;
                    left < BLOCK_MAP_W
                    && player->block_map[top][left] != 1;
                    left++);
            if (left < BLOCK_MAP_W)
                break;
        }

        if (left == BLOCK_MAP_W)
            break;

        mark_block(player->block_map, &group_list[mark-2], mark, left, top, bottom);
        (*mark_nr)++;
        mark++;
    }
    if (mark > 2)
        return 1;
    return 0;
}

static void fall_list(Uchar map[BLOCK_MAP_H][BLOCK_MAP_W], block_group_t * blocks_list)
{
    int step = BLOCK_MAP_H;
    block_list_t * head = blocks_list->head;

    while (head) {
        int j;
        block_list_t * tmp = head;
        head = tmp->next;
        for (j = 1; j <= step; j++)
            if (tmp->y+j == BLOCK_MAP_H || map[tmp->y+j][tmp->x]) {
                step = j - 1;
                break;
            }
    }

    if (step > 0) {
        head = blocks_list->head;
        while (head) {
            block_list_t * tmp = head;
            head = tmp->next;
            tmp->y += step;
            map[tmp->y][tmp->x] = 1;
        }
    }
}

/*下落标记过的块组*/
static void fall_mark_group(Uchar map[BLOCK_MAP_H][BLOCK_MAP_W], block_group_t * blocks_list, int mark_nr)
{
    block_group_t * tmp[mark_nr];
    block_group_t * min;
    int i, y;

    for (y = 0; y < BLOCK_MAP_H; y++) {
        int x;
        for (x = 0; x < BLOCK_MAP_W; x++)
            if (map[y][x] > 1)
                map[y][x] = 0;
    }
    for (i = 0; i < mark_nr; i++)
        tmp[i] = blocks_list + i;

    for (i = 0; i < mark_nr; i++) {
        int j;
        int max = 0, top;
        for (j = 0; j < mark_nr; j++) {
            if (tmp[j] && tmp[j]->top > max) {
                max = tmp[j]->top;
                top = j;
            }
        }

        if (max == 0)
            break;
        min = tmp[top];
        tmp[top] = 0;

        fall_list(map, min);
    }
}

static void reset_block_map(Player * player)
{
    int i;
    int mark_nr = 0;
    block_group_t group_list[20];

    for (i = 0; i < 20; i++) {
        group_list[i].top = BLOCK_MAP_H;
        group_list[i].bottom = 0;
        group_list[i].head = 0;
    }

    if (mark_block_group(player, group_list, &mark_nr)) {
        fall_mark_group(player->block_map, group_list, mark_nr);
        for (i = 0; i < mark_nr; i++)
            release_group_list(group_list + i);

        reset_block_map(player);
    }
}

/*粘合正在凝固的方块*/
static void player_solidify_block(Player * player, Mtime time)
{
    int i;
    Block * blocks = player->blocks;

    if (player->solidify == 0)
        return;

    if (player->time > time) {
        player->time -= time;
        return;
    }

    for (i = 0; i < 4; i++) {
        int x = blocks[i].x;
        int y = blocks[i].y;
        player->block_map[y][x] = 1;
    }

    reset_block_map(player);
    player->time = 0;
    player->solidify = 0;
    player->cur_shape = NULL;
    return;
}

static inline void solidify_block(RussiaBlock * rb, Mtime time)
{
    int i;
    for (i = 0; i < 2; i++) if (rb->match[i])
        player_solidify_block(rb->match[i], time);
}

static int check_collide(Player * player)
{
    Block * blocks = player->blocks;
    int i;

    for (i = 0; i < 4; i++) {
        if (player->block_map[blocks[i].y][blocks[i].x])
            return 1;
    }

    return 0;
}

void cp_next_shape(Player * player)
{
    int x, y;
    Shape * shape = player->next_shape;

    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++)
            player->next_matrix[y][x] = shape->matrix[y][x];
	}
}

static void player_frame_start(Player * player, IOOPS * io_ops)
{
}

static void frame_start(RussiaBlock * rb)
{
    int i;
    for (i = 0; i < 2; i++) if (rb->match[i])
        player_frame_start(rb->match[i], rb->io_ops);
}

static int player_frame_end(Player * player, IOOPS * io_ops)
{
    if (player->cur_shape)
        return 1;

    if (player->grade > player->old_grade) {
        if (player->grade / BLOCK_GRADE && !(player->grade % BLOCK_GRADE)) {
            player->speed *= 0.9;
            player->level++;
        }
        io_ops->out_grade(player);
        player->old_grade = player->grade;
    }

    player->cur_shape = player->next_shape;
    player->next_shape = shapes[rand_id()];
    cp_next_shape(player);

    io_ops->out_next_block(player);

    player->direct = 0;
    player->x = BLOCK_MAP_W / 2 - player->cur_shape->w / 2;
    player->y = 0;

    cp_shape_to_block(player->blocks, player->x, player->y, player->cur_shape);

    if (check_collide(player))
        return 0;

    return 1;
}

static inline void set_player_status(Player * player, int status, int set)
{
    if (set)
        player->status |= status;
    else
        player->status &= ~status;
}

static inline int get_player_status(Player * player, int status)
{
    return player->status & status;
}

void init_player(RussiaBlock * rb, Player * player)
{
    player->event_buf = (DataBuffer *)malloc_data_buf(KEYEVENT_BUF);
    player->old_grade = -1;
    player->grade = 0;
    player->speed = BLOCK_INIT_SPEED;
    player->slice = 0;
    player->cur_shape = 0;
    player->next_shape = shapes[rand_id()];
    cp_next_shape(player);

    if (rb->io_ops)
        rb->io_ops->init(player);

    return;
}

static void frame_end(RussiaBlock * rb)
{
    switch (rb->type) {
        case GAMETYPE_ALONE_SINGLE:
            if (!player_frame_end(rb->match[0], rb->io_ops))
                rb->status &= ~STATUS_RUN;
        break;

        case GAMETYPE_ALONE_MATCH:
			if (!player_frame_end(rb->match[0], rb->io_ops) || 
					!player_frame_end(rb->match[1], rb->io_ops)) {
                rb->status &= ~STATUS_RUN;
			}
		break;

        case GAMETYPE_COMPUTER_MATCH:
        case GAMETYPE_ONLINE_SERVER:
        {
            int i;
 
            for (i = 0; i < 2; i++) {
                if (!player_frame_end(rb->match[i], rb->io_ops)) {
                    set_player_status(rb->match[i], PLAYER_STATUS_LOSE, 1);
                    set_player_status(rb->match[i], PLAYER_READY, 0);
                    if (rb->rb_ops->del_match)
                        rb->rb_ops->del_match(rb, rb->match[i]);
                    rb->status &= ~STATUS_RUN;
                }
            }

        }
        break;
    }
}

static void clock_block_fall(RussiaBlock * rb, Mtime time)
{
    int c, i;

    for (i = 0; i < 2; i++) {
        if (rb->match[i]) {
            rb->match[i]->slice += time;
            if ((c = rb->match[i]->slice / rb->match[i]->speed) > 0) {
                block_fall(rb->match[i], c, SOLIDIFY_TIME);
                out_win_map(rb->match[i], rb->io_ops);
                rb->match[i]->slice %= rb->match[i]->speed;
            }
        }
    }

    return;
}

RussiaBlock * new_game(int type, void * data)
{
    RussiaBlock * rb;

    if (!(rb = (RussiaBlock *)calloc(1, sizeof(RussiaBlock))))
        return NULL;

    rb->type = type;

    switch (type) {
    case GAMETYPE_ALONE_MATCH:
        /*重定向输入输出到本地双人模式*/
        rb->rb_ops = &native_rb_ops;
        rb->io_ops = &native_io_ops;
    break;

    case GAMETYPE_ALONE_SINGLE:
        /*重定向输入输出到本地单人模式*/
        rb->rb_ops = &native_rb_single_ops;
        rb->io_ops = &native_io_ops;
    break;

    case GAMETYPE_COMPUTER_MATCH:
    break;

    case GAMETYPE_ONLINE_SERVER:
        /*重定向输入输出到网络服务器模式*/
        rb->rb_ops = &server_rb_ops;
        rb->io_ops = &server_io_ops;
    break;

    case GAMETYPE_ONLINE_CLIENT:
        /*重定向输入输出到网络客户端模式*/
        rb->rb_ops = &client_rb_ops;
        rb->io_ops = &native_io_ops;
    break;
    }

    if (rb->rb_ops && rb->rb_ops->init(rb, data))
        return rb;
    return NULL;
}

/*填加新玩家*/
void add_player(RussiaBlock * rb)
{
    Player * player;
    RBOPS * rb_ops;

    if ((rb->status & STATUS_ADD_FINISH) || !(rb_ops = rb->rb_ops))
        return;

    if (rb_ops->add_player && (player = rb_ops->add_player(rb))) {
        init_player(rb, player);
        if (rb_ops->add_match)
            rb_ops->add_match(rb);
    }

    return;
}

int ready_event(Player * player)
{
    if (rb_event(player) == EVENT_SPACE)
		return 1;

	return 0;
}

void game_frame(RussiaBlock * rb, Mtime time)
{
    frame_start(rb);

    clock_block_fall(rb, time);

    response_event(rb);

    solidify_block(rb, time);

    frame_end(rb);
}
