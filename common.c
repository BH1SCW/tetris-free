#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"

int write_data_to_buf(DataBuffer * netbuf, char * buf, int len)
{
    int size;

    if (len > netbuf->buf_size - netbuf->start - netbuf->len)
        len = netbuf->buf_size - netbuf->start - netbuf->len;

    if (netbuf->start + netbuf->len + len > netbuf->buf_size)
        size = netbuf->buf_size - (netbuf->start + netbuf->len);
    else
        size = len;

    memcpy(netbuf->buf + netbuf->start + netbuf->len, buf, size);

    if (len > size)
        memcpy(netbuf->buf, buf + size, len - size);

    netbuf->len += len;

    return len;
}

int read_data_from_buf(DataBuffer * netbuf, char * buf, int len)
{
    int size;

    if (netbuf->len == 0)
        return 0;

    if (len > netbuf->len)
        len = netbuf->len;

    if (netbuf->start + netbuf->len + len > netbuf->buf_size)
        size = netbuf->buf_size - (netbuf->start + netbuf->len);
    else
        size = len;

    memcpy(buf, netbuf->buf + netbuf->start, size);

    if (len > size) {
        netbuf->start = 0;
        memcpy(buf + size, netbuf->buf, len - size);
        size = len - size;
    }

    netbuf->len -= len;
    netbuf->start += size;

    if (netbuf->len == 0)
        netbuf->start = 0;

    return len;
}

DataBuffer * malloc_data_buf(int size)
{
    DataBuffer * buf;

    if (!(buf = (DataBuffer *)malloc(sizeof(DataBuffer) + size)))
        return 0;

    buf->buf_size = size;
    buf->start = 0;
    buf->len = 0;
    buf->buf = (char *)((DataBuffer *)buf + 1);

    return buf;
}

void release_data_buf(DataBuffer * buf)
{
    free(buf);
}
