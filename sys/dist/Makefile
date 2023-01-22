#
# Makefile for /sys/dist and assorted other goodies

#   Author:	unknown
#   Date:	unknown
#  @(#)Makefile	1.59	ULTRIX	1/29/87
#
#Modification History
#~~~~~~~~~~~~~~~~~~~~
# 14	14-Jan-1986	robin
#	added license for use by /etc/rc.
#
# 13	04-Dec-1986	robin
#	added upgrade rules for user upgrade data files.
#
# 12	26-nov-1986	ccb
#	add install for newinv and genset, remove install
#		for genrx.
#
#	09-Oct-86 Tom Tresvik
#	fix install rule for finder
#
# 10	02-Oct-86 Tom Tresvik
#	resolve modes on install rules for etc and sys/dist
#	split ETCFILES into ETCFILES_754 and ETCFILES_644
#
# 09	02-Oct-86 Fred L. Templin
#	added the rpc file to the ETCFILES list
#
# 08    20-Aug-86 Tungning Cherng
#	added the ask_filesys ask_part delet_part size_part
#
# 07	11-Jun-86
#	Put install_product back in and deleted netsetup and uucpsetup.
#
# 06	17-Apr-86 ccb,rnf
#	begin cleanup of installation & release eng tools for 2.0
#
# 05	21-Jan-86  pete keilty
#	add elcsd.conf
#
# 04	23-Aug-85, Greg Tarsa
#	Fixed code to keep missing databases from blowing up the
#	build during the install phase.
#
# 03	1-Jul-85, Greg Tarsa
#	Added update files and changed code to put proper scripts into
#	/sys/dist as the result of a regular build.
#	Added netsetup and uucpsetup to etc directory
#	Added creation of subsets.database and upd.database to output
#	pack. Minor addition to cpack script.
#
# 02    21-May-85, Lu Anne Van de Pas
#	Added install_product to the files to be installed in /etc 
# 
# 01	18-Apr-85, Greg Tarsa
#	Added master database file building and comments
#

CFLAGS=	-O
SUBSCRIPTS= 

INSTSCRIPTS=.miniprofile .rootprofile ask_filesys ask_part delet_part \
	size_part gethost
BLDSCRIPTS= buildmini buildroot kits newinv fixroot
MEDIASCRIPTS= gentapes genra genset genboots genupgrade

SCRIPTS=addgroup adduser lockpw unlockpw removeuser

ETCFILES_754=rc rc.local doconfig sizer install_product license

ETCFILES_644=elcsd.conf disktab fstab.README fstab.hp fstab.hp400m fstab.ra60 \
	fstab.ra80 fstab.ra81 fstab.rb80 fstab.rc25 fstab.rk07 fstab.rm03 \
	fstab.rm05 fstab.rm80 fstab.rp06 fstab.rp07 fstab.up \
	fstab.up160m fstab.up300m ftpusers gettytab group hosts \
	hosts.equiv inetd.conf motd networks passwd printcap \
	protocols remote services ttys rpc

BLDPROGS= sizer delet_part size_part makeupgrade license
CFILES= sizer.o finder.o delet_part.o size_part.o makeupgrade.o license.o
SASTOOLS= fsmrg rxintlv compress make.sas finder
UPGRADE=0 2 8 16 32 64

DATAFILES=ULTRIX.mi ULTRIX.k EXTRA.k CRYPT.k kitcap

all: ${CFILES} ${BLDPROGS} ${SASTOOLS} ${INSTSCRIPTS}

sccsget:
	sccs get SCCS
	(cd setld;sccs get Makefile;make sccsget)
	(cd tools;sccs get Makefile;make sccsget)

sccsinfo:
	sccs info

install: all
# create directories

	-[ -d $(DESTROOT)/dev ] || \
		mkdir $(DESTROOT)/dev

	-[ -d $(DESTROOT)/usr/sys ] || \
		mkdir $(DESTROOT)/usr/sys

	-[ -d $(DESTROOT)/usr/sys/dist ] || \
		mkdir ${DESTROOT}/usr/sys/dist

	-[ -d $(DESTROOT)/usr/sys/dist/upgrade ] || \
		mkdir ${DESTROOT}/usr/sys/dist/upgrade

	-[ -d $(DESTROOT)/usr/hosts ] || \
		mkdir $(DESTROOT)/usr/hosts

	-[ -d $(DESTROOT)/usr/adm ] || \
		mkdir $(DESTROOT)/usr/adm

	-[ -d $(DESTROOT)/usr/adm/bin ] || \
		mkdir $(DESTROOT)/usr/adm/bin

#Create and install the user limit upgrade data files
	for i in ${UPGRADE}; \
	do \
		makeupgrade $$i; \
		install -m 400 -c upgrade$$i ${DESTROOT}/usr/sys/dist/upgrade; \
	done

# link upgrade0 to upgrade.nolimit because upgrade0 turns off login limit checks
	-( cd ${DESTROOT}/usr/sys/dist/upgrade; ln upgrade0 upgrade.nolimit)
# install the capacity upgrade utility
	install -m 700 -c install_upgrade ${DESTROOT}/etc
# install MAKEHOSTS
	install -m 754 -c MAKEHOSTS ${DESTROOT}/usr/hosts/MAKEHOSTS

# install all /etc files into /etc
	for i in ${ETCFILES_754}; \
	do \
	    install -m 754 -c $$i ${DESTROOT}/etc/$$i; \
	done

	for i in ${ETCFILES_644}; \
	do \
	    install -m 644 -c $$i ${DESTROOT}/etc/$$i; \
	done

# make fstab.ra60 the default fstab
	install -c fstab.ra60 ${DESTROOT}/etc/fstab

# install all shell scripts
	for i in $(SCRIPTS) ;\
	do \
	    install -m 754 -c $$i.sh ${DESTROOT}/etc/$$i;\
	done

# install MAKEDEV
	install -c -m 755 MAKEDEV ${DESTROOT}/dev

# create empty MAKEDEV.local and install in /dev
# ULGY
	> ${DESTROOT}/dev/MAKEDEV.local
	chmod 644 ${DESTROOT}/dev/MAKEDEV.local
	chgrp system ${DESTROOT}/dev/MAKEDEV.local
	/etc/chown root ${DESTROOT}/dev/MAKEDEV.local

	for file in $(SASTOOLS) $(INSTSCRIPTS) $(BLDSCRIPTS) $(MEDIASCRIPTS); \
	do \
		install -c -m 755 $$file $(DESTROOT)/usr/sys/dist/$$file; \
	done

	for dir in setld tools; \
	do \
		(cd $$dir;make install DESTROOT=$(DESTROOT)); \
	done

# Copy the databases to the proper area
# Dashes keep the installs from killing the build if the databases
# are not created.
	for file in $(DATAFILES); \
	do \
		install -c -m 644 $$file $(DESTROOT)/usr/sys/dist/$$file; \
	done
	
#
# GETHOST FOR NETBOOT
#

gethost: gethost.c
	cc ${CFLAGS} -o gethost gethost.c
#
# SAS TOOLS
#
rxintlv: rxintlv.c
	cc ${CFLAGS} -o rxintlv rxintlv.c

fsmrg: fsmrg.c
	cc ${CFLAGS} -o fsmrg fsmrg.c

compress: compress.c
	cc -O -DBSD4_2 -DSACREDMEM=0 -DUSERMEM=8192 -DBITS=16\
		-o compress compress.c
#
# end of SAS TOOLS
#

rldist: rlcfiles
	rm -fr rlkit
	-if [ ! -d rlkit ]; \
	then \
		mkdir rlkit; \
	else \
		true; \
	fi
	cp ifempty.sh rldist.sh createmd rlkit
	(cd rlkit ; sh rldist.sh /Ultrix )

rlcfiles: createmd

createmd: createmd.o
	cc -o createmd createmd.o

sizer:	sizer.o
	cc -o sizer sizer.o

finder: finder.o
	cc -o finder finder.o

delet_part: delet_part.o
	cc -o delet_part delet_part.o

size_part: size_part.o
	cc -o size_part size_part.o

makeupgrade: makeupgrade.o
	cc -o makeupgrade makeupgrade.o

license: license.o
	cc -o license license.o
