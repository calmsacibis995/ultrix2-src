# @(#)Makefile	1.8	ULTRIX	12/4/84
#
#	Makefile	4.6	83/09/21
#
ALL=	libplot lib4014 lib300 lib300s lib450 libvt0
SUBDIRS=plot t4014 t300 t300s t450 vt0

all:	${ALL}

libplot: FRC
	cd plot; make ${MFLAGS}

lib4014: FRC
	cd t4014; make ${MFLAGS}

lib300: FRC
	cd t300; make ${MFLAGS}

lib300s: FRC
	cd t300s; make ${MFLAGS}

lib450: FRC
	cd t450; make ${MFLAGS}

libvt0: FRC
	cd vt0; make ${MFLAGS}

FRC:

install: all

	for i in ${ALL}; do \
		install -c -m 644 $$i ${DESTROOT}/usr/lib/$$i.a; \
		ranlib ${DESTROOT}/usr/lib/$$i.a; \
	done
	
	install -c ${DESTROOT}/usr/lib/lib300.a ${DESTROOT}/usr/lib/libt300.a
	install -c ${DESTROOT}/usr/lib/lib300s.a ${DESTROOT}/usr/lib/libt300s.a
	install -c ${DESTROOT}/usr/lib/lib4014.a ${DESTROOT}/usr/lib/libt4014.a
	install -c ${DESTROOT}/usr/lib/lib450.a ${DESTROOT}/usr/lib/libt450.a

clean:
	rm -f ${ALL} errs a.out core
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} clean); \
	done

clobber: clean
	for i in ${SUBDIRS}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f lib* Makefile

sccsinfo:
	sccs info
	for i in ${SUBDIRS}; do (cd $$i make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIRS}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done

.DEFAULT:
	sccs get $<
