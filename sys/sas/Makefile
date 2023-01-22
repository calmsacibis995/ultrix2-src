#
#  @(#)Makefile	1.8	ULTRIX	10/3/86
#
#			Copyright (c) 1985 by
#		Digital Equipment Corporation, Maynard, MA
#			All rights reserved.
#
#   This software is furnished under a license and may be used and
#   copied  only  in accordance with the terms of such license and
#   with the  inclusion  of  the  above  copyright  notice.   This
#   software  or  any  other copies thereof may not be provided or
#   otherwise made available to any other person.  No title to and
#   ownership of the software is hereby transferred.
#
#   This software is  derived  from  software  received  from  the
#   University    of   California,   Berkeley,   and   from   Bell
#   Laboratories.  Use, duplication, or disclosure is  subject  to
#   restrictions  under  license  agreements  with  University  of
#   California and with AT&T.
#
#   The information in this software is subject to change  without
#   notice  and should not be construed as a commitment by Digital
#   Equipment Corporation.
#
#   Digital assumes no responsibility for the use  or  reliability
#   of its software on equipment which is not supplied by Digital.
#
#
DESTROOT=
CFLAGS=-O
# Relocation factor boot path and loader path at 1 1/2 Meg 
RELOC=180000
# Relocation factor network loader path at 2 1/2 Meg (3 Meg minimum)
NRELOC=280000

BOOTS= vaxboot ultrixboot
LOADERS= ultrixload netload

SRCS=	bootblk.c vaxboot.c ultrixboot.c sys.c common.c \
	qdcons.c qdio.c cons640.c qvcons.c ../vaxuba/qfont.c \
	vaxstart.c ultrixstart.c pcsload.c qio.c get.c prf.c geterr.c \
	netload.c ultrixload.c decompress.c emulate.c ../emul/vaxstring.s

#
# General maintenance rules
#
ALL = ${BOOTS} ${LOADERS}
all: ${ALL}

install: all
	-if [ ! -d ${DESTROOT}/usr/mdec ]; \
	then \
		mkdir ${DESTROOT}/usr/mdec; \
		/etc/chown root ${DESTROOT}/usr/mdec; \
	else \
		true; \
	fi
	install -m 644 -c vaxboot ${DESTROOT}/usr/mdec/vaxboot
	install -m 644 -c ultrixload ${DESTROOT}/usr/mdec/ultrixload
	install -m 644 -c netload ${DESTROOT}/usr/mdec/netload
	install -m 644 -c ultrixboot ${DESTROOT}/ultrixboot

clean:
	rm -f *.out core ${BOOTS} ${LOADERS} bootblk *.o errs \
		makefile.bak

clobber:
	for i in `echo [a-z]*`; do \
		if test -f $$i; \
		then \
		rm -f $$i; \
		fi; \
	done
	rm -f Makefile

sccsget:
	sccs get ${DATE} -s SCCS

sccsinfo:
	sccs info

#
# Specific rules
#

print:
	@pr Makefile
	@pr *.h *.c *.s

bootblk: bootblk.o
	strip bootblk.o; dd if=bootblk.o bs=32 skip=1 of=b.out
	dd if=b.out of=bootblk conv=sync count=1

bootblk.o: bootblk.c
	- cc -E ${COPTS} bootblk.c | as -o bootblk.o

qio.o: qio.c
	cc -E ${COPTS} qio.c \
	| as -o qio.o

qdio.o: qdio.c
	cc -E ${COPTS} qdio.c \
	| as -o qdio.o

cons640.o: cons640.c
	cc -E ${COPTS} cons640.c \
	| as -o cons640.o

vaxstart.o: vaxstart.c
	cc -E ${COPTS} vaxstart.c | as -o vaxstart.o

common.o: common.c
	cc -E ${COPTS} common.c | as -o common.o
	cc -E -DSECONDARY ${COPTS} common.c | as -o common.SECONDARY.o

ultrixstart.o: ultrixstart.c
	cc -E ${COPTS} ultrixstart.c | as -o ultrixstart.o

pcsload.o: pcsload.c
	cc -E ${COPTS} pcsload.c | as -o pcsload.o

emulate.o: emulate.c
	cc -E ${COPTS} emulate.c | as -o emulate.o

vaxstring.o: ../emul/vaxstring.s
	/lib/cpp -DMVAX ../emul/vaxstring.s | as -o vaxstring.o

sys.o: sys.c
	cc -c ${CFLAGS} -DSECONDARY sys.c
	mv sys.o sys.SECONDARY.o
	cc -c ${CFLAGS} sys.c

qfont.o: ../vaxuba/qfont.c
	cc ${CFLAGS} -c ../vaxuba/qfont.c

ultrixboot.o: ultrixboot.c
	cc ${CFLAGS} -c ultrixboot.c

vaxboot.o: vaxboot.c
	cc ${CFLAGS} -c vaxboot.c

ultrixload.o: ultrixload.c
	cc ${CFLAGS} -c ultrixload.c

netload.o: netload.c
	cc ${CFLAGS} -c netload.c

vaxboot: bootblk vaxstart.o common.o vaxboot.o sys.o qio.o 
	ld -T ${RELOC} -N vaxstart.o common.o vaxboot.o sys.o qio.o -lc
	strip a.out;dd if=a.out of=b.out ibs=32 skip=1
	cat bootblk b.out > vaxboot

ultrixboot: ultrixstart.o common.o pcsload.o ultrixboot.o sys.o \
		qdio.o cons640.o qio.o get.o prf.o qdcons.o qvcons.o \
		qfont.o geterr.o emulate.o vaxstring.o 
	ld -T ${RELOC} -N -o ultrixboot ultrixstart.o \
		common.SECONDARY.o pcsload.o ultrixboot.o \
		sys.SECONDARY.o qdcons.o qdio.o cons640.o qvcons.o \
		qfont.o qio.o get.o prf.o geterr.o emulate.o vaxstring.o -lc

ultrixload: bootblk vaxstart.o common.o ultrixload.o qdcons.o qdio.o \
		cons640.o qvcons.o qfont.o qio.o get.o prf.o \
		decompress.o geterr.o emulate.o vaxstring.o 
	ld -T ${RELOC} -N vaxstart.o common.SECONDARY.o ultrixload.o \
		qdcons.o qdio.o cons640.o qvcons.o \
		qfont.o qio.o get.o prf.o decompress.o geterr.o \
		emulate.o vaxstring.o -lc
	strip a.out;dd if=a.out of=b.out ibs=32 skip=1
	cat bootblk b.out > a.out
	dd if=a.out of=ultrixload conv=sync
	rm -f a.out b.out

netload: vaxstart.o common.o netload.o qdcons.o qdio.o \
	cons640.o qvcons.o qfont.o qio.o prf.o geterr.o \
	emulate.o vaxstring.o
	ld -T ${NRELOC} -N -o netload \
		vaxstart.o common.SECONDARY.o netload.o qdcons.o \
		qdio.o cons640.o qvcons.o qfont.o qio.o prf.o \
		geterr.o emulate.o vaxstring.o -lc

depend:
	grep '^#include' ${SRCS} | grep -v '<' | \
	sed -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' > xtmp 
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep <xtmp
	rm -f xtmp
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile makefile.bak
	ed - Makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend needs it

bootblk.o: vmb.h
vaxboot.o: vmb.h ../h/param.h
ultrixboot.o: vmb.h ../h/param.h ../h/reboot.h ../vax/cpu.h ../vax/mtpr.h
ultrixboot.o: ../vax/rpb.h
sys.o: ../h/param.h ../h/gnode_common.h ../ufs/ufs_inode.h ../h/gnode.h
sys.o: ../h/dir.h saio.h vmb.h ../vax/rpb.h
common.o: ../vax/mtpr.h ../vax/cpu.h vmb.h
qdcons.o: ../h/types.h
qvcons.o: ../h/types.h ../vaxuba/qvioctl.h ../vax/cpu.h
vaxstart.o: ../vax/mtpr.h vmb.h
ultrixstart.o: ../vax/mtpr.h vmb.h
qio.o: vmb.h
get.o: ../h/param.h vmb.h ../vax/mtpr.h ../vax/cons.h
prf.o: ../h/param.h vmb.h ../vax/mtpr.h ../vax/cons.h ../vax/cpu.h
geterr.o: vmb.h
netload.o: vmb.h ../h/param.h
ultrixload.o: vmb.h ../h/param.h
emulate.o: ../emul/vaxemul.h ../emul/vaxregdef.h
../emul/vaxstring.s: ../emul/vaxemul.h ../machine/psl.h ../emul/vaxregdef.h
