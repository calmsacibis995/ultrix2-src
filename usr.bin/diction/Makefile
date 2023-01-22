# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	@(#)Makefile	4.5	(Berkeley)	83/06/30
#
#	make file for STYLE and DICTION
#
#	BIN is the directory for storing the object code
#		for the 3 programs that make up style
#	DICT is the full pathname of the file dict.d, the
#		dictionary file used by DICTION
#
DESTROOT=
SRCS = \
	abbrev.c conp.h dict.c dict.d \
	diction.c diction.sh edict.c ehash.c \
	explain.d \
	end.l getopt.c names.h nhash.c \
	nwords.l outp.c part.l pscan.c \
	style.h style.sh explain.sh ydict.c \
	README

BIN = ${DESTROOT}/usr/lib
DICT= -DDICT=\"/usr/lib/dict.d\"

all: diction style

style: style1 style2 style3

install: style diction style.sh diction.sh explain.sh dict.d explain.d
	install -c -s style1 ${DESTROOT}/usr/lib
	install -c -s style2 ${DESTROOT}/usr/lib
	install -c -s style3 ${DESTROOT}/usr/lib
	install -c -s dprog ${DESTROOT}/usr/lib
	install -c style.sh ${DESTROOT}/usr/bin/style
	install -c diction.sh ${DESTROOT}/usr/bin/diction
	install -c explain.sh ${DESTROOT}/usr/bin/explain
	install -c dict.d ${DESTROOT}/usr/lib
	install -c explain.d ${DESTROOT}/usr/lib
#	ln ${DESTROOT}/usr/bin/explain ${DESTROOT}/usr/bin/suggest

clean:
	rm -f prtlex.c wdslex.c endlex.c
	rm -f *.o style1 style2 style3 dprog

clobber: clean
	rm -f *.[hcdl] *.sh Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

style3: prtlex.o pscan.o outp.o
	cc prtlex.o pscan.o outp.o -ll  -o style3
prtlex.c: part.l
	lex part.l
	mv lex.yy.c prtlex.c
prtlex.o: names.h prtlex.c conp.h style.h names.h
	cc -c -O prtlex.c
pscan.o: names.h conp.h pscan.c
	cc -c -O pscan.c
outp.o: names.h conp.h style.h outp.c
	cc -c -O outp.c

style1: wdslex.o
	cc wdslex.o -ll  -o style1
wdslex.o: wdslex.c nhash.c dict.c ydict.c names.h abbrev.c
	cc -c wdslex.c
wdslex.c: nwords.l
	lex nwords.l
	mv lex.yy.c wdslex.c

style2: endlex.o
	cc endlex.o -ll  -o style2
endlex.o: names.h endlex.c ehash.c edict.c
	cc -c endlex.c
endlex.c: end.l
	lex end.l
	mv lex.yy.c endlex.c

diction: dprog
dprog: diction.c
	cc -O $(DICT) diction.c -o dprog

srcs:	sources

sources: $(SRCS)

$(SRCS):
	sccs get $@

