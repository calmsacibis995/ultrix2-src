#  @(#)Makefile	1.26	ULTRIX	10/3/86
#
#	Makefile	4.17	83/08/19
#
#	Modification History
#
#	Ray Glaser,	xx-Jun-86
# 002 -	Added  domainname  for  Yellow Pages
#
#	Stephen Reilly, 04-Apr-84
# 001 - Added code that depends on the environment variable "target".
#	The reason is that some of the modules must be compiled 
#	differently for MicroVax than for the big vax.
#
# DESTROOT=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	adb as awk csh diff ed ltf make sed sh sh5 s5make tar tp

TOOLSDIR= as awk csh ed make sed sh tar

# Shell scripts that need only be installed and are never removed.
#
SCRIPT=	false true

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	ar cat cc chgrp chmod cmp date dd df domainname du echo expr \
	grep hostid hostname kill ld ln login ls mail mkdir mt nice \
	nm od pagesize passwd pr pwd rm rmail rmdir size \
	strip stty su sync tee test time wall who write

TOOLSSTD= ar cat cc chgrp chmod cmp date dd echo expr grep hostname ld \
	  ln ls mkdir nm od pagesize pwd rm rmdir size strip sync test

# Programs that must run setuid to root
#
SETUID=	chgrp df login mail passwd su

TOOLSSUID= chgrp

# C programs that live in the current directory and need
# explicit make lines.
#
NSTD=	ps

CC=/bin/cc

all:	${SUBDIR} ${STD} ${NSTD} cp mv

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

${STD} mv cp:
	${CC} ${CFLAGS} -o $@ $@.c

# take care with cp and mv, install uses them
tools: ${TOOLSDIR} ${TOOLSSTD} cp mv
	-if [ ! -d ${DESTROOT}/bin ] ;\
	then \
		mkdir ${DESTROOT}/bin; \
		/etc/chown root ${DESTROOT}/bin; \
		chgrp system ${DESTROOT}/bin; \
		chmod 0755 ${DESTROOT}/bin; \
	else true; \
	fi

	install -s cp ${DESTROOT}/bin/newcp; \
		mv ${DESTROOT}/bin/newcp ${DESTROOT}/bin/cp;
	rm -f ${DESTROOT}/bin/mv; cp mv ${DESTROOT}/bin/mv; rm -f mv; \
		strip ${DESTROOT}/bin/mv
	for i in ${TOOLSDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/bin/$$i); done
	for i in ${TOOLSSTD}; do (install -c -s $$i ${DESTROOT}/bin/$$i); done
	for i in ${TOOLSSUID}; do (/etc/chown root ${DESTROOT}/bin/$$i; \
		chmod 4755 ${DESTROOT}/bin/$$i); done
	rm -f ${DESTROOT}/bin/[; ln ${DESTROOT}/bin/test ${DESTROOT}/bin/[

# take care with cp and mv, install uses them
install: ${STD} ${NSTD} cp mv
	-if [ ! -d ${DESTROOT}/bin ] ;\
	then \
		mkdir ${DESTROOT}/bin; \
		/etc/chown root ${DESTROOT}/bin; \
		chgrp system ${DESTROOT}/bin; \
		chmod 0755 ${DESTROOT}/bin; \
	else true; \
	fi

	install -s cp ${DESTROOT}/bin/newcp; \
		mv ${DESTROOT}/bin/newcp ${DESTROOT}/bin/cp;
	rm -f ${DESTROOT}/bin/mv; cp mv ${DESTROOT}/bin/mv; rm -f mv; \
		strip ${DESTROOT}/bin/mv
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	for i in ${SCRIPT}; do (install -c $$i.sh ${DESTROOT}/bin/$$i); done
	for i in ${STD} ${NSTD}; do (install -c -s $$i ${DESTROOT}/bin/$$i); done
	for i in ${SETUID}; do (/etc/chown root ${DESTROOT}/bin/$$i; \
		chmod 4755 ${DESTROOT}/bin/$$i); done
	rm -f ${DESTROOT}/bin/[; ln ${DESTROOT}/bin/test ${DESTROOT}/bin/[

clean:
	rm -f a.out core *.s *.o errs
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD} expr.c

clobber: clean
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} clobber); done
	rm -f *.[chlyp] Makefile mv cp

sccsinfo:
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} sccsinfo); done
	sccs info

sccsget:
	for i in ${SUBDIR}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); \
			done
	sccs get SCCS

.DEFAULT:
	sccs get $<

depend:
	cat </dev/null >x.c
	for i in ${STD} ${NSTD}; do \
		(echo $$i: $$i.c >>makedep; \
		/bin/grep '^#[ 	]*include' x.c $$i.c | sed \
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

ps:
	cc ${CFLAGS} -o ps ps.c -lm

expr:	expr.c

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
