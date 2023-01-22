# @(#)Makefile	1.12	ULTRIX	4/16/85
#
#	@(#)Makefile	4.2	(Berkeley)	11/22/82
#
#	Modifications
#
#	David L Ballenger, 1-Apr-1985
# 001-	Add libcV to subdirectories.
#

DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	c2 cpp libc pcc libcV
TOOLNORM= c2 cpp pcc
TOOLSPEC= libc

all:	${SUBDIR}

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

install:  all
	-if [ ! -d ${DESTROOT}/lib ] ; then \
		mkdir ${DESTROOT}/lib; \
		chmod 755 ${DESTROOT}/lib; \
		/etc/chown root ${DESTROOT}/lib; \
		chgrp system  ${DESTROOT}/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system  ${DESTROOT}/usr/lib; \
	else true; \
	fi
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done

tools:
	-if [ ! -d ${DESTROOT}/lib ] ; then \
		mkdir ${DESTROOT}/lib; \
		chmod 755 ${DESTROOT}/lib; \
		/etc/chown root ${DESTROOT}/lib; \
		chgrp system  ${DESTROOT}/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system  ${DESTROOT}/usr/lib; \
	else true; \
	fi
	for i in ${TOOLNORM}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${TOOLSPEC}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} tools); done

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile

sccsinfo:
	for i in ${SUBDIR}; do (cd $$i;echo $$i;make ${MFLAGS} sccsinfo); done
	sccs info

sccsget:
	for i in ${SUBDIR}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	sccs get SCCS

.DEFAULT:
	sccs get $<
