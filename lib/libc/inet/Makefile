# @(#)Makefile	1.4	ULTRIX	11/20/84
#
# @(#)Makefile	4.2 (Berkeley) 6/27/83
#
# 	Modifications
#
# 001-	David L Ballenger,	28-Aug-1984
#	Make GFLOAT version of library routines for libc.
#

OBJS=	inet_addr.o inet_network.o \
	inet_netof.o inet_ntoa.o inet_lnaof.o \
	inet_makeaddr.o
CFLAGS=	-O

.c.o:
	${CC} -p -c ${CFLAGS} -Md $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -c ${CFLAGS} -Mg $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c -Md $*.c
	-ld -x -r $*.o
	mv a.out $*.o

inetlib profiled/inetlib gfloat/inetlib: ${OBJS}
	@echo "building profiled inetlib"
	@cd profiled; ar cru inetlib ${OBJS}
	@echo "building gfloat inetlib"
	@cd gfloat; ar cru inetlib ${OBJS}
	@echo "building normal inetlib"
	@ar cru inetlib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -Md -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal inetlib"
	ar cru inetlib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f inetlib profiled/inetlib gfloat/inetlib \
	      ${OBJS} profiled/*.o gfloat/$*.o errs a.out core

clobber: clean
	rm -f *.c Makefile	

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
