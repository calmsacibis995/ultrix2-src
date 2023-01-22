# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.1	12/25/82
#
CFLAGS = -O
DESTROOT=

all: libln.a

libln.a:
	cc -c -O allprint.c main.c reject.c yyless.c
	cc -c -O yywrap.c
	rm -f libln.a
	ar rvc libln.a allprint.o main.o reject.o yyless.o yywrap.o
	rm allprint.o main.o reject.o yyless.o yywrap.o

tools install: all
	install -c libln.a ${DESTROOT}/usr/lib
	rm -f ${DESTROOT}/usr/lib/libl.a
	ln ${DESTROOT}/usr/lib/libln.a ${DESTROOT}/usr/lib/libl.a
	ranlib ${DESTROOT}/usr/lib/libln.a

clean:
	rm -f *.o libln.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

