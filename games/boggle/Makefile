# @(#)Makefile	1.4	ULTRIX	11/26/84
#
#	@(#)Makefile	4.1	12/24/82
#
DESTROOT=
CC = /bin/cc
CFLAGS = -O

all:	boggle bogdict

bogdict: comp sfile w2a
	sed -f sfile w2a | uniq | comp >bogdict

install:
	install boggle ${DESTROOT}/usr/games/boggle
	install bogdict ${DESTROOT}/usr/games/bogdict

boggle: boggle.c
	cc -o boggle ${CFLAGS} boggle.c

comp: comp.c
	cc -o comp ${CFLAGS} comp.c

clean:
	rm -f bogdict boggle comp

clobber: clean
	rm -f sfile inst *.c

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
