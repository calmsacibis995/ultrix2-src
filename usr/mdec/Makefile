#
#	2/13/86	ULTRIX	@(#)Makefile	1.6
#
# Makefile to install pcs750.bin, vmb.exe and ci780.bin
#
#
DESTROOT=
TARGET=/usr/mdec

all:

install: all

	-if [ ! -d ${DESTROOT}${TARGET} ] ; then \
		mkdir ${DESTROOT}${TARGET}; \
		chmod 755 ${DESTROOT}${TARGET} ; \
	else \
		true; \
	fi
	install -c pcs750.bin ${DESTROOT}${TARGET}/pcs750.bin
	install -c pcs750.bin ${DESTROOT}/pcs750.bin
	install -c vmb.exe ${DESTROOT}${TARGET}/vmb.exe
	install -c vmb.exe ${DESTROOT}/vmb.exe
	install -c ci780.bin ${DESTROOT}${TARGET}/ci780.bin

clean:
clobber:
sccsget: 
	sccs get SCCS
sccsinfo: 
	sccs info
