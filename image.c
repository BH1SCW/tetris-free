#include <stdlib.h>

#include "image.h"

void fill_image(PixImage * bitmap, Color32 color, int x, int y, int w, int h)
{
    if (w == 0)
        w = bitmap->w - x;
    if (h == 0)
        h = bitmap->h- y;
    if (x + w > bitmap->w)
        w = bitmap->w - x;
    if (y + h > bitmap->h)
        h = bitmap->h - y;

    if (bitmap->pixelbytes == 2) {
        Color16 * buf = (Color16 *)(bitmap->pixels + bitmap->rowbytes*y + x*bitmap->pixelbytes);
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++)
                buf[x] = (Color16)color;
            buf += bitmap->w;
        }
    }
    else if (bitmap->pixelbytes == 4) {
        Color32 * buf = (Color32 *)(bitmap->pixels + bitmap->rowbytes*y + x*bitmap->pixelbytes);
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++)
                buf[x] = color;
            buf += bitmap->w;
        }
    }
}

static Color16 read_pixel16(PixImage * bitmap, int x, int y)
{
	pixel_t * line = bitmap->pixels + y * bitmap->rowbytes;
	Color16 pixel = 0;

	switch (bitmap->pixelbytes) {
		case 2:
			pixel =  ((Color32 *)line)[x];
		break;

		case 4:
		{
			Color32 t = ((Color32 *)line)[x];
			pixel = ((t&0xf80000)>>8) + ((t&0xfc00)>>5) + ((t&0xf8)>>3); 
		}
		break;
	}

	return pixel;
}

static void image_copy16(PixImage * dst_bitmap, int dx, int dy, PixImage * src_bitmap, int sx, int sy, int w, int h)
{
    int x, y;
    Color16 * dst_buf = (Color16 *)(dst_bitmap->pixels + dst_bitmap->rowbytes * dy + dx * dst_bitmap->pixelbytes);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++)
            dst_buf[x] = read_pixel16(src_bitmap, x, y);

        dst_buf += dst_bitmap->w;
    }
}

static Color32 read_pixel32(PixImage * bitmap, int x, int y)
{
	pixel_t * line = bitmap->pixels + y * bitmap->rowbytes;
	Color32 pixel;
	pixel_t * bytes = (pixel_t *)(&pixel);

	switch (bitmap->pixelbytes) {
		case 1:
		break;

		case 2:
		{
			Color16 t = ((Color16 *)line)[x];
			bytes[0] = (t << 3) & ~0x7;
			bytes[1] = (t >> 3) & ~0x4;
			bytes[2] = (t >> 8) & ~0x7;
			bytes[3] = 0;
		}
		break;

		case 4:
			return ((Color32 *)line)[x];
	}	

	return pixel;
}

static void image_copy32(PixImage * dst_bitmap, int dx, int dy, PixImage * src_bitmap, int sx, int sy, int w, int h)
{
    int x, y;
    Color32 * dst_buf = (Color32 *)(dst_bitmap->pixels + dst_bitmap->rowbytes * dy + dx * dst_bitmap->pixelbytes);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++)
            dst_buf[x] = read_pixel32(src_bitmap, x, y);

        dst_buf += dst_bitmap->w;
    }
}

void image_copy(PixImage * dst_bitmap, int dx, int dy, PixImage * src_bitmap, int sx, int sy, int w, int h)
{
    if (w == 0)
        w = src_bitmap->w;
    if (h == 0)
        h = src_bitmap->h;
    if (sx >= dst_bitmap->w)
        return;
    if (sy >= dst_bitmap->h)
        return;
    if (sx + w > src_bitmap->w)
        w = src_bitmap->w - sx;
    if (w > dst_bitmap->w - dx)
        w = dst_bitmap->w - dx;
    if (sy + h > src_bitmap->h)
        h = src_bitmap->h - sy;
    if (h > dst_bitmap->h - dy)
        h = dst_bitmap->h - dy;

    if (dst_bitmap->pixelbytes == 2)
        image_copy16(dst_bitmap, dx, dy, src_bitmap, sx, sy, w, h);
    else if (dst_bitmap->pixelbytes == 4)
        image_copy32(dst_bitmap, dx, dy, src_bitmap, sx, sy, w, h);
}

void clear_image(PixImage * bitmap)
{
	int i, n;

	if (!bitmap->pixels)
		return;

	n = bitmap->w * bitmap->h;

	if (bitmap->pixelbytes == 2) {
		Color16 * pixels16 = (Color16 *)bitmap->pixels;
		for (i = 0; i < n; i++)
			pixels16[i] = 0;
	}
	else if (bitmap->pixelbytes == 4) {
		Color32 * pixels32 = (Color32 *)bitmap->pixels;
		for (i = 0; i < n; i++)
			pixels32[i] = 0;	
	}
}


PixImage * create_image(int w, int h, int pixelbytes)
{
	PixImage * bitmap;

	if (!(bitmap = malloc(sizeof(PixImage))))
		return NULL;

	bitmap->w = w;
	bitmap->h = h;
	bitmap->pixelbytes = pixelbytes;
	bitmap->rowbytes = w * pixelbytes;
	bitmap->pixels = (pixel_t *)malloc(bitmap->rowbytes * h);
	if (bitmap->pixels == NULL) {
		free(bitmap);
		return NULL;
	}

	return bitmap;
}

void release_bitmap(PixImage * bitmap)
{
	if (bitmap->pixels) {
		free(bitmap->pixels);
		bitmap->pixels = NULL;
	}
	free(bitmap);
}
