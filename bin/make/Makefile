# @(#)Makefile	1.5	ULTRIX	12/4/84
#
# Description file for the Make command
# Makefile 4.3 82/10/19

OBJECTS=ident.o main.o doname.o misc.o files.o dosys.o gram.o 
LIBES= 
LINT=	lint -ps
CFLAGS=	-O -DASCARCH
DESTROOT=

all:	make

make:	${OBJECTS}
	${CC} -o make ${CFLAGS} ${OBJECTS} ${LIBES}

${OBJECTS}:  defs

clean:
	-rm -f *.o gram.c make a.out errs

clobber: clean
	rm -f Makefile *.[ych] defs

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<

install: make
	install -c -s make ${DESTROOT}/bin/make

lint :  dosys.c doname.c files.c main.c misc.c ident.c gram.c
	${LINT} dosys.c doname.c files.c main.c misc.c ident.c gram.c
	rm -f gram.c
