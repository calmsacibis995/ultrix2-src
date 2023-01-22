#	1.4	Makefile	84/11/26
OBJS=	courierd.o
CFLAGS=	-O
DESTROOT=
TARGET=/usr/new/lib/courier

courierd: ${OBJS}
	${CC} ${CFLAGS} -o courierd ${OBJS}

install: courierd
	@-if [ ! -d ${DESTROOT}${TARGET} ]; \
	then \
		mkdir ${DESTROOT}${TARGET}; \
	else \
		true; \
	fi
	@install -c -s courierd ${DESTROOT}${TARGET}/courierd

clean:
	@rm -f ${OBJS} courierd

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info

clobber: clean
	@rm -f Makefile *.[chy]
