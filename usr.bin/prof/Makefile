# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.1 (Berkeley) 6/2/83
#
# If you don't want to plot, take out the -Dplot and the ref. to plot.a
PLOT = -lplot -Dplot
CFLAGS=-O 
DESTROOT =

prof:	prof.o
	$(CC) -o prof prof.o $(PLOT)

prof.o: prof.c
	cc -c $(CFLAGS) $(PLOT) prof.c

install: prof
	install -c -s prof $(DESTROOT)/usr/bin

clean :
	rm -f *.o prof

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
