#
#  @(#)Makefile	1.10	ULTRIX	10/16/86
#
# 	Modifications
#
# 003-	Fred L. Templin	16-Oct-86
#	Removed getservbyaddr, getservbyname
#
#	Replaced getservent.c with SUN code.
#
# 002-	Ray Glaser	xx-Jun-86
#	Remove:
#	gethostbyaddr, gethostbyname, getnetbyaddr, getnetbyname
#	getproto, getprotoname
#
#	Replaced by Sun Yellow Pages modules
#	gethostent.c, genetent.c, getprotoent.c
#
#	These modules combine entry points from the delete modules.
#
#	Added Sun YP modules:
#	innetgr.c, getnetgrent.c, getrpcent.c, 
#
#	Replaced:
#	rcmd.c	with Sun YP  version
#	
# 001-	David L Ballenger,	28-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
#

CFLAGS=	-O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo

OBJS=	innetgr.o \
	gethostent.o \
	getnetent.o getnetgrent.o \
	getprotoent.o getrpcent.o \
	getservent.o \
	rcmd.o rexec.o ruserpass.o


.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -c ${CFLAGS} $*.c
	-ld -s -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

netlib profiled/netlib gfloat/netlib: ${OBJS}
	@echo "building profiled netlib"
	@cd profiled; ${ECHO} ${CWD}; ar cru netlib ${OBJS}
	@echo "building gfloat netlib"
	@cd gfloat; ${ECHO} ${CWD}; ar cru netlib ${OBJS}
	@echo "building normal netlib"
	@ar cru netlib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal netlib"
	ar cru netlib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o gfloat/*.o errs a.out core \
	       netlib profiled/netlib gfloat/netlib

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

