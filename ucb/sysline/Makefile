# @(#)Makefile	1.3	ULTRIX	12/4/84
#
# $Header: /usr/src/ucb/sysline/RCS/Makefile,v 1.2 83/02/10 02:15:06 layer Exp $
#  See the sysline program source for options you may want to modify
#
DESTROOT =

CFLAGS = -O -n

.DEFAULT:sysline

sysline: sysline.c
	${CC} ${CFLAGS} -o sysline sysline.c -lcurses -ltermlib

install: sysline
	install -c -s sysline $(DESTROOT)/usr/ucb

clean:
	rm -f sysline

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
