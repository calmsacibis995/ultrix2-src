# @(#)Makefile	1.2	ULTRIX	6/28/84
#
#
DESTROOT=
CFLAGS = -n -s -O

all:	sh

cp:	sh
	cp sh /bin/sh
	rm sh *.o

cmp:	sh
	cmp sh /bin/sh
	rm sh *.o

sh:	setbrk.o
sh:	builtin.o blok.o stak.o
sh:	cmd.o fault.o main.o
sh:	word.o string.o name.o args.o
sh:	xec.o service.o error.o io.o
sh:	print.o macro.o expand.o
sh:	ctype.o msg.o
sh:	compat.o
blok.o:		brkincr.h
fault.o:	brkincr.h
main.o:		brkincr.h
stak.o:		brkincr.h

sh:;		cc -o sh -n *.o

newsys:;	:newsys file=msg; make sh

install:;	install -s sh $(DESTROOT)/bin

clean:; rm -f sh *.o a.out errs core *.s

clobber: clean
	rm -f Makefile *.[ch]

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<

diff:;		:diff

list:;		list Makefile *.h *.s *.c

.c.o:;	cc -O  -c $<
