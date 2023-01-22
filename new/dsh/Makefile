#	1.4	Makefile	85/02/12
DESTROOT=

all:
	@echo Make ../new/dsh
	@(cd src; make ${MFLAGS})

install: all
	@echo Install ../new/dsh
	@(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@(cd src; make ${MFLAGS} clean)
clobber: clean
	@(cd src; make ${MFLAGS} clobber)
	@(cd man; make ${MFLAGS} clobber)
	@rm -f Makefile README

sccsget:
	@sccs get -s SCCS
	@(cd src; sccs get -s Makefile; make sccsget)
	@(cd man; sccs get -s Makefile; make sccsget)

sccsinfo:
	@echo -n ../new/dsh:
	@sccs info
	@echo -n ./src:
	@(cd src; make sccsinfo)
	@echo -n ./man:
	@(cd man; make sccsinfo)
