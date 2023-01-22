#		@(#)Makefile	1.3	(ULTRIX)	7/8/85
#
#	Makefile -- for libmalloc.a
#
#	Modification History
#
# 001	David L Ballenger, 29-May-1985
#	Modify for use with ULTRIX build procedures.
#

DESTROOT=
LINT	= lint
LINTFLAGS = -p
CFLAGS	= -O
OBJS	= malloc.o


all:	libmalloc.a llib-lmalloc.ln

libmalloc.a: ${OBJS}
	ar cru libmalloc.a ${OBJS}

llib-lmalloc.ln: llib-lmall.c
	lint -Cmalloc -p llib-lmall.c

tools: libmalloc.a
	install -c -m 644 libmalloc.a ${DESTROOT}/usr/lib
	ranlib ${DESTROOT}/usr/lib/libmalloc.a

install: tools llib-lmalloc.ln
	@-if [ ! -d ${DESTROOT}/usr/lib/lint ] ; then \
		mkdir ${DESTROOT}/usr/lib/lint ; \
		/etc/chown root ${DESTROOT}/usr/lib/lint ; \
		chgrp system ${DESTROOT}/usr/lib/lint ; \
		chmod 0755 ${DESTROOT}/usr/lib/lint ; \
	fi
	install -c -m 644 llib-lmalloc.ln ${DESTROOT}/usr/lib/lint

clean:
	rm -f ${OBJS} libmalloc.a llib-lmalloc.ln

clobber: clean
	rm -f Makefile *.h *.c

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info
