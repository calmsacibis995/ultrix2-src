# Makfile for the Bourne Shell
# 		@(#)Makefile	1.5	ULTRIX	11/25/85
#
#	Original SCCS ID: Makefile	4.5	83/07/01
#
CFLAGS = -O -w
DESTROOT=

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
blok.o:		brkincr.h
fault.o:	brkincr.h
main.o:		brkincr.h
stak.o:		brkincr.h

sh:
	cc -o sh *.o

install: sh
	install -c -s sh $(DESTROOT)/bin
clean:
	rm -f sh *.o
	rm -f a.out core *.s errs

clobber: clean
	rm -f Makefile *.[ch]
	
sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
