# @(#)Makefile	1.4	ULTRIX	12/4/84
#
# Copyright (c) 1982 Regents of the University of California
#
#	@(#)Makefile	4.10	(Berkeley)	6/12/83
#
DESTROOT=
CFLAGS= -O
FILES=	main.c restore.c dirs.c symtab.c tape.c utilities.c
OBJS=	restore.o dirs.o symtab.o utilities.o
ROBJS=  ${OBJS} rmain.o rtape.o dumprmt.o rstatchk.o	

all: restore rrestore

restore: ${OBJS} main.o tape.o statchk.o
	cc ${LDFLAGS} ${CFLAGS} -o restore ${OBJS} main.o tape.o statchk.o

rrestore: ${ROBJS}
	cc ${LDFLAGS} -o rrestore ${ROBJS}

rmain.o: main.c
	cp main.c rmain.c
	cc ${CFLAGS} -c -DRRESTORE rmain.c
	rm -f rmain.c

statchk.o: ../dump/statchk.c
	cc ${CFLAGS} -c ../dump/statchk.c

rstatchk.o: ../dump/statchk.c
	cp ../dump/statchk.c ../dump/rstatchk.c
	cc ${CFLAGS} -c -DRRESTORE ../dump/rstatchk.c
	rm -f ../dump/rstatchk.c

rtape.o: tape.c
	cp tape.c rtape.c
	cc ${CFLAGS} -c -DRRESTORE rtape.c
	rm -f rtape.c

dumprmt.o: ../dump/dumprmt.c
	cc ${CFLAGS} -c ../dump/dumprmt.c

install: rrestore restore
	install -c restore ${DESTROOT}/etc/restore
	install -c -m 4755 rrestore ${DESTROOT}/etc/rrestore

clean:
	rm -f restore rrestore *.o errs a.out core

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

lint:
	lint ${FILES}

print:
	@pr /usr/include/dumprestor.h restore.h ${FILES}

${OBJS}: restore.h
