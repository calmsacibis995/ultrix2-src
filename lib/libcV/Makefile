# @(#)Makefile	1.4	ULTRIX	1/29/87
#
#	Makefile	4.6	83/07/02
#
# The system has two monitoring schemes, utilizing either ``prof''
# or ``gprof''. DFLMON determines the default profiler; mon.o
# specifies that ``prof'' is the default, gmon.o specifies that
# ``gprof'' is the default.
#
# Machine dependent routines are located in a subtree which parallels
# the top directories.  This subtree is identified by the machine name.
#
#
DESTROOT=
INSTALL=install -c -m 644
DFLMON=	mon.o
MACHINE=vax
ALL=	gen stdio sys ${MACHINE}

libcV.a libcVg.a libcV_p.a: ${ALL}
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		rm -f tmp/*; \
	fi
	cd tmp; for i in ${ALL}; do (ar x ../$$i/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libcV.a `lorder \`cat t1\` | tsort` 
	ar m  libcV.a 
	rm -f t1 tmp/*
	cd tmp; for i in ${ALL}; do (ar x ../$$i/gfloat/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libcVg.a `lorder \`cat t1\` | tsort` 
	ar m  libcVg.a 
	rm -f t1 tmp/*
	cd tmp; for i in ${ALL}; do (ar x ../$$i/profiled/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libcV_p.a `lorder \`cat t1\` | tsort`
	ar m  libcV_p.a 
	rm -rf t1 tmp

${ALL}: FRC
	cd $@; make ${MFLAGS}

FRC:

clean:
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clean); done
	rm -rf t1 tmp *.a

clobber: clean
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile

sccsinfo:
	sccs info
	for i in ${ALL}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${ALL}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done

install: libcV.a libcV_p.a 
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true ; \
	fi
	${INSTALL} libcV.a ${DESTROOT}/usr/lib/libcV.a
	ranlib ${DESTROOT}/usr/lib/libcV.a
	${INSTALL} libcVg.a ${DESTROOT}/usr/lib/libcVg.a
	ranlib ${DESTROOT}/usr/lib/libcVg.a
	${INSTALL} libcV_p.a ${DESTROOT}/usr/lib/libcV_p.a
	ranlib ${DESTROOT}/usr/lib/libcV_p.a

