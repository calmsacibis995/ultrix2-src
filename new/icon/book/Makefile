#	1.5	Makefile	84/10/12
DESTROOT=
# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	01 02 04 05 07 08 09 10 11 12 13 14 15 16 17 18 19 20 f 

all:

install:

clean:
	@for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	@for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} clobber); done
	@rm -f Makefile *.ls Readme

sccsinfo:
	@sccs info
	@for i in ${SUBDIR}; do ( cd $$i; echo -n ./book/$$i: ;make ${MFLAGS} sccsinfo); done

sccsget:
	@sccs get -s SCCS
	@for i in ${SUBDIR}; do \
		(cd $$i; sccs get -s Makefile; make ${MFLAGS} sccsget); \
			done

