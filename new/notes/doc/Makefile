#	@(#)Makefile	1.3	(ULTRIX)	2/8/85
#	Makefile for the notesfile reference manual
#	March 29, 1982
#	Ray Essick, University of Illinois
#	uiucdcs!essick     uiucdcs!notes
#
#       Modified by Rick Spickelmier, UCB
#       Modified by Lou Salkind, NYU

TROFF = nroff
FILES = 0 1 2 3 4 5 a.checklist b.future c.update vtoc

all: doc

doc:	$(FILES) sum.tbl.out
	$(TROFF) -ms $(FILES)

sum.tbl.out: summary
	tbl summary > sum.tbl.out

clean:
	rm -f sum.tbl.out doc

clobber: clean
	rm Makefile [a-z]* [0-9]

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
