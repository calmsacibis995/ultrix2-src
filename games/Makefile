# @(#)Makefile	1.13	ULTRIX	6/11/85
#
#	Makefile	4.13	83/08/02
#
DESTROOT=
CFLAGS=	-O

# The following sources are incomplete and cannot be compiled without
# some work
WORK=	aard ddl mpu

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	adventure backgammon boggle ching compat cribbage doctor fortune \
	hangman mille monop quiz rogue sail snake starship trek zork

# Shell scripts that need only be installed and are never removed.
#
#SCRIPT=	

# C programs that live in the current directory and do not need
# explicit make lines.
#
STD=	arithmetic btlgammon banner bcd cfscores chess factor \
	fish number wump wargames

# C programs that live in the current directory and need explicit make lines.
#
NSTD=	canfield primes rain worm worms

all:	${SUBDIR} ${STD} ${NSTD}

${SUBDIR}: /tmp
	cd $@; make ${MFLAGS}

${STD}:
	cc ${CFLAGS} -o $@ $@.c

install: all
	-if [ ! -d ${DESTROOT}/usr/games ]; then \
		mkdir ${DESTROOT}/usr/games; \
		chmod 755 ${DESTROOT}/usr/games; \
		/etc/chown root ${DESTROOT}/usr/games; \
		chgrp system ${DESTROOT}/usr/games; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/games/lib ]; \
	then \
		mkdir ${DESTROOT}/usr/games/lib; \
		chmod 755 ${DESTROOT}/usr/games/lib; \
		/etc/chown root ${DESTROOT}/usr/games/lib; \
		chgrp system ${DESTROOT}/usr/games/lib; \
	else true; \
	fi
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
#	for i in ${SCRIPT}; do (install -c $$i ${DESTROOT}/usr/games/$$i); done
	for i in ${STD} ${NSTD}; do \
		(install -c -s $$i ${DESTROOT}/usr/games/$$i); \
	done
	install -c chess.bin ${DESTROOT}/usr/games/lib/chess
	cat >${DESTROOT}/usr/games/lib/cfscores </dev/null
	chmod 777 ${DESTROOT}/usr/games/lib/cfscores

clean:
	rm -f a.out core *.s *.o
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done
	rm -f ${STD} ${NSTD}

clobber: clean
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f *.[ch]  
sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo ); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIR}; do \
		 (cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done

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

canfield: canfield.c
	cc -o canfield ${CFLAGS} canfield.c -lcurses -ltermcap

primes: primes.c
	cc -o primes ${CFLAGS} primes.c -lm

rain: rain.c
	cc -o rain ${CFLAGS} rain.c -lcurses -ltermcap

worm: worm.c
	cc -o worm ${CFLAGS} worm.c -lcurses -ltermcap

worms: worms.c
	cc -o worms ${CFLAGS} worms.c -lcurses -ltermcap

# DO NOT DELETE THIS LINE -- make depend uses it

arithmetic: arithmetic.c
arithmetic: /usr/include/stdio.h
arithmetic: /usr/include/signal.h
btlgammon: btlgammon.c
btlgammon: /usr/include/stdio.h
banner: banner.c
banner: /usr/include/stdio.h
bcd: bcd.c
cfscores: cfscores.c
cfscores: /usr/include/pwd.h
chess: chess.c
chess: /usr/include/stdio.h
fish: fish.c
fish: /usr/include/stdio.h
number: number.c
wump: wump.c
wump: /usr/include/stdio.h
wump: /usr/include/sgtty.h
canfield: canfield.c
canfield: /usr/include/curses.h
canfield: /usr/include/ctype.h
canfield: /usr/include/signal.h
canfield: /usr/include/sys/types.h
rain: rain.c
rain: /usr/include/stdio.h
rain: /usr/include/sgtty.h
rain: /usr/include/signal.h
worm: worm.c
worm: /usr/include/ctype.h
worm: /usr/include/curses.h
worm: /usr/include/signal.h
worms: worms.c
worms: /usr/include/stdio.h
worms: /usr/include/sgtty.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
