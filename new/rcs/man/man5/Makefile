#		@(#)Makefile	1.3	(ULTRIX)	2/1/85
#
DESTROOT      =
DEST	      = ${DESTROOT}/usr/new/man/man5
MAKEFILE      = Makefile
PRINT	      = vtroff
SRCS	      = rcsfile.5
SUFFIX	      = .1:s

all:	${SRCS}

clean:;

depend:;	@mkmf -f $(MAKEFILE)

install: all
	for i in $(SRCS); do cp $$i $(DEST)/`basename $$i .5`.n; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;

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
