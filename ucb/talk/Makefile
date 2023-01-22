# @(#)Makefile	1.5	ULTRIX	12/4/84
#
# $Header: /a/guest/moore/talk/RCS/Makefile.dist,v 1.3 83/06/23 02:20:37 moore Exp $

DESTROOT=
DOBJECT = talkd.o announce.o process.o table.o print.o
TOBJECT = talk.o get_names.o display.o io.o ctl.o init_disp.o\
	  msgs.o get_addrs.o ctl_transact.o invite.o look_up.o
DSOURCE = talkd.c announce.c process.c table.c print.c
TSOURCE = talk.c get_names.c display.c io.c ctl.c init_disp.c\
	  msgs.c get_addrs.c ctl_transact.c invite.c look_up.c
INCLUDE = talk.h ctl.h talk_ctl.h
SOURCE = ${DSOURCE} ${TSOURCE}
CFLAGS= -O

all:	talk talkd

talkd:	${DOBJECT}
	cc ${CFLAGS} -o talkd ${DOBJECT}

talk:	${TOBJECT}
	cc -o talk ${TOBJECT} -lcurses -ltermlib

${DOBJECT} : ctl.h
${TOBJECT} : talk.h ctl.h talk_ctl.h

install: talk talkd
	-if [ ! -d ${DESTROOT}/etc ] ; then \
		mkdir ${DESTROOT}/etc; \
		chmod 755 ${DESTROOT}/etc; \
		/etc/chown root ${DESTROOT}/etc; \
		chgrp system ${DESTROOT}/etc; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/ucb ] ; then \
		mkdir ${DESTROOT}/usr/ucb; \
		chmod 755 ${DESTROOT}/usr/ucb; \
		/etc/chown root ${DESTROOT}/usr/ucb; \
		chgrp system ${DESTROOT}/usr/ucb; \
	else true; \
	fi
	install -c -s talk ${DESTROOT}/usr/ucb/talk
	install -c -s talkd ${DESTROOT}/etc/talkd

lint:
	lint ${DSOURCE}

clean:
	rm -f *.o talk talkd

clobber: clean
	rm -f *.[ch] README Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

tags:	${SOURCE} ${INCLUDE}
	ctags ${SOURCE} ${INCLUDE}
