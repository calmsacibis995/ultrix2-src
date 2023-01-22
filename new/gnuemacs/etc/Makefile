# @(#)Makefile	1.1	Ultrix	7/3/86
#
#	Makefile for GNU Emacs "etc"subdirectory
#

# Symbols below are derived from top level Makefile.
DESTROOT=
USERPATH=
PATHEXEC=

STD= test-distrib etags ctags loadst make-docfile digest-doc movemail cvtmail \
     fakemail yow

ASCFILES=	APOLLO CCADIFF COOKIES COPYING DEBUG DIFF DISTRIB GNU \
		GOSDIFF LEDIT MACHINES MAILINGLISTS NEWS ONEWS RCP SERVICE \
		TERMS TO_DO TUTORIAL refcard.troff emacs.1 emacs.names
### These files below are, however ascii files, they can't be restored as same 
###	that they were without useing "uudecode/uuencode". See sccsget:.
### CAUTION ! AND YET,
###	yow.lines are never restored to what they were
###	before "sccs admin", even though using "uuencode/uudecode".
###	yow.lines has not lost any significant information.
NOASCFILES=	XDOC echo.msg yow.lines testfile
TEXTFILES=	${ASCFILES} ${NOASCFILES}

LEDITFILES=	ledit.l leditcfns.c

CFLAGS= -g

all: ${STD}

test-distrib: test-distrib.c
	cc -o test-distrib test-distrib.c
	./test-distrib

etags: etags.c
	cc -o etags ${CFLAGS} -DETAGS etags.c

ctags: etags.c
	cc -o ctags ${CFLAGS} -DCTAGS etags.c

loadst: loadst.c
	cc -o loadst ${CFLAGS} loadst.c

make-docfile: make-docfile.c
	cc -o make-docfile ${CFLAGS} make-docfile.c

digest-doc: digest-doc.c
	cc -o digest-doc ${CFLAGS} digest-doc.c

movemail: movemail.c
	cc -o movemail ${CFLAGS} movemail.c

cvtmail: cvtmail.c
	cc -o cvtmail ${CFLAGS} cvtmail.c

fakemail: fakemail.c
	cc -o fakemail ${CFLAGS} fakemail.c

yow: yow.c
	cc -o yow ${CFLAGS} yow.c

install:	all
	install -c -m 755 etags ${DESTROOT}${USERPATH}
	install -c -m 755 ctags ${DESTROOT}${USERPATH}
	install -c -m 755 loadst ${DESTROOT}${PATHEXEC}
### These commands seem to be worthless.
###	at any rate, I install them.
	for i in digest-doc movemail cvtmail fakemail yow yow.lines; do \
		install -c -m 755 $$i ${DESTROOT}${PATHEXEC}; done
### Text and Ledit files are not needed in the BIN kit.
### There is, therefore, no use to install them.
###	for i in ${TEXTFILES} ${LEDITFILES}; do \
###		install -c -m 644 $$i ${DESTROOT}${PATHEXEC}; done

clean:
	rm -f a.out core *.o *.s
	rm -f ${STD}

clobber: clean
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

