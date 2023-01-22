#  @(#)Makefile	1.6	ULTRIX	10/3/86
#
DESTROOT=
CFLAGS=-O
tset:	tset.c
	${CC} ${CFLAGS} tset.c -o tset -ltermlib
install: tset
	install -c -s tset ${DESTROOT}/usr/ucb
clean:
	rm -f *.o tset

clobber: clean
	rm -f *.[ch] del.h.cb del.h.u4 del.h.4bsd Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
