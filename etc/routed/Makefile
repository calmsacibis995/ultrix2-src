# @(#)Makefile	1.9	ULTRIX	9/30/85
#
#	Makefile (Berkeley) 6/12/83
#
ALL=	routed
OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o inet.o

CFLAGS=	-O -c
C2=	/lib/c2
ASM.SED= /sys/machine/asm
DESTDIR=
LDFLAGS=

# The following can be deleted where not appropriate to use the kernel's
# inline code expansions.
#.c.o:
#	${CC} -S ${CFLAGS} $*.c
#	${C2} $*.s | ${ASM.SED} | ${AS} -o $*.o
#	rm -f $*.s

.c.o:
	${CC} ${CFLAGS} $*.c

all:	${ALL}

routed: ${OBJS}
	${CC} -o routed ${LDFLAGS} ${OBJS}

install: routed
	install -c -s routed ${DESTROOT}/etc

clean:
	rm -f routed *.o core a.out t.c linterrs typescript errs
	(cd tools; make clean)

clobber: clean
	rm -f *.[ch] Makefile
	(cd tools; make clobber)

sccsinfo:
	sccs info
	(cd tools; make sccsinfo)

sccsget:
	sccs get SCCS
	(cd tools; sccs get Makefile; make ${MFLAGS} sccsget)

.DEFAULT:
	sccs get $<
