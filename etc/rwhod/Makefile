# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.1	83/07/01
#
CFLAGS=-O
DESTROOT=

rwhod:	rwhod.o
	${CC} -o rwhod rwhod.o

install: rwhod
	install -c -s rwhod ${DESTROOT}/etc/rwhod
	-mkdir ${DESTROOT}/usr/spool/rwho

clean:
	rm -f rwhod rwhod.o a.out core errs

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
