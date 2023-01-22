# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	@(#)Makefile	4.1	12/25/82
#
DESTROOT=
CFLAGS=-O
OBJS= pow.o gcd.o msqrt.o mdiv.o mout.o mult.o madd.o util.o

libmp.a: $(OBJS)
	ar cr libmp.a $(OBJS)

install: libmp.a
	install -c libmp.a ${DESTROOT}/usr/lib/libmp.a
	ranlib ${DESTROOT}/usr/lib/libmp.a

clean:
	rm -f *.o libmp.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
