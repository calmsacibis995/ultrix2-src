#  @(#)Makefile	1.10	ULTRIX	10/3/86
#
#	Makefile	4.13	83/08/18
#
# makefile for line printer spooling system
#
# Ownerships {see note at the top of lpd.c}
# ROOT		a user that can access any and all files
# DAEMON	someone special
# SPGRP		the group id of the spooling programs
#
CFLAGS=-O
DESTROOT=
LIBDIR=/usr/lib
BINDIR=/usr/ucb
SPOOLDIR=/usr/spool/lpd
CXREF=/usr/ucb/ctags -x
ROOT=root
DAEMON=daemon
SPGRP=daemon
SRCS=	lat_conn.c lpd.c lpr.c lpq.c lprm.c pac.c lpd.c cmds.c cmdtab.c \
	printjob.c recvjob.c displayq.c rmjob.c \
	startdaemon.c common.c printcap.c lpdchar.c
ALL=	lpd lpr lpq lprm pac lpc

all:	${ALL} FILTERS

lpd:	lpd.o printjob.o recvjob.o displayq.o rmjob.o
lpd:	startdaemon.o lpdchar.o common.o printcap.o lat_conn.o
	${CC} -o lpd lpd.o printjob.o recvjob.o displayq.o rmjob.o \
		startdaemon.o lpdchar.o common.o printcap.o lat_conn.o

lpr:	lpr.o startdaemon.o printcap.o
	${CC} -o lpr lpr.o startdaemon.o printcap.o

lpq:	lpq.o displayq.o common.o printcap.o
	${CC} -o lpq lpq.o displayq.o common.o printcap.o -ltermcap

lprm:	lprm.o rmjob.o startdaemon.o common.o printcap.o
	${CC} -o lprm lprm.o rmjob.o startdaemon.o common.o printcap.o

lpc:	lpc.o cmds.o cmdtab.o startdaemon.o common.o printcap.o
	${CC} -o lpc lpc.o cmds.o cmdtab.o startdaemon.o common.o printcap.o

pac:	pac.o printcap.o
	${CC} -o pac pac.o printcap.o

lpd.o lpr.o lpq.o lprm.o pac.o: lp.h lp.local.h
recvjob.o printjob.o displayq.o rmjob.o common.o: lp.h lp.local.h
startdaemon.o: lp.local.h
lpc.o cmdtab.o: lpc.h
cmds.o: lp.h lp.local.h

FILTERS:
	cd filters; make ${MFLAGS}

install: all
	-if [ ! -d ${DESTROOT}/${LIBDIR} ]; then \
		mkdir ${DESTROOT}/${LIBDIR}; \
		chmod 755 ${DESTROOT}/${LIBDIR}; \
		/etc/chown root ${DESTROOT}/${LIBDIR}; \
		chgrp system  ${DESTROOT}/${LIBDIR}; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/etc ]; then \
		mkdir ${DESTROOT}/etc; \
		chmod 755 ${DESTROOT}/etc; \
		/etc/chown root ${DESTROOT}/etc; \
		chgrp system  ${DESTROOT}/etc; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/${BINDIR} ]; then \
		mkdir ${DESTROOT}/${BINDIR}; \
		chmod 755 ${DESTROOT}/${BINDIR}; \
		/etc/chown root ${DESTROOT}/${BINDIR}; \
		chgrp system  ${DESTROOT}/${BINDIR}; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/spool ]; then \
		mkdir ${DESTROOT}/usr/spool; \
		chmod 755 ${DESTROOT}/usr/spool; \
		/etc/chown root ${DESTROOT}/usr/spool; \
		chgrp system  ${DESTROOT}/usr/spool; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/${SPOOLDIR} ]; then \
		mkdir ${DESTROOT}/${SPOOLDIR}; \
		chmod 755 ${DESTROOT}/${SPOOLDIR}; \
		/etc/chown root ${DESTROOT}/${SPOOLDIR}; \
		chgrp system  ${DESTROOT}/${SPOOLDIR}; \
	else true; \
	fi
	install -c -s -g system -m 700 lpd ${DESTROOT}/${LIBDIR}/lpd
	install -c -s -g ${SPGRP} -m 6711 lpr ${DESTROOT}/${BINDIR}/lpr
	install -c -s -g ${SPGRP} -m 6711 lpq ${DESTROOT}/${BINDIR}/lpq
	install -c -s -g ${SPGRP} -m 6711 lprm ${DESTROOT}/${BINDIR}/lprm
	install -c -s -g ${SPGRP} -m 2711 lpc ${DESTROOT}/etc/lpc
	install -c -s pac ${DESTROOT}/etc/pac
	/etc/chown ${DAEMON} ${DESTROOT}/${SPOOLDIR}
	chgrp ${SPGRP} ${DESTROOT}/${SPOOLDIR}
	chmod 775 ${DESTROOT}/${SPOOLDIR}
	cd filters; make ${MFLAGS} DESTROOT=${DESTROOT} install

clean:
	rm -f ${ALL} *.o
	(cd filters; make ${MFLAGS} clean)

clobber: clean
	rm -f *.[ch] Makefile etc.printcap
	(cd filters; make ${MFLAGS} clobber)

sccsinfo:
	(cd filters; make ${MFLAGS} sccsinfo)
	sccs info

sccsget:
	(cd filters;sccs get Makefile; make ${MFLAGS} sccsget)
	sccs get SCCS

print:
	@pr makefile
	@${CXREF} *.c | pr -h XREF
	@pr *.h *.c
