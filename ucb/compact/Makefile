# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	Makefile	4.3	83/03/10

CFLAGS=-O
DESTROOT=

all: compact uncompact

compact: compact.o tree.o
	cc compact.o tree.o -o compact

uncompact: uncompact.o tree.o
	cc uncompact.o tree.o -o uncompact

compact.o uncompact.o tree.o: compact.h

install: all
	install -c -s compact ${DESTROOT}/usr/ucb
	install -c -s uncompact ${DESTROOT}/usr/ucb
	install -c -m 755 ccat.sh ${DESTROOT}/usr/ucb/ccat

clean:
	rm -f compact.o uncompact.o tree.o

clobber: clean
	rm -f *.[och] *.sh Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
