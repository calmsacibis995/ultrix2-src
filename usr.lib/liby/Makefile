#
#
#
#
#
CFLAGS =	-O
DESTROOT =

all:	liby.a

liby.a:	main.o yyerror.o
	rm -f liby.a
	ar rc liby.a main.o yyerror.o

install:	all
	install -c -m 644 liby.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/liby.a

clean:
	rm -f *.o liby.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
