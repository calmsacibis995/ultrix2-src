#	1.7	Makefile	85/02/12
DESTROOT=

all:
	@(cd src; make ${MFLAGS})

install: all
	@(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@(cd lib; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	
#  (cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)

	@(cd doc; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@(cd src; make ${MFLAGS} clean)
sccsget:
	@(cd src; sccs get Makefile; make ${MFLAGS} sccsget)
	
#   (cd man; make sccsget)

#	@(cd lib; sccs get Makefile; make ${MFLAGS} sccsget)
	@(cd lib; sccs get Makefile READ_ME)
	@(cd doc; sccs get Makefile; make ${MFLAGS} sccsget)
	@sccs get -s SCCS

sccsinfo:
	@echo -n ../new/apl:
	@sccs info 
	@echo -n ./src:
	@(cd src; make ${MFLAGS} sccsinfo)
	
#   (cd man; make sccsinfo)
	@echo -n ./lib:
	@(cd lib; make ${MFLAGS} sccsinfo)
	@echo -n ./doc:
	@(cd doc; make ${MFLAGS} sccsinfo)

clobber: clean
	@(cd src; make ${MFLAGS} clobber)
#	@(cd lib; make ${MFLAGS} clobber)
	@(cd lib; rm -f Makefile READ_ME)
	@(cd doc; make ${MFLAGS} clobber)
	@rm -f Makefile README

