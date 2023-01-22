# @(#)Makefile	1.4	ULTRIX	9/23/85
#
# @(#)Makefile	4.4 12/18/82
OBJS=	host.o inet.o if.o main.o mbuf.o route.o unix.o
CFLAGS=-O
DESTROOT=

netstat: ${OBJS}
	${CC} ${OBJS} -o netstat

install: netstat
	install -c -s netstat ${DESTROOT}/usr/ucb

clean:
	rm -f *.o core a.out errs

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
