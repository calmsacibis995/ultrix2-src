#ifndef lint
static char	*sccsid = "@(#)Makefile	1.1	(ULTRIX)	2/13/86";
#endif lint
CFLAGS = -O
DESTROOT=

all: liberrlog.a

liberrlog.a:
	cc -c -O window.c logerr.c
	rm -f liberrlog.a
	ar rvc liberrlog.a window.o logerr.o
	rm window.o logerr.o

tools install: all
	install -c liberrlog.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/liberrlog.a

clean:
	rm -f *.o liberrlog.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

