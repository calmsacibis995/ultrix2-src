
#	@(#)Makefile	1.1	(ULTRIX)	10/21/85
#
#
#	makefile to make upars library
#
#	D. C. Treff, Square Trend Studio, for Digital Equipment Corporation
#	(c)Copyright l984, Digital Equipment Corporation, Tewksbury, Ma.
#
#

#
#	definitions
#

DEBUG =
DESTROOT =
DESTDIR =
LIBDIR =	/usr/lib
INCDIR =	/usr/include
LIBR =

CFLAGS =	-c $(DEBUG)
LIBFLAGS = 	rv

SOURCES =	parser.c tlex.c nlex.c lex_utils.c	
OBJECTS = 	parser.o tlex.o nlex.o lex_utils.o	


#
#	relationships
#



libup.a:	$(OBJECTS)
		ar $(LIBFLAGS) libup.a $(OBJECTS) 


tools install:	libup.a
		install -c -m 644 libup.a ${DESTROOT}/usr/lib
		ranlib $(DESTROOT)/usr/lib/libup.a


lex_utils.o:	../h/upars.h ../h/keydefs.h ../h/keywords.h lex_utils.c
		cc $(CFLAGS)  lex_utils.c

nlex.o:		../h/upars.h ../h/keydefs.h nlex.c
		cc $(CFLAGS)  nlex.c

tlex.o:		../h/upars.h ../h/keydefs.h tlex.c
		cc $(CFLAGS)  tlex.c

parser.o:	../h/upars.h ../h/utables.h parser.c
		cc $(CFLAGS)  parser.c


clean:
		rm -f *.o *.s core a.out libup.a

clobber:
		rm -f *.c Makefile

sccsinfo:
		sccs info

sccsget:
		sccs get SCCS

print:		$(SOURCES) 
		print $?
		touch print

