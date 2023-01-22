# @(#)Makefile	1.1	Ultrix	7/3/86
#
#	Makefile for GNU Emacs "info" subdirectory.
#

# Symbols below are derived from top level Makefile
DESTROOT=
INFODIR=

FILES= dir info texinfo emacs

all:

install:
	for i in ${FILES}; do \
		install -c -m 644 $$i ${DESTROOT}${INFODIR}; done

clean:

clobber:	clean
	sccs clean

sccsinfo:
	@sccs info

sccsget:
	sccs get SCCS


