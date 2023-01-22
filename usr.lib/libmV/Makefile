# @(#)Makefile	1.1	ULTRIX	4/1/85
#
#	@(#)Makefile	4.1	12/25/82
#
SCCSID = "@(#)Makefile 4.1 12/25/82"

CFLAGS=-O -DSYSTEM_FIVE
DESTROOT=

INSTALL= install

SRCS= 	asin.c atan.c hypot.c jn.c j0.c j1.c pow.c gamma.c \
	log.c sin.c sqrt.c tan.c tanh.c sinh.c erf.c exp.c floor.c\
	fabs.c fmod.c matherr.c
FILES= 	asin.o atan.o hypot.o jn.o j0.o j1.o pow.o gamma.o \
	log.o sin.o sqrt.o tan.o tanh.o sinh.o erf.o exp.o floor.o\
	fabs.o fmod.o matherr.o

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -Mg ${CFLAGS} -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

libmV.a libmVg.a libmV_p.a : $(FILES)
	cd profiled; ar cru ../libmV_p.a $(FILES)
	cd gfloat; ar cru ../libmVg.a ${FILES}
	ar cru libmV.a $(FILES)

install: libmV.a libmVg.a libmV_p.a
	${INSTALL} -c libmVg.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmVg.a
	${INSTALL} -c libmV.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmV.a
	${INSTALL} -c libmV_p.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmV_p.a

tools:
	for i in ${FILES}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	ar cru libmV.a $(FILES)
	rm -f ${FILES}
	${INSTALL} -c libmV.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmV.a

clean:
	rm -f *.o profiled/*.o gfloat/*.o libmV.a libmVg.a libmV_p.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

${FILES}: ${SRCS}

.DEFAULT:
	sccs get $<
