#	1.6	Makefile	85/02/12
#MAKEFILE to install bib manuals   MODIFIED BY: R. N. Fontaine  8/29/84
#


DESTROOT=
TARGET=/usr/new/lib/bmac


PRINT	      = vtroff

SRCS	      = bib.astro \
		bib.foot \
		bib.hnf \
		bib.list \
		bib.opena \
		bib.openn \
		bib.spe \
		bib.spe2 \
		bib.stda \
		bib.stdn \
		bib.stdsn \
		bib.supn \
		bibinc.fullnames \
		bibinc.publishers \
		bibinc.shortnames \
		bibmac.me \
		bmac.open \
		bmac.std \
		common

all:;

clean:;


install:
		@-if [ ! -d ${DESTROOT}${TARGET} ]; \
		then \
			mkdir ${DESTROOT}${TARGET} ; \
		else \
			true; \
		fi
		@for i in $(SRCS); do install -c $$i ${DESTROOT}${TARGET}/$$i; done

update:;

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info
clobber: clean
	@rm -f [a-z]*
	@rm -f Makefile
