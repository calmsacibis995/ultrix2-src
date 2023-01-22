# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.1	(Berkeley)	10/19/82
#
DESTROOT=
CFLAGS=-O 
SOURCES=vfontedpr.c vgrindefs.c regexp.c retest.c
CMDS=vfontedpr retest
OBJS=retest.o regexp.o vfontedpr.o vgrindefs.o

all:	$(CMDS) ${OBJS}

vfontedpr: vfontedpr.o vgrindefs.o regexp.o
	cc ${CFLAGS} -o vfontedpr vfontedpr.o vgrindefs.o regexp.o
retest: retest.o regexp.o
	cc ${CFLAGS} -o retest retest.c regexp.o

install: all
	-if [ ! -d ${DESTROOT}/usr/lib/tmac ] ; then \
		mkdir ${DESTROOT}/usr/lib/tmac; \
		chmod 755 ${DESTROOT}/usr/lib/tmac; \
		/etc/chown root ${DESTROOT}/usr/lib/tmac; \
		chgrp system ${DESTROOT}/usr/lib/tmac; \
	else true; \
	fi
	install -c -s vfontedpr ${DESTROOT}/usr/lib/vfontedpr
	install -c vgrind.sh ${DESTROOT}/usr/ucb/vgrind
	install -c vgrindefs.src ${DESTROOT}/usr/lib/vgrindefs
	install -c tmac.vgrind ${DESTROOT}/usr/lib/tmac/tmac.vgrind

clean:
	rm -f ${CMDS} ${OBJS}

clobber: clean
	rm -f *.c tmac.vgrind *.sh vgrindefs.src

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

vprintall:
	csh /usr/ucb/vprint ${SOURCES}
