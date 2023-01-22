#	1.2	Makefile	84/10/11
Listall:
	@pr *.[hs]
	@date >List

List:		ctype.h defs.s err.h gc.h record.h rt.h
	@pr $?
	@date >List
clean:
clobber: clean
	@rm -f *.h *.h.gen Makefile
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS
all:
install: all

dist-clean:
	rm -f `gcomp Makefile *.h *.s *.gen`
