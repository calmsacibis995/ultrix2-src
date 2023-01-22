#	1.7	Makefile	84/11/26
DESTROOT=
SOURCES= man.0 man.1 man.2 man.3 man.4 man.5 man.6 man.7 man.8 man.9\
	purdue charset syscom quadf ibeams 
INDENT= 0.7i
WIDTH=  7.0i
TARGET=/usr/new/doc/apl

all:

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info

install: ${SOURCES}
		@-if [ ! -d ${DESTROOT}/usr/new/doc/apl ] ; then\
			mkdir ${DESTROOT}/usr/new/doc/apl; \
			chmod 755 ${DESTROOT}/usr/new/doc/apl; \
			/etc/chown root ${DESTROOT}/usr/new/doc/apl; \
			chgrp system ${DESTROOT}/usr/new/doc/apl; \
		else true; \
		fi

		@for i in ${SOURCES}; do \
			install -c $$i ${DESTROOT}${TARGET}/$$i; \
		done

		@install -c eed.doc ${DESTROOT}/usr/new/doc/apl/eed.doc
		@install -c bah.Makefile ${DESTROOT}/usr/new/doc/apl/Makefile

nman:	${SOURCES}
	echo ".nr PO" ${INDENT} > man.indent
	echo ".nr LL" ${WIDTH} >> man.indent
	echo ".nr LT" ${WIDTH} >> man.indent
	echo ".nr FL" ${WIDTH} >> man.indent
	nroff -ms man.indent man.? | tr -d "\010_" > manual.nrf

tman:	${SOURCES}
	echo ".nr LL" ${WIDTH} > man.indent
	echo ".nr LT" ${WIDTH} >> man.indent
	echo ".nr FL" ${WIDTH} >> man.indent
	troff -ms -t man.indent man.? > manual.trf

clean:
	@rm -f manual.nrf manual.trf man.indent

clobber: clean
	@rm -f [a-z]* [a-z]*.* Makefile READ_ME
