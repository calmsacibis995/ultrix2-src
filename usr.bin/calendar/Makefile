# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	Makefile	4.1	83/08/13
#
DESTROOT=
CFLAGS=	-O

calendar: calendar.c
	${CC} ${CFLAGS} -o calendar calendar.c

install: calendar
	install -c -s calendar $(DESTROOT)/usr/lib
	install -c -m 755 calendar.sh $(DESTROOT)/usr/bin/calendar

clean:

	rm -f calendar *.o

clobber: clean
	rm -f *.c *.sh makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
