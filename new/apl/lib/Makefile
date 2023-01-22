#	@(#)Makefile	1.2	Ultrix	1/3/85
#
DESTROOT=
ALL=	AW anovaws aw base editws fftws getws gradesys \
	graph2ws graphws iows news ngradesys plotws shapews

TARGET=/usr/new/lib/apl
all:	;

sccsget:
	@sccs get -s SCCS
	@(cd fn; sccs get -s SCCS)

sccsinfo:
	@sccs info
install:
	@-if [ ! -d ${DESTROOT}/usr/new/lib/apl ]; \
	then \
		mkdir ${DESTROOT}/usr/new/lib/apl ; \
	else \
		true; \
	fi

	@for i in ${ALL}; do \
		install -c -m 644 $$i ${DESTROOT}${TARGET}/$$i; done

clean:	;

clobber: clean
	@rm -f fn/[a-z]* READ_ME Makefile

