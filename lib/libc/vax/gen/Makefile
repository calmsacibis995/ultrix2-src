#	@(#)Makefile	1.7	ULTRIX	11/20/84
#
#	Makefile	4.4	83/07/02
#
#	Modification History
#
# 003-	David L Ballenger,	29-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
#	Stephen Reilly, 14-May-84
# 002-  Changed the symbol MVAXI to GFLOAT to reflect the meaning
#	of the code.
#
#	Stephen Reilly, 04-Apr-84
# 001 - Added code that depends on the environment variable "target".
#	The reason is that some of the modules must be compiled 
#	differently for MicroVax than for the big vax.
#
DESTROOT=

SOBJS=	_setjmp.o abort.o abs.o alloca.o atof.o bcmp.o bcopy.o bzero.o \
	fabs.o ffs.o insque.o ldexp.o modf.o nargs.o remque.o setjmp.o \
	udiv.o urem.o

COBJS=	disktab.o ecvt.o frexp.o index.o rindex.o strcat.o strcmp.o strcpy.o \
	strlen.o strncat.o strncmp.o strncpy.o

OBJS=	${COBJS} ${SOBJS}

CFLAGS=	-O

.s.o:
	/lib/cpp -E -DPROF $*.s | ${AS} -o $*.o
	ld -X -r -o profiled/$*.o $*.o;
	/lib/cpp -E -DGFLOAT $*.s | ${AS} -o $*.o
	ld -x -r -o gfloat/$*.o $*.o
	/lib/cpp -E $*.s | ${AS} -o $*.o
	ld -x -r $*.o
	mv a.out $*.o

.c.o:
	cc -p -c -Md ${CFLAGS} $*.c
	-ld -X -r -o profiled/$*.o $*.o
	cc -c -Mg ${CFLAGS} $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	cc -c -Md ${CFLAGS} $*.c
	-ld -x -r $*.o
	mv a.out $*.o

genlib profiled/genlib gfloat/genlib: ${OBJS}
	@echo "building profiled genlib"
	@cd profiled; ar cru genlib ${OBJS}
	@echo "building gfloat genlib"
	@cd gfloat; ar cru genlib ${OBJS}
	@echo "building normal genlib"
	@ar cru genlib ${OBJS}

tools:
	for i in ${COBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -Md -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	for i in ${SOBJS}; do \
		echo $$i; \
		/lib/cpp -E `basename $$i .o`.s | ${AS} -o $$i; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal genlib"
	ar cru genlib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o errs a.out core \
	      genlib profiled/genlib gfloat/genlib

clobber: clean
	rm -f *.[chs] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
