#  @(#)Makefile	1.23	ULTRIX	10/3/86
#	Makefile	4.6	83/08/13
#
#	Modifications
#
#	John Hoffman	   04-Sep-1986
#	Add imghdr.h
#
#	Marc Teitelbaum	   25-Jul-1986
#	Add symbolic link /usr/include/lat -> /sys/lat
#
# 	Tom Tresvik        08-Jul-1986
# 	Add symbolic link /usr/include/sas -> /usr/sys/sas
#
#	Barb Glover	   07-Apr-1986
# 005-	Add elwindow.h
#	
#	pete keilty	   21-Jan-1986
# 004-  Add elcsd.h
#
#	David L ballenger, 24-Apr-1985
# 003-	Add ttyent.h and fix problem with string.h symbolic link.
#
#	David L Ballenger, 23-Apr-1985
# 002-	Remove mnttab.h and tp_defs.h from Makefile, they were accidentally
#	add from the System V emulation package.
#
#	David L Ballenger, 31-Mar-1985
# 001-	Add include files for System V emulation.
#
#
# Doing a make install builds /usr/include
#
# Define SHARED to indicate whether you want
# symbolic links to the system source (``symlinks''),
# or a separate copy (``copies'').
# (latter useful in environments where it's
# not possible to keep /sys publicly readable)
#
# The ``rm -rf''s used below are safe because rm doesn't
# follow symbolic links.
#
DESTROOT=
SUBDIRS=arpa pascal
SYS_V=	dial.h execargs.h fatal.h ftw.h limits.h \
	macros.h malloc.h memory.h misc.h mon.h \
	nan.h prof.h regexp.h search.h stand.h term.h termio.h \
	unctrl.h unistd.h ustat.h values.h
STD=	a.out.h ar.h assert.h ctype.h curses.h dbm.h \
	disktab.h dumprestor.h elcsd.h elwindow.h fcntl.h fstab.h grp.h \
	imghdr.h lastlog.h math.h mp.h mtab.h netdb.h nlist.h pwd.h ranlib.h \
	setjmp.h sgtty.h stab.h stdio.h strings.h ttyent.h\
	struct.h syscall.h sysexits.h syslog.h utmp.h varargs.h vfont.h
LINKS=	errno.h signal.h time.h
MACHINE=vax
MACHDEP=${MACHINE} vaxif vaxmba vaxuba
NETDIRS=net netimp netinet netpup netdnet nfs rpc rpcsvc netbsc
SYSDIRS=${NETDIRS} stand sas lat ${MACHDEP} ufs
SHARED=	symlinks

all:

install: ${DESTROOT}/usr/include/. ${SHARED}
	-for i in ${STD} ${SYS_V}; do \
		install -c -m 444 $$i ${DESTROOT}/usr/include/$$i; \
	done
	-for i in ${SUBDIRS}; do \
		if [ ! -d ${DESTROOT}/usr/include/$$i ]; \
		then \
			mkdir ${DESTROOT}/usr/include/$$i; \
		fi; \
		(cd $$i; for j in *.[ih]; do \
			install -c -m 444 $$j ${DESTROOT}/usr/include/$$i/$$j; \
		done); \
	done
	-for i in ${LINKS}; do \
		rm -f ${DESTROOT}/usr/include/$$i; \
		ln -s sys/$$i ${DESTROOT}/usr/include/$$i; \
	done
	rm -f ${DESTROOT}/usr/include/string.h
	ln -s ./strings.h ${DESTROOT}/usr/include/string.h
	rm -f ${DESTROOT}/usr/include/machine
	ln -s ./${MACHINE} ${DESTROOT}/usr/include/machine
	rm -f ${DESTROOT}/usr/include/frame.h
	ln -s machine/frame.h ${DESTROOT}/usr/include/frame.h

${DESTROOT}/usr/include/.: 
	@-if [ ! -d ${DESTROOT}/usr/include ]; \
	then \
		rm -fr ${DESTROOT}/usr/include; \
		mkdir ${DESTROOT}/usr/include; \
		/etc/chown root ${DESTROOT}/usr/include; \
		chgrp system ${DESTROOT}/usr/include; \
		chmod 755 ${DESTROOT}/usr/include; \
	fi

symlinks:
	for i in ${SYSDIRS}; do \
		rm -rf ${DESTROOT}/usr/include/$$i; \
		ln -s /sys/$$i ${DESTROOT}/usr/include/$$i; \
	done
	rm -rf ${DESTROOT}/usr/include/sys
	ln -s /sys/h ${DESTROOT}/usr/include/sys

copies:
	for i in ${SYSDIRS}; do \
		rm -rf ${DESTROOT}/usr/include/$$i; \
		cd /sys; \
		tar cf - $$i/*.h | (cd /usr/include; tar xpf -); \
	done
	rm -rf ${DESTROOT}/usr/include/sys;
	mkdir ${DESTROOT}/usr/include/sys; 
	chmod 775 ${DESTROOT}/usr/include/sys;
	(cd /sys/h; tar cf - *.h | (cd ${DESTROOT}/usr/include/sys; tar xpf -))

clean:

clobber: clean
	for i in ${SUBDIRS}; do \
	(cd $$i; make ${MFLAGS} clobber); \
	done
	rm *.h Makefile
sccsget: 
	for i in ${SUBDIRS}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); \
	done
	sccs get SCCS

sccsinfo:
	sccs info
	for i in ${SUBDIRS}; do \
		(cd $$i; make sccsinfo); \
	done

