#  @(#)Makefile	1.19	ULTRIX	10/3/86
#
#	Modification History
#
# 	22-Apr-1985,	dlb
# 001	Add FRC rule to force subdirectories to be "made".
#

DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	apl courier cpm dsh hyper learn mh newcsh news \
	tools bib icon ingres rcs notes gnuemacs
#  sccstorcs   ! spms not here right now - lp
SAFEDIR= apl bib cpm dsh hyper ingres learn mh \
	newcsh notes news rcs tools gnuemacs

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

FRC:;

install: all
	@-if [ ! -d ${DESTROOT}/usr/new ] ; then \
		mkdir ${DESTROOT}/usr/new ; \
		chmod 755 ${DESTROOT}/usr/new ; \
		/etc/chown root ${DESTROOT}/usr/new ; \
		chgrp system ${DESTROOT}/usr/new ; \
	else true; \
	fi

	@-if [ ! -d ${DESTROOT}/usr/new/doc ] ; then \
		mkdir ${DESTROOT}/usr/new/doc ; \
		chmod 755 ${DESTROOT}/usr/new/doc ; \
		/etc/chown root ${DESTROOT}/usr/new/doc ; \
		chgrp system ${DESTROOT}/usr/new/doc ; \
	else true; \
	fi

	@-if [ ! -d ${DESTROOT}/usr/new/doc/manuals ] ; then \
		mkdir ${DESTROOT}/usr/new/doc/manuals ; \
		chmod 755 ${DESTROOT}/usr/new/doc/manuals ; \
		/etc/chown root ${DESTROOT}/usr/new/doc/manuals ; \
		chgrp system ${DESTROOT}/usr/new/doc/manuals ; \
	else true; \
	fi

	@-if [ ! -d ${DESTROOT}/usr/new/lib ] ; then \
		mkdir ${DESTROOT}/usr/new/lib ; \
		chmod 755 ${DESTROOT}/usr/new/lib ; \
		/etc/chown root ${DESTROOT}/usr/new/lib ; \
		chgrp system ${DESTROOT}/usr/new/lib ; \
	else true; \
	fi

	@for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	@(cd spms; make ${MFLAGS} DESTROOT=${DESTROOT} install)


clean:
	@for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done
clobber: clean
	@rm -f Makefile README
	@for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done

sccsget:
	@echo Getting ../new
	@sccs get -s SCCS
	@for i in ${SUBDIR}; do (cd $$i;sccs get -s Makefile; make ${MFLAGS} sccsget); done

sccsinfo:
	@echo -n ../new:
	@sccs info 
	@for i in ${SUBDIR}; do (cd $$i; make sccsinfo); done
