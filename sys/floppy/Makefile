#	@(#)Makefile	1.9	(ULTRIX) 9/30/85	
#									*
#			Copyright (c) 1984 by				*
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

#
# Make a 780 rx01 floppy
# 

# Modification History
#
# 25-Sep-85 --tresvik
#	include proper initialization for distribution build (install)
#
# 15-Jan-85 - tresvik
#	removed drtest and format from update.  
#
# 25-sep-84 - tresvik
#
#	changed make install to cause it to install boot_justask 
#	instead of boot.
#
#	Also included installation of diagnostic boot procedures
#
#
BOOTASK=boot.
BOOTS=	boot
STDBOO=	defboo.cmd anyboo.cmd restar.cmd
DEFBOO=	defboo.hk defboo.hp defboo.ra defboo.up
DEVBOO=	hkmboo.cmd hksboo.cmd hpmboo.cmd hpsboo.cmd \
	ramboo.cmd rasboo.cmd upmboo.cmd upsboo.cmd
UTILS=	copy rabads
NOSUPPORT= format drtest rccopy
# order is NOT random...
OBJS=	${BOOTS} ${STDBOO} ${DEFBOO} ${DEVBOO} ${UTILS}
OBJS1=	${BOOTASK} ${STDBOO} ${DEFBOO} ${DEVBOO} ${UTILS}
DIAGS=	s*boo.cmd
FROMDEC=du* dm* db* s[mbr]* *boo.* vmb.exe *.hlp cs* ci*
FLOPPY=	/dev/floppy

all:	floppy

floppy:	boot_justask ${STDBOO} ${DEFBOO} ${DEVBOO} ${UTILS} ${NOSUPPORT}
	cp /dev/null floppy
	cp boot_justask boot.
	/etc/arff ivf floppy 494,1,ULTRIX-32,Distribution,
	/etc/arff rvf floppy ${OBJS1} ${NOSUPPORT}
	rm boot.

install: floppy
	dd if=floppy of=${FLOPPY}

# build a boot floppy from a DEC console floppy
update:	${OBJS} ${DIAGS}
	-if [ ! -d fromdec ]; \
	then \
		mkdir fromdec; \
	else \
		true; \
	fi
	cd fromdec; /etc/arff xf ${FLOPPY}; rm -f ${FROMDEC} ${OBJS}; \
		/etc/flcopy -t3 -f ${FLOPPY}; rm -f floppy; /etc/arff rcf ${FLOPPY} *
	rm -rf fromdec
	/etc/arff rf ${FLOPPY} ${OBJS} ${DIAGS}

clean:
	rm -f boot_justask ${BOOTS} ${BOOTASK} ${UTILS} ${NOSUPPORT} floppy
	rm -rf fromdec
