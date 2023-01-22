#
# Sccsid = @(#)Makefile	1.8	2/12/85
#
DESTROOT=
BINKIT=${DESTROOT}/usr/new/notes
SRCDIR= src
COPYDIR= bin.kit doc help man

all:
	for i in ${SRCDIR}; \
	do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} all); \
	done

clean:
	for i in ${SRCDIR}; \
	do \
		(cd $$i; make ${MFLAGS} clean); \
	done

clobber:
	for i in ${COPYDIR}; \
	do \
		(cd $$i; rm -f Makefile r* [0-9] [a-z]*); \
	done
	for i in ${SRCDIR}; \
	do \
		(cd $$i; make clobber); \
	done
	rm -f Makefile README

install: all
	-rm -f -r ${BINKIT}
	-if [ ! -d ${BINKIT} ] ; then \
		mkdir ${BINKIT}; \
		/etc/chown root ${BINKIT}; \
		chmod 755 ${BINKIT}; \
	else true; \
	fi
	-if [ ! -d ${BINKIT}/doc ] ; then \
		mkdir ${BINKIT}/doc; \
		/etc/chown root ${BINKIT}/doc; \
		chmod 755 ${BINKIT}/doc; \
	else true; \
	fi
	-cp src/bnewsinput ${BINKIT}/bnewsinput
	-cp src/checknotes ${BINKIT}/checknotes
	-cp src/main ${BINKIT}/main
	-cp src/mknf ${BINKIT}/mknf
	-cp src/newsinput ${BINKIT}/newsinput
	-cp src/newsoutput ${BINKIT}/newsoutput
	-cp src/nfarchive ${BINKIT}/nfarchive
	-cp src/nfpipe ${BINKIT}/nfpipe
	-cp src/nfprint ${BINKIT}/nfprint
	-cp src/nfrcv ${BINKIT}/nfrcv
	-cp src/nfstats ${BINKIT}/nfstats
	-cp src/nfxmit ${BINKIT}/nfxmit
	-cp src/rmnf ${BINKIT}/rmnf
	-cp doc/[0-5] doc/Makefile doc/a.checklist doc/b.future ${BINKIT}/doc
	-cp doc/c.update doc/summary doc/vtoc ${BINKIT}/doc
	-cp help/access.help ${BINKIT}/access.help
	-cp help/dir.help ${BINKIT}/dir.help
	-cp help/index.help ${BINKIT}/index.help
	-cp help/lim.help ${BINKIT}/lim.help
	-cp help/read.help ${BINKIT}/read.help
	-cp man/notes.1 ${BINKIT}/notes.1
	-cp man/notes.8 ${BINKIT}/notes.8
	-cp bin.kit/Makefile ${BINKIT}/Makefile
	-cp bin.kit/README ${BINKIT}/README
	-cp bin.kit/README_TO_INSTALL ${BINKIT}/README_TO_INSTALL

sccsinfo:
	sccs info
	for i in ${COPYDIR}; \
	do \
		(echo $$i; cd $$i; sccs info); \
	done
	for i in ${SRCDIR}; \
	do \
		(cd $$i; make sccsinfo); \
	done

sccsget:
	sccs get SCCS
	for i in ${COPYDIR}; \
	do \
		(cd $$i; sccs get SCCS); \
	done
	for i in ${SRCDIR}; \
	do \
		(cd $$i; sccs get Makefile; make sccsget); \
	done
