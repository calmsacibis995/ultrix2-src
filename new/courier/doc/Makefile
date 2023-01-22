#	1.5	Makefile	84/11/26
#
DESTROOT=
TARGET= /usr/new/doc/courier

all:

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info

clean:

install:
		@-if [ ! -d ${DESTROOT}${TARGET} ]; then \
			mkdir ${DESTROOT}${TARGET} ; \
			chmod 755 ${DESTROOT}/${TARGET} ; \
			/etc/chown root ${DESTROOT}${TARGET}; \
		else true; \
		fi

		@install -c courier.tbl.me ${DESTROOT}${TARGET}/courier.tbl.me
		@install -c grammar ${DESTROOT}${TARGET}/grammar
clobber: clean
	@rm -f Makefile *.[chy]
