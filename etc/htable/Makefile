# @(#)Makefile	1.7	ULTRIX	12/4/84
#
# @(#)Makefile	4.5 5/28/83
OPTIONS=-DLOCALNET=0
CFLAGS=	-O
DESTROOT=
ALL=	htable

all:	${ALL}

htable:	htable.o parse.o scan.o
	${CC} htable.o parse.o scan.o -o htable

htable.o: htable.c htable.h
	${CC} ${CFLAGS} ${OPTIONS} -c htable.c

scan.o:	scan.c htable.h y.tab.h

parse.o: parse.c htable.h

scan.c:	scan.l

parse.c: parse.y

y.tab.h:
	${YACC} -d parse.y
	rm y.tab.c

clean:
	rm -f ${ALL} *.o *.s y.tab.h errs

clobber: clean
	rm -f Makefile *.[lyhc] 
	rm -f networks localhosts *.txt gateways hosts localnetworks

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

install: all
	install -c -s htable ${DESTROOT}/etc/htable
