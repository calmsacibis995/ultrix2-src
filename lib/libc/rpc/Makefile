#	@(#)Makefile	1.3	ULTRIX	4/11/86
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
# rpc Makefile
#
# Modification history
#
# 15-Mar-86 - ricky palmer
#
#	Created and filled in original file for gfs/nfs effort.
#
OBJS = auth_none.o auth_unix.o authunix_prot.o clnt_perror.o \
	clnt_raw.o clnt_simple.o clnt_tcp.o clnt_udp.o \
	pmap_clnt.o pmap_getmaps.o pmap_getport.o pmap_prot.o \
	pmap_rmt.o rpc_prot.o svc.o svc_auth.o svc_auth_unix.o \
	svc_raw.o svc_simple.o svc_tcp.o svc_udp.o xdr.o \
	xdr_array.o xdr_float.o xdr_mem.o xdr_rec.o xdr_reference.o \
	xdr_stdio.o

CFLAGS= -O
DESTROOT=

.c.o: 
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} -c ${CFLAGS} $*.c
	-ld -s -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

rpclib profiled/rpclib gfloat/rpclib:  ${OBJS}
	@echo "building profiled rpclib"
	@cd profiled; ar cru rpclib ${OBJS}
	@echo "building gfloat rpclib"
	@cd gfloat; ar cru rpclib ${OBJS}
	@echo "building normal rpclib"
	@ar cru rpclib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal rpclib"
	ar cru rpclib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o gfloat/$*.o errs a.out core \
	       rpclib profiled/rpclib gfloat/rpclib

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
