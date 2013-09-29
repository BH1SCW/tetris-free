/* 本地输入输出代码*/

#include <stdio.h>
#include <stdlib.h>

#include "block.h"
#include "xvid.h"
#include "native.h"
#include "image.h"
#include "digit.h"

extern PixImage screen_image;

static int native_init(RussiaBlock * rb, void * data);
static int native_single_init(RussiaBlock * rb, void * data);
static Player * native_add_player(RussiaBlock * rb);
static Player * native_add_single_player(RussiaBlock * rb);
static void native_refresh_screen(Mtime time);
static int native_recv_player_data(RussiaBlock * rb);

static int native_io_init(Player * player);
static void native_io_out_grade(Player * player);
static void native_io_out_next_block(Player * player);
static void native_io_out_win_map(Player * player, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W]);


HandleIO io_handle_1 = {0, 0, 0, 0, {K_LEFT, K_RIGHT, K_DOWN, K_UP, K_SPACE, K_ESCAPE}};
HandleIO io_handle_2 = {REMOTE_WIN_LEFT, 0, 0, 0, {'a', 'd', 's', 'w', 'e', 'b'}};

IOOPS native_io_ops = {
init:               native_io_init,
out_grade:          native_io_out_grade,
out_next_block:     native_io_out_next_block,
out_win_map:        native_io_out_win_map,
refresh_screen:     native_refresh_screen,
//reset_out:          native_io_reset_win, 
};

RBOPS native_rb_ops = {
init:                   native_init,
add_player:             native_add_player,
add_match:              NULL,
del_match:              NULL,
recv_player_data:       native_recv_player_data,
send_player_data:       NULL,
exec_misc_cmd:          NULL,
};

RBOPS native_rb_single_ops = {
init:                   native_single_init,
add_player:             native_add_single_player,
add_match:              NULL,
del_match:              NULL,
recv_player_data:       native_recv_player_data,
send_player_data:       NULL,
exec_misc_cmd:          NULL,
};

static void set_block_image(int x, int y, int left, int top, char value)
{
    Color32 color = value ? 0x00ff00ff : 0;
    int pixel_x = x * BLOCK_SIZE + left + BORDER_W;
    int pixel_y = y * BLOCK_SIZE + top;

    fill_image(&screen_image, color, pixel_x, pixel_y, BLOCK_SIZE, BLOCK_SIZE);
}

static void matrix_to_screen(Uchar matrix[4][4], int left, int top)
{
    int y, x;

    for (y = 0; y < 4; y++)
        for (x = 0; x < 4; x++)
            set_block_image(x, y, left, top, matrix[y][x]);
}

static void native_io_out_next_block(Player * player)
{
    NatPlayer * np = (NatPlayer *)player;
    HandleIO * handle = np->io_handle;

    matrix_to_screen(player->next_matrix,
            handle->left + (BLOCK_MAP_W + 1) * BLOCK_SIZE,
            handle->top);
}

static void native_io_out_grade(Player * player)
{
    PixImage * image;
    NatPlayer * np = (NatPlayer *)player;
    HandleIO * handle = np->io_handle;
    int left = handle->left;

    image = digit_to_image(player->grade);
    image_copy(&screen_image, left + BLOCK_MAP_W * BLOCK_SIZE + WHOLE_DIGIT_W + 10, 150, image, 0, 0, 0, 0);

    if (!(player->grade % BLOCK_GRADE)) {
        image = digit_to_image(player->grade / BLOCK_GRADE + 1);
        image_copy(&screen_image,
                left + BLOCK_MAP_W * BLOCK_SIZE + WHOLE_DIGIT_W + 10,
                handle->top + 200, image, 0, 0, 0, 0);
    }
}

static void native_io_out_win_map(Player * player, Uchar map[BLOCK_MAP_H][BLOCK_MAP_W])
{
    NatPlayer * np = (NatPlayer *)player;
    HandleIO * handle = np->io_handle;
    int x, y;

    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
            if (np->old_map[y][x] != map[y][x]) {
                np->old_map[y][x] = map[y][x];
                set_block_image(x, y, handle->left, handle->top, map[y][x]);
            }
        }
    }
}

static int native_io_init(Player * player)
{
    NatPlayer * np = (NatPlayer *)player;
    HandleIO * handle = np->io_handle;
    int x, y;

    for (y = 0; y < BLOCK_MAP_H; y++) {
        for (x = 0; x < BLOCK_MAP_W; x++) {
			player->block_map[y][x] = 0;
            np->old_map[y][x] = 0;
		}
    }

    fill_image(&screen_image,
            0xff,
            handle->left,
            handle->top,
            5,
            BLOCK_MAP_H * BLOCK_SIZE);

    fill_image(&screen_image,
            0xff,
            handle->left + BLOCK_MAP_W * BLOCK_SIZE + BORDER_W,
            handle->top,
            5,
            BLOCK_MAP_H * BLOCK_SIZE);

    fill_image(&screen_image,
            0x0,
            handle->left + BORDER_W,
            handle->top,
            BLOCK_MAP_W * BLOCK_SIZE,
            BLOCK_MAP_H * BLOCK_SIZE);

    return 0;
}

static void native_refresh_screen(Mtime time)
{
    static Mtime slice = 0;

    slice += time;
    if ((slice / 20) == 0)
        return;
    slice %= 20;

    x_paint();
    return;
}

static int native_recv_player_data(RussiaBlock * rb)
{
    int key;

    while ((key = x_key_event()) >= 0) {
        int p;
        for (p = 0; p < 2; p++) {
            int i;
            HandleIO * handle;
            if (!rb->match[p])
                continue;
            handle = ((NatPlayer *)rb->match[p])->io_handle;
            for (i = 0; i < EVENT_NUM; i++)
                if (rb->match[p] && key == handle->event[i])
                    write_data_to_buf(rb->match[p]->event_buf, (char *)&i, sizeof(i));
        }
    }

    return 0;
}

static Player * native_add_single_player(RussiaBlock * rb)
{
    NatPlayer * player;

	if (rb->status & STATUS_ADD_FINISH)
		return 0;

    if (!rb->match[0]) {
        player = (NatPlayer *)calloc(1, sizeof(NatPlayer));
        player->io_handle = &io_handle_1;

        rb->match[0] = (Player *)player;
        rb->match[0]->status |= PLAYER_READY;
    }
    else {
        player = 0;
        rb->status |= STATUS_ADD_FINISH;
    }

    return (Player *)player;
}

static Player * native_add_player(RussiaBlock * rb)
{
    NatPlayer * player;

	if (rb->status & STATUS_ADD_FINISH)
		return 0;

    if (!rb->match[0]) {
        player = (NatPlayer *)calloc(1, sizeof(NatPlayer));
        player->io_handle = &io_handle_1;

        rb->match[0] = (Player *)player;
        rb->match[0]->status |= PLAYER_READY;
    }
    else if (!rb->match[1]) {
        player = (NatPlayer *)calloc(1, sizeof(NatPlayer));
        player->io_handle = &io_handle_2;

        rb->match[1] = (Player *)player;
        rb->match[1]->status |= PLAYER_READY;
    }
    else {
        player = 0;
        rb->status |= STATUS_ADD_FINISH;
    }

    return (Player *)player;
}

static int native_init(RussiaBlock * rb, void * data)
{
    if (x_init_display(SCREEN_W, SCREEN_H) < 0) {
        fprintf(stderr, "init display fail.\n");
        return 0;
    }

    if (init_digit() < 0) {
        fprintf(stderr, "Cannt init digit.\n");
        return 0;
    }

    return 1;
}

static int native_single_init(RussiaBlock * rb, void * data)
{
    if (x_init_display(SCREEN_W / 2, SCREEN_H) < 0) {
        fprintf(stderr, "init display fail.\n");
        return 0;
    }

    if (init_digit() < 0) {
        fprintf(stderr, "Cannt init digit.\n");
        return 0;
    }

    return 1;
}
