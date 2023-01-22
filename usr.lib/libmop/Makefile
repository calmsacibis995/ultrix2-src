# @(#)Makefile	1.2	(ULTRIX)	10/21/85
#
CFLAGS=-O
DESTROOT=
OBJS=getnodeent.o putnodeent.o vio.o dnet_addr.o dnet_getloca.o

libmop.a: ${OBJS}
	ar cr libmop.a ${OBJS}

tools install: libmop.a
	install -c -m 644 libmop.a $(DESTROOT)/usr/lib
	ranlib $(DESTROOT)/usr/lib/libmop.a

clean:
	rm -f libmop.a errs core ${OBJS}

clobber: clean
	rm -f *.[hc] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
