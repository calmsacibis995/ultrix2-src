# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.1	83/03/30
#
CFLAGS=-O
DESTROOT=

graph: graph.c
	${CC} $(CFLAGS) -o graph graph.c -lplot -lm

install: graph
	install -c -s graph ${DESTROOT}/usr/bin/graph

clean:
	rm -f *.o graph

clobber: clean
	rm -f graph.c Makefile

sccsinfo:
	sccs info 

sccsget:	
	sccs get SCCS

.DEFAULT:
	sccs get $<
