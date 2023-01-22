# @(#)Makefile	1.12	ULTRIX	11/27/84
#
#	@(#)Makefile	1.3	(Berkeley)	4/17/83
#
DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	pi pc0 px pxp pdx utilities

TOOLSDIR= pc0 utilities

all:	${SUBDIR}

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

tools:
	for i in ${TOOLSDIR}; do \
	 (echo $$i; cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} tools); done

install: all
	for i in ${SUBDIR}; do \
	 (echo $$i; cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do \
		(echo $$i; cd $$i; make ${MFLAGS} clean); \
	done

clobber: clean
	rm -f Makefile
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f src/*.[ch] src/[a-z]* src/*.awk src/*.csh

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS 
	for i in ${SUBDIR}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	(cd src; sccs get SCCS)
