# @(#)Makefile	1.7	ULTRIX	12/4/84
#
#	Makefile	4.2	83/07/08
#
DESTROOT=
CFLAGS=-O
YFLAGS=-d
FILES=awk.lx.o b.o main.o token.o tran.o lib.o run.o parse.o proctab.o freeze.o
SOURCE=awk.def awk.g.y awk.lx.l b.c lib.c main.c parse.c \
	proc.c freeze.c\
	run.c token.c tran.c

awk:	$(FILES) awk.g.o
	cc $(CFLAGS) awk.g.o  $(FILES)  -lm -o awk

y.tab.h:	awk.g.o

awk.h:	y.tab.h
	-cmp -s y.tab.h awk.h || cp y.tab.h awk.h

$(FILES):	awk.h awk.def

token.c:	awk.h
	ex - <tokenscript

install: awk
	install -c -s awk ${DESTROOT}/bin

profile:	awk.g.o $(FILES) mon.o
	cc -p -i awk.g.o $(FILES) mon.o -lm

find:
	egrep -n "$(PAT)" *.[ylhc] awk.def

list:
	-pr $(SOURCE) Makefile tokenscript README EXPLAIN

lint:
	lint -spu b.c main.c token.c tran.c run.c lib.c parse.c -lm |\
		egrep -v '^(error|free|malloc)'

proctab.c:	proc
	proc >proctab.c

proc:	awk.h proc.o token.o
	cc -o proc proc.o token.o

clean:
	-rm -f *.o t.* *temp* *.out *junk* y.tab.* awk.h proc proctab.c awk yacc*

clobber: clean
	rm -f ${FILES} $(SOURCE) Makefile README EXPLAIN
	rm -f *.[chlpy]

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
diffs:
	-for i in $(SOURCE); do echo $$i:; diff $$i /usr/src/cmd/awk | ind; done
