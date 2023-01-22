#	manufacturing makefile, ULTRIX product manufacturing
#
#  @(#)Makefile	1.6	ULTRIX	10/3/86
#
#			Copyright (c) 1986 by
#		Digital Equipment Corporation, Maynard, MA
#			All rights reserved.
#								
#	This software is furnished under a license and may be used and
#	copied  only  in accordance with the terms of such license and
#	with the  inclusion  of  the  above  copyright  notice.   This
#	software  or  any  other copies thereof may not be provided or
#	otherwise made available to any other person.  No title to and
#	ownership of the software is hereby transferred.		
#								
#	The information in this software is subject to change  without
#	notice  and should not be construed as a commitment by Digital
#	Equipment Corporation.					
#								
#	Digital assumes no responsibility for the use  or  reliability
#	of its software on equipment which is not supplied by Digital.
#
#	mods:
#		000	ccb	1986.03.06
#		make invcutter

# files
ALL=invcutter tarsets fverify

CFLAGS=-O

sccsget:
	sccs get SCCS

install:	all
	install -c -m 755 fverify ${DESTROOT}/etc
	install -c -m 755 tarsets ${DESTROOT}/sys/dist
	install -c -m 755 invcutter ${DESTROOT}/sys/dist

all:	$(ALL)

# compile utilities
invcutter:	invcutter.o getsum.o emalloc.o
	ld -X $(DESTROOT)/lib/crt0.o -o invcutter invcutter.o getsum.o emalloc.o -lc

tarsets:	tarsets.o emalloc.o
	ld -X $(DESTROOT)/lib/crt0.o -o tarsets tarsets.o emalloc.o -lc

fverify:	fverify.o getsum.o invdist.h
	ld -X $(DESTROOT)/lib/crt0.o -o fverify fverify.o getsum.o -lc

mass:	mass.o
	ld -X $(DESTROOT)/lib/crt0.o -o mass mass.o -lc

