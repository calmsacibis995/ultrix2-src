#  @(#)Makefile	1.3	ULTRIX	2/12/87
#****************************************************************
#*								*
#*  Licensed to Digital Equipment Corporation, Maynard, MA	*
#*		Copyright 1985 Sun Microsystems, Inc.		*
#*			All rights reserved.			*
#*								*
#****************************************************************
#
CFLAGS=	-O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo
MAKE=/bin/make
SCCS=/usr/ucb/sccs

SRCS= nfsstat.c ping.c rpc.yppasswd.c rpcinfo.c showmount.c rwalld.c


# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIRS=	yp ypserv

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ping rpc.yppasswdd rpcinfo nfsstat showmount rwalld

# Programs that must run setuid to root
#
SETUID=	ping

# C programs that live in the current directory and need
# explicit make lines.
#
NSTD=

all:	${SUBDIRS} ${STD}

${SUBDIRS}: FRC
	cd $@; ${ECHO} ${CWD}; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c


install: ${STD}
	-(if [ ! -d ${DESTROOT}/usr/etc ] ; then \
	    mkdir ${DESTROOT}/usr/etc; \
	    chown bin ${DESTROOT}/usr/etc; \
	    chmod 755 ${DESTROOT}/usr/etc; \
	    else true; \
	fi)
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
#	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/etc/$$i); done
	for i in ${STD}; do \
		(install -s -c $$i ${DESTROOT}/usr/etc/$$i); done
	for i in ${SETUID}; do (chown root ${DESTROOT}/usr/etc/$$i; \
		chmod 4755 ${DESTROOT}/usr/etc/$$i); done

clean:
	rm -f a.out core *.s *.o ${STD}
	for i in ${SUBDIRS}; do (cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} clean); done

lint:
	lint $(SRCS)

clobber: clean
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${MAKE} clobber; \
		 ${ECHO} Exiting ${CWD}); done
	${SCCS} clean

sccsget:
	${SCCS} get ${DATE} SCCS
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${SCCS} get ${DATE} Makefile; ${MAKE} sccsget; \
		${ECHO} Exiting ${CWD}); done

sccsinfo:
	@${SCCS} info;
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${MAKE} sccsinfo; \
		 ${ECHO} Exiting ${CWD}); done
	
FRC:

depend:
	cat </dev/null >x.c
	for i in ${STD}; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
			-e '/\.\.\/h/d' \
			-e '/\.\.\/ufs/d' \
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


# DO NOT DELETE THIS LINE -- make depend uses it
# IF YOU PUT STUFF HERE IT WILL GO AWAY
ping: ping.c
ping: /usr/include/stdio.h
ping: /usr/include/sys/types.h
ping: /usr/include/sys/socket.h
ping: /usr/include/netinet/in.h
ping: /usr/include/netdb.h
ping: /usr/include/sys/time.h
ping: /usr/include/netinet/in_systm.h
ping: /usr/include/netinet/ip.h
ping: /usr/include/netinet/ip_icmp.h
ping: /usr/include/signal.h
ping: /usr/include/sys/wait.h
ping: /usr/include/ctype.h
rpc.yppasswdd: rpc.yppasswdd.c
rpc.yppasswdd: /usr/include/stdio.h
rpc.yppasswdd: /usr/include/signal.h
rpc.yppasswdd: /usr/include/sys/file.h
rpc.yppasswdd: /usr/include/rpc/rpc.h
rpc.yppasswdd: /usr/include/pwd.h
rpc.yppasswdd: /usr/include/rpcsvc/yppasswd.h
rpc.yppasswdd: /usr/include/errno.h
rpc.yppasswdd: /usr/include/sys/ioctl.h
rpc.yppasswdd: /usr/include/sys/socket.h
rpc.yppasswdd: /usr/include/sys/wait.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
