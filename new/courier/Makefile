#	1.8	Makefile	84/10/11
#
# Makefile for UNIX Courier
# To make the examples, everything must first be installed.
#
DESTROOT=
TARGET=/usr/new/lib/courier
ALL=	compiler lib daemon man doc
EXAMPLES= examples/passwd examples/time

all:	${ALL}
	@echo Make ../new/courier
	@for d in ${ALL}; do (cd $$d; make ${MFLAGS}); done

examples: ${EXAMPLES}
	@for d in ${EXAMPLES}; do (cd $$d; make ${MFLAGS}); done

install: all
	@-if [ ! -d ${DESTROOT}${TARGET} ] ; then \
		mkdir ${DESTROOT}${TARGET} ; \
	else true; \
	fi

	@for d in ${ALL}; \
		do (cd $$d; make ${MFLAGS} DESTROOT=${DESTROOT} install);\
	 done
#	@echo "Please change /etc/rc.local to run \"/usr/new/lib/courierd &\""

clean:
	@for d in ${ALL} ${EXAMPLES}; do (cd $$d; make ${MFLAGS} clean); done

sccsget:
	@echo ../new/courier:
	@sccs get -s SCCS
	@for d in ${ALL}; do (cd $$d;echo ./$$d: ;sccs get -s Makefile; make sccsget); done
	@for d in ${EXAMPLES}; do (cd $$d;echo ./$$d: ;sccs get -s \
		Makefile; make sccsget); done

sccsinfo:
	@echo -n ../new/courier:
	@sccs info
	@for d in ${ALL}; do (cd $$d;echo -n ./$$d: ;  make sccsinfo); done
	@for d in ${EXAMPLES}; do (cd $$d;echo -n ./$$d: ; make sccsinfo); done

clobber: clean
	@echo ../new/courier:
	@for d in ${ALL}; do (cd $$d;echo ./$$d: ; make clobber); done
	@for d in ${EXAMPLES}; do (cd $$d;echo ./$$d: ; make clobber); done
	@rm -f Makefile README

