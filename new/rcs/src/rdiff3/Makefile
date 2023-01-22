#		@(#)Makefile	1.5	(ULTRIX)	2/1/85
#
# Note: merge uses /usr/new/lib/rdiff3

CFLAGS	      = -O -d2
DESTROOT      = 
DEST	      = /usr/new/lib
EXTHDRS	      =
LDFLAGS	      =
LIBS	      =
LINKER	      = cc
MAKEFILE      = Makefile
OBJS	      = diff3.o
PRINT	      = pr
PROGRAM	      = rdiff3
HDRS	      =
SRCS	      = diff3.c
FILES	      = ${SRCS} Makefile

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)

# ../../Makefile should have created DEST.
#
install:	all
		install -c -s $(PROGRAM) ${DESTROOT}$(DEST)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

clean:
	@rm -f $(OBJS) ${PROGRAM}

clobber: clean
	rm -f *.[ch] Makefile

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
