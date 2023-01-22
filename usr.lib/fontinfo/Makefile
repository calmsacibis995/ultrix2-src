
#
# %W%	ULTRIX	%G%
#
FILES = apl basker.b basker.i basker.r bocklin bodoni.b bodoni.i \
	bodoni.r chess clarendon cm.b cm.i cm.r countdown cyrillic \
	delegate.b delegate.i delegate.r fix gacham.b gacham.i \
	gacham.r graphics greek h19 hebrew meteor.b meteor.i \
	meteor.r mona nonie.b nonie.i nonie.r oldenglish otimes.b \
	otimes.i otimes.r pip playbill script seal shadow sign \
	stare.b stare.i stare.r times.b times.i times.r ugramma \
	wumpus xx

all:

install:

	for i in ${FILES}; do \
		(install -c $$i ${DESTROOT}/usr/lib/fontinfo/$$i); done


sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

clean:


clobber:

