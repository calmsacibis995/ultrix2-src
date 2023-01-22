# @(#)Makefile	1.11	ULTRIX	10/3/86
# Makefile to bootstrap SPMS
#
#
DESTROOT=

all:;		@cd bin; pmake

clean:;

clobber:;

depend:;	@cd bin; pdepend

install:	all
		@cd src/bin; make ${MFLAGS} DESTROOT=${DESTROOT} install;
		@cd src/lib/pgrep; make ${MFLAGS} DESTROOT=${DESTROOT} \
install;
		@cd src/lib/phelp; make ${MFLAGS} DESTROOT=${DESTROOT} \
install;
		@cd src/lib/x.Makefile; make ${MFLAGS} DESTROOT=${DESTROOT} \
install;

update:;	@pexec -P~root\^usr\^spms -Tupdate make DESTROOT=$(DESTROOT) update
sccsget:
		sccs get SCCS
		@cd doc; sccs get SCCS
		@cd include; sccs get SCCS
		@echo "the following areas are NOT under SCCS: "
		@echo "spms/bin,spms/lib,spms/man,spms/src,spms/work"
		@echo "However, they are 'gotten' correctly - rsp"

sccsinfo:

		sccs info
		@cd doc;sccs info
		@cd include;sccs info
		@echo "the following areas are NOT under SCCS: "
		@echo "spms/bin,spms/lib,spms/man,spms/src,spms/work"
