#	1.15	Makefile	86/09/11
#	Makefile for icon
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 01	21-Oct-85, Greg Tarsa
#	Added code to create necessary directories if they do not exist.
#	Compressed some existing directory creations into the new scheme.
#

DESTROOT=
BINDIR = ${DESTROOT}/usr/new

SUBS= man doc samples book cmp/bin cmp/h cmp/link cmp/rt \
	cmp/functions cmp/lib cmp/operators \
	int/bin int/h int/link int/rt int/functions \
	int/lib int/operators int/tran int/iconx

LIB = ${DESTROOT}/usr/new/lib/icon

DIRS=	/usr /usr/new /usr/new/lib /usr/new/man /usr/new/man/man1 \
	/usr/new/man/man5 /usr/new/lib/icon \
	/usr/new/lib/icon/iconc /usr/new/lib/icon/icont

#

all:
	@echo Make ../new/icon
	@(cd cmp/bin; make ${MFLAGS})
	@(cd int/bin; make ${MFLAGS})

install: all copy
	@echo Install ../new/icon
	@(cd man; make ${MFLAGS} DESTROOT=${DESTROOT} install)

copy:
#
# Create any necessary directories, if they do not already exist
#
	@-for i in ${DIRS}; do \
		(if [ ! -d ${DESTROOT}$$i ] ; then \
			mkdir ${DESTROOT}$$i; \
			chmod 755 ${DESTROOT}$$i; \
			/etc/chown root ${DESTROOT}$$i; \
			chgrp system ${DESTROOT}$$i;  \
		 else true;\
		 fi \
		) ; done
#	@-if [ ! -d ${DESTROOT}/usr/new/lib ]; \
#	then \
#		mkdir ${DESTROOT}/usr/new/lib; \
#	else \
#		true; \
#	fi
#	@-if [ ! -d ${LIB} ]; \
#	then \
#		mkdir ${LIB}; \
#	else \
#		true; \
#	fi
#	@-if [ ! -d ${LIB}/iconc ]; \
#	then \
#		mkdir ${LIB}/iconc; \
#	else \
#		true; \
#	fi
#	@-if [ ! -d ${LIB}/icont ]; \
#	then \
#		mkdir ${LIB}/icont; \
#	else \
#		true; \
#	fi
#	cd cmp/bin; cp pstart.o pstop.o start.o ulink utran libi.a ${LIB}/iconc
#	ranlib ${LIB}/iconc/libi.a
	@(cd cmp/bin; cp ulink utran libi.a ${LIB}/iconc)
	@(cd int/bin; cp ulink utran iconx.hdr ${LIB}/icont)
	@(cd cmp/bin; install -c -s iconc ${BINDIR})
	@(cd int/bin; install -c -s icont ${BINDIR}; install -c -s iconx ${BINDIR})
	@(cd doc; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	@for i in ${SUBS}; do (cd $$i; make ${MFLAGS} clean); done

clobber: clean
	@for i in ${SUBS}; do (cd $$i; make ${MFLAGS} clobber); done
	@rm -f Makefile README config

sccsget: 
	@for i in ${SUBS}; do (cd $$i; sccs get -s Makefile;make ${MFLAGS} sccsget); done
	@sccs get SCCS
	@chmod 555 config

sccsinfo: 
	@echo -n ../new/icon:
	@sccs info
	@for i in ${SUBS}; do (cd $$i; echo -n ./$$i:; make ${MFLAGS} sccsinfo); done

