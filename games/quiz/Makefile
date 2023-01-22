# @(#)Makefile	1.2	ULTRIX	6/29/84
#
#	@(#)Makefile	4.1	(Berkeley)	11/28/82
#
DESTROOT=
CFLAGS=	-O

quiz: quiz.c
	cc ${CFLAGS} -o quiz quiz.c

install: quiz
	install -c quiz ${DESTROOT}/usr/games/quiz
	-if [ ! -d ${DESTROOT}/usr/games/lib ]; \
	then \
		mkdir ${DESTROOT}/usr/games/lib; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/games/lib/quiz.k ]; \
	then \
		mkdir ${DESTROOT}/usr/games/lib/quiz.k; \
	else \
		true; \
	fi
	cp quiz.k/[a-z]* ${DESTROOT}/usr/games/lib/quiz.k

clean:
	rm -f a.out core *.s *.o quiz

clobber: clean
	rm -f *.c Makefile
	(cd quiz.k; rm -f [a-z]*)

sccsinfo:
	sccs info
	(cd quiz.k; sccs info)

sccsget:
	sccs get SCCS 
	(cd quiz.k; sccs get SCCS)

.DEFAULT:
	sccs get $<

depend:
	cat </dev/null >x.c
	for i in quiz; do \
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

quiz: quiz.c
quiz: /usr/include/stdio.h
quiz: /usr/include/signal.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
