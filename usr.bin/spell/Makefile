# @(#)Makefile	1.7	ULTRIX	11/20/84
#
#	@(#)Makefile	4.3	5/25/83

CFLAGS= -O
DESTROOT=

all:	spell hlist hlista hlistb hstop spellin spellout

install:	all
	-if [ ! -d ${DESTROOT}/usr/dict ];\
	then \
		mkdir ${DESTROOT}/usr/dict; \
		/etc/chown root ${DESTROOT}/usr/dict; \
		chgrp system ${DESTROOT}/usr/dict; \
		chmod 0755 ${DESTROOT}/usr/dict; \
	else true; \
	fi
	install -c -s spell ${DESTROOT}/usr/lib/spell
	install -c -s spellin ${DESTROOT}/usr/bin/spellin
	install -c -s spellout ${DESTROOT}/usr/bin/spellout
	install -c hlista ${DESTROOT}/usr/dict/hlista
	install -c hlistb ${DESTROOT}/usr/dict/hlistb
	install -c hstop ${DESTROOT}/usr/dict/hstop
	install -c spell.sh ${DESTROOT}/usr/bin/spell
	install -c words ${DESTROOT}/usr/dict/words
	install -c /dev/null ${DESTROOT}/usr/dict/spellhist

clean:	
	rm -f *.o spell spellin hlista hlistb hlist hstop spellout

clobber: clean
	rm -f american british local *.[ch] spell.sh stop Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

spell: spell.o
	cc spell.o -o spell
spellin: spellin.o
	cc spellin.o -o spellin
spellout: spellout.o
	cc spellout.o -o spellout

hlist: words spellin
	spellin <words >hlist
hlista: american local hlist spellin
	(cat american local)|spellin hlist >hlista
hlistb: british local hlist spellin
	(cat british local)|spellin hlist >hlistb
hstop: stop spellin
	spellin <stop >hstop
