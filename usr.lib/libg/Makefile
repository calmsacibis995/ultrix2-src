# @(#)Makefile	1.4	ULTRIX	11/21/84
#
DESTROOT=

libg.a:	dbxxx.s
	as dbxxx.s -o libg.a

tools install: libg.a
	install -c libg.a ${DESTROOT}/usr/lib

clean:
	rm -f libg.a

clobber: clean
	rm -f *.s Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
