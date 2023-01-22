#	1.3	Makefile	84/11/26
DESTROOT=
TARGET=/usr/new/doc/icon

all:

install: all
	@-if [ ! -d ${DESTROOT}/usr/new/doc ]; \
	then \
		mkdir ${DESTROOT}/usr/new/doc; \
	else \
		true; \
	fi
	@-if [ ! -d ${DESTROOT}${TARGET} ]; \
	then \
		mkdir ${DESTROOT}${TARGET}; \
	else \
		true; \
	fi
	@install -c Makefile ${DESTROOT}${TARGET}/Makefile
	@install -c defaults ${DESTROOT}${TARGET}/defaults.txt
	@install -c extensions ${DESTROOT}${TARGET}/extensions.ms
	@install -c overview ${DESTROOT}${TARGET}/overview.tr
	@install -c runerrs ${DESTROOT}${TARGET}/runerrs.txt
	@install -c tmac.tr ${DESTROOT}${TARGET}/tmac.tr
	@install -c touradd ${DESTROOT}${TARGET}/touradd.tr
	@install -c v5gover ${DESTROOT}${TARGET}/v5gover.tr
clean:
clobber: clean
	@rm -f [a-z]* *.tr Makefile
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS

dist-clean:
	rm -rf `gcomp Makefile defaults install runerrs tmac.tr v5gover touradd overview extensions icont.1 iconc.1'
