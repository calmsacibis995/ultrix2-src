#		@(#)Makefile	1.2	(ULTRIX)	1/31/85
#
#	Makefile to build INGRES special files
#

INGRES= ..
FILES= ${INGRES}/files
CPUTYPE=VAX
INSTALL=install
ROMODE=	444
SRCS=	dbtmplt7 equel7 error7_0 error7_1 error7_2 error7_3 \
	error7_4 error7_5 error7_6 proctab7 startup usage

build: ${SRCS}

install: build
	for i in ${SRCS}; do\
		(${INSTALL} -c -m ${ROMODE} -o ingres $$i ${FILES}); done

equel7: equel7.${CPUTYPE}
	${INSTALL} -c -m ${ROMODE} $? $@

proctab7: proctab7.${CPUTYPE}
	${INSTALL} -c -m ${ROMODE} $? $@

clean:

clobber: clean
	rm -f Makefile ${SRCS} *.VAX *.PDP READ_ME

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
