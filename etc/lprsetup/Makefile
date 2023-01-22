#  @(#)Makefile	1.3	ULTRIX	10/16/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
######################################################################
#
# Makefile for lprsetup program
#

SOURCES= lprsetup.c globals.h misc.c printlib.c

OBJS= lprsetup.o misc.o printlib.o

#OPTIONS= -DDEBUG
OPTIONS=

CFLAGS= -O ${OPTIONS}
DESTROOT=
SETUPDIR=/etc

all:	lprsetup

lprsetup: ${OBJS}
	cc ${CFLAGS} ${OBJS} -o lprsetup

install: all
	install -c -m 755 lprsetup ${DESTROOT}/${SETUPDIR}/lprsetup

clean:
	rm -f lprsetup *.o a.out core log

clobber:
	sccs clean

tags: ${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
lint:
	${LINT} ${LINTFLAGS} *.c

lprsetup.o: lprsetup.c lprsetup.h globals.h
misc.o: misc.c lprsetup.h
printlib.o: printlib.c
