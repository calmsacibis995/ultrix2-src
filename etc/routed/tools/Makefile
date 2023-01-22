# @(#)Makefile	1.3	ULTRIX	9/23/85
#
# Makefile 10/21/82
ALL= query.c trace.c
CFLAGS=	-O
DESTROOT=
LDFLAGS=

all:	query trace


# test programs and utilities
query:	query.o
	${CC} -o query ${LDFLAGS} query.o

trace:	trace.o
	${CC} -o trace ${LDFLAGS} trace.o

install:
	@echo Nothing to install

lint:
	lint -hbxn query.c trace.c

clean:
	rm -f *.o core a.out t.c query linterrs trace typescript errs

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
