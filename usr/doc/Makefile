#
# @(#)Makefile	1.2	2/15/85
#
# Makefile to move doc.C around
#
# doc.C is a compressed tar image containing the /usr/doc
# files.  These are not supported or maintained by DIGITAL
#
#
FILES = README Makefile

all:

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

install:
	-if [ ! -d ${DESTROOT}/usr/doc ]; \
	then \
	    mkdir ${DESTROOT}/usr/doc; \
	else \
	    true; \
	fi
	install -c README ${DESTROOT}/usr/doc/README
	install -c doc.C ${DESTROOT}/usr/doc/doc.C

clean:

clobber:
	rm -f ${FILES}
