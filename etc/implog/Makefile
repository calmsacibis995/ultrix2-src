# @(#)Makefile	1.3	ULTRIX	12/4/84
#
# @(#)Makefile	4.5 6/12/83
ALL=	implog implogd
DESTROOT=
CFLAGS=-O

all:	${ALL}

implog:	implog.o
	${CC} implog.o -o implog

implogd: implogd.o
	${CC} implogd.o -o implogd

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

clobber: clean
	rm -f Makefile *.[hc]

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

install: ${ALL}
	install -c implog ${DESTROOT}/etc
	install -c implogd ${DESTROOT}/etc
