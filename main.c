#include <stdio.h>
#include <unistd.h>

#include "block.h"
#include "frame.h"
#include "mtime.h"


int main(int argc, char * argv[])
{
    Mtime newtime, oldtime;
    char ch;
    int type = GAMETYPE_ALONE_SINGLE;
    const char * addr = NULL;

    while ((ch = getopt(argc, argv, "sc:ow")) != -1)
    switch (ch) {
        case 'o':
            type = GAMETYPE_ALONE_SINGLE;
            break;

        case 'w':
            type = GAMETYPE_ALONE_MATCH;
            break;

        case 's':
            type = GAMETYPE_ONLINE_SERVER;
            addr = NULL;
            break;

        case 'c':
            type = GAMETYPE_ONLINE_CLIENT;
            addr = optarg;
            break;
    }

    if (!(russia_block = new_game(type, (void *)addr)))
        return 1;

    oldtime = get_msec();
    while (!(russia_block->status & STATUS_EXIT)) {
        newtime = get_msec();

        clock_frame(newtime - oldtime);

        oldtime = newtime;
    }

    return 0;
}
