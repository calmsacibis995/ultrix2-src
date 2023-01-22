# MAKEFILE			AUTHOR R. N. Fontaine
#
#copy files in the vfont directory 
#
#

DESTROOT=
FILES= README	fonts.C

all:

install:
	for i in ${FILES}; do \
		install -c $$i ${DESTROOT}/usr/lib/vfont/$$i ; \
	done

clean:

clobber:

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info SCCS
