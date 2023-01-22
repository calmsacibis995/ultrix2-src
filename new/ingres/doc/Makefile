#		@(#)Makefile	1.4	(ULTRIX)	2/1/85
#
INGRES	= ../..
DOC	= ${INGRES}/doc
NROFF	= nroff
TROFF	= troff
NMACS	= ../naa
TMACS	= ../taa

ERROR_S	= error-02parser.nr error-03qrymod.nr error-04Aovqp.nr \
	  error-04Bdecomp.nr error-05dbu.nr
FILES_S	= files-dayfile.nr files-dbtmplt.nr files-error.nr files-grafile.nr \
	  files-libq.nr files-proctab.nr files-startup.nr files-ttytype.nr \
	  files-users.nr
OTHER_S	= howto_setup.nr indconv.nr lockdev.nr maintain.nr setup_geo.nr \
	  tutorial.nr v5v6diff.nr v61v62diff.nr v6v61diff.nr design.roff
QUEL_S	= quel-append.nr quel-copy.nr quel-create.nr quel-define.nr \
	  quel-delete.nr quel-destroy.nr quel-help.nr quel-index.nr \
	  quel-integrity.nr quel-macros.nr quel-modify.nr quel-monitor.nr \
	  quel-permit.nr quel-print.nr quel-quel.nr quel-range.nr \
	  quel-replace.nr quel-retrieve.nr quel-save.nr quel-view.nr
UNIX_S	= unix-copydb.nr unix-creatdb.nr unix-destroydb.nr unix-equel.nr \
	  unix-geoquel.nr unix-helpr.nr unix-ingres.nr unix-printr.nr \
	  unix-purge.nr unix-restore.nr unix-sysmod.nr unix-usersetup.nr
REF_SRC	= ${QUEL_S} ${UNIX_S} ${FILES_S} ${ERROR_S}
SRCS	= naa taa iaa 0aa.nr 1aa.nr toc.ex nmacs ${OTHER_S} ${REF_SRC}
EXAMPLES= equeldemo.q equeltut.q example.c
OTHERS	= design howto_setup indconv lockdev maintain\
	  tutorial v5v6diff v61v62diff v6v61diff
DOCS	= reference_manual ${OTHERS} ${EXAMPLES}
FILES	= ${SRCS}

all:	${DOCS} 

${OTHERS}: naa
	nroff naa $? > $@

reference_manual: naa 0aa.nr 1aa.nr ${REF_SRC} toc.ex
	-rm -f $@ toc.nr
	echo '.th "TABLE OF CONTENTS" INGRES 3/1/81' > toc.nr
	@echo '[Read Only] and "Substitute pattern match failed" errors are normal here'
	for i in ${REF_SRC}; do (ex - $$i < toc.ex); done
	${NROFF} naa 0aa.nr 1aa.nr toc.nr ${REF_SRC} > $@

install: all
	for i in ${DOCS}; do \
		(install -c -m 444 -o ingres $$i ${DOC} ) ; done

clean:
	rm -f ${DOCS} toc.nr

clobber: clean
	rm -f ${SRCS} *.sh Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: ${SRCS}

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


ref:
	$(NROFF) naa 0aa.nr 1aa.nr toc.nr
	chdir quel; $(NROFF) $(NMACS) *.nr
	chdir unix; $(NROFF) $(NMACS) *.nr
	chdir files; $(NROFF) $(NMACS) *.nr
	chdir error; $(NROFF) $(NMACS) *.nr

tref:
	$(TROFF) taa 0aa.nr 1aa.nr toc.nr
	chdir quel; $(TROFF) $(TMACS) *.nr
	chdir unix; $(TROFF) $(TMACS) *.nr
	chdir files; $(TROFF) $(TMACS) *.nr
	chdir error; $(TROFF) $(TMACS) *.nr

design:	design.roff
howto_setup: howto_setup.nr
indconv: indconv.nr
lockdev: lockdev.nr
maintain: maintain.nr
setup_geo: setup_geo.nr
tutorial: tutorial.nr
v5v6diff: v5v6diff.nr
v61v62diff: v61v62diff.nr
v6v61diff: v6v61diff.nr
