# @(#)Makefile	1.8	(ULTRIX)	1/9/87
#
#	@(#)Makefile	4.1	12/25/82
#

CFLAGS=-O
DESTROOT=

INSTALL= install

SRCS =	acosh.c asincos.c asinh.c atan.c atanh.c cosh.c erf.c \
	exp.c exp__E.c expm1.c floor.c gamma.c j0.c j1.c jn.c log.c \
	log10.c log1p.c log__L.c pow.c sinh.c tanh.c 

FILES =	acosh.o asincos.o asinh.o atan.o atanh.o cosh.o erf.o \
	exp.o exp__E.o expm1.o floor.o gamma.o j0.o j1.o jn.o log.o \
	log10.o log1p.o log__L.o pow.o sinh.o tanh.o 

DSRCS = argred.s d_atan2.s d_cabs.s d_cbrt.s infnan.s sincos.s sqrt.s \
	d_support.s tan.s

DFILES =argred.o d_atan2.o d_cabs.o d_cbrt.o infnan.o sincos.o sqrt.o \
	d_support.o tan.o

GSRCS =	atan2.c cabs.c cbrt.c support.c trig.c

GFILES=	atan2.o cabs.o cbrt.o support.o trig.o

all: libm.a libm_p.a libmg.a

libm.a libm_p.a : ${FILES} ${DFILES} fabs.o
	cd profiled; ar cru ../libm_p.a $(FILES) $(DFILES) fabs.o
	ar cru libm.a $(FILES) $(DFILES) fabs.o

libmg.a : ${FILES} ${GFILES} fabs.o
	cd gfloat; ar cru ../libmg.a ${FILES} ${GFILES} fabs.o

fabs.o : fabs.s mcount.sed
	sed -f mcount.sed fabs.s | as -o fabs.o
	ld -x -r fabs.o
	mv a.out profiled/fabs.o
	as -o fabs.o fabs.s
	ld -x -r fabs.o
	mv a.out fabs.o
	cp fabs.o gfloat/fabs.o

.c.o:
	${CC} -p -DVAX ${CFLAGS} -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -Mg -DIEEE ${CFLAGS} -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} -DVAX ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o


.s.o:
	sed -f mcount.sed $*.s | as -o $*.o
	ld -x -r $*.o
	mv a.out profiled/$*.o

	as -o $*.o $*.s
	ld -x -r $*.o
	mv a.out $*.o

install: libm.a libm_p.a libmg.a
	${INSTALL} -c libmg.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmg.a
	${INSTALL} -c libm.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libm.a
	${INSTALL} -c libm_p.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libm_p.a

tools:
	for i in ${FILES}; do \
		echo $$i; \
		${CC} -DVAX ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	for i in ${DFILES}; do \
		echo $$i; \
		as -o $$i `basename $$i .o`.s; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	as -o fabs.o fabs.s
	ld -x -r fabs.o
	mv a.out fabs.o
	ar cru libm.a $(FILES) $(DFILES) fabs.o
	rm -f ${FILES} $(DFILES) fabs.o
	${INSTALL} -c libm.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libm.a

clean:
	rm -f *.o profiled/*.o gfloat/*.o libm.a libmg.a libm_p.a

clobber: clean
	rm -f *.c *.s mcount.sed READ_ME Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

${FILES}: ${SRCS}

${DFILES}: ${DSRCS}

${GFILES}: ${GSRCS}

.DEFAULT:
	sccs get $<
