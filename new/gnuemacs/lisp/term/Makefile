# @(#)Makefile	1.1	Ultrix	7/3/86
#
#	Makefile for GNU Emacs "lisp/term" subdirectory.
#

# Symbols below are derived from top level makefile
DESTROOT=
PATHLOADSEARCH=

DIRNAME= term

ASCFILES=	bbn.el bg.el bgnv.el bgrv.el bobcat.el supdup.el \
		vt100.el vt200.el vt220.el xterms.el
NOASCFILES=	xterm.el
FILES=		${ASCFILES} ${NOASCFILES}

all:

install:
	for i in ${FILES} ;do \
		install -c -m 644 $$i ${DESTROOT}${PATHLOADSEARCH}/${DIRNAME}; \
	done

clean:

clobber:	clean
	sccs clean
	rm -f ${NOASCFILES}

sccsinfo:
	@sccs info

sccsget:
# NOASCFILES file encoded to printable ASCII code befor put into SCCS library
# using following command.
# uuencode xxx /tmp/xxx | sed '1s/[0-9][0-9][0-9]/777/' > xxx.enc
# Because each NOASCFILES file end in premature eof.
# Notice: If you changed any NOASCFILES file, you must encode it using
#         uuencode. And You must specify /tmp/ as directory of remote
#         destination. Also, You must edit mode described in encoded file
#         to "777".
	sccs get SCCS
	for i in ${NOASCFILES}; do \
          uudecode $$i.enc ; \
	  cp /tmp/$$i . ; \
	  rm -f /tmp/$$i $$i.enc; \
	  chmod 444 $$i ; \
	done
