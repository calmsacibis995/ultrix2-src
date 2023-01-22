# @(#)Makefile	1.4	ULTRIX	12/4/84
#
# Makefile 6/12/83
ALL=	tftp
DESTROOT=
CFLAGS=-O

all: ${ALL}

tftp:	main.o tftp.o
	${CC} main.o tftp.o -o tftp

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

install: all
	install -c -s tftp ${DESTROOT}/usr/ucb
