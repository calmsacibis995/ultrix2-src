#		@(#)Makefile	1.1	(ULTRIX)	1/29/85
#
# Makefile to install the demo data files.
#

INGRES	= ../..
DEMO	= ${INGRES}/demo
INSTALL	= install
ROMODE	= 444

# These data files are not text files and are not kept under SCCS
# control.  The clean and clobber entries must not delete them.
#
SRCS	= DEMO dept employee item parts qry sale store supplier supply

all: ${SRCS}

install: all
	for i in ${SRCS}; do \
		(${INSTALL} -c -m ${ROMODE} -o ingres $$i ${DEMO}); done

clean:

clobber:
	rm -f Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: $(SRCS)

save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ${PATH}
	@for i in ${FILES}; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done

show_current:
	@echo ${PATH}
	@sccs what ${FILES} | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo
