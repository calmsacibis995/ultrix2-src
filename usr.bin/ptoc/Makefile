DESTDIR=
# @(#)Makefile	1.0	ULTRIX	5/07/86
#
#
DESTROOT=
CFLAGS=-O
LFLAGS=-h
SOURCES=main.c sc.c prsubr.c prdecl.c prstmt.c prexpr.c emit.c builtins.c
OBJS=main.o sc.o prsubr.o prdecl.o prstmt.o prexpr.o emit.o builtins.o

ptoc: ${OBJS}
	cc ${CFLAGS} -o ptoc ${OBJS}

lint: ${SOURCES}
	lint ${LFLAGS} ${SOURCES}

${OBJS}: ptoc.h

# Utilities

install: ptoc
	install -c -s ptoc ${DESTROOT}/usr/bin/ptoc

clean:
	rm -f ptoc ${OBJS}

clobber: clean
	rm -f Makefile *.[hc]

sccsget:
	sccs get SCCS 

sccsinfo:
	sccs info 
