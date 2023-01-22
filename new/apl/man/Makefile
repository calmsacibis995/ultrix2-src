# MAKEFILE for apl manuals   MODIFIED BY : R. N. Fontaine    8/29/84
#
#
DESTROOT=
TARGET= /usr/man/mann


PRINT	      = vtroff

SRCS	      = apl.1 \
		aplcvt.1 \
		xed.1

SUFFIX	      = .1:s

all:;

clean:;


install:	@echo Installing ${SRCS} in ${DESTROOT}${TARGET}
		-if [ ! -d ${DESTROOT}${TARGET} ]; \
		then \
			mkdir ${DESTROOT}${TARGET}; \
		else \
			true; \
		fi
		@for i in ${SRCS}; do cp $$i ${DESTROOT}${TARGET}/`basename $$i .1`.n; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;

sccsget:
	sccs get SCCS
sccsinfo:
	sccs info
