# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.3	83/07/04
#
CFLAGS=	-O
DESTROOT=

ALL=	tek t300 t300s t450 vplot

all:	${ALL}

tek:	driver.o
	cc -o tek  driver.o -l4014 -lm

t300:	driver.o 
	cc -o t300 driver.o -l300 -lm

t300s:	driver.o 
	cc -o t300s driver.o -l300s -lm

t450:	driver.o 
	cc -o t450 driver.o -l450 -lm

vplot:	vplot.o chrtab.o
	cc -o vplot vplot.o chrtab.o

install: all
	install -c -s tek ${DESTROOT}/usr/bin
	install -c -s t300 ${DESTROOT}/usr/bin
	install -c -s t300s ${DESTROOT}/usr/bin
	install -c -s t450 ${DESTROOT}/usr/bin
	install -c -s vplot ${DESTROOT}/usr/bin
	install -c plot.sh ${DESTROOT}/usr/bin/plot
clean:
	rm -f *.o ${ALL} a.out core errs

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
