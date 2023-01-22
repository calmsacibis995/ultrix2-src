#  @(#)Makefile	1.4	ULTRIX	10/6/86
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
# Make a 780/785 rx01 distribution (install) or console (update) floppy
# 

STDBOO= cira.cmd mbahp.cmd ubara.cmd cnsl.cmd
DIAGS=	
DIST=	${STDBOO} vmb.exe ci780.bin
FROMDEC=du* dm* db* s[mbr]*.cmd *boo.cmd *gen *xdt boot copy rabads format
FLOPPY=	/dev/floppy

all:

floppy:	${STDBOO} /usr/mdec/vmb.exe /usr/mdec/ci780.bin 
	cp /dev/null floppy
	/etc/arff ivf floppy 494,1,ULTRIX-32,Distribution,
	cp cnsl.cmd defboo.cmd
	cp /usr/mdec/vmb.exe vmb.exe
	cp /usr/mdec/ci780.bin ci780.bin
	/etc/arff rvf floppy defboo.cmd ${DIST}
	rm -rf vmb.exe defboo.cmd ci780.bin

install: floppy
	dd if=floppy of=${FLOPPY}

# build a boot floppy from a DEC console floppy
update:	defboo.cmd askboo.cmd ${STDBOO}
	@rm -rf fromdec floppy
	@mkdir fromdec
	@cd fromdec ;\
	echo "" ;\
	echo "Extracting files from the console diskette." ;\
	echo "This takes several minutes." ;\
	echo "" ;\
	dd if=${FLOPPY} of=../floppy bs=128x26x3 count=1 ;\
	/etc/arff xf ${FLOPPY} ;\
	cp /usr/mdec/vmb.exe vmb.exe ;\
	cp /usr/mdec/ci780.bin ci780.bin ;\
	rm -f ${FROMDEC} ;\
	echo "" ;\
	echo "Remove the RX01 console diskette.  Insert a blank RX01 diskette." ;\
	echo "" ;\
	/etc/arff if ../floppy 494,1,ULTRIX-32,Console, ;\
	echo "" ;\
	echo "Building console diskette for Ultrix." ;\
	echo "This takes several minutes." ;\
	echo "" ;\
	/etc/arff rf ../floppy * 
	@rm -rf fromdec
	@/etc/arff rf ./floppy ${STDBOO} defboo.cmd askboo.cmd  
	@dd if=./floppy of=${FLOPPY} conv=sync
	@echo "Directory listing of the new console diskette follows."
	@/etc/arff tvf ${FLOPPY}

clean:
	rm -f floppy
	rm -rf fromdec

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
