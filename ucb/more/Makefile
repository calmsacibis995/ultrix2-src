# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.2	83/07/03
#
CFLAGS=	-O
DESTROOT=

more:	more.o
	${CC} -o more more.o -lcurses -ltermcap

install: more more.help
	install -c -s more ${DESTROOT}/usr/ucb/more
	rm -f ${DESTROOT}/usr/ucb/page
	ln ${DESTROOT}/usr/ucb/more ${DESTROOT}/usr/ucb/page
	install -c more.help ${DESTROOT}/usr/lib/more.help

more.o:	more.c
more.o:	/usr/include/stdio.h
more.o:	/usr/include/ctype.h
more.o:	/usr/include/signal.h
more.o:	/usr/include/errno.h
more.o:	/usr/include/sgtty.h
more.o:	/usr/include/setjmp.h
more.o:	/usr/include/sys/types.h
more.o:	/usr/include/sys/stat.h

clean:
	rm -f more more.o core a.out errs

clobber: clean
	rm -f *.c more.help Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
