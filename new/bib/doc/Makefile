#	1.9	Makefile	85/02/12
#MAKEFILE for bib documents  AUTHOR: R. N. Fontaine       8/29/84
#
#

FILES=	bibdoc.ms	styletop	testrefs	teststyle  tstyles
DESTROOT=
TARGET=/usr/new/doc/manuals/bib


all:

sccsget:
	@sccs get -s SCCS
sccsinfo:
	@sccs info
clean:

install:
			-if [ ! -d ${DESTROOT}/usr/new/doc ]; \
			then \
				mkdir ${DESTROOT}/usr/new/doc; \
			else \
				true; \
			fi
			-if [ ! -d ${DESTROOT}/usr/new/doc/manuals ]; \
			then \
				mkdir ${DESTROOT}/usr/new/doc/manuals; \
			else \
				true; \
			fi
			-if [ ! -d ${DESTROOT}/usr/new/doc/manuals/bib ]; \
			then \
				mkdir ${DESTROOT}/usr/new/doc/manuals/bib; \
			else \
				true; \
			fi
			-if [ ! -d ${DESTROOT}${TARGET} ]; \
			then \
				mkdir ${DESTROOT}${TARGET}; \
			else \
				true; \
			fi

		@for i in ${FILES}; do \
			install -c $$i ${DESTROOT}${TARGET}/$$i ; \
		done
clobber: clean
	@rm -f Makefile [a-z]*
