#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "common.h"

int get_msec(void)
{
    struct timeval tp;
    struct timezone tzp;
    static Mtime secbase = 0, oldsec, oldusec;
    Mtime newsec, newusec, udiff;

    gettimeofday(&tp, &tzp);
    newsec = tp.tv_sec;
    newusec = tp.tv_usec;

    if (!secbase) {
        oldsec = newsec = secbase = tp.tv_sec;
        oldusec = newusec = tp.tv_usec;
        return tp.tv_usec / 1000;
    }

    if (newsec > oldsec)
        udiff = 1000000 - oldusec + newusec;
    else
        udiff = newusec - oldusec;

    if (udiff < 1000) {
        usleep(1000 - udiff);
    }

    gettimeofday(&tp, &tzp);
    oldsec = tp.tv_sec;
    oldusec = tp.tv_usec;

    return ((tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000);
}

int rand_id(void)
{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);
    srand(tp.tv_usec);
    return rand() % 7;
}
