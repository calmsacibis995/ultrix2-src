#	@(#)Makefile	1.4	ULTRIX	12/4/84
#
SCCSID = "@(#)Makefile 1.4 12/4/84"

CFLAGS=-O
DESTROOT=

INSTALL= install

FILES=	acos.o asin.o atan.o cbrt.o pow.o exp.o log.o sin.o \
	sinh.o sqrt.o tan.o tanh.o

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

.s.o:
	sed -f mcount.sed $*.s | ${AS} -o $*.o
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${AS} -o $*.o $*.s
	-ld -x -r $*.o
	mv a.out $*.o

libnm.a libnm_p.a : $(FILES)
	cd profiled; ar cru ../libnm_p.a $(FILES)
	ar cru libnm.a $(FILES)

install: libnm.a libnm_p.a
	${INSTALL} -c libnm.a ${DESTROOT}/usr/lib/libnm.a
	ranlib ${DESTROOT}/usr/lib/libnm.a
	${INSTALL} -c libnm_p.a ${DESTROOT}/usr/lib/libnm_p.a
	ranlib ${DESTROOT}/usr/lib/libnm_p.a

clean:
	rm -f *.o profiled/*.o libnm.a libnm_p.a

clobber: clean
	rm -f README *.[sc] mcount.sed sqrt.s.old Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
