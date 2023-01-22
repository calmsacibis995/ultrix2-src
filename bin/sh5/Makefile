
# @(#)Makefile	1.0	ULTRIX	7/3/84
#
#	Makefile for System 5 shell (/bin/sh5)
#
TESTDIR = .
DESTROOT=
INSDIR  = $(ROOT)/bin
# Use these CFLAGS to lower priority of shell scripts:
# CFLAGS  = -O -DNICE -DNICEVAL=4
CFLAGS  = -O
SOURCES = setbrk.c blok.c stak.c cmd.c fault.c main.c \
	word.c string.c name.c args.c xec.c service.c \
	error.c io.c macro.c expand.c \
	test.c defs.c echo.c hash.c hashserv.c pwd.c \
	func.c ctype.c msg.c print.c

OBJECTS = setbrk.o blok.o stak.o cmd.o fault.o main.o \
	word.o string.o name.o args.o xec.o service.o \
	error.o io.o macro.o \
	test.o defs.o echo.o hash.o hashserv.o pwd.o \
	func.o ctype.o msg.o print.o expand.o

all:	sh5

sh5:	${OBJECTS} defs.h
	$(CC) $(OBJECTS) -o $(TESTDIR)/sh5
	@echo

blok.o fault.o main.o stak.o:	brkincr.h

io.o main.o:	dup.h

xec.o hash.o hashserv.o:	hash.h

cmd.o macro.o main.o msg.o word.o xec.o:	sym.h

main.o:	timeout.h

pwd.o: mac.h

install_manual: all
	@(echo "/bin/sh5 must be installed MANUALLY")
	sync

install: all
	install -c -m 755 -s sh5 ${DESTROOT}/bin
	rm -f ${DESTROOT}/bin/rsh5
	ln ${DESTROOT}/bin/sh5 ${DESTROOT}/bin/rsh5 

clean:
	  rm -f *.o a.out core log 

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

