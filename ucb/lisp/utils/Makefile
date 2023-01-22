# @(#)Makefile	1.4	ULTRIX	8/16/85
#
# $Header: Makefile 1.1 83/04/20 10:29:24 sklower Exp $
# $Locker:  $
#
#	Modifications
#
#	David L Ballenger, 6-May-1985
# 001-	Fix this Makefile so that it works correctly with the ULTRIX
#	build procedures.
#

DESTROOT=
LIBDIR=   /usr/lib/lisp
INSDIR=	  ${DESTROOT}${LIBDIR}
SOURCES=  Makefile append.c tackon.c
OBJS=	  append tackon


all: ${OBJS}

append: append.c
	cc -O -o append append.c

tackon: tackon.c
	cc -O -o tackon -I../franz/h tackon.c

install: all
	install -c append ${INSDIR}
	install -c tackon ${INSDIR}

clean:
	rm -f ${OBJS}

clobber: clean
	rm -f ${SOURCES} append tackon

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
