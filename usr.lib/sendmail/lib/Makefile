# @(#)Makefile	1.6	ULTRIX	9/23/85
#
#  Makefile for Sendmail library
#
#	@(#)Makefile.m4	3.6		2/9/83
#  EDIT HISTORY
#  15-Jan-1987  Marc Teitelbaum
#      REMOVE DUPLICATE SYSLOG LIBRARY.  It will come from libc...
#	THis was an out of date syslog library.  This sendmail
#	library is now empty.  The syslog.c file contains a dummy
#	routine to keep ld from complaing that libsys.a has no table of contents.
#

DESTROOT=
ALL=	sendmail.hf libsys.a
SRCS=	sendmail.hf syslog.c
LIBOBJS=syslog.o

GET=	sccs get
CCONFIG=-DVMUNIX
CFLAGS=	$O -I../include $(CCONFIG)
O=	-O
ASMSED=	../include/asm.sed

.c.o:
	cc -S ${CFLAGS} $*.c
	sed -f $(ASMSED) $*.s | as -o $*.o
	rm -f $*.s

all: $(ALL)

libsys.a: $(LIBOBJS)
	ar rv libsys.a $(LIBOBJS)
	ranlib libsys.a

ndir: /tmp
	cd libndir; make
	ar rv libsys.a libndir/*.o
	ranlib libsys.a
	rm -f ../include/dir.h
	cp libndir/dir.h ../include

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) SCCS/s.$@

clean:
	rm -f libsys.a core a.out
	rm -f *.o 
	(cd libndir; make ${MFLAGS} clean)

clobber:
	rm -f Makefile Makefile.m4 crontab aliases* *.c *.sh *.hf

sccsinfo:
	sccs info
	(cd libndir; make ${MFLAGS} sccsinfo)

sccsget:
	sccs get SCCS 
	(cd libndir; make ${MFLAGS} sccsget)

.DEFAULT:
	sccs get $<
