#	1.6	Makefile	84/11/27
DESTROOT=
TARGET=/usr/new/doc
all:;

install:
	@-if [ ! -d ${DESTROOT}${TARGET} ]; \
	then \
		mkdir ${DESTROOT}${TARGET} ; \
	else \
		true; \
	fi
	@install -c courier.1 ${DESTROOT}${TARGET}/courier.n
	@-mkdir ${DESTROOT}${TARGET}/courier
	@install -c courier.1 ${DESTROOT}${TARGET}/courier/courier.1

clean:;

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info

clobber: clean
	@rm -f Makefile courier.1
