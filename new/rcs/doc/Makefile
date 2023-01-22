#		@(#)Makefile	1.3	(ULTRIX)	2/12/85
#
DESTROOT=
DEST	= ${DESTROOT}/usr/new/doc/rcs
PRINT	= vtroff
SRCS	= pretty_keys.ms rcs_functs.ms rdiff.ms rdiff3.ms release3.ms\
	  revision_no.ms tutorial.ms
DOCS	= pretty_keys rcs_functs rdiff rdiff3 release3\
	  revision_no tutorial

all:	${SRCS}

${DOCS}:
	nroff -ms $? > $@

pretty_keys: pretty_keys.ms
rcs_functs: rcs_functs.ms
rdiff: rdiff.ms
rdiff3: rdiff3.ms
release3: release3.ms
revision_no: revision_no.ms
tutorial: tutorial.ms

clean:
	rm -f ${DOCS}

install: all
	for i in $(SRCS); do install -c -m 444 $$i $(DEST) ; done

print:		$(SRCS)
		@$(PRINT) -ms $?

clobber: clean
	rm -f ${SRCS} Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: $(SRCS)

$(SRCS):
	sccs get $@

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
