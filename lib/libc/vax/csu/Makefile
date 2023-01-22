# @(#)Makefile	1.7	ULTRIX	10/2/84
#
# @(#)Makefile	4.7 (Berkeley) 12/10/82
#
#	crt0	Normal C run time startoff
#	mcrt0	C run time start off for profiling, ``prof'' conventions
#	gcrt0	C run time start off for profiling, ``gprof'' conventions
#
INSTALL= install -m 644
DESTROOT=/Ultrix
OBJS=	crt0.o mcrt0.o gcrt0.o mon.o gmon.o
DFLMON=

all: ${OBJS}

install: ${OBJS}
	${INSTALL} crt0.o ${DESTROOT}/lib/crt0.o
	${INSTALL} mcrt0.o ${DESTROOT}/lib/mcrt0.o
	${INSTALL} gcrt0.o ${DESTROOT}/usr/lib/gcrt0.o
	${INSTALL} ${DFLMON} ${DESTROOT}/lib/${DFLMON}

crt0.o:	crt0.c
	cc -S ${DFLAGS} -DCRT0 crt0.c
	../../../cpp/cpp crt0.s > x.s
	as -o x.o x.s
	ld -x -r -o crt0.o x.o
	rm -f x.s x.o crt0.s

moncrt0.o: crt0.c
	cc -S ${DFLAGS} -DMCRT0 crt0.c
	../../../cpp/cpp crt0.s > x.s
	as -o x.o x.s
	ld -x -r -o moncrt0.o x.o
	rm -f x.s x.o crt0.s

gcrt0.o: moncrt0.o gmon.o
	ld -x -r -o gcrt0.o moncrt0.o gmon.o

mcrt0.o: moncrt0.o mon.o
	ld -x -r -o mcrt0.o moncrt0.o mon.o

mon.o: mon.c mon.ex
	cc -S ${DFLAGS} mon.c
	ex - mon.s < mon.ex
	as -o x.o mon.s
	ld -x -r -o mon.o x.o
	rm -f x.o mon.s

gmon.o: gmon.c gmon.h gmon.ex
	cc -S ${DFLAGS} gmon.c
	ex - gmon.s < gmon.ex
	as -o x.o gmon.s
	ld -x -r -o gmon.o x.o
	rm -f x.o gmon.s

clean:
	rm -f ${OBJS} *.o *.s core errs

clobber: clean
	rm -f *.c *.h *.ex Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

lint:	crt0.c
	lint crt0.c -DMCRT0
	lint crt0.c -DGCRT0
	lint crt0.c -DCRT0
