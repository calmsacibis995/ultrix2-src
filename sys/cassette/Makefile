 #	@(#)Makefile	1.12	(ULTRIX)	10/21/85
 #									*
 #			Copyright (c) 1983 by				*
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

 # ------------------------------------------------------------------------
 # Modification History: /sys/cassette/Makefile
 #
 # 15-Oct-85 --tresvik
 #	removed dependency of ../mdec/tuboot from the make install
 #	so that distribution cassettes can be made on other than a
 #	source system.
 #
 # 15-Jan-85 --tresvik
 #	removed drtest and format from update
 #
 # 19 Jul 84 --reilly
 #	Added new module rccopy.c
 #
 # 19 Jul 84 --tresvik
 #	included diagnostic boot files to the update procedure
 #
 #  9 Dec 83 --jmcg (1)
 #	Removed distinctions between boot.730 and boot.750 (see comments
 #	in /sys/stand/Makefile).  Extensive changes for building cassettes
 #	to reduce time required.  Distribution media contain a `boot'
 #	that was compiled with the flag JUSTASK set in to avoid premature
 #	attempts at automatic boot.
 #
 #  9 Dec 83 --jmcg
 #	Derived from 4.2BSD, labeled:
 #		Makefile	6.2	83/09/25
 #
 # ------------------------------------------------------------------------

#
# Make a 750/730 tu58 cassette
#
#	Note that the console TU58 drivers may be intolerant of other
#	system activity, particularly typing on the console.  These
#	operations should be performed with a quiescent system.
# 
STDBOO=	defboo.cmd anyboo.cmd restar.cmd
DEFBOO=	defboo.rb defboo.rl defboo.hk defboo.up defboo.ra
DEVBOO=	rbmboo.cmd rbsboo.cmd rlsboo.cmd hkmboo.cmd hksboo.cmd \
	upmboo.cmd upsboo.cmd ramboo.cmd rasboo.cmd
# order is NOT random, it makes a big difference in how long it takes
# to reboot and to reload microcode on power-up.
OBJS0=	${STDBOO} boot.
OBJS1=	${STDBOO} boot
DIAGS=	s*boo.cmd	
FROMDEC= consol.exe power.cmd consle.cpu mmie.cpu power.cpu \
	code??.cmd fp.cpu fpsp.cpu bitfld.cpu cm.cpu basic.cpu \
	irdfpa.cpu queue.cpu idc.cpu
OBJS2= ${DEFBOO} ${DEVBOO} copy rabads rccopy 
NOSUPPORT= format drtest
UNNEEDED=d[lmq]* cs* *boo.* boot.exe vmb.exe
CASSETTE=/dev/tu0
BCASSETTE=/dev/tu1

all:	cassette

cassette: ${STDBOO} boot_justask ${OBJS2} ${NOSUPPORT}
	cp /dev/null cassette
	cp boot_justask boot.
	/etc/arff rmcf cassette ${OBJS0} ${OBJS2} ${NOSUPPORT}
	rm boot.

#
# Used only if a source site
#
#../mdec/tuboot: ../mdec/tuboot.s
#	cd ../mdec; make ${MFLAGS} tuboot

install: cassette 
	dd if=cassette of=${CASSETTE} bs=20k conv=sync
	dd if=../mdec/tuboot of=${CASSETTE} bs=1k count=1

# build a 730 boot cassette from a DEC console cassette
# DONT TRY THIS ON A 750 W/O MRSP, THE SYSTEM WILL CRASH
update:	boot ${STDBOO} ${OBJS2} ${DIAGS}
	-if [ ! -d /tmp/fromdec ]; \
	then \
		mkdir /tmp/fromdec; \
	else \
		true; \
	fi
	dd if=${BCASSETTE} of=/tmp/cassette
	cd /tmp/fromdec; /etc/arff xmf /tmp/cassette
	/etc/arff rcmf /tmp/cassette ${OBJS1}
	cd /tmp/fromdec; /etc/arff rmf /tmp/cassette ${FROMDEC}
	/etc/arff rmf /tmp/cassette ${OBJS2} ${DIAGS}
	dd if=/tmp/cassette of=${CASSETTE} bs=20k conv=sync
	@echo Console cassette update complete.
	@echo The new cassette should be installed in the internal
	@echo tu58 slot '(DD1:).'


clean:
	rm -f boot boot. boot_justask format copy rccopy rabads drtest
	rm -fr /tmp/fromdec /tmp/cassette
