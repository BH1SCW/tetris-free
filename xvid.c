/*X11ДњТы*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "common.h"
#include "block.h"
#include "xvid.h"
#include "image.h"
#include "digit.h"

PixImage screen_image = {0, 0, 0, SCREEN_W, SCREEN_H};
int vid_xpos = 0;
int vid_ypos = 0;
static Display * dpy;
static GC x_gc;
static Window x_win;
static XVisualInfo * visual_info;
static XImage * x_fb;

static XImage * reset_frame_buf(XVisualInfo * vinfo)
{
    XImage * fb;
	void * mem;

    if ((screen_image.pixelbytes = vinfo->depth / 8) == 3)
        screen_image.pixelbytes = 4;

	mem = malloc(((screen_image.w * screen_image.pixelbytes + 7) & ~7) * screen_image.h);

	fb = XCreateImage(dpy, vinfo->visual, vinfo->depth, ZPixmap,
                        0, mem, screen_image.w, screen_image.h, 32, 0);

	screen_image.rowbytes = fb->bytes_per_line;
	screen_image.pixels = (pixel_t *)fb->data;

    return fb;
}

static Window create_x_window(XVisualInfo * vinfo, Window root)
{
    XSetWindowAttributes attribs;
    Colormap tmpcmap;
    Window win;

    tmpcmap = XCreateColormap(dpy, root, vinfo->visual, AllocNone);

    attribs.event_mask = KeyPressMask;
    attribs.border_pixel = 0;
    attribs.colormap = tmpcmap;

    win = XCreateWindow(dpy, root,
            WIN_XPOS, WIN_YPOS,
            screen_image.w, screen_image.h,
            0,
            vinfo->depth,
            InputOutput,
            vinfo->visual,
            CWEventMask | CWColormap | CWBorderPixel, 
            &attribs);

    XStoreName(dpy, win, "RussiaBlock");

    if (vinfo->class != TrueColor)
        XFreeColormap(dpy, tmpcmap);

    return win;
}

int x_init_display(int w, int h)
{
	int vnum;
	XVisualInfo vinfo;
	Window root;

	if (!(dpy = XOpenDisplay(0))) {
        printf("Could not open display [%s]\n", getenv("DISPLAY"));
        return -1;
	}

	XSynchronize(dpy, True);

    vinfo.visualid = XVisualIDFromVisual(XDefaultVisual(dpy, XDefaultScreen(dpy)));

	visual_info = XGetVisualInfo(dpy, VisualIDMask, &vinfo, &vnum);
	if (vnum == 0) {
        printf("Bad visual id %d\n", (int)vinfo.visualid);
        return -1;
	}

	root = XRootWindow(dpy, visual_info->screen);
	screen_image.w = w;
	screen_image.h = h;
    x_win = create_x_window(visual_info, root);

	{
		XGCValues xgcvalues;
		xgcvalues.graphics_exposures = False;
		x_gc = XCreateGC(dpy, x_win, GCGraphicsExposures, &xgcvalues);
	}

	XMapWindow(dpy, x_win);
	XMoveWindow(dpy, x_win, WIN_XPOS, WIN_YPOS);

    x_fb = reset_frame_buf(visual_info);

	return 0;
}

static int trans_key(XKeyEvent * event)
{
	int key = -1;
	KeySym keysym;
	char buf[100];

	XLookupString(event, buf, 100, &keysym, 0);

	switch (keysym) {
		case XK_Left:   key = K_LEFT;   break;
		case XK_Right:  key = K_RIGHT;  break;
		case XK_Down:   key = K_DOWN;   break;
		case XK_Up:     key = K_UP;     break;
		case XK_space:  key = K_SPACE;  break;
		case XK_Escape: key = K_ESCAPE; break;

        default:
            key = *(unsigned char *)buf;
            if (key >= 'A' && key <= 'Z')
                key = key - 'A' + 'a';
	}

	return key;
}

int x_key_event(void)
{
    int key = -1;
    XEvent event;

    while (XPending(dpy)) {
        XNextEvent(dpy, &event);
        if (event.type == KeyPress)
            key = trans_key(&event.xkey);
    }

    return key;
}

void x_paint(void)
{
    XPutImage(dpy, x_win, x_gc, x_fb, 0, 0, 0, 0, screen_image.w, screen_image.h);
    XSync(dpy, False);
}
