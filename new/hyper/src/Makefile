#	1.3	Makefile	84/10/11
DESTROOT=
LFLAGS=	-v
CFLAGS=	-O
ALL=	hyroute hylog

all:	${ALL}

hyroute: hyr_main.o hyr_lex.o
	${CC} ${CFLAGS} -o hyroute hyr_main.o hyr_lex.o -ll

hylog:	hylog.c
	${CC} -o hylog ${CFLAGS} hylog.c

install: all
	@install -c -s hyroute ${DESTROOT}/usr/new/hyroute
	@install -c -s hylog ${DESTROOT}/usr/new/hylog

hyr_main.o:	hyr_main.c hyr_sym.h
hyr_lex.o:	hyr_lex.l hyr_sym.h

clean:
	@rm -f *.o hyroute hyr_lex.c hylog
clobber: clean
	@rm -f Makefile *.[clyh] *.input

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info
