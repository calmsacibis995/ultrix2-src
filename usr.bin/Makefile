#  @(#)Makefile	1.44	ULTRIX	10/3/86
#
#	Makefile	4.29	83/08/20
#
# - - - - -
# 02	xx-Jun-86, Ray Glaser
#	Added  Yellow Pages  modules
#
# 01	30-May-86, Greg Tarsa
#	Added ptoc for Al Delorey.  Added refer to the list of TOOL1DIRs
#	that need to be made for build to operate.
#
CFLAGS=	-O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	calendar cflow ctrace cxref dc diction diff3 efl eqn f77 \
	graph learn lex lint m4 modula2 neqn \
	nroff plot prof ptoc ratfor refer sccs spell struct tbl tip troff \
	yacc xsend
# Removed xsend from build list - lp

TOOL1DIR= lex m4 yacc refer
TOOL2DIR= f77 lex lint m4 modula2 nroff yacc

#  
# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	diffmk install lorder man nohup

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ar11 at basename cal cb checkeq col comm cpio cut deroff \
	fgrep file find iostat ipcrm ipcs join look mesg paste ptx ranlib \
	rev sleep sort spline split sum tabs tc tk touch tr trace tsort \
	tty uniq units 2780e ypcat ypmatch yppasswd ypwhich

TOOLSSTD= basename cpio file find join ranlib sort touch tsort uniq
TOOLSNSTD= egrep

#       removed crypt from std list  - lp

# C programs that live in the current directory and need explicit make lines.
# (make depend has special rules for these files)
#
NSTD=	bc egrep dgate 3780e

# programs that need suid

SETUID=	at dgate

all:	${SUBDIR} ${STD} ${NSTD}

${SUBDIR}: /tmp
	cd $@; ${ECHO} ${CWD}; make ${MFLAGS}

bc egrep:
	${YACC} ${YFLAGS} $@.y
	${CC} -c ${CFLAGS} y.tab.c
	mv y.tab.o $@.o
	${CC} -o $@ $@.o
	rm -f $@.o

dgate: subr.o dgate.h
	${CC} ${CFLAGS} -o $@ $@.c subr.o

3780e:	
	${CC} ${CFLAGS} -Dd3780 -o 3780e 2780e.c

subr.o: dgate.h
	${CC} ${CFLAGS} -c $<

${STD}:
	${CC} ${CFLAGS} -o $@ $@.c

tools1:	${TOOLSSTD} ${TOOLSNSTD}
	-if [ ! -d ${DESTROOT}/usr/lib ]; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ]; then \
		mkdir ${DESTROOT}/usr/bin; \
		chmod 755 ${DESTROOT}/usr/bin; \
		/etc/chown root ${DESTROOT}/usr/bin; \
		chgrp system ${DESTROOT}/usr/bin; \
	else true; \
	fi
	for i in ${TOOL1DIR}; do \
		(cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} DESTROOT=${DESTROOT} tools1); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/bin/$$i); done
	for i in ${TOOLSSTD} ${TOOLSNSTD}; do \
		(install -c -s $$i ${DESTROOT}/usr/bin/$$i); \
	done

tools2:	${TOOLSSTD}
	-if [ ! -d ${DESTROOT}/usr/lib ]; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ]; then \
		mkdir ${DESTROOT}/usr/bin; \
		chmod 755 ${DESTROOT}/usr/bin; \
		/etc/chown root ${DESTROOT}/usr/bin; \
		chgrp system ${DESTROOT}/usr/bin; \
	else true; \
	fi
	for i in ${TOOL2DIR}; do \
		(cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} DESTROOT=${DESTROOT} tools2); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/bin/$$i); done
	for i in ${TOOLSSTD}; do \
		(install -c -s $$i ${DESTROOT}/usr/bin/$$i); \
	done

install: all
	-if [ ! -d ${DESTROOT}/usr/lib ]; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/bin ]; then \
		mkdir ${DESTROOT}/usr/bin; \
		chmod 755 ${DESTROOT}/usr/bin; \
		/etc/chown root ${DESTROOT}/usr/bin; \
		chgrp system ${DESTROOT}/usr/bin; \
	else true; \
	fi
	for i in ${SUBDIR}; do \
		(cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/usr/bin/$$i); done
	for i in ${STD} ${NSTD}; do \
		(install -c -s $$i ${DESTROOT}/usr/bin/$$i); \
	done
	for i in ${SETUID}; do \
		(/etc/chown root ${DESTROOT}/usr/bin/$$i; \
		 chmod 4755 ${DESTROOT}/usr/bin/$$i); \
	done
	install -c units.lib ${DESTROOT}/usr/lib/units
	chmod 4711 ${DESTROOT}/usr/bin/2780e
	chmod 4711 ${DESTROOT}/usr/bin/3780e
	-mkdir ${DESTROOT}/usr/spool/at
	chmod 755 ${DESTROOT}/usr/spool/at
	/etc/chown root ${DESTROOT}/usr/spool/at
	-mkdir ${DESTROOT}/usr/spool/at/past
	chmod 755 ${DESTROOT}/usr/spool/at/past
	/etc/chown root ${DESTROOT}/usr/spool/at/past

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD}

clobber: clean
	rm -f *.[ych] Makefile q
	for i in ${SUBDIR}; do (cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} clobber); done

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; ${ECHO} ${CWD}; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIR}; do (cd $$i; ${ECHO} ${CWD}; sccs get Makefile; make ${MFLAGS} sccsget); done

.DEFAULT:
	sccs get $<

# beware of the non-standard stuff for bc and egrep
# Only make depend lines that reflect our directory structure 

depend:
	cp /dev/null makedep1
	cp /dev/null makedep2
	for i in ${STD}; do \
		echo $$i: $$i.c >> makedep1; \
		/lib/cpp $$i.c | grep /include/ | \
			awk '{print substr($$3,2,length($$3)-2)}' | sort | \
			uniq >> makedep1; \
		cat makedep1 | \
		awk 	'$$1 ~ /:/  {filename = $$1;print $$0} \
			/include/  {print filename " " $$1}' >> makedep2; \
		rm makedep1; done
	for i in ${NSTD}; do \
		echo $$i: $$i.y >> makedep1; \
		/lib/cpp $$i.y | grep /include/ | \
			awk '{print substr($$3,2,length($$3)-2)}' | sort | \
			uniq >> makedep1; \
		cat makedep1 | \
		awk 	'$$1 ~ /:/  {filename = $$1;print $$0} \
			/include/  {print filename " " $$1}' >> makedep2; \
		rm makedep1; done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep2' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep2
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# Files listed in ${NSTD} have explicit make lines given below.

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
