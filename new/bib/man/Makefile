#	1.6	Makefile	85/02/12
#MAKEFILE to install bib manuals  MODIFIED BY: R. N. Fontaine   8/29/84
#
#
DESTROOT=
TARGET=/usr/new/doc

PRINT	      = vtroff

SRCS	      = bib.1 \
		invert.1

SUFFIX	      = .1:s

all:;

clean:;

install:	

# @echo Installing $(SRCS) in $(DEST)
		-if [ ! -d ${DESTROOT}${TARGET} ]; \
		then \
			mkdir ${DESTROOT}${TARGET} ; \
		else \
			true; \
		fi
		@for i in ${SRCS}; do cp $$i ${DESTROOT}${TARGET}/`basename $$i .1`.n; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info
clobber: clean
	@rm -f [a-z]*
	@rm -f Makefile
