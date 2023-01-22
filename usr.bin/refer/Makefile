# @(#)Makefile	1.7	ULTRIX	2/12/87
#
#	@(#)Makefile	4.5	(Berkeley)	84/06/05
#	@(#)Makefile	4.4	(Berkeley)	83/06/22
#----------
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 02	2-Jul-86, Greg Tarsa
#	Changed tha 'all' in tools1 to the specific programs needed.
#
# 01	29-May-86, Greg Tarsa
#	Added Rbstjissue, Rv7man and runinv to this makefile and directory.
#	These have been removed from /usr/src/usr/dict/papers because
#	Ind.i[abc] need to be built from them at every build.  The
#	necessary rules have been added.
#
#	Added 4.3BSD changes to sources.  Added LDFLAGS to allow compilation
#	with -g switch when necessary.
#

DESTROOT=
CFLAGS = -O
LDFLAGS =

PUBLISTS = Rbstjissue Rv7man

all:	mkey inv hunt refer addbib lookbib sortbib runinv INDfiles

mkey: mkey1.o mkey2.o mkey3.o deliv2.o
	cc $(LDFLAGS) mkey?.o deliv2.o -o mkey
inv: inv1.o inv2.o inv3.o inv5.o inv6.o deliv2.o
	cc $(LDFLAGS) inv?.o deliv2.o -o inv
hunt: hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o hunt7.o glue5.o
hunt: refer3.o hunt9.o shell.o deliv2.o hunt8.o glue4.o tick.o
	cc $(LDFLAGS) hunt?.o refer3.o glue5.o glue4.o shell.o deliv2.o tick.o -o hunt

runinv: runinv.sh
	cp runinv.sh runinv

# This rule makes Ind.i[abc]
INDfiles: ${PUBLISTS}
	sh runinv

glue3.o: refer..c
hunt2.o: refer..c
hunt3.o: refer..c
refer0.o: refer..c
refer1.o: refer..c
refer2.o: refer..c
refer3.o: refer..c
refer4.o: refer..c
refer6.o: refer..c
refer5.o: refer..c
refer: glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o
refer: refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt5.o hunt6.o hunt8.o
refer: glue3.o hunt7.o hunt9.o glue2.o glue4.o glue5.o refer0.o shell.o
	cc $(LDFLAGS) glue?.o refer[01245678].o hunt[2356789].o mkey3.o shell.o deliv2.o -o refer

addbib: addbib.o
	cc $(LDFLAGS) addbib.o -o addbib
lookbib: lookbib.o
	cc $(LDFLAGS) lookbib.o -o lookbib
sortbib: sortbib.o
	cc $(LDFLAGS) sortbib.o -o sortbib

tools1: mkey inv hunt
	-if [ ! -d ${DESTROOT}/usr/lib/refer ]; then \
		mkdir ${DESTROOT}/usr/lib/refer; \
	else true; \
	fi
	install -c -s mkey $(DESTROOT)/usr/lib/refer
	install -c -s inv  $(DESTROOT)/usr/lib/refer
	install -c -s hunt $(DESTROOT)/usr/lib/refer

install: all tools1
	install -c -s refer $(DESTROOT)/usr/bin
	install -s -c addbib $(DESTROOT)/usr/bin
	install -c -s sortbib $(DESTROOT)/usr/bin
	install -c roffbib.sh $(DESTROOT)/usr/bin/roffbib
	install -c indxbib.sh $(DESTROOT)/usr/bin/indxbib
	install -c -s lookbib $(DESTROOT)/usr/bin
	install -c tmac.bib $(DESTROOT)/usr/lib/tmac

#
# Create the papers directory, if necessary.
# Build the index files for the publication lists
#
	-if [ ! -d ${DESTROOT}/usr/dict ]; \
	then \
		mkdir ${DESTROOT}/usr/dict; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/dict/papers ]; \
	then \
		mkdir ${DESTROOT}/usr/dict/papers; \
	else \
		true; \
	fi
	for i in ${PUBLISTS} Ind.ia Ind.ib Ind.ic; do \
	    install -c $$i ${DESTROOT}/usr/dict/papers/$$i;\
	done
	install -c runinv ${DESTROOT}/usr/dict/papers

clean:
	rm -f refer inv hunt mkey addbib lookbib sortbib *.o

clobber: clean
	rm -f *.c *.bib *.sh TEST TOFIX Makefile Ind* runinv R*

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

whatabout: what1.o what2.o what3.o what4.o shell.o mkey3.o
	cc $(LDFLAGS) what?.o shell.o mkey3.o  -o whatabout
deliv: deliv1.o deliv2.o
	cc $(LDFLAGS) deliv?.o  -o deliv
refpart: refer0.o refer1.o refer2.o refer3.o refer4.o refer5.o
refpart: refer6.o refer7.o refer8.o deliv2.o glue4.o
	cc $(LDFLAGS) refer?.o deliv2.o glue4.o  -o refpart
