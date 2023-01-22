# @Makefile  1.2		ULTRIX   8/22/84    AUTHOR: R. N. Fontaine
#
#
# make install will copy the me troff and nroff files in this directory 
#
#

FILES= acm.me chars.me deltext.me eqn.me float.me footnote.me index.me \
	local.me null.me refer.me revisions sh.me tbl.me thesis.me

DESTROOT=
TARGET=${DESTROOT}/usr/lib/me


all:

install: ${FILES}
	for i in ${FILES}; do \
		install -c $$i ${TARGET}/$$i; \
	done


clean:

clobber:
	rm -f ${FILES}

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
