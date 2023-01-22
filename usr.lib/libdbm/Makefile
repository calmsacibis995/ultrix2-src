# @(#)Makefile	1.5 	ULTRIX	12/4/84
#
#	Makefile	4.2	83/08/20
#
CFLAGS=-O -c
DESTROOT=

libdbm.a: dbm.c
libdbm.a: dbm.h
	${CC} ${CFLAGS} dbm.c
	mv dbm.o libdbm.a 

tools install: libdbm.a
	install -c -m 644 libdbm.a $(DESTROOT)/usr/lib
	install -c dbm.h $(DESTROOT)/usr/include

clean:
	rm -f libdbm.a errs a.out core

clobber: clean
	rm -f *.[hc] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
