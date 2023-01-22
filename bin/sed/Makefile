# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	Makefile	1.4	84/12/04
#
CFLAGS=-O -w
DESTROOT=

sed:	sed0.o sed1.o
	cc -o sed *.o

sed0.o: sed0.c sed.h
sed1.o: sed1.c sed.h

install: sed
	install -c -s sed $(DESTROOT)/bin
clean:
	rm -f *.o sed

clobber: clean
	rm -f *.[hc] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<	
