#		@(#)Makefile	1.3	(ULTRIX)	2/1/85
#
#	Make file for RCS.
#

DESTROOT=

SUBDIR	= src/rcs src/rdiff src/rdiff3 man/man1 man/man5 doc
DIRS	= /usr /usr/new /usr/new/lib /usr/new/man \
	  /usr/new/man/man1 /usr/new/man/man5 \
	  /usr/new/doc /usr/new/doc/rcs

all:	${SUBDIR}

${SUBDIR}:	FRC
	cd $@; make ${MFLAGS}

install: all 
	-for i in ${DIRS}; do \
		(if [ ! -d ${DESTROOT}$$i ] ; then \
			mkdir ${DESTROOT}$$i; \
			chmod 755 ${DESTROOT}$$i; \
			/etc/chown root ${DESTROOT}$$i; \
			chgrp system ${DESTROOT}$$i;  \
		 else true;\
		 fi \
		) ; done
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done

clean:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done


clobber: clean
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i;echo $$i;make ${MFLAGS} sccsinfo); done

sccsget:
	for i in ${SUBDIR}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	sccs get SCCS

.DEFAULT:
	sccs get $<
depend:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} depend); done

FRC:
