#	@(#)Makefile	1.2	(ULTRIX)	10/21/85
#
DESTDIR=
LIBR=
CFLAGS=	-O
DESTROOT=

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	uupars ulib

all:	${SUBDIR} 

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

tools install:
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} LIBR=${LIBR} DESTROOT=${DESTROOT} install); done
	install -c -m 644 h/upars.h ${DESTROOT}/usr/include/upars.h
	install -c -m 644 h/utables.h ${DESTROOT}/usr/include/utables.h

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber:
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	cd h; rm -f *.h
	rm Makefile

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} sccsinfo); done
	cd h; sccs info

sccsget:
	sccs get SCCS
	for i in ${SUBDIR}; do (cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done
	cd h; sccs get SCCS

FRC:

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
