#ifndef __COMMON_H
#define __COMMON_H

#define KEYEVENT_BUF        100
#define BUFFREE(buf)        (buf->buf_size - buf->len)
#define BUFEMPTY(buf)       (buf->len == 0)
#define CLEARBUF(buf) \
{ \
    (buf)->len = 0; \
    (buf)->start = 0; \
}

#include "list.h"

typedef unsigned char Uchar;
typedef int Mtime;
typedef struct _DataBuffer DataBuffer;

struct _DataBuffer {
    int buf_size;
    int start;
    int len;
    char * buf;
};

int read_data_from_buf(DataBuffer * netbuf, char * buf, int len);
int write_data_to_buf(DataBuffer * netbuf, char * buf, int len);
DataBuffer * malloc_data_buf(int size);
void release_data_buf(DataBuffer * buf);
#endif
