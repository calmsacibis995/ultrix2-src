# @(#)Makefile	1.7	ULTRIX	9/30/85
#
#	Makefile	4.2	(Berkeley)	82/10/27
#
# For performance (get about 10%) we have added the -O option to CFLAGS
# For porting to new hardware and such you may want to turn this off
CFLAGS=-c -w -DCOPYCODE -DC2 -O
LDFLAGS=
ASDIR = ../../bin/as
DESTROOT=
VERSION=1.0
PATH=.../lib/c2:

SRCS = Makefile c2.h c20.c c21.c c22.c
FILES=${SRCS}

c2:	c20.o c21.o c22.o
	$(CC) $(LDFLAGS) -z -o c2 c2?.o

c20.o:	c20.c c2.h
c21.o:	c21.c c2.h
c22.o:  c22.c c2.h instrs.c2
	$(CC) $(CFLAGS) -R c22.c

instrs.c2: $(ASDIR)/instrs
	(echo FLAVOR C2 ; cat $(ASDIR)/instrs) | awk -f ${ASDIR}/instrs > instrs.c2

clean:
	rm -f *.o *.s errs core a.out instrs.c2

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

install: c2
	install -s c2 $(DESTROOT)/lib

sources: $(SRCS)

$(ASDIR)/instrs:
	(cd $(ASDIR); sccs get instrs)

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
