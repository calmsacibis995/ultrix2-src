
#
# %W% 	ULTRIX	%G%
#
#
FILES = bigram code updatedb

all:

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

install:
	for i in ${FILES}; do \
		(install -c $$i ${DESTROOT}/usr/lib/find/$$i); done
