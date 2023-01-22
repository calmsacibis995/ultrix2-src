
#	@(#)Makefile	1.1	(ULTRIX)	10/21/85
#
#
#	makefile to make upars compiler
#
#	D. C. Treff, Square Trend Studio, for Digital Equipment Corporation
#	(c)Copyright l984, Digital Equipment Corporation, Tewksbury, Ma.
#
#

#
#	definitions
#

DESTROOT =
DEBUG =
LIBR =
INCDIR =	/usr/include

OFLAGS =	-O $(DEBUG)

CFLAGS =	-c $(DEBUG)

SOURCES =	keywords.c tables.c
OBJECTS = 	y.tab.o keywords.o tables.o


#
#	relationships
#

upars:	$(OBJECTS)
		cc  $(OFLAGS) $(OBJECTS) -o upars -ll 

install: upars
		install -c -m 755 upars ${DESTROOT}/usr/bin


y.tab.o:	y.tab.c lex.yy.c ../h/upars.h
		cc $(CFLAGS) y.tab.c

y.tab.c :	upars.y 
		yacc -v upars.y

lex.yy.c :	upars.x
		lex upars.x


keywords.o :	keywords.c ../h/keywords.h ../h/upars.h ../h/keydefs.h
		cc $(CFLAGS) keywords.c

tables.o :	tables.c ../h/utables.h ../h/upars.h
		cc $(CFLAGS) tables.c


clean:
		rm -f *.o *.s core a.out y.output y.tab.c lex.yy.c upars

clobber:
		rm -f Makefile

sccsinfo:
		sccs info

sccsget:
		sccs get SCCS

print:		$(SOURCES) upars.y upars.x
		print $?
		touch print
