# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.1	2/9/83
#
DIFF=	/bin/diff
DIFFH=	/usr/lib/diffh
PR=	/bin/pr
CFLAGS=	-O -DDIFF='"${DIFF}"' -DDIFFH='"${DIFFH}"' -DPR='"${PR}"'
DESTROOT=

SRCS=	diff.c diffdir.c diffreg.c
HDRS=	diff.h
OBJS=	diff.o diffdir.o diffreg.o

all: diff diffh

diff: ${OBJS}
	cc ${CFLAGS} -o diff ${OBJS} ${LDFLAGS}

diffh: diffh.o
	cc -o diffh diffh.o ${LDFLAGS}

clean:
	rm -f diff diffh ${OBJS} diffh.o
	rm -f a.out core errs

clobber: clean
	rm -f *.[hoc] Makefile

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<

install: diff diffh
	install -c -s diff ${DESTROOT}/bin/diff
	install -c -s diffh ${DESTROOT}/usr/lib/diffh
