# @(#)Makefile	1.1	Ultrix	7/3/86
#
#	Makefile for GNU Emacs "lisp"subdirectory.
#

# Symbols below are derived from top level Makefile.  See Makefile followed by
# cd .. and sccs get Makefile.
DESTROOT=
PATHLOADSEARCH=

SUBDIR= term

ELFILES=   abbrev.el add-log.el aton.el backquote.el bg-mouse.el buff-menu.el \
	   bytecomp.el c-mode.el chistory.el compare-w.el compile.el debug.el \
	   dired.el dissociate.el doctor.el ebuff-menu.el echistory.el \
	   electric.el emacsbug.el files.el fill.el gosling.el grow-vers.el \
	   hanoi.el helper.el inc-vers.el indent.el info.el isearch.el \
	   ledit.el lisp-mode.el lisp.el loaddefs.el loadup.el lpr.el \
	   macros.el mail-utils.el mailalias.el makesum.el man.el medit.el \
	   mh-e.el mim-mode.el mim-syntax.el mlconvert.el mlsupport.el \
	   novice.el nroff-mode.el options.el outline.el page.el paragraphs.el \
	   paths.el picture.el rect.el register.el replace.el rmail.el \
	   rmailedit.el rmailkwd.el rmailmsc.el rmailout.el rmailsum.el \
	   rnews.el scheme.el sendmail.el shell.el simple.el spell.el \
	   startup.el subr.el tabify.el tags.el telnet.el tex-mode.el \
	   tex-start.el texinfo.el text-mode.el time.el underline.el \
	   undigest.el unused.el userlock.el version.el view.el window.el \
	   x-mouse.el xscheme.el yow.el

ELCFILES=  abbrev.elc aton.elc backquote.elc bg-mouse.elc buff-menu.elc \
	   bytecomp.elc c-mode.elc chistory.elc compare-w.elc compile.elc \
	   debug.elc dired.elc dissociate.elc doctor.elc ebuff-menu.elc \
	   echistory.elc electric.elc files.elc fill.elc hanoi.elc helper.elc \
	   indent.elc info.elc isearch.elc lisp-mode.elc lisp.elc lpr.elc \
	   macros.elc mail-utils.elc mailalias.elc makesum.elc man.elc \
	   medit.elc mh-e.elc mim-mode.elc mim-syntax.elc mlconvert.elc \
	   mlsupport.elc nroff-mode.elc options.elc outline.elc page.elc \
	   paragraphs.elc picture.elc rect.elc register.elc replace.elc \
	   rmail.elc rmailedit.elc rmailkwd.elc rmailmsc.elc rmailout.elc \
	   rmailsum.elc rnews.elc scheme.elc sendmail.elc shell.elc simple.elc \
	   spell.elc startup.elc subr.elc tabify.elc tags.elc telnet.elc \
	   tex-mode.elc texinfo.elc text-mode.elc time.elc underline.elc \
	   undigest.elc view.elc window.elc x-mouse.elc yow.elc

all:
	for i in ${SUBDIR}; do \
		(cd $$i; \
		make ${MFLAGS} \
			DESTROOT=${DESTROOT} \
			PATHLOADSEARCH=${PATHLOADSEARCH} \
		all); \
	done

install: # This target calls this makefile with checkdir as target recursively.
	for i in ${ELFILES} ${ELCFILES}; do \
		install -c -m 644 $$i ${DESTROOT}${PATHLOADSEARCH}; done
	for i in ${SUBDIR}; do \
		(make checkdir "CHECKDIR=${DESTROOT}${PATHLOADSEARCH}/$$i";\
		cd $$i; \
		make ${MFLAGS} \
			DESTROOT=${DESTROOT} \
			PATHLOADSEARCH=${PATHLOADSEARCH} \
		install); \
	done

checkdir:
	@-if [ ! -d ${CHECKDIR} ] ; then \
		mkdir ${CHECKDIR}; \
		chmod 755 ${CHECKDIR}; \
	else true; \
	fi

clean:
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

clobber:	clean
	sccs clean
	rm -f ${ELCFILES}
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clobber); done

sccsinfo:
	@sccs info
	@for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} sccsinfo); done

#
# All .elc file encoded to printable ASCII code befor put into SCCS library
# using following command.
# uuencode xxx.elc /tmp/xxx.elc.enc |sed '1s/[0-9][0-9][0-9]/777/' > xxx.elc.enc
# Because each .elc file contain binary data.
# Notice: If you changed any .elc file, you must encode .elc file using
#         uuencode. And You must specify /tmp/ as directory of remote
#         destination. Also, You must edit mode described in encoded file
#         to "777".
#
sccsget:
	sccs get SCCS
	for i in ${ELCFILES}; do \
          uudecode $$i.enc ; \
	  cp /tmp/$$i . ; \
	  rm -f /tmp/$$i $$i.enc; \
	  chmod 444 $$i ; \
	done
	for i in ${SUBDIR}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done

