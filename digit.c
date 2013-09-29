/*功能:显示数字*/
#include <stdlib.h>

#include "digit.h"

static const char * digit_str = {
    " 11111111111 "
    " 11111111111 "
    "2 111111111 3"
    "22         33"
    "222       333"
    "222       333"
    "222       333"
    "222       333"
    "222       333"
    "22         33"
    "2 444444444 3"
    " 44444444444 "
    "5 444444444 6"
    "55         66"
    "555       666"
    "555       666"
    "555       666"
    "555       666"
    "555       666"
    "55         66"
    "5 777777777 6"
    " 77777777777 "
    " 77777777777 "
};

static char digit_mask[10][7] = {
    {1, 2, 3, 5, 6, 7, 0},
    {3, 6, 0, 0, 0, 0, 0},
    {1, 3, 4, 5, 7, 0, 0},
    {1, 3, 4, 6, 7, 0, 0},
    {2, 3, 4, 6, 0, 0, 0},
    {1, 2, 4, 6, 7, 0, 0},
    {1, 2, 4, 5, 6, 7, 0},
    {1, 3, 6, 0, 0, 0, 0},
    {1, 2, 3, 4, 5, 6, 7},
    {1, 2, 3, 4, 6, 7, 0},
};

extern PixImage screen_image;
static PixImage digit_image;
static PixImage whole_digit_image;
static char digit_map[DIGIT_MAP_SIZE] = {0};
int init_digit(void)
{
    int i;

    for (i = 0; i < DIGIT_MAP_SIZE; i++) {
        if (digit_str[i] == ' ')
            digit_map[i] = 0;
        else
            digit_map[i] = digit_str[i] - '0';
    }

    digit_image.w = DIGIT_W;
    digit_image.h = DIGIT_H;
    digit_image.pixelbytes = screen_image.pixelbytes;
    digit_image.rowbytes = DIGIT_W * screen_image.pixelbytes;
    digit_image.pixels = (pixel_t *)malloc(DIGIT_H * digit_image.rowbytes); 

    whole_digit_image.w = WHOLE_DIGIT_W;
    whole_digit_image.h = WHOLE_DIGIT_H;
    whole_digit_image.pixelbytes = screen_image.pixelbytes;
    whole_digit_image.rowbytes = WHOLE_DIGIT_W * screen_image.pixelbytes;
    whole_digit_image.pixels = (pixel_t *)malloc(WHOLE_DIGIT_H * whole_digit_image.rowbytes);

    return 0;
}

static void paint_pos_digit(int pos, int digit);
PixImage * digit_to_image(int digit)
{
    int i;

    fill_image(&whole_digit_image, 0, 0, 0, 0, 0);

    if (digit > 0) {
        for (i = 0; digit > 0; i++) {
            paint_pos_digit(i, digit % 10);
            digit /= 10;
        }
    }
    else
        paint_pos_digit(0, 0);

    return &whole_digit_image;
}

static void digit_map_to_image(PixImage * image, const char * map, Color32 color)
{
    int i;
    pixel_t * buf = image->pixels;

    if (image->pixelbytes == 2) {
        for (i = 0; i < DIGIT_H * DIGIT_W; i++) {
            if (map[i])
                ((Color16 *)buf)[i] = (Color16)color;
            else
                ((Color16 *)buf)[i] = 0;
        }
    }
    else if (image->pixelbytes == 4) {
        for (i = 0; i < DIGIT_H * DIGIT_W; i++) {
            if (map[i])
                ((Color32 *)buf)[i] = color;
            else
                ((Color32 *)buf)[i] = 0;
        }
    }
}

static void paint_pos_digit(int pos, int digit)
{
    int x, i, j;
    char map[DIGIT_MAP_SIZE] = {0};
    char * mask = digit_mask[digit];

    fill_image(&digit_image, 0, 0, 0, 0, 0);

    for (i = 0; i < 7 && mask[i] > 0; i++) {
        for (j = 0; j < DIGIT_MAP_SIZE; j++) {
            if (digit_map[j] == mask[i])
                map[j] = 1;
        }
    }

    digit_map_to_image(&digit_image, map, DIGIT_COLOR_32);
    x = WHOLE_DIGIT_W - DIGIT_W - (SPACE+DIGIT_W) * pos;
    image_copy(&whole_digit_image, x, 0, &digit_image, 0, 0, 0, 0);
}
