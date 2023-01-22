#	1.4	Makefile	84/10/11
DESTROOT=

all:	
	@echo Make ../new/hyper
	@(cd src; make ${MFLAGS})

install: all
	@echo Install ../new/hyper
	@(cd src; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@install -c README ${DESTROOT}/usr/new/doc/README.hyper

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
	@echo -n ../new/hyper:
	@sccs info
	@echo -n ./src:
	@(cd src; make sccsinfo)
	@echo -n ./man:
	@(cd man; make sccsinfo)
