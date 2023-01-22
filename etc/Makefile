#  @(#)Makefile	1.54	ULTRIX	2/12/87
#
#	Makefile	4.29	83/08/28
#
#	Modification History
#
#	Ray Glaser,	xx-Jun-86
# 001 - Added  ypbind  for Yellow Pages
#
#
DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	config dump dump.4.1 fsck ftpd getty htable implog lprsetup restore \
	routed rwhod termcap opser lcp arff mop uerf 2780d ris

TOOLSDIR= config fsck termcap arff 

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	mklost+found chk nfssetup ypsetup

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ac accton arp bad144 badsect biod bscconfig catman chpt \
	chown clri \
	comsat cron dcheck dgated diskpart dumpfs edquota elcsd flcopy \
	fsirand \
	gettable halt icheck ifconfig inetd init kern_loop kern_unloop \
	kgmon miscd mkfs mknod \
	mkproto mount mountd ncheck netsetup newfs nfsd nfs_mount nfs_umount \
	portmap pstat quot quotaon quotacheck radisk rdate rdt \
	reboot renice repquota rexecd rlogind rmt route rshd rxformat sa \
	savecore swapon telnetd tftpd trpt tunefs ufs_mount umount \
	update vipw ypbind

TOOLSSTD= chown diskpart flcopy newfs mkfs mknod mkproto mount \
	ufs_mount nfs_mount nfs_umount umount

# C programs that live in the current directory and need explicit make lines.
#
NSTD= shutdown eli snapcopy

all:	${SUBDIR} ${STD} ${NSTD}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

${NSTD}:
	cc ${CFLAGS} $@.c -lerrlog -o $@

tools: ${TOOLSDIR} ${TOOLSSTD}
	@-if [ ! -d ${DESTROOT}/etc ] ; then \
		mkdir ${DESTROOT}/etc; \
		chmod 755 ${DESTROOT}/etc; \
		/etc/chown root ${DESTROOT}/etc; \
		chgrp system ${DESTROOT}/etc; \
		else true; \
	fi
	for i in ${TOOLSDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/etc/$$i); done
	for i in ${TOOLSSTD}; do (install -s -c $$i ${DESTROOT}/etc/$$i); done

install: ${STD} ${NSTD}
	@-if [ ! -d ${DESTROOT}/etc ] ; then \
		mkdir ${DESTROOT}/etc; \
		chmod 755 ${DESTROOT}/etc; \
		/etc/chown root ${DESTROOT}/etc; \
		chgrp system ${DESTROOT}/etc; \
		else true; \
	fi
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/etc/$$i); done
	for i in ${STD} ${NSTD}; do (install -s -c $$i ${DESTROOT}/etc/$$i); done
	rm -f ${DESTROOT}/etc/quotaoff
	ln ${DESTROOT}/etc/quotaon ${DESTROOT}/etc/quotaoff
	install -c -m 644 -o root acucap ${DESTROOT}/etc/acucap
	install -c -m 644 -o root /dev/null ${DESTROOT}/etc/dgateway
	install -c -m 644 -o root /dev/null ${DESTROOT}/etc/mtab
	install -c -m 644 -o root /dev/null ${DESTROOT}/etc/utmp
	install -c -m 644 -o root /dev/null ${DESTROOT}/etc/dumpdates
	install -c -m 644 -o root printcap.examples ${DESTROOT}/etc/printcap.examples
	ln -s /usr/etc/nfsstat $(DESTROOT)/etc/nfsstat
	ln -s /usr/etc/ping $(DESTROOT)/etc/ping
	ln -s /usr/etc/rpc.yppasswdd $(DESTROOT)/etc/rpc.yppasswdd
	ln -s /usr/etc/rpcinfo $(DESTROOT)/etc/rpcinfo
	ln -s /usr/etc/showmount $(DESTROOT)/etc/showmount

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD}

clobber: clean
	rm -f Makefile *.[chlyp] printcap.examples
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIR}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done

FRC:

depend:
	cat </dev/null >x.c
	for i in ${STD} ${NSTD}; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[	]*include' x.c $$i.c | sed \
			-e '/\.\.\/h/d' \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c//' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# Files listed in ${NSTD} have explicit make lines given below.

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
