#		@(#)Makefile	1.2	(ULTRIX)	1/31/85
#
#	Makefile to build all of INGRES.
#
#

INGRES=	../ingres
BIN=	${INGRES}/bin
LIB=	${INGRES}/lib
CPUTYPE=VAX

# Subdirectories for various INGRES utilities, libraries and tools.
#
SUBDIR= h gutil iutil ctlmod dbu decomp equel libq monitor \
	 ovqp parser qrymod support conf

all:	 ${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} CPUTYPE=${CPUTYPE}

FRC:

#
# All the needed directories are made by ../Makefile which also sets
# up the definition of INGRES relative to DESTROOT for an "install".
# 
#
install:
	for i in ${SUBDIR}; do \
		(cd $$i;\
		 make ${MFLAGS} "INGRES=${INGRES}"\
		 		"CPUTYPE=${CPUTYPE}" install\
		); done

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i;echo $$i;make ${MFLAGS} sccsinfo); done

sccsget:
	for i in ${SUBDIR}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	sccs get SCCS

.DEFAULT:
	sccs get $<
