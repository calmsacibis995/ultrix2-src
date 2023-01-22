#
#  @(#)Makefile	1.6	ULTRIX	10/3/86
#
#************************************************************************
#*									*
#*			Copyright (c) 1986 by				*
#*		Digital Equipment Corporation, Maynard, MA		*
#*			All rights reserved.				*
#*									*
#*   This software is furnished under a license and may be used and	*
#*   copied  only  in accordance with the terms of such license and	*
#*   with the  inclusion  of  the  above  copyright  notice.   This	*
#*   software  or  any	other copies thereof may not be provided or	*
#*   otherwise made available to any other person.  No title to and	*
#*   ownership of the software is hereby transferred.			*
#*									*
#*   The information in this software is subject to change  without	*
#*   notice  and should not be construed as a commitment by Digital	*
#*   Equipment Corporation.						*
#*									*
#*   Digital assumes no responsibility for the use  or	reliability	*
#*   of its software on equipment which is not supplied by Digital.	*
#*									*
#************************************************************************
#
# rpcsvc Makefile
#
# Modification history
#
# 09-Jul-86 - Chet Juszczak
#	Add rquotaxdr.o to list of object files.
#
# xx-Jun-86 - Ray Glaser
#	Removed the following files that are in ../net
#	for Sun V3.0 :
#		getnetgrent.o
#		innetgr.o
#
#	Further modified for Sun V3.0 Yellow pages.
#	Note: On the Sun tree -
#	      yp*  routines are in  lib/libc/yp
#
# 15-Mar-86 - ricky palmer
#
#	Created and filled in original file for gfs/nfs effort.
#
CFLAGS= -O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo

OBJS=	mountxdr.o rquotaxdr.o rstatxdr.o rusersxdr.o util.o \
	yp_all.o yp_bind.o yp_enum.o yp_master.o \
	yp_match.o yp_order.o yperr_string.o ypprot_err.o \
	ypv1_xdr.o ypxdr.o yppasswdxdr.o


.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -c ${CFLAGS} $*.c
	-ld -s -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

rpcsvclib profiled/rpcsvclib gfloat/rpcsvclib: ${OBJS}
	@echo "building profiled rpcsvclib"
	@cd profiled; ${ECHO} ${CWD}; ar cru rpcsvclib ${OBJS}
	@echo "building gfloat rpcsvclib"
	@cd gfloat; ${ECHO} ${CWD}; ar cru rpcsvclib ${OBJS}
	@echo "building normal rpcsvclib"
	@ar cru rpcsvclib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal rpcsvclib"
	ar cru rpcsvclib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o gfloat/*.o errs a.out core \
	       rpcsvclib profiled/rpcsvclib gfloat/rpcsvclib

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
