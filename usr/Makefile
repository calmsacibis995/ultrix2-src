#  @(#)Makefile	1.37	ULTRIX	1/29/87
#
# - - - - -
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 01	30-May-86, Greg Tarsa
#	Removed refer papers from the file.
#
DESTROOT=
DIRS= mdec pub spool skel doc 

MANDIRS = man1 man2 man3 man4 man5 man6 man7 man8

# //library to remind me that there's probably a chroot going on
LIBRARY=//library

all: whatis

# Install is essentially handwired right now.  
# It does lots of misc. things while the real work is in
# usr.lib & usr.bin's Makefiles
install:
	-if [ ! -d ${DESTROOT}/usr/spool ]; \
	then \
		mkdir ${DESTROOT}/usr/spool; \
		chmod 755 ${DESTROOT}/usr/spool; \
	else \
		true; \
	fi
	(cd spool;  make DESTROOT=${DESTROOT} install )
	-if [ ! -d ${DESTROOT}/usr/skel ]; \
	then \
		mkdir ${DESTROOT}/usr/skel; \
		chmod 755 ${DESTROOT}/usr/skel; \
	else \
		true; \
	fi

	(cd doc; make DESTROOT=${DESTROOT} install )

	(cd skel;  make DESTROOT=${DESTROOT} install )
	install -c w2006 ${DESTROOT}/usr/lib/w2006
	-if [ ! -d ${DESTROOT}/usr/pub ]; \
	then \
		mkdir ${DESTROOT}/usr/pub; \
		chmod 755 ${DESTROOT}/usr/pub; \
	else \
		true; \
	fi
	(cd pub; make DESTROOT=${DESTROOT} install)
	-if [ ! -d ${DESTROOT}/usr/mdec ]; \
	then \
		mkdir ${DESTROOT}/usr/mdec; \
		chmod 755 ${DESTROOT}/usr/mdec; \
	else \
		true; \
	fi
	(cd mdec; make DESTROOT=${DESTROOT} install)
	-if [ ! -d ${DESTROOT}/usr/tmp ]; \
	then \
		mkdir ${DESTROOT}/usr/tmp; \
		chmod 777 ${DESTROOT}/usr/tmp; \
	else \
		true; \
	fi

	-if [ ! -d ${DESTROOT}/usr/adm ]; \
	then \
		mkdir ${DESTROOT}/usr/adm; \
		chmod 755 ${DESTROOT}/usr/adm; \
		/etc/chown root ${DESTROOT}/usr/adm; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/crash ]; \
	then \
		mkdir ${DESTROOT}/usr/adm/crash; \
		chmod 755 ${DESTROOT}/usr/adm/crash; \
		/etc/chown root ${DESTROOT}/usr/adm/crash; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/syserr ]; \
	then \
		mkdir ${DESTROOT}/usr/adm/syserr; \
		chmod 755 ${DESTROOT}/usr/adm/syserr; \
		/etc/chown root ${DESTROOT}/usr/adm/syserr; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/snap ]; \
	then \
		mkdir ${DESTROOT}/usr/adm/snap; \
		chmod 755 ${DESTROOT}/usr/adm/snap; \
		/etc/chown root ${DESTROOT}/usr/adm/snap; \
	else \
		true; \
	fi
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/acct
	install -c -m 600 -o uucp /dev/null ${DESTROOT}/usr/adm/aculog
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/lastlog
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/lpd-errs
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/messages
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/msgbuf
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/shutdownlog
	install -c -m 644 /dev/null ${DESTROOT}/usr/adm/wtmp
	-if [ ! -d ${DESTROOT}/tmp ]; \
	then \
		mkdir ${DESTROOT}/tmp; \
		chmod 777 ${DESTROOT}/tmp; \
		/etc/chown root ${DESTROOT}/tmp; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/mnt ]; \
	then \
		mkdir ${DESTROOT}/mnt; \
		chmod 755 ${DESTROOT}/mnt; \
		/etc/chown root ${DESTROOT}/mnt; \
	else \
		true; \
	fi
	install -c -o root /dev/null ${DESTROOT}/usr/restoresymtable
# Move the man
	-if [ ! -d ${DESTROOT}/usr/man ]; \
	then \
		mkdir ${DESTROOT}/usr/man; \
		/etc/chown root ${DESTROOT}/usr/man; \
		chmod 755 ${DESTROOT}/usr/man; \
	else \
		true; \
	fi
	(cd man; tar -cf - . | (cd ${DESTROOT}/usr/man; tar -xf -))
	for i in ${MANDIRS}; do \
		(cd ${DESTROOT}/usr/man/$$i; chmod 444 *); done

	-if [ ! -d ${DESTROOT}/usr/users ]; \
	then \
		mkdir ${DESTROOT}/usr/users; \
	else \
		true; \
	fi

	-if [ ! -d ${DESTROOT}/library ]; \
	then \
		mkdir ${DESTROOT}/library; \
	else \
		true; \
	fi

	@echo 'install kits'
	-if [ ! -d ${DESTROOT}/usr/kits ]; \
	then \
		mkdir ${DESTROOT}/usr/kits; \
	else \
		true; \
	fi
	(cd kits; make ${MFLAGS} LIBRARY=${LIBRARY} DESTROOT=${DESTROOT} install)
	

whatis:


sccsget:
	sccs get SCCS
	@for i in ${DIRS}; do (cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done
	(cd kits; sccs get Makefile; make ${MFLAGS} sccsget LIBRARY='../../')

sccsinfo:
	sccs info
	@for i in ${DIRS}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo); done
	@(cd kits; echo 'kits'; sccs info);

clean:
	(cd doc; make clean)
	(cd kits; make clean)
clobber:
	(cd doc; make clobber)
	(cd kits; make clobber)

