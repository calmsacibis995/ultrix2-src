#	%W%	%E%
#
DESTROOT=
# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	chproject mkmf mkproject pcp pd pdiff pexec pfind pgrep \
phelp plog pman pmkdir pmv ppd prmdir pwp rmproject 

SUBDIRR= ptest

all:

install:
	@for i in ${SUBDIR}; do \
	( echo Installing ./$$i; cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	@(echo Installing ./${SUBDIRR}; cd ${SUBDIRR}; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} clean); done

clobber: clean
	rm -f Makefile
	@for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} clobber); done

sccsinfo:
	@for i in ${SUBDIR}; do (echo -n ./$$i:; cd $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	@sccs get -s SCCS
	@for i in ${SUBDIR}; do \
	(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); \
	done
