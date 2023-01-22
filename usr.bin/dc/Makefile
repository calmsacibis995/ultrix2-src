# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	@(#)Makefile	1.4	(Berkeley)	83/06/30
#
DESTROOT=
SRCS=	dc.c dc.h
CFLAGS=	-O

all: dc

dc:	dc.c dc.h
	${CC} ${CFLAGS} dc.c -o dc

install: dc
	install -c -s dc ${DESTROOT}/usr/bin/dc

clean:
	rm -f *.o dc a.out errs core

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<	
