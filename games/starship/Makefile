DESTDIR=
# @(#)Makefile	1.0	ULTRIX	4/23/85
#
#
DESTROOT=
CFLAGS=-O -DDRONE -DSHMEM
SFLAGS=-O -DDRONE
# debug flags
# CFLAGS=-DDEBUG -DDRONE -DSHMEM -g 
LFLAGS=-h

SOURCES=main.c cmd.c screen.c action.c drone.c starsleep.c
OBJS=main.o cmd.o screen.o action.o drone.o starsleep.o

all: starship rmstar singleship

starship: ${OBJS}
	cc ${CFLAGS} -o starship ${OBJS} -lcurses -ltermlib

singleship: ${OBJS}
	cc ${SFLAGS} -o singleship ${OBJS} -lcurses -ltermlib

rmstar: rmstar.o
	cc -o rmstar rmstar.o
rmstar.o: star.h

lint: ${OBJS}
	lint ${LFLAGS} ${CFLAGS} ${SOURCES}

${OBJS}: star.h

install: starship rmstar
	install  -c -o daemon -s -m 4711 starship ${DESTROOT}/usr/games/starship
	install -c -o daemon -s -m 4711 rmstar ${DESTROOT}/usr/games/rmstar
	install -c -o daemon -m 744 README ${DESTROOT}/usr/games/README

clean:
	rm -f starship rmstar rmstar.o ${OBJS}

clobber: clean
	rm -f Makefile README *.[hc]

sccsget:
	sccs get SCCS 

sccsinfo:
	sccs info 
