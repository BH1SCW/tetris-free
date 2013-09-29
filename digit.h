#ifndef __DIGIT_H
#define __DIGIT_H

#include "image.h"

#define DIGIT_W             13
#define DIGIT_H             23
#define SPACE               5
#define WHOLE_DIGIT_W       (DIGIT_W*3 + SPACE*2)
#define WHOLE_DIGIT_H       DIGIT_H
#define DIGIT_MAP_SIZE      (DIGIT_W * DIGIT_H)

#define DIGIT_COLOR_16      0xff00
#define DIGIT_COLOR_32      0xff00ff

int init_digit(void);
PixImage * digit_to_image(int digit);

#endif
