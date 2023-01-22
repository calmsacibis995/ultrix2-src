#		@(#)Makefile	1.4	(ULTRIX)	2/12/85
#
#	Makefile to build all of INGRES.
#
#

DESTROOT=
USR=	${DESTROOT}/usr
SYSBIN=	${DESTROOT}/usr/new
SYSLIB=	${DESTROOT}/usr/lib
NEWLIB=	${DESTROOT}/usr/new/lib
INGRES= ${NEWLIB}/ingres
BIN=	${INGRES}/bin
LIB=	${INGRES}/lib
FILES=	${INGRES}/files
DEMO=	${INGRES}/demo
DATA=	${INGRES}/data
DBASE=	${DATA}/base
DOC=	${INGRES}/doc
DIRS=	${USR} ${SYSBIN} ${NEWLIB} ${SYSLIB} \
	${INGRES} ${BIN} ${LIB} ${FILES} ${DEMO} ${DATA} ${DBASE} ${DOC}
INSTALL=install
CFLAGS=	-O


SUBDIR=	 source files demo doc

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} 

FRC:

#
# Make all the needed directories here instead of having the Makefile
# for each subdirectory worry about it.  The "install" entry here does
# not depend on the "all" entry, since the "install" entry in the
# lower level directories depends on the "all".  Also, the "install" entry
# for all lower level directories "installs" things relative to INGRES,
# which is itself relative to DESTROOT.
#
install:  Makefile.install README
	-for i in ${DIRS}; do \
		(if [ ! -d $$i ] ; then \
			mkdir $$i ; \
			case $$i in  \
			  ${DATA}) chmod 700 $$i ;; \
			  ${DBASE}) chmod 777 $$i ;; \
			  *) chmod 755 $$i ;; \
			esac ; \
			case $$i in \
			  ${INGRES}*) /etc/chown ingres $$i ;; \
			  *) /etc/chown root $$i ;; \
			esac ; \
			chgrp system $$i ; \
		 else true ; \
		 fi \
		) ; done
	for i in ${SUBDIR}; do \
		(cd $$i;\
		 make ${MFLAGS} "INGRES=${INGRES}" install\
		); done
	${INSTALL} -c -m 444 -o ingres README ${INGRES}
	${INSTALL} -c -m 444 -o ingres Makefile.install ${INGRES}/Makefile
	cd ${INGRES} ; make "BIN=${SYSBIN}" \
			    "LIB=${NEWLIB}" \
			    "ETC=${SYSBIN}"

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile Makefile.install README

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i;echo $$i;make ${MFLAGS} sccsinfo); done

sccsget:
	for i in ${SUBDIR}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	sccs get SCCS

.DEFAULT:
	sccs get $<
