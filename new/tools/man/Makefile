#	@(#)Makefile	1.3	84/10/14
DESTROOT=
DEST	      = $(DESTROOT)/usr/new/doc

MAKEFILE      = Makefile

PRINT	      = vtroff

SRCS	      = jot.1 \
		lam.1 \
		rs.1

SUFFIX	      = .1:s

all:;

clean:;
clobber: clean
	@rm -f [a-z]* Makefile

depend:;	@mkmf -f $(MAKEFILE)

install:;	@echo Installing $(SRCS) in $(DEST)
		@for i in $(SRCS); do install -c $$i $(DEST)/$$i; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info
