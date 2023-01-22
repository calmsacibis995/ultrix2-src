#	1.4	Makefile	85/10/21
DEST	      = $(DESTROOT)/usr/new/man/man1

MAKEFILE      = Makefile

PRINT	      = vtroff

SRCS	      = iconc.1 \
		icont.1

SUFFIX	      = .1:s

all:;

clean:;
clobber: clean
	@rm -f Makefile icon?.*
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS

depend:;	@mkmf -f $(MAKEFILE)

install:;	@echo Installing $(SRCS) in $(DEST)
		@for i in $(SRCS); do cp $$i $(DEST)/`basename $$i .1`.n; done

print:		$(SRCS)
		@$(PRINT) -man $?
		@touch print

update:;
