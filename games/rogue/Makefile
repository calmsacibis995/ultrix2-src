# @(#)Makefile	1.2	ULTRIX	6/29/84
#
#	@(#)Makefile	4.4	(Berkeley)	83/08/14
#
DESTROOT=
CFLAGS=	-O -DSCOREFILE='"/usr/games/lib/rogue_roll"' -DNAMELIST='"/dev/vmunix"'\
	-DLOADAV -DMAXLOAD=50

rogue: mach_dep.o distmod.obj
	${CC} -o rogue distmod.obj mach_dep.o -lcurses -ltermlib

install: rogue
	install -o daemon -s -m 4711 rogue ${DESTROOT}/usr/games/rogue
	install -c -o daemon -m 644 /dev/null \
		${DESTROOT}/usr/games/lib/rogue_roll

clean:
	rm -f rogue mach_dep.o errs a.out

clobber: clean
	rm -f Makefile READ_ME rogue.doc *.[hc]

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
