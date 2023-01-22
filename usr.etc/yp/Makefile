#  @(#)Makefile	1.3	ULTRIX	10/3/86
#****************************************************************
#*								*
#*  Licensed to Digital Equipment Corporation, Maynard, MA	*
#*		Copyright 1985 Sun Microsystems, Inc.		*
#*			All rights reserved.			*
#*								*
#****************************************************************
#
CFLAGS = -O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo
LIBS = -ldbm
NSTD= makedbm ypxfr
MAKE=/bin/make
SCCS=/usr/ucb/sccs
STD=  yppush ypset yppoll stdhosts
SUBDIRS = revnetgroup

SRCS = makedbm.c stdhosts.c yppush.c ypset.c ypxfr.c yppoll.c 
SCRIPT= ypinit ypxfr_1perday ypxfr_2perday ypxfr_1perhour

all:	${STD} ${NSTD} ${SUBDIRS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

${NSTD}:
	cc ${CFLAGS} -o $@ $@.c ${LIBS}

${SUBDIRS}: FRC
	cd $@; ${ECHO} ${CWD}; make ${MFLAGS}
FRC:

clean:
	rm -f *.o makedbm yppush stdhosts ypset ypxfr yppoll
	for i in $(SUBDIRS); do (cd $$i; ${ECHO} ${CWD}; make $(MFLAGS) clean); done

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
	@${SCCS} info
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${MAKE} sccsinfo; \
		 ${ECHO} Exiting ${CWD}); done
	
install: all
	-(if [ ! -d ${DESTROOT}/usr/etc/yp ] ; then \
		mkdir ${DESTROOT}/usr/etc/yp; \
		chown bin ${DESTROOT}/usr/etc/yp; \
		chmod 755 ${DESTROOT}/usr/etc/yp; \
		else true; \
	fi)
	install -s -c makedbm $(DESTROOT)/usr/etc/yp
	install -s -c yppush $(DESTROOT)/usr/etc/yp
	install -s -c ypset $(DESTROOT)/usr/etc/yp
	install -s -c ypxfr $(DESTROOT)/usr/etc/yp
	install -s -c yppoll $(DESTROOT)/usr/etc/yp
	install -s -c stdhosts $(DESTROOT)/usr/etc/yp
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/etc/yp/$$i); done
	for i in $(SUBDIRS); do \
		(cd $$i; ${ECHO} ${CWD}; make $(MFLAGS) DESTROOT=$(DESTROOT) install); done
	install -c make.script ${DESTROOT}/usr/etc/yp/Makefile
	rm -rf ${DESTROOT}/etc/yp
	ln -s /usr/etc/yp ${DESTROOT}/etc/yp


lint:
	lint $(SRCS)

depend:
	cp /dev/null x.c
	for i in $(SRCS) ; do \
		(/bin/grep '^#[ 	]*include' x.c $$i | sed \
			-e '/\.\.\/h/d' \
			-e '/\.\.\/ufs/d' \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/\.o/' >>makedep); done
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
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
