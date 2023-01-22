#		@(#)Makefile	1.5	(ULTRIX)	2/1/85
#
#  Makefile for INGRES header files
#
#	David L Ballenger, 16-Jan-1984
# 001	Integrate into ULTRIX build procedures.
#

SRCS=	access.h aux.h batch.h catalog.h func.h ildr.h \
	ingres.h lock.h opsys.h pv.h range.h resp.h \
	symbol.h trace.h tree.h useful.h version.h \
	pmon.h depend.sed

PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Header Files

all: $(SRCS)

install: all

print: sources
	$(PR) Makefile *.[hm] *.[csy]

vgrind: sources
	$(VGRIND) -h "$(HEADER) (Release 7)" -n Makefile
	$(VGRIND) -h "$(HEADER) (Release 7)" *.[hm]

clean:
	rm -f *.o
	rm -f core a.out

clobber: clean
	rm -f *.h Makefile depend.sed

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

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
