#include <stdio.h>

#include "block.h"

RussiaBlock * russia_block = NULL;

static void add_match(RussiaBlock * rb)
{
    RBOPS * rb_ops = rb->rb_ops;

    if (rb_ops->add_match)
        rb_ops->add_match(rb);
}

static inline int match_ready(RussiaBlock * rb)
{
    return ((rb->match[0]->status & PLAYER_READY)
                && (rb->match[1]->status & PLAYER_READY));
}

static void init_match_player(RussiaBlock * rb)
{
    int i;
    for (i = 0; i < 2; i++)
        init_player(rb, rb->match[i]);
}

static void game_start(RussiaBlock * rb)
{
    switch (rb->type) {
    case GAMETYPE_ALONE_SINGLE:
        if (ready_event(rb->match[0])) {
			init_player(rb, rb->match[0]);
            rb->status |= STATUS_RUN;
		}
        else
            rb->status &= ~STATUS_RUN;
    break;

    case GAMETYPE_ALONE_MATCH:
    {
		if (ready_event(rb->match[0])) {
			init_player(rb, rb->match[0]);
			init_player(rb, rb->match[1]);
            rb->status |= STATUS_RUN;
		}
        else
            rb->status &= ~STATUS_RUN;
    }
    break;

    case GAMETYPE_COMPUTER_MATCH:
    break;

    case GAMETYPE_ONLINE_SERVER:
        if (!(rb->match[0] && rb->match[1])) {
            add_match(rb);
		}
        else if (match_ready(rb)) {
            rb->status |= STATUS_RUN;
            init_match_player(rb);
        }
        else
            rb->status &= ~STATUS_RUN;
    break;

    case GAMETYPE_ONLINE_CLIENT:
    break;
    }
}

/*接收玩家数据(本地主要是控制事件, 网络包括输入输出等信息)*/
static void recv_player_data(RussiaBlock * rb)
{
    RBOPS * rb_ops = rb->rb_ops;

    if (rb_ops->recv_player_data)
        rb_ops->recv_player_data(rb);
}

/*发送玩家数据件*/
static void send_player_data(RussiaBlock * rb)
{
    RBOPS * rb_ops = rb->rb_ops;

    if (rb_ops->send_player_data)
        rb_ops->send_player_data(rb);

    return;
}

/*刷新屏幕*/
static int refresh_screen(RussiaBlock * rb, Mtime time)
{
    IOOPS * io_ops = rb->io_ops;

    if (io_ops->refresh_screen)
        io_ops->refresh_screen(time);

    return 0;
}

static int exec_misc_cmd(RussiaBlock * rb)
{
    if (rb->rb_ops->exec_misc_cmd)
        rb->rb_ops->exec_misc_cmd(rb);

    return 0;
}

void clock_frame(Mtime time)
{
    add_player(russia_block);

    recv_player_data(russia_block);

    if (!(russia_block->status & STATUS_RUN))
        game_start(russia_block);
    else
        game_frame(russia_block, time);

    send_player_data(russia_block);

    exec_misc_cmd(russia_block);

    refresh_screen(russia_block, time);

    return;
}

