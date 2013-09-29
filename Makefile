CC = gcc

LDFLAGS=-L/usr/X11R6/lib
CFLAGS=-Wall -g
all:block

block:main.c block.o frame.o mtime.o xvid.o image.o digit.o native.o common.o server.o client.o
	${CC} ${CFLAGS} -o block $^ ${LDFLAGS} -lX11

frame.o:frame.c block.h
	${CC} ${CFLAGS} -c frame.c

mtime.o:mtime.c common.h
	${CC} ${CFLAGS} -c mtime.c

block.o:block.c block.h common.h native.h server.h
	${CC} ${CFLAGS} -c block.c

image.o:image.c image.h
	${CC} ${CFLAGS} -c image.c

xvid.o:xvid.c common.h xvid.h block.h image.h digit.h
	${CC} ${CFLAGS} -c xvid.c

digit.o:digit.c digit.h image.h
	${CC} ${CFLAGS} -c digit.c

native.o:native.c xvid.h block.h image.h digit.h
	${CC} ${CFLAGS} -c native.c

common.o:common.c common.h list.h
	${CC} ${CFLAGS} -c common.c

server.o:server.c server.h block.h
	${CC} ${CFLAGS} -c server.c

client.o:client.c client.h
	${CC} ${CFLAGS} -c client.c

clean:
	rm -f *.o
	rm -f block
