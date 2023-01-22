# Makefile to copy files in pub directory 
#
#
# @(#)Makefile	1.4	Ultrix	10/3/84

DESTROOT=
FILES=  greek eqnchar ascii



all:

sccsget:
	sccs get SCCS
install: all
		for i in ${FILES} ; do \
			install -c $$i ${DESTROOT}/usr/pub/$$i ; \
		done


clean:
clobber: clean

sccsinfo:
	sccs info

