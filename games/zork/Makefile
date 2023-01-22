# @(#)Makefile	1.2	ULTRIX	6/28/84
#
#	@(#)Makefile	4.1	(Berkeley)	11/28/82
#
DESTROOT=
CFLAGS=	-O

zork: zork.c
	cc ${CFLAGS} -o zork zork.c

install:
	install zork ${DESTROOT}/usr/games/zork
	cp zorklib/dtext.dat ${DESTROOT}/usr/games/lib
	cp zorklib/dindex.dat ${DESTROOT}/usr/games/lib
	cp zorklib/doverlay ${DESTROOT}/usr/games/lib
	cp zorklib/dungeon ${DESTROOT}/usr/games/lib

clean:
	rm -f a.out core *.s *.o zork

clobber: clean
	rm Makefile *.[hc]

sccsinfo:
	sccs info 
	(cd zorklib; sccs info)

sccsget:
	sccs get SCCS
	(cd zorklib; sccs get SCCS)

.DEFAULT:
	sccs get $<

depend:
	cat </dev/null >x.c
	for i in zork; do \
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

# DO NOT DELETE THIS LINE -- make depend uses it

zork: zork.c
zork: /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
