# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.2	83/08/23
#
# Bug report processor and associated programs
#

CFLAGS = -O -DUNIXCOMP
LIBDIR=	/usr/lib
DESTROOT=

bugfiler: bugfiler.c
	${CC} ${CFLAGS} -o bugfiler bugfiler.c

install: bugfiler sendbug.sh bugformat
	install -c -s bugfiler ${DESTROOT}${LIBDIR}/bugfiler
	install -c sendbug.sh ${DESTROOT}/usr/ucb/sendbug
	install -c -m 644 bugformat ${DESTROOT}/usr/ucb/bugformat

clean: 
	rm -f a.out errs *.o

clobber: clean
	rm -f *.c bugformat sendbug.sh Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

