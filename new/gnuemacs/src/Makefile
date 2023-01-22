# @(#)Makefile	1.1	Ultrix	7/3/86
#
#	Makefile for GNU Emacs "src"subdirectory.
#

# Symbols below are derived from top level Makefile.
#	see Makefile after doing cd .. and sccs get Makefile.
DESTROOT=
EMACSBIN=
PATHEXEC=
PATHLOADSEARCH=
PATHLOCK=
PATHSUPERLOCK=

CPP = /lib/cpp

all: xemacs

xemacs: xmakefile doxemacs

doxemacs:
	make -f xmakefile ${MFLAGS} \
		DESTROOT=${DESTROOT} \
		PATHEXEC=${PATHEXEC} \
		PATHLOADSEARCH=${PATHLOADSEARCH} \
		PATHLOCK=${PATHLOCK} \
		PATHSUPERLOCK=${PATHSUPERLOCK} xemacs
.RECURSIVE: doxemacs

temacs: xmakefile dotemacs

dotemacs:
	make -f xmakefile ${MFLAGS}
		DESTROOT=${DESTROOT} \
		PATHEXEC=${PATHEXEC} \
		PATHLOADSEARCH=${PATHLOADSEARCH} \
		PATHLOCK=${PATHLOCK} \
		PATHSUPERLOCK=${PATHSUPERLOCK} temacs
.RECURSIVE: dotemacs

### In this make procedure, use the "xmakefile" made in advance.
### Needn't the procedure below here which is executed in original Makefile.
###
###xmakefile: ymakefile config.h
###	rm -f xmakefile
###	$(CPP) ymakefile | sed -e 's/^#.*//' -e 's/^[ \f	]$$//' | \
###	sed -n -e '/^..*$$/p' > xmakefile

install:	all
	install -c -m 755 xemacs ${DESTROOT}${EMACSBIN}/emacs
### install ../DOC* in PATHEXEC
###	../etc/digest-doc is a command to convert DOC... file
###	to nroff format, and ../etc/Makefile installs digest-doc.
	for i in  ../etc/DOC.17.62.*; do \
		install -c -m 644 $$i ${DESTROOT}${PATHEXEC}; done

clean:
	make -f xmakefile ${MFLAGS} clean

clobber:	clean
	sccs clean

sccsinfo:
	@sccs info

sccsget:
	sccs get SCCS

