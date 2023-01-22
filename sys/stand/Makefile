 #***********************************************************************
 #									*
 #			Copyright (c) 1985 by				*
 #		Digital Equipment Corporation, Maynard, MA		*
 #			All rights reserved.				*
 #									*
 #   This software is furnished under a license and may be used and	*
 #   copied  only  in accordance with the terms of such license and	*
 #   with the  inclusion  of  the  above  copyright  notice.   This	*
 #   software  or  any  other copies thereof may not be provided or	*
 #   otherwise made available to any other person.  No title to and	*
 #   ownership of the software is hereby transferred.			*
 #									*
 #   This software is  derived  from  software  received  from  the	*
 #   University    of   California,   Berkeley,   and   from   Bell	*
 #   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 #   restrictions  under  license  agreements  with  University  of	*
 #   California and with AT&T.						*
 #									*
 #   The information in this software is subject to change  without	*
 #   notice  and should not be construed as a commitment by Digital	*
 #   Equipment Corporation.						*
 #									*
 #   Digital assumes no responsibility for the use  or  reliability	*
 #   of its software on equipment which is not supplied by Digital.	*
 #									*
 #***********************************************************************

 #	@(#)Makefile	1.32 Ultrix 	1/9/87";

 # ------------------------------------------------------------------------
 # Modification History: /sys/stand/Makefile
 #
 # 8-Jan-87 -- tresvik
 #	moving boot code high enough to load a V2.0 kernel which
 #	now steps on the boot image at B0000.  It is now moved to
 #	180000.
 #
 # 13-Aug-85 -- rjl
 #	Generalized virtual console support for uVAXen to allow
 #	a qdss of a qvss as the system console.
 #
 # 12-Jul-85 --tresvik
 #	Kernels can now grow large enough to blow away the boot code.
 # 	Relocation is now changed from 70000 to B0000 (hex) to allow
 #	enough room for the kernel to load in.  This leaves 256k worth
 #	of space in front of hardware memsize location of f0000 used with
 #	MVAX.
 #	Made *srto.o objects depend on the Makefile.
 #
 # 18-Jun-85 -- Jaw
 #	add support for VAX8200 console floppy.
 #
 # 17-Jun-85 -- Jaw
 #	add support for VAX8200 console floppy.
 #
 # 01-May-85 --tresvik
 #	Support for VAX8600 and VAX8200 has grown the size of boothp
 #	beyond the limit of 7680 (15 blocks).  A special version of 
 #	autoconf.c is now built for 750 and used to build only boothp.
 #	A sizing mechanism is also added to show the builder of stand
 #	the size of all boot images.
 #
 # 29-mar-85 -- jaw
 #	add support for 8200
 #
 # 14-Mar-85 -tresvik
 # 	Installs to ../rl02 of boot copy rabads for VAX8600
 #
 #  7-Mar-84 -- darrell
 #	Changed how rqbads is made.  It needed to be compiled with
 #	MVAX defined.
 #
 # 27-Feb-85 -tresvik
 #	Changes for support of the VAX8600 were merged in.
 #
 # 11-Feb-85 -- afd
 #	Added tape copy image called tpcopy-boot.
 #	Standalone tape copy and boot for MicroVAX.
 #
 # 27-Dec-84 -- afd
 #	Added tmscp to the sources list and the drivers list.
 #	So tpcopy will support tmscp devices.
 #
 # 29-Nov-84  --Tresvik
 # 	Made a smaller sys.o called sys.small.o for use when building
 #      the bootxx images since all were over the 15 block limit. 
 #      sys.small.o is built by compilig sys.c with the SMALLSYS 
 #	option turned on.
 #
 # 13-Nov-84  --tresvik
 #	fixed 'clean' to clea all and added support for installation of
 #	rabads.
 #
 # 19 Jul 84 --Reilly
 #	Add new module called rccopy for image copying of rc25
 #
 #  7 Jul 84 -- rjl
 #	Added qvss console driver.
 #
 # 10 Jul 84 --tresvik
 #	Turned on the DS option to boot for the diagnostic Supervisor
 #	load support.
 #
 # 10 Jan 84 --tresvik
 #	Add pcs750 driver.
 #
 # 29 Dec 83 --jmcg
 #	Corrected dependencies on libsa.a from libsa.a.
 #
 # 28 Dec 83 --jmcg
 #	Added rules for rqboot--bootstrap for RQDX1 and MicroVAX 1
 #
 #  9 Dec 83 --jmcg
 #	Deleted special references to old 730 bug.  There is no longer a
 #	distinction between versions for 780, 750, and 730.  Distribution
 #	media, however, contain `boot_justask' as `boot' to avoid attempts
 #	to boot prematurely.
 #
 #  9 Dec 83 --jmcg
 #	Derived from 4.2BSD, labeled:
 #		Makefile	6.2	83/09/25
 #
 # ------------------------------------------------------------------------

DESTROOT=
DESTDIR=${DESTROOT}
CFLAGS=	-O -DSTANDALONE ${COPTS} 
MCFLAGS= -O -DSTANDALONE ${MCOPTS} 
COPTS=	-DVAX8600 -DVAX8200 -DVAX780 -DVAX750 -DVAX730 
MCOPTS= -DMVAX
RELOC=	180000

SRCS=	sys.c conf.c get.c prf.c machdep.c \
	autoconf.c hp.c hpmaptype.c ht.c idc.c mba.c mt.c rk.c \
	rl.c cs.c tm.c ts.c qvcons.c qdcons.c\
	up.c upmaptype.c uba.c uda.c ut.c \
	drtest.c format.c up.old.c hp.old.c pcs750.s \
	rabads.c udabads.c tmscp.c

DRIVERS=hp.o hpmaptype.o ht.o idc.o mba.o mt.o \
	rk.o rl.o cs.o tm.o ts.o \
	up.o upmaptype.o uba.o uda.o ut.o tmscp.o

# These drivers don't have ecc correction and bad sector forwarding;
# they are placed in the file system boot area for 750's.  If your
# root has bad sectors you can try and squeeze the newer drivers in...

ODRIVERS=hp.old.o up.old.o
BOOTS32 =boothp boothk bootup bootra bootrl bootcs 
BOOTS32M=bootrq 

ALL=	libsa.a srt0.o boot tpboot copy rccopy tpcopy \
	format tpformat drtest boot_justask rabads ${BOOTS32}
MALL=   rqbads rqcopy tpcopy-boot bootqv ${BOOTS32M}

all: ${ALL} ${MALL} size

libsa.a: sys.o conf.o autoconf.o ${DRIVERS} get.o prf.o machdep.o dkbad.o pcs750.o
	ar crv libsa.a $?
	ranlib libsa.a

libqsa.a: sys.o conf.o qvautoconf.o ${DRIVERS} qvcons.o qdcons.o \
	qvget.o qvprf.o qfont.o machdep.o dkbad.o pcs750.o
	ar crv libqsa.a $?
	ranlib libqsa.a

mlibsa.a: libsa.a mautoconf.o
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		true; \
	fi
	cd tmp; ar xov ../libsa.a
	cp mautoconf.o tmp/autoconf.o
	cd tmp; ar crv ../mlibsa.a *
	rm -rf tmp
	ranlib mlibsa.a

mautoconf.o: autoconf.c
	cp autoconf.c mautoconf.c
	cc ${MCFLAGS} -c mautoconf.c

${ODRIVERS} ${DRIVERS}: savax.h
	cc -c -S ${COPTS} $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s

pcs750.o: pcs750.s
	/lib/cpp pcs750.s | as -o pcs750.o

dkbad.o: ../vax/dkbad.c
	${CC} -c ${CFLAGS} ../vax/dkbad.c

boot.o: ../h/param.h ../h/inode.h ../h/fs.h ../h/vm.h ../vax/mtpr.h \
	../vax/cpu.h saio.h savax.h ../h/reboot.h ../machine/pte.h \
	../vax/nexus.h ../vaxuba/ubareg.h ../vaxmba/mbareg.h
	cc -O -DDS ${COPTS} -c boot.c

# startups

srt0.o: Makefile srt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E -DRELOC=0x${RELOC} ${COPTS} srt0.c | as -o srt0.o

tpsrt0.o: Makefile srt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E -DRELOC=0x${RELOC} -DTP ${COPTS} srt0.c | as -o tpsrt0.o 

uvtpsrt0.o: Makefile srt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E -DRELOC=0x${RELOC} -DTP -DUV1 -DREL ${COPTS} srt0.c | as -o uvtpsrt0.o


relsrt0.o: Makefile srt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E -DRELOC=0x${RELOC} -DREL ${COPTS} srt0.c | as -o relsrt0.o

uv1srt0.o: Makefile srt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E -DRELOC=0x${RELOC} -DREL -DUV1 ${COPTS} srt0.c | as -o uv1srt0.o

# qvss versions of some of the files for microvax-I tertiary boot

qfont.o: ../vaxuba/qvreg.h ../vaxuba/qfont.c
	cc -c -S ${MCOPTS}  ../vaxuba/qfont.c
	/lib/c2 -i qfont.s | as -o qfont.o
	rm -f qfont.s

qdcons.o: qdcons.c ../vaxuba/qdreg.h ../vaxuba/qfont.c
	cc -c -S ${MCOPTS} -DVCONS  $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s

qvcons.o: qvcons.c ../vaxuba/qvreg.h ../vaxuba/qfont.c
	cc -c -S ${MCOPTS} -DVCONS  $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s

qvget.o: get.c
	cp get.c qvget.c
	cc -c -S ${MCOPTS} -DVCONS  $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s 

qvprf.o: prf.c
	cp prf.c qvprf.c
	cc -c -S ${MCOPTS} -DVCONS  $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s 

qvautoconf.o: autoconf.c
	cp autoconf.c qvautoconf.c
	cc -c -S ${MCOPTS} -DVCONS  $*.c
	/lib/c2 -i $*.s | as -o $*.o
	rm -f $*.s 

# bootable from tape

tpboot:	tpboot.o relsrt0.o libsa.a
	ld -N -T ${RELOC} relsrt0.o tpboot.o libsa.a -lc
	cp a.out b.out; strip b.out; dd if=b.out of=tpboot ibs=32 skip=1; rm b.out

tpboot.o: boot.c ../h/param.h ../h/inode.h ../h/fs.h ../vax/mtpr.h ../vax/cpu.h
tpboot.o: saio.h ../h/reboot.h ../h/vm.h 
	cp boot.c tpboot.c; chmod +w tpboot.c
	cc -c -O -DJUSTASK tpboot.c
	rm -f tpboot.c

tpcopy-boot: tpcopy-boot.o uvtpsrt0.o libqsa.a
	ld -N -T ${RELOC} uvtpsrt0.o tpcopy-boot.o libqsa.a -lc
	cp a.out b.out; strip b.out; dd if=b.out of=tpcopy-boot ibs=32 skip=1; rm b.out

tpcopy-boot.o: ../h/param.h ../h/inode.h ../h/fs.h saio.h /usr/include/a.out.h

tpcopy:	copy.o tpsrt0.o libsa.a
	ld -N tpsrt0.o copy.o libsa.a -lc
	cp a.out b.out; strip b.out; \
		dd if=b.out of=tpcopy ibs=32 skip=1; rm b.out

tpformat: format.o tpsrt0.o confhpup.o libsa.a
	cp format.c tpformat.c; chmod +w tpformat.c
	cc -c -O -DJUSTEXIT tpformat.c
	rm -f tpformat.c
	ld -N tpsrt0.o tpformat.o confhpup.o libsa.a -lc
	cp a.out b.out; strip b.out; \
		dd if=b.out of=tpformat ibs=32 skip=1; rm b.out

# bootable from floppy or real disks

boot:	boot.o relsrt0.o bootconf.o libsa.a
	ld -N -T ${RELOC} -o boot relsrt0.o boot.o bootconf.o libsa.a -lc
	strip boot

bootqv:	boot.o relsrt0.o bootconf.o libqsa.a
	ld -N -T ${RELOC} -o bootqv relsrt0.o boot.o bootconf.o libqsa.a -lc
	strip bootqv

boot_justask:	boot_justask.o relsrt0.o bootconf.o libsa.a
	ld -N -T ${RELOC} -o boot_justask relsrt0.o boot_justask.o bootconf.o libsa.a -lc

boot_justask.o: boot.c saio.h ../vax/mtpr.h ../vax/cpu.h
boot_justask.o: ../h/param.h ../h/inode.h ../h/fs.h ../h/vm.h ../h/reboot.h
	cp boot.c boot_justask.c
	cc -c ${COPTS} -DJUSTASK boot_justask.c
	rm -f boot_justask.c

bootconf.o: conf.c ../h/param.h ../h/inode.h ../machine/pte.h
bootconf.o: ../h/fs.h saio.h ../vaxmba/mbareg.h
	cp conf.c bootconf.c
	cc -c ${COPTS} -DBOOT bootconf.c
	rm -f bootconf.c

copy:	copy.o srt0.o conf.o libsa.a
	ld -N -o copy srt0.o copy.o conf.o libsa.a -lc

rccopy:	rccopy.o srt0.o conf.o libsa.a
	ld -N -o rccopy srt0.o rccopy.o conf.o libsa.a  -lc

rqcopy:	rccopy.o srt0.o conf.o libqsa.a
	ld -N -o rqcopy srt0.o rccopy.o conf.o libqsa.a  -lc

format:	format.o srt0.o confhpup.o libsa.a
	ld -N -o format srt0.o format.o confhpup.o libsa.a -lc

drtest:	drtest.o srt0.o confhpup.o libsa.a
	ld -N -o drtest srt0.o drtest.o confhpup.o libsa.a -lc

# bootstrap from ether

### not yet, rosin, not yet ###

# getting booted from disc

sys.small.o: sys.c
	cp sys.c sys.small.c
	cc ${CFLAGS} -DSMALLSYS -c sys.small.c

autoconf750.o: sys.c
	cp autoconf.c autoconf750.c
	cc -O -DSTANDALONE -DVAX750 -c autoconf750.c

boothk: relsrt0.o boothk.o confrk.o sys.small.o libsa.a
	ld -N -T ${RELOC} relsrt0.o boothk.o confrk.o sys.small.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=boothk ibs=32 skip=1;rm b.out

boothp: relsrt0.o boothp.o confhp.o autoconf750.o sys.small.o hp.old.o libsa.a
	ld -N -T ${RELOC} relsrt0.o boothp.o confhp.o autoconf750.o \
		sys.small.o hp.old.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=boothp ibs=32 skip=1;rm b.out

bootup: relsrt0.o bootup.o confup.o up.old.o sys.small.o libsa.a
	ld -N -T ${RELOC} relsrt0.o bootup.o confup.o up.old.o sys.small.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=bootup ibs=32 skip=1;rm b.out

bootra: relsrt0.o bootra.o confra.o sys.small.o libsa.a
	ld -N -T ${RELOC} relsrt0.o bootra.o confra.o sys.small.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=bootra ibs=32 skip=1;rm b.out

bootrq: uv1srt0.o bootrq.o confra.o sys.small.o mlibsa.a
	ld -N -T ${RELOC} uv1srt0.o bootrq.o confra.o sys.small.o mlibsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=bootrq ibs=32 skip=1;rm b.out

bootrl: relsrt0.o bootrl.o confrl.o sys.small.o libsa.a
	ld -N -T ${RELOC} relsrt0.o bootrl.o confrl.o sys.small.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=bootrl ibs=32 skip=1;rm b.out

bootcs: relsrt0.o bootcs.o confcs.o sys.small.o libsa.a
	ld -N -T ${RELOC} relsrt0.o bootcs.o confcs.o sys.small.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=bootcs ibs=32 skip=1;rm b.out

bootrq.o:	bootrq.c
	cc ${MCFLAGS} -c bootrq.c

boothp.o: boothp.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
boothp.o: ../h/fs.h saio.h
boothk.o: boothk.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
boothk.o: ../h/fs.h saio.h
bootup.o: bootup.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
bootup.o: ../h/fs.h saio.h
bootra.o: bootra.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
bootra.o: ../h/fs.h saio.h
bootrq.o: bootrq.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
bootrq.o: ../h/fs.h saio.h
bootrl.o: bootrl.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
bootrl.o: ../h/fs.h saio.h
bootcs.o: bootcs.c ../h/param.h ../h/inode.h ../machine/pte.h ../h/reboot.h
bootcs.o: ../h/fs.h saio.h

boothk.c: bootxx.c
	sed -e 's/xx/hk/g' <bootxx.c >boothk.c
boothp.c: bootxx.c
	sed -e 's/xx/hp/g' <bootxx.c >boothp.c
bootup.c: bootxx.c
	sed -e 's/xx/up/g' <bootxx.c >bootup.c
bootra.c: bootxx.c
	sed -e 's/xx/ra/g' <bootxx.c >bootra.c
bootrq.c: bootxx.c
	sed -e 's/xx/ra/g' <bootxx.c >bootrq.c
bootrl.c: bootxx.c
	sed -e 's/xx/rl/g' <bootxx.c >bootrl.c
bootcs.c: bootxx.c
	sed -e 's/xx/cs/g' <bootxx.c >bootcs.c

confrk.o: confrk.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h
confhp.o: confhp.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h
confup.o: confup.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h
confra.o: confra.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h
confrl.o: confrl.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h
confcs.o: confcs.c ../h/param.h ../h/fs.h ../machine/pte.h ../h/inode.h saio.h

confrk.c: confxx.c
	sed -e 's/XX/hk/' -e 's/xx/rk/g' <confxx.c >confrk.c
confhp.c: confxx.c
	sed -e 's/XX/hp/' -e 's/xx/hp/g' <confxx.c >confhp.c
confup.c: confxx.c
	sed -e 's/XX/up/' -e 's/xx/up/g' <confxx.c >confup.c
confra.c: confxx.c
	sed -e 's/XX/ra/' -e 's/xx/ra/g' <confxx.c >confra.c
confrl.c: confxx.c
	sed -e 's/XX/rl/' -e 's/xx/rl/g' <confxx.c >confrl.c
confcs.c: confxx.c
	sed -e 's/XX/cs/' -e 's/xx/cs/g' <confxx.c >confcs.c

rabads: srt0.o udabads.o rabads.o libsa.a
	ld -N -o rabads srt0.o rabads.o udabads.o libsa.a -lc

rqbads: srt0.o udabads.o rqbads.o libqsa.a
	ld -N -o rqbads srt0.o rqbads.o udabads.o libqsa.a -lc

rabads.o: rabads.c sa_defs.h ../h/param.h ../h/inode.h ../h/fs.h \
	saio.h ra_saio.h
	cc -c -S ${COPTS} rabads.c
	/lib/c2 -i rabads.s | as -o rabads.o
	rm rabads.s

rqbads.o: rabads.c sa_defs.h ../h/param.h ../h/inode.h ../h/fs.h \
	saio.h ra_saio.h 
	cc -c -S ${MCOPTS} rabads.c
	/lib/c2 -i rabads.s | as -o rqbads.o
	rm rabads.s

udabads.o: udabads.c ../machine/pte.h ../h/param.h ../h/inode.h \
	../h/fs.h saio.h savax.h ../vaxuba/udareg.h ../vaxuba/ubareg.h \
	../vax/mscp.h ra_saio.h
	cc -c -S -DRABADS ${COPTS} udabads.c
	/lib/c2 -i udabads.s | as -o udabads.o
	rm udabads.s

# utilities

size:
	@echo "The following images cannot exceed a byte count of 7680."
	@echo "This must be verified."
	@ls -l ${BOOTS32} ${BOOTS32M}

print:
	@pr makefile
	@ls -l | pr 
	@pr *.h *.c

clean:
	rm -f *.o *.exe *.i errs sys.small.c autoconf750.c
	rm -f qvautoconf.c qvprf.c qvget.c
	rm -f a.out b.out boot cat tpboot tpcopy copy rccopy rqcopy tpformat tpcopy-boot
	rm -f boot[a-z]? boot[a-wyz][a-z].c conf[a-wyz][a-z].c
	rm -f rabads rqbads format drtest core bootconf.c 
	rm -f boot_justask libsa.a libqsa.a mlibsa.a mautoconf.c
	rm -rf tmp

lint:
	lint ${COPTS} -hxbn boot.c ${SRCS} | \
	    grep -v 'possible pointer alignment' | \
	    grep -v 'struct/union .* never defined'

install32m: ${MALL}
	-if [ ! -d ${DESTDIR}/tp ]; \
	then \
		mkdir ${DESTDIR}/tp; \
	else \
		true; \
	fi
	install -c rqbads ${DESTDIR}/tp/rabads
	install -c tpcopy-boot ${DESTDIR}/tp/tpcopy-boot
	-if [ ! -d ${DESTDIR}/usr/mdec ]; \
	then \
		mkdir ${DESTDIR}/usr/mdec; \
	else \
		true; \
	fi
	install -c bootrq ${DESTDIR}/usr/mdec/bootra
	install -c bootqv ${DESTDIR}/usr/mdec/boot.mvax
	install -c rqbads ${DESTDIR}/usr/mdec/rabads
	install -c rqcopy ${DESTDIR}/usr/mdec/rccopy

install: ${ALL}
	-if [ ! -d ${DESTDIR}/tp ]; \
	then \
		mkdir ${DESTDIR}/tp; \
	else \
		true; \
	fi
	cp rabads ${DESTDIR}/tp/rabads
	cp tpcopy ${DESTDIR}/tp/copy
	cp tpboot ${DESTDIR}/tp/boot
	cp tpformat ${DESTDIR}/tp/format
	cp boot a.out; strip a.out; \
		dd if=a.out of=../floppy/boot bs=32 skip=1
	cp ../floppy/boot ../cassette/boot
	cp ../floppy/boot ../rl02/boot
	cp boot_justask a.out; strip a.out; \
		dd if=a.out of=../floppy/boot_justask bs=32 skip=1
	cp ../floppy/boot_justask ../cassette/boot_justask
	cp copy a.out; strip a.out; \
		dd if=a.out of=../floppy/copy bs=32 skip=1
	cp ../floppy/copy ../cassette/copy
	cp ../floppy/copy ../rl02/copy
	cp rccopy a.out; strip a.out; \
		dd if=a.out of=../floppy/rccopy bs=32 skip=1
	cp ../floppy/rccopy ../cassette/rccopy
	cp format a.out; strip a.out; \
		dd if=a.out of=../floppy/format bs=32 skip=1
	cp ../floppy/format ../cassette/format
	cp drtest a.out; strip a.out; \
		dd if=a.out of=../floppy/drtest bs=32 skip=1
	cp ../floppy/drtest ../cassette/drtest
	-if [ ! -d ${DESTDIR}/usr/mdec ]; \
	then \
		mkdir ${DESTDIR}/usr/mdec; \
	else \
		true; \
	fi
	cp -r bootup boothk boothp bootra bootrl bootcs ${DESTDIR}/usr/mdec
	cp rabads a.out; strip a.out; \
		dd if=a.out of=../floppy/rabads bs=32 skip=1
	cp ../floppy/rabads ../cassette/rabads
	cp ../floppy/rabads ../rl02/rabads
	install -c boot ${DESTDIR}/usr/mdec
	install -c libsa.a ${DESTDIR}/usr/lib
	ranlib ${DESTDIR}/usr/lib/libsa.a
	-if [ ! -d ${DESTDIR}/sys/rx50 ]; \
	then \
		mkdir ${DESTDIR}/sys/rx50 ; \
	else \
		true; \
	fi
	install -c -s boot_justask ${DESTDIR}/sys/rx50/boot
	install -c -s copy ${DESTDIR}/sys/rx50
	install -c -s rabads ${DESTDIR}/sys/rx50

setup:	${ALL}
	cp boot a.out; strip a.out; \
		dd if=a.out of=../floppy/boot bs=32 skip=1
	cp ../floppy/boot ../cassette/boot
	cp ../floppy/boot ../rl02/boot
	cp boot_justask a.out; strip a.out; \
		dd if=a.out of=../floppy/boot_justask bs=32 skip=1
	cp ../floppy/boot_justask ../cassette/boot_justask
	cp copy a.out; strip a.out; \
		dd if=a.out of=../floppy/copy bs=32 skip=1
	cp ../floppy/copy ../cassette/copy
	cp ../floppy/copy ../rl02/copy
	cp rccopy a.out; strip a.out; \
		dd if=a.out of=../floppy/rccopy bs=32 skip=1
	cp ../floppy/rccopy ../cassette/rccopy
	cp format a.out; strip a.out; \
		dd if=a.out of=../floppy/format bs=32 skip=1
	cp ../floppy/format ../cassette/format
	cp drtest a.out; strip a.out; \
		dd if=a.out of=../floppy/drtest bs=32 skip=1
	cp ../floppy/drtest ../cassette/drtest
	cp rabads a.out; strip a.out; \
		dd if=a.out of=../floppy/rabads bs=32 skip=1
	cp ../floppy/rabads ../cassette/rabads
	cp ../floppy/rabads ../rl02/rabads

# beware...

vmbblk: vmbblk.s
	as -o vmbblk.o vmbblk.s

vmbqio: vmbqio.s
	as -o vmbqio.o vmbqio.s

vmbsrt0.o: vmbsrt0.c ../vax/mtpr.h ../vax/cpu.h
	cc -E ${COPTS} vmbsrt0.c \
	| as -o vmbsrt0.o

vmbboot: vmbblk.o vmbsrt0.o vmbboot.o vmbsys.o vmbqio.o vmbconf.o \
		libsa.a
	ld -N -T 1d0000 vmbblk.o vmbsrt0.o vmbboot.o vmbsys.o \
		vmbconf.o vmbqio.o libsa.a -lc
	cp a.out b.out;strip b.out;dd if=b.out of=vmbboot ibs=32 skip=1;rm b.out

