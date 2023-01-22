# @(#)Makefile	1.4	ULTRIX	11/21/84
#
#	@(#)Makefile	4.1	(Berkeley)	10/29/82
#
DESTROOT=
CFLAGS=	-O

all:

install:
	install -c ching.sh ${DESTROOT}/usr/games/ching
	-if [ ! -d ${DESTROOT}/usr/games/lib ]; \
	then \
		mkdir ${DESTROOT}/usr/games/lib; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/games/lib/ching.d ]; \
	then \
		mkdir ${DESTROOT}/usr/games/lib/ching.d; \
	else \
		true; \
	fi
	install -c cno ${DESTROOT}/usr/games/lib/ching.d/cno
	install -c phx ${DESTROOT}/usr/games/lib/ching.d/phx
	install -c macros ${DESTROOT}/usr/games/lib/ching.d/macros
	install -c hexagrams ${DESTROOT}/usr/games/lib/ching.d/hexagrams

clean:
	rm -f a.out core *.s *.o

clobber: clean
	rm -f Makefile ching.sh macros hexagrams

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $< 
