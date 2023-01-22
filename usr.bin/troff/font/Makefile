#
#	@(#)Makefile	1.2 ULTRIX	7/9/84
#
mkfont: mkfont.o
	cc mkfont.o -o mkfont

clean:
	rm -f *.o a.out errs

clobber: clean
	rm -f *.n linkrc Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
