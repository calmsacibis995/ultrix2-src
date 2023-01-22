#  @(#)Makefile	1.44	ULTRIX	10/3/86
#
#	Makefile	4.10	83/07/09
#
#	Modifications
#
# 006	John Hoffman, 26-Aug-1986
#	Add ps [PostScript filters] in SUBDIRECTORY.
#
# 005	Aya Konishi, 1-Aug-1986
#	Add tmac in SUBDIRECTORY.
#
# 004	Barb Glover, 20-Jan-1986
#	Add liberrlog subdirectory.
#
#	David Metsky, 19-Sep-1985
# 003	Add liby (yacc) subdirectory.
#
#	David Ballenger, 29-May-1985
# 002-	Add libmalloc subdirectory.
#
#	David Ballenger, 1-Apr-1985
# 001-	Add libmV subdirectory.
#

DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	lib2648 libF77 libI77 libU77 libcurses libdbm libg libln \
	libm libmp libnm libpc libplot libtermlib lpr sendmail \
	uucp libmV libmalloc liby libmop ps upars liberrlog tmac
#	refer struct removed since these live in usr.bin - lp

TOOL1DIR= libdbm libg libln libm libpc libtermlib

TOOL2DIR= lib2648 libcurses libdbm libg libln libm libmp libnm libpc \
	  libplot libtermlib uucp tmac libmop upars liberrlog

MYSUBDIRS = adb find font fontinfo learn lisp me ms \
	tabset tmac vfont

# Shell scripts + data files that need only be installed and are never removed.
#	Note that makewhatis is copied on install & an old copy 
#	of whatis (may not be current) is also copied.
SCRIPT=	makewhatis 
DFILES= eign whatis

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	atrun getNAME makekey

# C programs that live in the current directory and need explicit make lines.
#
NSTD=	

all:	${SUBDIR} ${STD} ${NSTD}

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

tools1:
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib ;\
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ] ; then \
		mkdir ${DESTROOT}/usr/bin ; \
		chmod 755 ${DESTROOT}/usr/bin ; \
		/etc/chown root ${DESTROOT}/usr/bin ; \
		chgrp system ${DESTROOT}/usr/bin ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/include ] ; then \
		mkdir ${DESTROOT}/usr/include ; \
		chmod 755 ${DESTROOT}/usr/include ; \
		/etc/chown root ${DESTROOT}/usr/include ; \
		chgrp system ${DESTROOT}/usr/include ; \
	else true; \
	fi
	-(for i in ${MYSUBDIRS}; do \
		if [ ! -d ${DESTROOT}/usr/lib/$$i ] ; then \
			mkdir ${DESTROOT}/usr/lib/$$i; \
			chmod 755 ${DESTROOT}/usr/lib/$$i; \
			/etc/chown root ${DESTROOT}/usr/lib/$$i; \
			chgrp system ${DESTROOT}/usr/lib/$$i; \
	else true; \
	fi; done)
	for i in ${TOOL1DIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} tools); done

tools2: ${TOOL2DIR}
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib ;\
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ] ; then \
		mkdir ${DESTROOT}/usr/bin ; \
		chmod 755 ${DESTROOT}/usr/bin ; \
		/etc/chown root ${DESTROOT}/usr/bin ; \
		chgrp system ${DESTROOT}/usr/bin ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/include ] ; then \
		mkdir ${DESTROOT}/usr/include ; \
		chmod 755 ${DESTROOT}/usr/include ; \
		/etc/chown root ${DESTROOT}/usr/include ; \
		chgrp system ${DESTROOT}/usr/include ; \
	else true; \
	fi
	-(for i in ${MYSUBDIRS}; do \
		if [ ! -d ${DESTROOT}/usr/lib/$$i ] ; then \
			mkdir ${DESTROOT}/usr/lib/$$i; \
			chmod 755 ${DESTROOT}/usr/lib/$$i; \
			/etc/chown root ${DESTROOT}/usr/lib/$$i; \
			chgrp system ${DESTROOT}/usr/lib/$$i; \
	else true; \
	fi; done)
	for i in ${TOOL2DIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done

install: all
	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib ;\
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ] ; then \
		mkdir ${DESTROOT}/usr/bin ; \
		chmod 755 ${DESTROOT}/usr/bin ; \
		/etc/chown root ${DESTROOT}/usr/bin ; \
		chgrp system ${DESTROOT}/usr/bin ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/include ] ; then \
		mkdir ${DESTROOT}/usr/include ; \
		chmod 755 ${DESTROOT}/usr/include ; \
		/etc/chown root ${DESTROOT}/usr/include ; \
		chgrp system ${DESTROOT}/usr/include ; \
	else true; \
	fi
	
	-(for i in ${MYSUBDIRS}; do \
		if [ ! -d ${DESTROOT}/usr/lib/$$i ] ; then \
			mkdir ${DESTROOT}/usr/lib/$$i; \
			chmod 755 ${DESTROOT}/usr/lib/$$i; \
			/etc/chown root ${DESTROOT}/usr/lib/$$i; \
			chgrp system ${DESTROOT}/usr/lib/$$i; \
	else true; \
	fi; done)
       

	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${MYSUBDIRS}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done

	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/lib/$$i); done
	install -c crontab ${DESTROOT}/usr/lib/crontab
	for i in ${STD} ${NSTD}; do (install -c $$i ${DESTROOT}/usr/lib/$$i); done
	install -c lib.b ${DESTROOT}/usr/lib/lib.b
	for i in ${DFILES}; do (install -m 644 -c $$i ${DESTROOT}/usr/lib/$$i); done

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR} adb; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD}

clobber: clean
	for i in ${SUBDIR} adb; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f *.c Makefile 

sccsinfo:
	for i in ${SUBDIR}; do (cd $$i;echo $$i; make ${MFLAGS} sccsinfo); done
	sccs info

sccsget:
	for i in ${SUBDIR} adb; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget) ; done
	sccs get SCCS	

depend:
	cat </dev/null >x.c
	for i in ${STD} ${NSTD}; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
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

atrun: atrun.c
atrun: /usr/include/stdio.h
atrun: /usr/include/sys/param.h
atrun: /usr/include/sys/dir.h
atrun: /usr/include/sys/time.h
atrun: /usr/include/sys/stat.h
getNAME: getNAME.c
getNAME: /usr/include/stdio.h
makekey: makekey.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
