# "@(#)Makefile	1.23	(ULTRIX)	10/10/86";
#
#	/* VAX: 	LDFLAGS=-s, IFLAGS=-s */
#	/* 11/70:	LDFLAGS=-s -n, IFLAGS=-s -i */
#	/* 11/45:	LDFLAGS=-s -n, IFLAGS=-s -i */
#	/* 11/34:	LDFLAGS=-s -n, IFLAGS=-s */
#	/* 11/23:	LDFLAGS=-s -n, IFLAGS=-s */
#	/* Concept 32:	LDFLAGS=-s -n, IFLAGS=-s -n */
LDFLAGS=
IFLAGS=
#
OWNER=uucp
GROUP=daemon
#
# This version uses 4.2bsd directory reading routines.
# Set the Make variable below to reference the "new directory" routines.
#
LIBNDIR=
TIPDIR=../../usr.bin/tip
ACULIB=$(TIPDIR)/aculib/aculib.a
#
########################################################################
# Wed Dec 30 21:24:31 EST 1981
# ittvax!swatt: many changes to make this easier to manage
#
# Common object files assembled into a library (uulib.a)
#
# Targets defined are:
#	all(default)	Make all UUCP commands
#	install 	Install all executables
#	save		save old executables and install new version
#	restore 	reinstall old executables
#	new		Install executables and make all necessary
#			directories
#	ndir		make and install directory sys call library
#			and include file
#	cp		Make and install executables
#	cpdirs:
#	cpfiles:
#	cpuucp		Install uucp program
#	cpuux		Install uux program
#	cpuuxqt 	Install uuxqt program
#	cpuucico	Install uucico program
#	cpuulog 	Install uulog program
#	cpuuclean	Install uuclean program
#	cpuuname	Install uuname program
#	cpuustat	Install uustat program
#	cpuusub 	Install uusub program
#	cpuupoll	Install uupoll program
#	cpuumon 	Install uumonitor program
#	cpuumkspool	Install uumkspool program
#	cpuucomp	Install uucomp program
#	cpuurespool		Install uurespool program
#	cpuucpsetup	Install uucpsetup
#	cpgetmodems	Install getmodems
#	cpaddoutsys	Install addoutsys
#	cpaddinsys	Install addinsys
#	cpmkpass	Install mkpass
#	clean		Remove all .o files
#	clobber 	Remove all files and executables
#	get:
#	cmp:		all
#	uucp		Make uucp program
#	uux		Make uux program
#	uuxqt		Make uuxqt program
#	uucico		Make uucico program
#	uulog		Make uulog program
#	uuclean 	Make uuclean program
#	uuname		Make uuname program
#	uustat		Make uustat program
#	uusub		Make uusub program
#	uupoll		Make uupoll program
#	uucomp		Make uucomp program
#	uurespool		Make uurespool program
#	uumon		Make uumonitor program
#	uumkspool	Make uumkspool program
#	mkpass		Make mkpass program
#	mkdirs		Create necessary directories
#	lint		Run lint on everything
#	lintuucp	Run lint on uucp sources
#	lintuucico	Run lint on uucico sources
#	lintuux 	Run lint on uux sources
#	lintuuxqt	Run lint on uuxqt sources
#	lintuulog	Run lint on uulog sources
#	lintuuclean	Run lint on uuclean sources
#	lintuuname	Run lint on uuname sources
#	lintuustat	Run lint on uustat sources
#	lintuusub	Run lint on uusub sources
#	lintuupoll	Run lint on uupoll sources
#	tar		Make exportable 'tar' archive of all
#			necessary sources
#	text		Copy ascii files of DISTRIB to
#			distribution file
########################################################################
CC=cc
#-DNEWLDISC
# CONDEVS contains devices used by uucico
CONDEVS=-DDF0 -DHAYES -DHAYSTONE -DVENTEL -DHAYESQ -DVADIC -DDF112 -DGENERIC
CFLAGS=-O -DULTRIX -DUUSTAT -DGETHOST $(CONDEVS) -DFLOCK -DONDELAY -DNEWLDISC
# The minimum subdirectory set. The order of the D. files is crucial!
SUBDIRS=C. C./OTHERS STST.  D.`uuname -l`X D.`uuname -l` D. X. TM.

#
# Files to export with tar
SOURCES=*.c
HEADRS= *.h pk.p
METOO=	[mM]akefile *INSTALL* *CHANGE* *README* *TODO*
TFILES= $(METOO) $(HEADRS) $(SOURCES)

DESTROOT=
FRC=
INSDIR=${DESTROOT}/usr/lib/uucp
BIN=${DESTROOT}/usr/bin
UPATH=.
PATH=
SPOOLBASE=${DESTROOT}/usr/spool
PUBDIR=${DESTROOT}/usr/spool/uucppublic
SPOOL=${DESTROOT}/usr/spool/uucp
XQTDIR=${DESTROOT}/usr/spool/uucp/.XQTDIR
OLDLOG=${DESTROOT}/usr/lib/uucp/.OLD
DISTRIB= L-devices L-dialcodes L.cmds L.sys LIST.DAY \
	LIST.HOUR LIST.LONGHALL LIST.NOON LIST.NIGHT L_stat Makefile \
	README USERFILE uucp.day uucp.hour uucp.longhall \
	uucp.night uucp.noon uucp.week

PKON=pkon.o
IOCTL=
CLEAN=
LINTOP=-hbau
COMMANDS=uucp uux uuxqt uucico uulog uuclean uuname uustat uusub uupoll uumon uucomp uumkspool uurespool uuencode uudecode uusend mkpass
COMMLIB=uulib.a
# Common object files
COMMON =anlwrk.o anyread.o assert.o cfgets.o chkpth.o \
	cpmv.o expfile.o gename.o getargs.o getopt.o getprm.o \
	getpw.o getpwinfo.o gio.o fio.o gnamef.o gnxseq.o gwd.o imsg.o \
	index.o lastpart.o logent.o mailst.o pk0.o pk1.o prefix.o \
	sdmail.o setline.o shio.o subdir.o sysacct.o systat.o ub_sst.o ulockf.o \
	us_crs.o us_open.o us_rrs.o us_sst.o uucpname.o \
	versys.o xqt.o uucpdefs.o
#
# Should steal "make depend" from kernel makefile
#
# Object files which depend on "pk.h"
#
PKDEP=	gio.o fio.o pk0.o pk1.o
#
# Object files which depend on "uucp.h"
#
UUCPDEP=anlwrk.o anyread.o assert.o chkpth.o cico.o cntrl.o conn.o \
	condevs.o condefs.o prefix.o \
	cpmv.o dialout.o expfile.o gename.o getpwinfo.o gio.o fio.o gnamef.o \
	gnsys.o gnxseq.o gwd.o imsg.o ioctl.o logent.o mailst.o \
	sdmail.o setline.o shio.o subdir.o systat.o ub_sst.o ulockf.o us_crs.o \
	us_open.o us_rrs.o us_sst.o uuclean.o uucp.o uucpdefs.o \
	uucpname.o uulog.o uuname.o uustat.o uusub.o uupoll.o uux.o uuxqt.o \
	versys.o xqt.o
#
# Object files which depend on "uust.h"
#
UUSTDEP=anlwrk.o cico.o cntrl.o us_crs.o us_open.o us_rrs.o us_sst.o \
	uucp.o uustat.o
#
# Ojbect files which depend on "uusub.h"
UUSUBDEP=cico.o ub_sst.o uusub.o

#
LFILES=assert.c cpmv.c expfile.c gename.c getpwinfo.c index.c lastpart.c \
	prefix.c shio.c ulockf.c xqt.c
OUUCP=uucp.o $(COMMLIB)
LUUCP=uucpdefs.c uucp.c gwd.c chkpth.c getargs.c logent.c uucpname.c\
	versys.c us_crs.c us_open.c
OUUX=uux.o $(COMMLIB)
LUUX=uucpdefs.c uux.c gwd.c anyread.c chkpth.c getargs.c getprm.c\
	logent.c uucpname.c versys.c
OUUXQT=uuxqt.o gnsys.o $(COMMLIB)
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c gnamef.c logent.c uucpname.c \
	chkpth.c getargs.c anyread.c gnsys.c
OUUCICO=cico.o cntrl.o conn.o condevs.o condefs.o dialout.o gnsys.o $(COMMLIB)
LUUCICO=uucpdefs.c cico.c cntrl.c conn.c dialout.c pk0.c pk1.c gio.c fio.c anyread.c \
	condevs.c condefs.o \
	anlwrk.c chkpth.c getargs.c gnamef.c gnsys.c gnxseq.c \
	imsg.c logent.c sysacct.c systat.c \
	mailst.c uucpname.c us_rrs.c us_sst.c us_open.c ub_sst.c setline.c
OUULOG=uulog.o $(COMMLIB)
LUULOG=uucpdefs.c uulog.c prefix.c xqt.c ulockf.c gnamef.c assert.c
OUUCLEAN=uuclean.o $(COMMLIB)
LUUCLEAN=uucpdefs.c uuclean.c gnamef.c prefix.c mailst.c getpwinfo.c\
	 getargs.c
OUUNAME=uuname.o $(COMMLIB)
LUUNAME=uuname.c uucpname.c uucpdefs.c
OUUSTAT=uustat.o $(COMMLIB)
LUUSTAT=uucpdefs.c uustat.c gnamef.c getpwinfo.c us_open.c getopt.c \
	lastpart.c cpmv.c
OUUSUB=uusub.o $(COMMLIB)
LUUSUB=uucpdefs.c uusub.c getpwinfo.c us_open.c xqt.c getopt.c
OUUPOLL=uupoll.o $(COMMLIB)
LUUSUB=uucpdefs.c uupoll.c systat.c xqt.c
OUUMON=uumon.o
LUUMON=uumon.c
OUUCOMP=uucompact.o $(COMMLIB)
LUUCOMP=uucompact.c uucpname.c
OUUMKSPOOL=mkspool.o $(COMMLIB)
LUUMKSPOOL=mkspool.c
OUURESPOOL=uurespool.o $(COMMLIB)
LUUMKSPOOL=uurespool.c

all:	$(COMMANDS)

$(COMMANDS):	$(COMMLIB)

$(COMMLIB):	$(COMMON)
	rm -f $@
	ar cr $@ $(COMMON)
	ranlib $@
$(ACULIB):
	(cd $(TIPDIR); make -k aculib/aculib.a)


install:	mkdirs all cp text $(CLEAN)

new:		mkdirs cpfiles

cp:	all cpdirs cpuucp cpuux cpuuxqt cpuucico cpuulog cpuuclean cpuuname \
	cpuustat cpuusub cpuupoll cpuumon cpuucomp cpuumkspool cpuurespool \
	cpuuencode cpuudecode cpuusend cpruusend cpuucpsetup cpaddmodems \
	cpaddoutsys cpaddinsys cpmkpass

cpdirs:
	/etc/chown $(OWNER) $(INSDIR)
	chgrp $(GROUP) $(INSDIR)
	chmod 755 $(INSDIR)

save:	all
	/etc/chown $(OWNER) $(INSDIR)
	chmod 755 $(INSDIR)
	-mv $(BIN)/uucp $(BIN)/OLDuucp
	cp uucp $(BIN)
	/etc/chown $(OWNER) $(BIN)/uucp
	chmod 4111 $(BIN)/uucp
	-mv $(BIN)/uux $(BIN)/OLDuux
	cp uux $(BIN)
	/etc/chown $(OWNER) $(BIN)/uux
	chmod 4111 $(BIN)/uux
	-mv $(BIN)/uupoll $(BIN)/OLDuupoll
	cp uupoll $(BIN)/uupoll
	/etc/chown $(OWNER) $(BIN)/uupoll
	chmod 4111 $(BIN)/uupoll
	-mv $(INSDIR)/uuxqt $(INSDIR)/OLDuuxqt
	cp uuxqt $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uuxqt
	chmod 4111 $(INSDIR)/uuxqt
	-mv $(INSDIR)/uucico $(INSDIR)/OLDuucico
	cp uucico $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uucico
	chmod 4111 $(INSDIR)/uucico
	-mv $(BIN)/uudecode $(BIN)/OLDuudecode
	cp uudecode $(BIN)
	/etc/chown $(OWNER) $(BIN)/uudecode
	chmod 111 $(BIN)/uudecode
	-mv $(BIN)/uuencode $(BIN)/OLDuuencode
	cp uuencode $(BIN)
	/etc/chown $(OWNER) $(BIN)/uuencode
	chmod 111 $(BIN)/uuencode
	-mv $(BIN)/uusend $(BIN)/OLDuusend
	cp uusend $(BIN)
	/etc/chown $(OWNER) $(BIN)/OLDuusend
	chmod 4111 $(BIN)/uusend
	rm -f ${BIN}/ruusend
	ln ${BIN}/uusend ${BIN}/ruusend
	-mv $(BIN)/uulog $(BIN)/OLDuulog
	cp uulog $(BIN)
	/etc/chown $(OWNER) $(BIN)/uulog
	chmod 4111 $(BIN)/uulog
	-mv $(INSDIR)/uuclean $(INSDIR)/OLDuuclean
	cp uuclean $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uuclean
	chmod 4111 $(INSDIR)/uuclean
	-mv $(BIN)/uuname $(BIN)/OLDuuname
	cp uuname $(BIN)
	/etc/chown $(OWNER) $(BIN)/uuname
	chmod 4111 $(BIN)/uuname
	-mv $(BIN)/uustat $(BIN)/OLDuustat
	cp uustat $(BIN)
	/etc/chown $(OWNER) $(BIN)/uustat
	chmod 4111 $(BIN)/uustat
	-mv $(INSDIR)/uusub $(INSDIR)/OLDuusub
	cp uusub $(INSDIR)
	chmod 100 $(INSDIR)/uusub
	/etc/chown $(OWNER) $(INSDIR)/uusub
	-mv $(INSDIR)/uumonitor $(INSDIR)/OLDuumonitor
	cp uumonitor $(INSDIR)
	chmod 04111 $(INSDIR)/uumonitor
	/etc/chown $(OWNER) $(INSDIR)/uumonitor
	-mv $(INSDIR)/uumkspool $(INSDIR)/OLDuumkspool
	cp uumkspool $(INSDIR)
	chmod 04111 $(INSDIR)/uumkspool
	/etc/chown $(OWNER) $(INSDIR)/uumkspool
	-mv $(INSDIR)/uurespool $(INSDIR)/OLDuurespool
	cp uurespool $(INSDIR)
	chmod 04111 $(INSDIR)/uurespool
	/etc/chown $(OWNER) $(INSDIR)/uurespool
	-mv $(INSDIR)/uucompact $(INSDIR)/OLDuucompact
	cp uucompact $(INSDIR)
	chmod 04111 $(INSDIR)/uucompact
	/etc/chown $(OWNER) $(INSDIR)/uucompact

restore:
	-chmod u+w $(BIN)/uucp
	-mv $(BIN)/OLDuucp $(BIN)/uucp
	/etc/chown $(OWNER) $(BIN)/uucp
	chmod 4111 $(BIN)/uucp
	-chmod u+w $(BIN)/uux
	-mv $(BIN)/OLDuux $(BIN)/uux
	/etc/chown $(OWNER) $(BIN)/uux
	chmod 4111 $(BIN)/uux
	-chmod u+w $(INSDIR)/uuxqt
	-mv $(INSDIR)/OLDuuxqt $(INSDIR)/uuxqt
	/etc/chown $(OWNER) $(INSDIR)/uuxqt
	chmod 4111 $(INSDIR)/uuxqt
	-chmod u+w $(INSDIR)/uucico
	-mv $(INSDIR)/OLDuucico $(INSDIR)/uucico
	/etc/chown $(OWNER) $(INSDIR)/uucico
	chmod 4111 $(INSDIR)/uucico
	-chmod u+w $(BIN)/uulog
	-mv $(BIN)/OLDuulog $(BIN)/uulog
	/etc/chown $(OWNER) $(BIN)/uulog
	chmod 4111 $(BIN)/uulog
	-chmod u+w $(INSDIR)/uuclean
	-mv $(INSDIR)/OLDuuclean $(INSDIR)/uuclean
	/etc/chown $(OWNER) $(INSDIR)/uuclean
	chmod 4111 $(INSDIR)/uuclean
	-chmod u+w $(BIN)/uupoll
	-mv $(BIN)/OLDuupoll $(BIN)/uupoll
	chown $(OWNER) $(BIN)/uupoll
	chmod 4111 $(BIN)/uupoll
	-chmod u+w $(BIN)/uudecode
	-mv $(BIN)/OLDuudecode $(BIN)/uudecode
	chown $(OWNER) $(BIN)/uudecode
	chmod 111 $(BIN)/uudecode
	-chmod u+w $(BIN)/uuencode
	-mv $(BIN)/OLDuuencode $(BIN)/uuencode
	chown $(OWNER) $(BIN)/uuencode
	chmod 111 $(BIN)/uuencode
	-chmod u+w $(BIN)/uusend
	-mv $(BIN)/OLDuusend $(BIN)/uusend
	chown $(OWNER) $(BIN)/uusend
	chmod 4111 $(BIN)/uusend
	rm -f $(BIN)/ruusend
	ln $(BIN)/uusend $(BIN)/ruusend
	-chmod u+w $(BIN)/uuname
	-mv $(BIN)/OLDuuname $(BIN)/uuname
	/etc/chown $(OWNER) $(BIN)/uuname
	chmod 4111 $(BIN)/uuname
	-chmod u+w $(BIN)/uustat
	-mv $(BIN)/OLDuustat $(BIN)/uustat
	/etc/chown $(OWNER) $(BIN)/uustat
	chmod 4111 $(BIN)/uustat
	-chmod u+w $(INSDIR)/uusub
	-mv $(INSDIR)/OLDuusub $(INSDIR)/uusub
	chmod 100 $(INSDIR)/uusub
	/etc/chown $(OWNER) $(INSDIR)/uusub
	-chmod u+w $(INSDIR)/uumonitor
	-mv $(INSDIR)/OLDuumonitor $(INSDIR)/uumonitor
	chmod 04111 $(INSDIR)/uumonitor
	/etc/chown $(OWNER) $(INSDIR)/uumonitor
	-chmod u+w $(INSDIR)/uumkspool
	-mv $(INSDIR)/OLDuumkspool $(INSDIR)/uumkspool
	chmod 04111 $(INSDIR)/uumkspool
	/etc/chown $(OWNER) $(INSDIR)/uumkspool
	-chmod u+w $(INSDIR)/uurespool
	-mv $(INSDIR)/OLDuurespool $(INSDIR)/uurespool
	chmod 04111 $(INSDIR)/uurespool
	/etc/chown $(OWNER) $(INSDIR)/uurespool
	-chmod u+w $(INSDIR)/uucompact
	-mv $(INSDIR)/OLDuucompact $(INSDIR)/uucompact
	chmod 04111 $(INSDIR)/uucompact
	/etc/chown $(OWNER) $(INSDIR)/uucompact


cpuucp:
	cp uucp $(BIN)
	/etc/chown $(OWNER) $(BIN)/uucp
	chgrp $(GROUP) $(BIN)/uucp
	chmod 6111 $(BIN)/uucp

cpuux:
	cp uux $(BIN)
	/etc/chown $(OWNER) $(BIN)/uux
	chgrp $(GROUP) $(BIN)/uux
	chmod 6111 $(BIN)/uux

cpuuxqt:
	cp uuxqt $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uuxqt
	chgrp $(GROUP) $(INSDIR)/uuxqt
	chmod 6111 $(INSDIR)/uuxqt

cpuucico:
	cp uucico $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uucico
	chgrp $(GROUP) $(INSDIR)/uucico
	chmod 6111 $(INSDIR)/uucico

cpuulog:
	cp uulog $(BIN)
	/etc/chown $(OWNER) $(BIN)/uulog
	chgrp $(GROUP) $(BIN)/uulog
	chmod 6111 $(BIN)/uulog

cpuuclean:
	cp uuclean $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uuclean
	chgrp $(GROUP) $(INSDIR)/uuclean
	chmod 6110 $(INSDIR)/uuclean

cpuuname:
	cp uuname $(BIN)
	/etc/chown $(OWNER) $(BIN)/uuname
	chgrp $(GROUP) $(BIN)/uuname
	chmod 6111 $(BIN)/uuname

cpuustat:
	cp uustat $(BIN)
	/etc/chown $(OWNER) $(BIN)/uustat
	chgrp $(GROUP) $(BIN)/uustat
	chmod 6111 $(BIN)/uustat

cpuusub:
	cp uusub $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uusub
	chgrp $(GROUP) $(INSDIR)/uusub
	chmod 100 $(INSDIR)/uusub

cpuumon:
	cp uumonitor $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uumonitor
	chgrp $(GROUP) $(INSDIR)/uumonitor
	chmod 04111 $(INSDIR)/uumonitor

cpuumkspool:
	cp uumkspool $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uumkspool
	chgrp $(GROUP) $(INSDIR)/uumkspool
	chmod 04111 $(INSDIR)/uumkspool

cpuucomp:
	cp uucompact $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uucompact
	chgrp $(GROUP) $(INSDIR)/uucompact
	chmod 04111 $(INSDIR)/uucompact

cpuurespool:
	cp uurespool $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uurespool
	chgrp $(GROUP) $(INSDIR)/uurespool
	chmod 04100 $(INSDIR)/uurespool

cpuupoll:
	cp uupoll $(BIN)
	/etc/chown $(OWNER) $(BIN)/uupoll
	chgrp $(GROUP) $(BIN)/uupoll
	chmod 6111 $(BIN)/uupoll
	cp uupoll $(INSDIR)
	/etc/chown $(OWNER) $(INSDIR)/uupoll
	chgrp $(GROUP) $(INSDIR)/uupoll
	chmod 6111 $(INSDIR)/uupoll

cpuusend:
	cp uusend $(BIN)
	/etc/chown $(OWNER) $(BIN)/uusend
	chgrp $(GROUP) $(BIN)/uusend
	chmod 6111 $(BIN)/uusend

cpruusend:
	rm -f ${BIN}/ruusend
	ln $(BIN)/uusend $(BIN)/ruusend

cpuudecode:
	cp uudecode $(BIN)
	/etc/chown $(OWNER) $(BIN)/uudecode
	chgrp $(GROUP) $(BIN)/uudecode
	chmod 111 $(BIN)/uudecode

cpuuencode:
	cp uuencode $(BIN)/uuencode
	/etc/chown $(OWNER) $(BIN)/uuencode
	chgrp $(GROUP) $(BIN)/uuencode
	chmod 111 $(BIN)/uuencode

cpuucpsetup:
	cp uucpsetup.sh ${DESTROOT}/etc/uucpsetup
	/etc/chown $(OWNER) ${DESTROOT}/etc/uucpsetup
	chgrp $(GROUP) ${DESTROOT}/etc/uucpsetup
	chmod 555 ${DESTROOT}/etc/uucpsetup

cpaddmodems:
	cp addmodems.sh $(INSDIR)/addmodems
	/etc/chown $(OWNER) $(INSDIR)/addmodems
	chgrp $(GROUP) $(INSDIR)/addmodems
	chmod 555 $(INSDIR)/addmodems

cpaddoutsys:
	cp addoutsys.sh $(INSDIR)/addoutsys
	/etc/chown $(OWNER) $(INSDIR)/addoutsys
	chgrp $(GROUP) $(INSDIR)/addoutsys
	chmod 555 $(INSDIR)/addoutsys

cpaddinsys:
	cp addinsys.sh $(INSDIR)/addinsys
	/etc/chown $(OWNER) $(INSDIR)/addinsys
	chgrp $(GROUP) $(INSDIR)/addinsys
	chmod 555 $(INSDIR)/addinsys

cpmkpass:
	cp mkpass $(INSDIR)/mkpass
	/etc/chown $(OWNER) $(INSDIR)/mkpass
	chgrp $(GROUP) $(INSDIR)/mkpass
	chmod 500 $(INSDIR)/mkpass

clean:
	-rm -f *.o

sccsget:
		sccs get SCCS

sccsinfo:
		sccs info

clobber:	
	-rm -f `ls | egrep '^[^S]'`


get:
	cp $(UPATH)/* .

cmp:	all
	cmp uucp $(BIN)/uucp
	cmp uux $(BIN)/uux
	cmp uuxqt $(INSDIR)/uuxqt
	cmp uucico $(INSDIR)/uucico
	cmp uulog $(BIN)/uulog
	cmp uuclean $(INSDIR)/uuclean
	cmp uuname $(BIN)/uuname
	cmp uustat $(BIN)/uustat
	cmp uusub $(INSDIR)/uusub
	cmp uupoll $(BIN)/uupoll
	cmp uusend $(BIN)/uusend
	cmp uuencode $(BIN)/uuencode
	cmp uudecode $(BIN)/uudecode
	cmp uumonitor $(INSDIR)/uumonitor
	cmp uumkspool $(INSDIR)/uumkspool
	cmp uurespool $(INSDIR)/uurespool
	cmp uucompact $(INSDIR)/uucompact

ndir:	./ndir/make install

uucp:	$(OUUCP)
	$(CC) $(LDFLAGS) $(OUUCP) $(LIBNDIR) -o uucp

uux:	$(OUUX)
	$(CC) $(LDFLAGS) $(OUUX) $(LIBNDIR) -o uux

uuxqt:	$(OUUXQT)
	$(CC) $(LDFLAGS) $(OUUXQT) $(LIBNDIR) -o uuxqt

uucico: $(OUUCICO)  $(IOCTL) $(PKON) $(ACULIB)
	$(CC) $(IFLAGS) $(OUUCICO) $(IOCTL) $(PKON) $(LIBNDIR) $(ACULIB) -o uucico

uulog:	$(OUULOG)
	$(CC) $(LDFLAGS) $(OUULOG) $(LIBNDIR) -o uulog

uuclean:  $(OUUCLEAN)
	$(CC) $(LDFLAGS) $(OUUCLEAN) $(LIBNDIR) -o uuclean

uuname: $(OUUNAME)
	$(CC) $(LDFLAGS) $(OUUNAME) $(LIBNDIR) -o uuname

uustat: $(OUUSTAT)
	$(CC) $(LDFLAGS) $(OUUSTAT) $(LIBNDIR) -o uustat

mkpass: mkpass.o
	${CC} -o mkpass mkpass.o

uusub:	$(OUUSUB)
	$(CC) $(LDFLAGS) $(OUUSUB) $(LIBNDIR) -o uusub

uumon:	$(OUUMON)
	$(CC)  $(OUUMON)  $(LIBNDIR) -o uumonitor

uucomp: $(OUUCOMP)
	$(CC)  $(OUUCOMP)  $(LIBNDIR) -o uucompact

uurespool:	$(OUURESPOOL)
	$(CC)  $(OUURESPOOL)  $(LIBNDIR) -o uurespool

uumkspool: $(OUUMKSPOOL)
	$(CC)  $(OUUMKSPOOL)  $(LIBNDIR) -o uumkspool

uupoll: $(OUUPOLL)
	$(CC) $(LDFLAGS) $(OUUPOLL) -o uupoll

uuencode: uuencode.o uulib.a
	${CC} -o uuencode ${LDFLAGS} uuencode.o uulib.a

uudecode: uudecode.o uulib.a
	${CC} -o uudecode ${LDFLAGS} uudecode.o uulib.a

uusend: uusend.o uulib.a
	${CC} -o uusend ${LDFLAGS} uusend.o uulib.a

#
# Header file dependencies
#
$(PKDEP):	pk.h
$(UUCPDEP):	uucp.h
$(UUSTDEP):	uust.h
$(UUSUBDEP):	uusub.h

FRC:

mkdirs:
	@-if [ ! -d ${SPOOLBASE} ] ;\
	then \
		mkdir ${SPOOLBASE}; \
		/etc/chown root ${SPOOLBASE}; \
		chgrp system ${SPOOLBASE}; \
		chmod 0755 ${SPOOLBASE}; \
	else true; \
	fi
	@-if [ ! -d ${INSDIR} ] ;\
	then \
		mkdir ${INSDIR}; \
		/etc/chown root ${INSDIR}; \
		chgrp system ${INSDIR}; \
		chmod 0755 ${INSDIR}; \
	else true; \
	fi
	@-if [ ! -d ${SPOOL} ] ;\
	then \
		mkdir ${SPOOL}; \
		/etc/chown ${OWNER} ${SPOOL}; \
		chgrp ${GROUP} ${SPOOL}; \
		chmod 0777 ${SPOOL}; \
	else true; \
	fi
	@-if [ ! -d ${SPOOL}/sys ] ;\
	then \
		mkdir ${SPOOL}/sys; \
		/etc/chown ${OWNER} ${SPOOL}/sys; \
		chgrp ${GROUP} ${SPOOL}/sys; \
		chmod 0755 ${SPOOL}/sys; \
	else true; \
	fi
	-./uumkspool DEFAULT
	@-if [ ! -d ${SPOOL}/TM. ] ;\
	then \
		mkdir ${SPOOL}/TM.; \
		/etc/chown ${OWNER} ${SPOOL}/TM.; \
		chgrp ${GROUP} ${SPOOL}/TM.; \
		chmod 0755 ${SPOOL}/TM.; \
	else true; \
	fi
	@-if [ ! -d ${SPOOL}/STST. ] ;\
	then \
		mkdir ${SPOOL}/STST.; \
		/etc/chown ${OWNER} ${SPOOL}/STST.; \
		chgrp ${GROUP} ${SPOOL}/STST.; \
		chmod 0755 ${SPOOL}/STST.; \
	else true; \
	fi
	@-if [ ! -d ${PUBDIR} ] ;\
	then \
		mkdir ${PUBDIR}; \
		/etc/chown ${OWNER} ${PUBDIR}; \
		chgrp ${GROUP} ${PUBDIR}; \
		chmod 0777 ${PUBDIR}; \
	else true; \
	fi
	@-if [ ! -d ${XQTDIR} ] ;\
	then \
		mkdir ${XQTDIR}; \
		/etc/chown ${OWNER} ${XQTDIR}; \
		chgrp ${GROUP} ${XQTDIR}; \
		chmod 0755 ${XQTDIR}; \
	else true; \
	fi
#	-mkdir $(OLDLOG)
#	chmod 777 $(OLDLOG)
#	/etc/chown $(OWNER) $(OLDLOG)
#	chgrp $(GROUP) $(OLDLOG)

cpfiles:
	cp $(PATH)/L* $(PATH)/USERFILE ${DESTROOT}$(INSDIR)
#	cp $(PATH)/uudemon* ${DESTROOT}$(INSDIR)
#	chmod 755 ${DESTROOT}$(INSDIR)/uudemon*
	chmod 600 $(DESTROOT)$(INSDIR)/L* ${DESTROOT}$(INSDIR)/USERFILE
	-/etc/chown $(OWNER) ${DESTROOT}$(INSDIR)/*
	-chgrp $(GROUP) ${DESTROOT}$(INSDIR)/*

lint:	lintuucp lintuucico lintuux lintuuxqt lintuulog lintuuclean\
	lintuuname lintuustat lintuusub
lintuucp:
	lint $(LINTOP) $(LUUCP) $(LFILES)

lintuucico:
	lint $(LINTOP) $(LUUCICO) $(LFILES)

lintuux:
	lint $(LINTOP) $(LUUX) $(LFILES)

lintuuxqt:
	lint $(LINTOP) $(LUUXQT) $(LFILES)

lintuulog:
	lint $(LINTOP) $(LUULOG)

lintuuclean:
	lint $(LINTOP) $(LUUCLEAN)

lintuuname:
	lint $(LINTOP) $(LUUNAME)

lintuustat:
	lint $(LINTOP) $(LUUSTAT) $(LFILES)

lintuusub:
	lint $(LINTOP) $(LUUSUB) $(LFILES)

# Make exportable 'tar' archive package
#
tar:
	tar cbf 1 uucp.tar $(TFILES)

text:
	for i in ${DISTRIB}; do \
	install -c $$i ${INSDIR}/$$i; done
	install -c -m 644 -o ${OWNER} -g ${GROUP} /dev/null ${SPOOL}/ERRLOG
	install -c -m 644 -o ${OWNER} -g ${GROUP} /dev/null ${SPOOL}/LOGFILE
	install -c -m 644 -o ${OWNER} -g ${GROUP} /dev/null ${PUBDIR}/.hushlogin
	install -c -m 644 -o ${OWNER} -g ${GROUP} /dev/null ${INSDIR}/INSECURE
	install -c -m 644 -o ${OWNER} -g ${GROUP} /dev/null ${INSDIR}/R_stat
