# @(#)Makefile	1.4	ULTRIX	11/20/84
#
#	Makefile	4.2	83/07/05
#

#	Modifications
# 001-		David L Ballenger,	27-Aug-1984
#	Make GFLOAT version of Library routines for Libc.
#

CFLAGS=	-O
STD=	ftime.o gtty.o nice.o pause.o rand.o stty.o tell.o \
	times.o utime.o vlimit.o vtimes.o
# these should just be pitched, but in the interest of compatibility...
TRASH=	getpw.o 
OBJS=	${STD} ${TRASH}
DESTROOT=

.c.o:
	${CC} -p ${CFLAGS} -Md -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -Mg -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -Md -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

compat-4.1lib profiled/compat-4.1lib gfloat/compat-4.1lib: ${OBJS}
	@echo "building profiled compat-4.1lib"
	@cd profiled; ar cru compat-4.1lib ${OBJS}
	@echo "building gfloat compat-4.1lib"
	@cd gfloat; ar cru compat-4.1lib ${OBJS}
	@echo "building normal compat-4.1lib"
	@ar cru compat-4.1lib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -Md -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal compat-4.1lib"
	ar cru compat-4.1lib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f compat-4.1lib compat-4.1lib_p ${OBJS} profiled/*.o
	rm -f profiled/compat-4.1lib gfloat/compat-4.1lib gfloat/*.o
	

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
