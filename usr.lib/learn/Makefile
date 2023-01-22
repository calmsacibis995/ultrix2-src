# @(#)Makefile	1.10	ULTRIX	4/18/85
#
#	Makefile	4.2	83/07/08
#
# Make install will build the learn lesson libraries
# from this directory. 
#
LESSONS=C editor eqn files macros morefiles
FILES=	Linfo Xinfo
SUBDIRS = eqn files macros morefiles
DESTROOT=

all:

install: ${LESSONS} ${FILES}
	-if [ ! -d ${DESTROOT}/usr/lib/learn ]; \
	then \
		mkdir ${DESTROOT}/usr/lib/learn; \
		chmod 755 ${DESTROOT}/usr/lib/learn; \
		/etc/chown root ${DESTROOT}/usr/lib/learn; \
		chgrp staff ${DESTROOT}/usr/lib/learn; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib/learn/log ]; \
	then \
		mkdir ${DESTROOT}/usr/lib/learn/log; \
		chmod 755 ${DESTROOT}/usr/lib/learn/log; \
		/etc/chown root ${DESTROOT}/usr/lib/learn/log; \
		chgrp staff ${DESTROOT}/usr/lib/learn/log; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib/learn/bin ]; \
	then \
		mkdir ${DESTROOT}/usr/lib/learn/bin; \
		chmod 755 ${DESTROOT}/usr/lib/learn/bin; \
		/etc/chown root ${DESTROOT}/usr/lib/learn/bin; \
		chgrp staff ${DESTROOT}/usr/lib/learn/bin; \
	else \
		true; \
	fi
	@for i in ${FILES}; do \
		echo copy $$i; \
		install -c -m 644 $$i ${DESTROOT}/usr/lib/learn/$$i; \
	done
	@for i in ${LESSONS}; do \
		echo tar $$i lessons; \
		tar cfF - $$i | (cd ${DESTROOT}/usr/lib/learn; tar xpf -); \
	done
	chmod 755 ${DESTROOT}/usr/lib/learn/eqn/Init
	chmod 755 ${DESTROOT}/usr/lib/learn/files/Init

clean:


clobber:
	for i in ${LESSONS}; do (cd $$i; rm -f [Lt]*.* [LIt]*); done	
	rm -f ${FILES} Makefile

sccsinfo:
	for i in ${LESSONS}; do (cd $$i; sccs info); done
	sccs info

sccsget:
	for i in ${LESSONS}; do (cd $$i;sccs get SCCS); done
	sccs get SCCS
	chmod 555 L* X* */L* */I*

.DEFAULT:
	sccs get $<
