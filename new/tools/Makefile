#	@(#)Makefile	1.6	85/02/12
DESTROOT=

all:
	cd src; make ${MFLAGS}

install: all
	@-if [ ! -d ${DESTROOT}/usr/new/doc ]; \
	then \
		mkdir ${DESTROOT}/usr/new/doc; \
	else \
		true; \
	fi
	/etc/chown root ${DESTROOT}/usr/new/doc
	(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@(cd src; make ${MFLAGS} clean)
clobber: clean
	@(cd src; make ${MFLAGS} clobber)
	@(cd man; make ${MFLAGS} clobber)
	@rm -f Makefile README

sccsget:
	@sccs get -s SCCS
	@(cd src;sccs get -s Makefile; make sccsget)
	@(cd man;sccs get -s Makefile; make sccsget)

sccsinfo:
	@(echo -n ../new/tools:; sccs info)
	@(cd src;echo -n ./src:; make sccsinfo)
	@(cd man;echo -n ./man:; make sccsinfo)
