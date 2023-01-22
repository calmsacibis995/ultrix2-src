# @Makefile  1.2		ULTRIX   8/22/84    AUTHOR: R. N. Fontaine
#
#
# make install will copy the ms troff and nroff files in this directory 
#
#

FILES= README end.awk endnote s.acc s.cov s.eqn s.ref s.tbl s.ths s.toc \
	swapacc

DESTROOT=
TARGET=${DESTROOT}/usr/lib/ms


all:

install: ${FILES}

	@for i in ${FILES} ; do \
		install -c $$i ${TARGET}/$$i; \
		done


clean:

clobber:
		rm -f ${FILES} 
