#	Makefile	1.10	85/02/12
#
DESTROOT=

all:
	@echo Make ../new/bib
	@echo ./src 
	@(cd src; make ${MFLAGS})

install: all
	@echo Installing ./src
	@(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@echo Installing ./macs
	@(cd macs; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@echo Installing ./man
	@(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@echo Installing ./doc
	@(cd doc; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@(cd src; make ${MFLAGS} clean)

sccsget:
	@sccs get -s SCCS
	@(cd src; sccs get Makefile; make sccsget)
	@(cd macs; sccs get Makefile; make sccsget)
	@(cd man; sccs get Makefile; make sccsget)
	@(cd doc; sccs get Makefile; make sccsget)

sccsinfo:
	@echo -n ../new/bib:
	@sccs info
	@echo -n ./src:
	@(cd src; make sccsinfo)
	@echo -n ./macs:
	@(cd macs; make sccsinfo)
	@echo -n ./man:
	@(cd man; make sccsinfo)
	@echo -n ./doc: 
	@(cd doc; make sccsinfo)
clobber: clean
	@(cd src; make clobber)
	@(cd macs; make clobber)
	@(cd man; make clobber)
	@(cd doc; make clobber)
	@rm -f Makefile README

