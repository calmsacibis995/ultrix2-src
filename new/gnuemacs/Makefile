#  @(#)Makefile	1.6	ULTRIX	10/3/86
#
#	Makefile for total GNU Emacs system.
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 02	9-Sep-86, Greg Tarsa
#	(blush) defined variable ORIGEMACSSRC so that install will
#	work properly.
#
# 01	5-Sep-86, Greg Tarsa
#	Added a line to copy the GNU Emacs source tar image to the binary
#	directory
#
DESTROOT=

SUBDIRS= src etc lisp info dbx cpp shortnames man

ORIGEMACSSOURCE = gnuemacs.tar

# Assign a directory name to EMACSBIN, which should be on a user's command
# search path and has "emacs","ctags" and "etags" eventurely after you
# run this Makefile with install as target.
# Original gnemacs defines this directory in the file named ./build-install,
# which is the build and install sh procedure.
# But this Makefile doesn't use this procedure.
EMACSBIN=/usr/new/emacs
USERPATH= ${EMACSBIN}

# Assign a directory name to EMACSLIB, to which many files are moved
# for emacs to work correctly after you run this Makefile with install
# as target.
# Original gnemacs defines this directory in the file named ./build-install,
# which is the build and install sh procedure.
# But this Makefile doesn't use this procedure.
EMACSLIB= /usr/new/emacs/lib

# In the original gnemacs, symbols below are deifned in the file named
# "src/paths.h" by copying from the template file paths.h.dist and
# modifying it.
# But this Makefile overrides definitions of path.h.
# If you want to change the definitions, assign to the symbols below here.
#
# PATH_LOADSEARCH
# PATH_EXEC
# PATH_LOCK
# PATH_SUPERLOCK

PATHLOADSEARCH= ${EMACSLIB}/lisp	# Don't follow pre ":"
PATHEXEC= ${EMACSLIB}/etc
PATHLOCK= ${EMACSLIB}/lock		# Don't end with slash here
PATHSUPERLOCK= ${EMACSLIB}/lock/!!!SuperLock!!!

# If you create the file "lisp/site-init.el" and, define the Info-directory by
# setq, also you must define this INFODIR macro.
INFODIR= ${EMACSLIB}/info

# this is a temporary macro in the recursive make.
CHECKDIR=

all:
	for i in ${SUBDIRS}; do \
		(cd $$i; \
		make ${MFLAGS} \
			DESTROOT=${DESTROOT} \
			EMACSBIN=${EMACSBIN} \
			USERPATH=${USERPATH} \
			PATHEXEC=${PATHEXEC} \
			PATHLOADSEARCH=${PATHLOADSEARCH} \
			PATHLOCK=${PATHLOCK} \
			PATHSUPERLOCK=${PATHSUPERLOCK} \
			INFODIR=${INFODIR} \
		all); \
	done

install:
# This for-loop call this makefile("checkdir" as label) recursively
	for i in ${DESTROOT} \
		${DESTROOT}/usr \
		${DESTROOT}/usr/local \
		${DESTROOT}${EMACSBIN} \
		${DESTROOT}${EMACSLIB} \
		${DESTROOT}${USERPATH} \
		${DESTROOT}${PATHEXEC} \
		${DESTROOT}${PATHLOADSEARCH} \
		${DESTROOT}${PATHLOCK} \
		${DESTROOT}${INFODIR}; do \
			make checkdir "CHECKDIR=$$i";\
	done
	install -c -m 644 ${ORIGEMACSSOURCE} ${DESTROOT}${EMACSBIN}
	for i in ${SUBDIRS}; do \
		(cd $$i; \
		make ${MFLAGS} \
			DESTROOT=${DESTROOT} \
			EMACSBIN=${EMACSBIN} \
			USERPATH=${USERPATH} \
			PATHEXEC=${PATHEXEC} \
			PATHLOADSEARCH=${PATHLOADSEARCH} \
			PATHLOCK=${PATHLOCK} \
			PATHSUPERLOCK=${PATHSUPERLOCK} \
			INFODIR=${INFODIR} \
		install); \
	done

checkdir:
	@-if [ ! -d ${CHECKDIR} ] ; then \
		mkdir ${CHECKDIR}; \
		chmod 755 ${CHECKDIR}; \
	else true; \
	fi

clean:
	for i in ${SUBDIRS}; do (cd $$i; make ${MFLAGS} clean); done

clobber:	clean
	sccs clean
	for i in ${SUBDIRS}; do (cd $$i; make ${MFLAGS} clobber); done

sccsinfo:
	@sccs info
	@for i in ${SUBDIRS}; do (cd $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIRS}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done


