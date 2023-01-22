# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.1	83/02/10

CFLAGS=-O
DESTROOT=

diff3: diff3.o
	cc -o diff3 diff3.o

install: diff3
	install -c -s diff3 $(DESTROOT)/usr/lib
	install -c diff3.sh $(DESTROOT)/usr/bin/diff3

clean:
	rm -f diff3.o diff3 errs *.s a.out	

clobber: clean
	rm -f *.c *.sh Makefile	

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
