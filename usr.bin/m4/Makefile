# @(#)Makefile	1.6	ULTRIX	2/8/85
#
# @(#)Makefile	1.1 (Berkeley) 12/15/82

DESTROOT =
CFLAGS=-O
m4:	m4.o m4y.o
	$(CC) -o m4 m4.o m4y.o 

install: m4
	install -c -s m4 $(DESTROOT)/usr/bin

tools1: m4
	install -c -s m4 $(DESTROOT)/usr/bin

tools2: m4
	install -c -s m4 $(DESTROOT)/usr/bin

clean:
	rm -f *.o

clobber: clean
	rm -f *.[chy] Makefile m4

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
