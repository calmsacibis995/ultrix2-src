#	%W%	Ultrix	%G%
#
# DESTROOT=
# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	
all:

install:

clean:
	rm -f *.[ch]
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} clobber); done
	rm -f  Makefile

sccsinfo:
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} sccsinfo); done
	sccs info

sccsget:
	for i in ${SUBDIR}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); \
			done
	sccs get SCCS

