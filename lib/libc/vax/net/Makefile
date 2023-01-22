# @(#)Makefile	1.5	ULTRIX	11/20/84
#	Makefile	4.1	83/06/27
#
# 	Modifications
#
# 001-	David L Ballenger,	29-Aug-1984
#	Make GFLOAT version of library routines for libc.
#

OBJS=	htonl.o htons.o ntohl.o ntohs.o
CFLAGS=	
DESTROOT=

.c.o:
	/lib/cpp -E -DPROF $*.c | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out profiled/$*.o
	/lib/cpp -E $*.c | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out $*.o

netlib profiled/netlib gfloat/netlib: ${OBJS}
	@echo "building profiled netlib"
	@cd profiled; ar cru netlib ${OBJS}
	@echo "building normal netlib"
	@ar cru netlib ${OBJS}
	@echo "building gfloat netlib"
	@cp netlib gfloat/netlib

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		/lib/cpp -E `basename $$i .o`.c | ${AS} -o $$i; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal netlib"
	ar cru netlib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o errs a.out core \
	      netlib profiled/netlib gfloat/netlib

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget: 
	sccs get SCCS

.DEFAULT:
	sccs get $<
