# @(#)Makefile	1.2	ULTRIX	7/16/84
#
#	@(#)Makefile	4.1	(Berkeley)	3/8/83
#
DESTROOT=

SOURCES=vprint.sh vtroff.sh vpq.sh vpr.sh vprm.sh

all:	${SOURCES}

install: all
	install -c vprint.sh ${DESTROOT}/usr/ucb/vprint
	install -c vtroff.sh ${DESTROOT}/usr/ucb/vtroff
	install -c vpq.sh ${DESTROOT}/usr/ucb/vpq
	install -c vpr.sh ${DESTROOT}/usr/ucb/vpr
	install -c vprm.sh ${DESTROOT}/usr/ucb/vprm

clean:
	rm -f errs

clobber: clean
	rm -f *.sh rvtroff Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

vprintall:
	csh /usr/ucb/vprint ${SOURCES}
