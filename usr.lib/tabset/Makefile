# MAKEFILE			AUTHOR R. N. FONTAINE
#
# copy files in this directory
#
#

DESTROOT=
FILES= 3101 aa beehive diablo std stdcrt teleray vt100 xerox1720


all:

install: 
	for i in ${FILES} ; do \
		cp $$i ${DESTROOT}/usr/lib/tabset/$$i ; \
	done

clean:
