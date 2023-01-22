#  @(#)Makefile	1.2	ULTRIX	10/3/86
#****************************************************************
#*								*
#*  Licensed to Digital Equipment Corporation, Maynard, MA	*
#*		Copyright 1985 Sun Microsystems, Inc.		*
#*			All rights reserved.			*
#*								*
#****************************************************************
#

CFLAGS=-O
# CFLAGS = -g
CWD=`pwd`
ECHO=/bin/echo
LDFLAGS = -ldbm 
SCCS=/usr/ucb/sccs
DESTROOT=
DESTDIR=${DESTROOT}

HDRS = ypsym.h
OBJS = ypserv.o ypserv_ancil.o ypserv_map.o ypserv_proc.o
SERVERDOTH =  ypsym.h /usr/include/rpcsvc/yp_prot.h \
/usr/include/rpcsvc/ypclnt.h /usr/include/rpcsvc/ypv1_prot.h
SRCS= ypserv.c ypserv_ancil.c ypserv_map.c ypserv_proc.c

all: ypserv

ypserv: ${OBJS}
	${CC} ${OBJS} -o ypserv ${LDFLAGS}

clean:
	rm -f ${OBJS} core ypserv a.out

clobber: clean
	${SCCS} clean

sccsget:
	${SCCS} get ${DATE} ${HDRS} ${SRCS};

sccsinfo:
	@${SCCS} info
	
install:	ypserv
	install -s -c ypserv ${DESTDIR}/usr/etc/ypserv

lint: ${SRCS}
	lint ${SRCS}

depend:
	cat </dev/null >x.c
	for i in ${SRCS}; do \
		(/bin/grep '^#[ 	]*include' x.c $$i | sed \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/.o/' >>makedep); done
	for i in ${SERVERDOTH}; do \
		(/bin/grep '^#[ 	]*include' x.c $$i | sed \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

#
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

