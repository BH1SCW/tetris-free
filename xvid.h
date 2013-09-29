#ifndef _XVID_H
#define _XVID_H

//#include "block.h"
//#include "digit.h"

#define     SCREEN_W            ((BLOCK_MAP_W * BLOCK_SIZE + 6 * BLOCK_SIZE) * 2)
#define     SCREEN_H            (BLOCK_MAP_H * BLOCK_SIZE)
#define     WIN_XPOS            0
#define     WIN_YPOS            0

#define     K_LEFT             1
#define     K_RIGHT            2
#define     K_DOWN             3
#define     K_UP               4
#define     K_SPACE            5
#define     K_ESCAPE           6

//extern PixImage screen_image;

int x_init_display(int w, int h);
void set_map_block(int x, int y, char value);
int key_event(void);
void clear_screen(void);
void xvid_paint(void);
void x_paint(void);
int x_key_event(void);

#endif
