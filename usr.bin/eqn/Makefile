# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	Makefile	4.1	83/02/11

CFLAGS=-O
YFLAGS=-d
DESTROOT=

SOURCE = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

FILES = e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o

eqn:	$(FILES)
	cc $(CFLAGS) $(FILES) -o eqn

e.c:	e.def

e.def:	e.y
	yacc -d e.y
	mv y.tab.c e.c
	mv y.tab.h e.def

$(FILES):	e.h e.def

install: eqn
	install -c -s eqn $(DESTROOT)/usr/bin

clean:
	rm -f *.o e.c e.def eqn

clobber: clean
	rm -f *.[chy] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
	
list:
	pr TODO $(SOURCE) makefile

lint:	y.tab.c
	lint -spb *.c
