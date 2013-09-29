#ifndef __NATIVE_H
#define __NATIVE_H

#include "mtime.h"
#include "list.h"

#define REMOTE_WIN_LEFT         (BLOCK_MAP_W * BLOCK_SIZE + 6 * BLOCK_SIZE)
#define REMOTE_WIN_TOP          0
#define BORDER_W                5

typedef struct _HandleIO {
    int left, top;
    int wid, hei;

    int event[6];
} HandleIO;

typedef struct {
    Player player;

    Uchar old_map[BLOCK_MAP_H][BLOCK_MAP_W];

    HandleIO * io_handle;
} NatPlayer;

extern RBOPS native_rb_single_ops;
extern RBOPS native_rb_ops;
extern IOOPS native_io_ops;
extern HandleIO io_handle_1;
extern HandleIO io_handle_2;
#endif
