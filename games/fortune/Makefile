# @(#)Makefile	1.5	ULTRIX	4/7/86
#
#	@(#)Makefile	4.2	(Berkeley)	83/08/14
#
DESTROOT=
FORTUNES=	scene 
SOURCE=		fortune.c strfile.h strfile.c unstr.c ${FORTUNES}
LIBDIR=		${DESTROOT}/usr/games/lib
BINDIR=		${DESTROOT}/usr/games
OWN=		daemon
CFLAGS=		-O

.DEFAULT:
	sccs get $<

all: fortune strfile unstr fortunes.dat

fortune: strfile.h fortune.c
	${CC} ${CFLAGS} -DFORTFILE='"${LIBDIR}/fortunes.dat"' -o fortune fortune.c

strfile: strfile.h strfile.c
	${CC} ${CFLAGS} -o strfile strfile.c

unstr: strfile.h unstr.c
	${CC} ${CFLAGS} -o unstr unstr.c

fortunes.dat: fortunes strfile
	./strfile fortunes

fortunes: $(FORTUNES)
	cat scene > fortunes
	echo "%%" >> fortunes

install: all
	install -m 600 -o ${OWN} fortunes.dat ${LIBDIR}/
	install -m 700 -o ${OWN} strfile ${LIBDIR}/
	install -m 4711 -o ${OWN} fortune ${BINDIR}/

clean:
	rm -f fortune fortunes fortunes.dat fortunes.tar strfile unstr

clobber: clean
	rm *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS 
