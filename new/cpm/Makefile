#	1.5	Makefile	84/10/11
#
DESTROOT=

all:
	@echo Make ../new/cpm
	@(cd src; make ${MFLAGS})

install: all
	@echo Install ../new/cpm
	@(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@install -c README ${DESTROOT}/usr/new/doc/README.cpm

clean:
	@(cd src; make ${MFLAGS} clean)
clobber: clean
	@(cd src; make clobber)
	@(cd man; make clobber)
	@rm -f README Makefile

sccsget:
	@sccs get -s SCCS
	@(cd src; sccs get -s Makefile; make sccsget)
	@(cd man; sccs get -s Makefile; make sccsget)

sccsinfo:
	@echo -n ../new/cpm:
	@sccs info
	@echo -n ./src:
	@(cd src; make sccsinfo)
	@echo -n ./man:
	@(cd man; make sccsinfo)
