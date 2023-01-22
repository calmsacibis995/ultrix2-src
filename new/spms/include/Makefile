#	1.6	Makefile	84/10/11
# $Header$
#
HDRS	      = bin.h \
		getarg.h \
		hash.h \
		macro.h \
		null.h \
		path.h \
		pdb.h \
		pdbuf.h \
		pdtyp.h \
		pld.h \
		slist.h \
		slslist.h \
		spms.h \
		system.h \
		tree.h \
		truefalse.h \
		yesno.h

MAKEFILE      = Makefile

PRINT	      = pr

VERSION	      = Ultrix-32

all:

clean:
	@rm -f *.h Makefile tags

clobber: clean

co:	@co -r$(VERSION) $(HDRS)

depend:

diff:	@rcsdiff -r$(VERSION) $(HDRS)

index:	@ctags -wx $(HDRS)

install:

lint:

print:	@$(PRINT) $(HDRS)

tags:   $(HDRS); @ctags $(HDRS)

update:

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info
###
