#		@(#)Makefile	1.19	(ULTRIX)	8/19/85
#
# $Header: Makefile,v 1.25 83/09/12 16:26:12 sklower Exp $
#  Makefile for total Franz Lisp system.
#
#	Modification History
#
#	David L Ballenger, 6-May-1985
# 001-	This Makefile and the Makefiles which it invokes have been 
#	extensively rewritten to make them work correctly in the 
#	ULTRIX system build.  
#

.DEFAULT: all

DESTROOT=
LIBDIR=   /usr/lib/lisp
OBJDIR=	  ${LIBDIR}
BINDIR=   /usr/ucb
DIRS=	  LIBDIR=${LIBDIR} OBJDIR=${OBJDIR} BINDIR=${BINDIR}
SUBDIR=	  doc franz lisplib liszt pearl utils
MACH=     vax
FranzD=   franz/${MACH}
LisztD=   liszt/${MACH}
Src=      Makefile ReadMe lispconf lispnews scriptcat Notice cvt.awk
VERSION=  38.79

FRC:;

build: 
	make ${MFLAGS} clean
	make ${MFLAGS} DESTROOT=${DESTROOT} slow
	make ${MFLAGS} DESTROOT=${DESTROOT} install

slow:
	-if [ ! -d ${DESTROOT}${LIBDIR} ]; then \
		mkdir  ${DESTROOT}${LIBDIR};\
		/etc/chown root  ${DESTROOT}${LIBDIR};\
		chgrp system  ${DESTROOT}${LIBDIR};\
	else true ;\
	fi
	date
	(cd utils    ; make all)
	(cd lisplib  ; make as nld )
	(TOP=`pwd`;\
	 cd ${FranzD};\
	 make LIBDIR=$${TOP}/lisplib LIBDIR=$${TOP}/lisplib snlisp)
	date
	(TOP=`pwd`;\
	 cd ${LisztD};\
	 make Lisp=$${TOP}/${FranzD}/snlisp snliszt)
	(TOP=`pwd`;\
	 cd ${LisztD};\
	 make Lisp=$${TOP}/${FranzD}/snlisp Liszt=./snliszt nliszt)
	date
	(TOP=`pwd`;\
	 cd ${FranzD};\
	 make Liszt=$${TOP}/${LisztD}/nliszt LIBDIR=$${TOP}/lisplib nlisp)
	(TOP=`pwd`;\
	 cd ${LisztD};\
	 make Lisp=$${TOP}/${FranzD}/nlisp Liszt=./nliszt nliszt)
	(TOP=`pwd`;\
	 cd lisplib;\
	 make DESTROOT=${DESTROOT} \
	      Liszt=$${TOP}/${LisztD}/nliszt all)
	(TOP=`pwd`;\
	 cd lisplib;\
	 make DESTROOT=${DESTROOT} \
	      Liszt=$${TOP}/${LisztD}/nliszt struct-again install)
	date

all:
	(cd utils     ; make ${DIRS} all)

install: all
	-if [ ! -d ${DESTROOT}${LIBDIR} ]; then \
		mkdir  ${DESTROOT}${LIBDIR};\
		/etc/chown root  ${DESTROOT}${LIBDIR};\
		chgrp system  ${DESTROOT}${LIBDIR};\
	else true ;\
	fi
	cd utils;   make DESTROOT=${DESTROOT} ${DIRS} install
	cd lisplib; make DESTROOT=${DESTROOT} ${DIRS} install
#
# Lisp and Liszt have to be remade now using the things installed in
# LIBDIR.
#
	(TOP=`pwd`;\
	 cd ${FranzD};\
	 make Liszt=$${TOP}/${LisztD}/nliszt LIBDIR=${LIBDIR} donlisp;\
	 make DESTROOT=${DESTROOT} LIBDIR=${LIBDIR} install)
	(cd ${LisztD};\
	 make Lisp=lisp donliszt;\
	 make DESTROOT=${DESTROOT} LIBDIR=${LIBDIR} install)
	(cd liszt;\
	 make Liszt=liszt lxref;\
	 make DESTROOT=${DESTROOT} LIBDIR=${LIBDIR} install)
#
# 	The lisp source files are no longer needed in the library
#	directory, so remove them.  They don't go out on the kit.
#
# DISABLE this for now...
#
#	rm -f ${DESTROOT}/${LIBDIR}/*.l
#

clean:
	cd utils    ; make clean
	cd lisplib  ; make clean
	cd franz    ; make clean
	cd franz/vax; make clean
	cd liszt    ; make clean
	cd liszt/vax; make clean
	cd doc      ; make clean

clobber: clean
	cd utils    ; make clobber
	cd lisplib  ; make clobber
	cd franz    ; make clobber
	cd franz/vax; make clobber
	cd liszt    ; make clobber
	cd liszt/vax; make clobber
	cd doc      ; make clobber
	rm -f Notice ReadMe cvt.awk lispconf lispnews scriptcal Makefile

sccsinfo:
	sccs info
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${SUBDIR}; do \
		(cd $$i; sccs get Makefile; make ${MFLAGSS} sccsget); done 

