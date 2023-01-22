#
# Sccsid = @(#)Makefile	1.8	2/8/85
#
DESTROOT=
CFLAGS=	-O
BINKIT= ${DESTROOT}/usr/new/news
BINMAN= ${BINKIT}/man
BINMISC= ${BINKIT}/misc

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	src man

# Directories that have no Makefiles

COPYDIR= bin.kit cvt doc misc uucp

all:	${SUBDIR}

${SUBDIR}: FRC
	cd $@; make ${MFLAGS}

install: ${SUBDIR}
	-rm -f -r ${BINKIT}
	-if [ ! -d ${BINKIT} ]; \
	then \
		mkdir ${BINKIT}; \
		/etc/chown root ${BINKIT}; \
		chmod 755 ${BINKIT}; \
	else true; \
	fi
	-cp src/active ${BINKIT}/active
	-cp src/batch ${BINKIT}/batch
	-cp src/caesar ${BINKIT}/caesar
	-cp src/checknews ${BINKIT}/checknews
	-cp src/expire ${BINKIT}/expire
	-cp src/help ${BINKIT}/help
	-cp src/inews ${BINKIT}/inews
	-cp src/install.v7 ${BINKIT}/install.v7
	-cp src/postnews ${BINKIT}/postnews
	-cp src/readnews ${BINKIT}/readnews
	-cp src/recmail ${BINKIT}/recmail
	-cp src/recnews ${BINKIT}/recnews
	-cp src/sendnews ${BINKIT}/sendnews
	-cp src/unbatch ${BINKIT}/unbatch
	-cp src/uurec ${BINKIT}/uurec
	-cp doc/etiquette ${BINKIT}/etiquette
	-cp doc/howto ${BINKIT}/howto
	-cp doc/login ${BINKIT}/login
	-cp doc/nglist ${BINKIT}/nglist
	-cp doc/standard ${BINKIT}/standard
	-cp doc/tencmdts ${BINKIT}/tencmdts
	-cp doc/install ${BINKIT}/maintenance.doc
	-cp cvt/newactive.sh ${BINKIT}/newactive.sh
	-cp cvt/cvt.mkdirs.sh ${BINKIT}/cvt.mkdirs.sh
	-cp bin.kit/Makefile ${BINKIT}/Makefile
	-cp bin.kit/README_TO_INSTALL ${BINKIT}/README_TO_INSTALL
	-if [ ! -d ${BINMAN} ]; \
	then \
		mkdir ${BINMAN}; \
		/etc/chown root ${BINMAN}; \
		chmod 755 ${BINMAN}; \
	else \
		true; \
	fi
	-cp man/checknews.1 ${BINMAN}/checknews.1
	-cp man/expire.8 ${BINMAN}/expire.8
	-cp man/getdate.3 ${BINMAN}/getdate.3
	-cp man/inews.1 ${BINMAN}/inews.1
	-cp man/news.5 ${BINMAN}/news.5
	-cp man/newsrc.5 ${BINMAN}/newsrc.5
	-cp man/postnews.1 ${BINMAN}/postnews.1
	-cp man/readnews.1 ${BINMAN}/readnews.1
	-cp man/recnews.8 ${BINMAN}/recnews.8
	-cp man/sendnews.8 ${BINMAN}/sendnews.8
	-cp man/uurec.8 ${BINMAN}/uurec.8
	-if [ ! -d ${BINMISC} ]; \
	then \
		mkdir ${BINMISC}; \
		/etc/chown root ${BINMISC}; \
		chmod 755 ${BINMISC}; \
	else \
		true; \
	fi
	-cp misc/adm ${BINMISC}/adm
	-cp misc/copyright ${BINMISC}/copyright
	-cp misc/dirform ${BINMISC}/dirform
	-cp misc/euuname.sh ${BINMISC}/euuname.sh
	-cp misc/fa.recording ${BINMISC}/fa.recording
	-cp misc/mhhooks ${BINMISC}/mhhooks
	-cp misc/net.recording ${BINMISC}/net.recording
	-cp misc/newsinfo ${BINMISC}/newsinfo
	-cp misc/rnews.ml ${BINMISC}/rnews.ml
	-cp misc/rnews.sh ${BINMISC}/rnews.sh
	-cp misc/sendbatchednew ${BINMISC}/sendbatchednew
	-cp misc/sendnewsmail ${BINMISC}/sendnewsmail
	-cp misc/spool ${BINMISC}/spool
	-cp misc/trimlib ${BINMISC}/trimlib
	-cp README ${BINKIT}/README

clean:
	for i in ${SUBDIR}; do (echo $$i; cd $$i; make ${MFLAGS} clean); done

clobber:
	rm -f Makefile wishlist
	for i in ${COPYDIR}; \
	do \
		(cd $$i; rm -f [a-z]* M* R* =*); \
	done
	for i in ${SUBDIR}; \
	do \
		(cd $$i; make clobber); \
	done

sccsinfo:
	sccs info
	for i in ${COPYDIR}; \
	do \
		(echo $$i; cd $$i; sccs info); \
	done
	for i in ${SUBDIR}; \
	do \
		(echo $$i; cd $$i; make sccsinfo); \
	done

sccsget:
	sccs get SCCS
	for i in ${COPYDIR}; \
	do \
		(cd $$i; sccs get SCCS); \
	done
	for i in ${SUBDIR}; \
	do \
		(cd $$i; sccs get Makefile; make sccsget); \
	done

FRC:
