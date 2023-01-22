# @(#)Makefile	1.4	ULTRIX 11/21/84
#
#	Makefile	4.1	83/07/04
#
DESTROOT=
CFLAGS=	-O
LIBS=	-lm -ltermcap
BIN=	$(DESTROOT)/usr/games
LIB=	$(DESTROOT)/usr/games/lib
USER=	daemon
UTILS=	snscore
OBJS=	snake.o move.o
ALL=	snake ${UTILS}

all:	${ALL}

snake:	$(OBJS)
	cc $(OBJS) -o snake $(LIBS)

snake.o move.o:snake.h

snscore: snscore.c
	cc $(CFLAGS) snscore.c -o snscore

busy:	busy.c
	cc $(CFLAGS) busy.c -o busy

install: all
	install -s -m 4755 -o ${USER} snake ${BIN}/snake
	install -s -m 755 -o ${USER} snscore ${BIN}/snscore
#	install -s -m 755 -o ${USER} busy ${BIN}/busy
	cat /dev/null >> $(LIB)/snakerawscores
	chmod 644 $(LIB)/snakerawscores
	/etc/chown $(USER) $(LIB)/snakerawscores
	install -c /dev/null ${LIB}/snake.log


clean:
	rm -f *.o ${ALL}

clobber: clean
	rm -f *.[ch] Makefile busy.sh

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sccsinfo:
	sccs info
