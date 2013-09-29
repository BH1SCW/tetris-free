#ifndef __IMAGE_H
#define __IMAGE_H

typedef unsigned short Color16;
typedef unsigned long Color32;
typedef unsigned char pixel_t;
typedef struct {
    pixel_t * pixels;
    int pixelbytes;
    int rowbytes;
    int w, h;
} PixImage;

void fill_image(PixImage * image, Color32 color, int x, int y, int w, int h);
void image_copy(PixImage * dst_image, int dx, int dy, PixImage * src_image, int sx, int sy, int w, int h);
#endif
