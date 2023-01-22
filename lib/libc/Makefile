#  @(#)Makefile	1.16	ULTRIX	10/3/86
#
#	Makefile	4.6	83/07/02
#
# The system has two monitoring schemes, utilizing either ``prof''
# or ``gprof''. DFLMON determines the default profiler; mon.o
# specifies that ``prof'' is the default, gmon.o specifies that
# ``gprof'' is the default.
#
# Machine dependent routines are located in a subtree which parallels
# the top directories.	This subtree is identified by the machine name.
#
# Compatibility routines are kept in directories with a prefixing
# ``compat'' (so they all sort together).
#
# The C run-time startup code is always machine dependent and expected
# to be located in ${MACHINE}/csu
#
DESTROOT=
INSTALL=install -c -m 644
DFLMON= mon.o
MACHINE=vax
COMPAT= compat-4.1 compat-sys5
ALL=	gen inet net rpc rpcsvc stdio ${MACHINE} ${COMPAT}

libc.a libcg.a libc_p.a: ${MACHINE}/csu ${ALL}
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		rm -f tmp/*; \
	fi
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libc.a `lorder \`cat t1\` | tsort` ${MACHINE}/csu/${DFLMON}
	ar m libc.a exit.o cleanup.o _exit.o cerror.o
	rm -f t1 tmp/*
	for i in ${ALL}; do (cd tmp; ar x ../$$i/gfloat/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libcg.a `lorder \`cat t1\` | tsort`
	ar m libcg.a exit.o cleanup.o _exit.o cerror.o
	rm -f t1 tmp/*
	for i in ${ALL}; do (cd tmp; ar x ../$$i/profiled/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libc_p.a `lorder \`cat t1\` | tsort` ${MACHINE}/csu/${DFLMON}
	ar m libc_p.a exit.o cleanup.o _exit.o cerror.o
	rm -rf t1 tmp

${MACHINE}/csu ${ALL}: FRC
	cd $@; make ${MFLAGS}

FRC:

clean:
	for i in ${MACHINE}/csu ${ALL}; do (cd $$i; make ${MFLAGS} clean); done
	rm -rf t1 tmp
	rm -f libc.a libc_p.a libcg.a

clobber: clean
	for i in ${MACHINE}/csu ${ALL}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile

sccsinfo:
	sccs info
	for i in ${MACHINE}/csu ${ALL}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${MACHINE}/csu ${ALL}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done

install: libc.a libc_p.a
	${INSTALL} libc.a ${DESTROOT}/lib/libc.a
	ranlib ${DESTROOT}/lib/libc.a
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true ; \
	fi
	${INSTALL} libcg.a ${DESTROOT}/usr/lib/libcg.a
	ranlib ${DESTROOT}/usr/lib/libcg.a
	${INSTALL} libc_p.a ${DESTROOT}/usr/lib/libc_p.a
	ranlib ${DESTROOT}/usr/lib/libc_p.a
	cd ${MACHINE}/csu; make DFLMON=${DFLMON} DESTROOT=${DESTROOT} install

tools: ${MACHINE}/csu
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} tools); done
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		rm -f tmp/*; \
	fi
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib); done
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libc.a `lorder \`cat t1\` | tsort` ${MACHINE}/csu/${DFLMON}
	ar m libc.a exit.o cleanup.o _exit.o cerror.o
	rm -rf t1 tmp
	${INSTALL} libc.a ${DESTROOT}/lib/libc.a
	ranlib ${DESTROOT}/lib/libc.a
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true ; \
	fi
	cd ${MACHINE}/csu; make DFLMON=${DFLMON} DESTROOT=${DESTROOT} install
