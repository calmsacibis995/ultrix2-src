# @(#)Makefile	1.2	ULTRIX	6/29/84
#
#	@(#)Makefile	4.2	(Berkeley)	83/07/26
#
DESTROOT=

doctor: /usr/ucb/lisp /usr/ucb/liszt
	liszt -mrq -o doctor doctor.l

install:
	install doctor $(DESTROOT)/usr/games

clean:
	rm -f doctor

clobber: clean
	rm -f doctor.l Makefile

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<
