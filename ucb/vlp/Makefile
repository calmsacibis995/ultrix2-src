# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#$Header: /usr/src/ucb/vlp/RCS/Makefile,v 1.4 83/07/06 01:34:13 layer Exp $
#
#			-[Thu Feb 10 13:15:17 1983 by jkf]-

# Makefile for VLP

# files required:
#	vlp.l	- lisp program to read source and add troff control
#		characters.
#	vlpmacs - troff macros used by vlp.
#
# vlp and vlpmacs are put in DESTROOT
# If DESTROOT is changed, then the source (vlp.l) must be modified since
# it explicitly points to vlpmacs.

DESTROOT =
.DEFAULT:vlp

vlp:	vlp.l
	/usr/ucb/liszt -qar -o vlp vlp.l

install: vlp vlpmacs
	install -c vlp ${DESTROOT}/usr/ucb
	install -c -m 664 vlpmacs ${DESTROOT}/usr/lib

clean:
	rm -f vlp

clobber: clean
	rm -f *.l vlpmacs Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
