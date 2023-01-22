#  @(#)Makefile	1.3	ULTRIX	10/3/86
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

#
# Make a 730 tu58 cassette
#
STDBOO=	ubara.cmd ubaidc.cmd cnsl.cmd
# order is NOT random, it makes a big difference in how long it takes
# to reboot and to reload microcode on power-up.
DIAGS=
FROMDEC= consol.exe power.cmd consle.cpu mmie.cpu power.cpu \
	code??.cmd fp.cpu fpsp.cpu bitfld.cpu cm.cpu basic.cpu \
	irdfpa.cpu queue.cpu idc.cpu
UNNEEDED=d[lmq]* cs* *boo.*
CASSETTE=/dev/tu0
BCASSETTE=/dev/tu1

all:

cassette: ${STDBOO} /usr/mdec/vmb.exe
	cp /dev/null cassette
	cp cnsl.cmd defboo.cmd
	cp /usr/mdec/vmb.exe .
	/etc/arff imvf cassette 512,1,ULTRIX-32,Distribution,
	/etc/arff rmvf cassette defboo.cmd vmb.exe ${STDBOO}
	rm -f vmb.exe defboo.cmd

install: cassette 
	dd if=cassette of=${CASSETTE} bs=20k conv=sync

# build a 730 boot cassette from a DEC console cassette
update:	defboo.cmd askboo.cmd ${STDBOO}
	@rm -rf fromdec
	@mkdir fromdec
	@echo ""
	@echo "Extracting files from the console cassette."
	@echo "This takes several minutes."
	@cd fromdec; arff xmf ${BCASSETTE} ; cp /usr/mdec/vmb.exe .
	@cp defboo.cmd askboo.cmd ${STDBOO} fromdec
	@dd if=${BCASSETTE} of=cassette count=16
	@echo ""
	@echo "Remove the TU58 cassette from the front drive.  Insert a"
	@echo "blank TU58 cassette.  Make sure the cassette is write-enabled."
	@/etc/arff imf cassette 512,1,ULTRIX-32,Console,
	@echo ""
	@echo "Building console cassette for ULTRIX."
	@echo "This takes several minutes."
	@cd fromdec; /etc/arff rmf ../cassette defboo.cmd \
		vmb.exe askboo.cmd ${FROMDEC} ${STDBOO}
	@dd if=cassette of=${CASSETTE} bs=20k conv=sync
	@echo ""
	@echo "Directory listing of the new console cassette follows."
	@/etc/arff tmvf ${CASSETTE}
	@echo ""
	@echo "Remove the TU58 cassette from the side drive.  Move the TU58"
	@echo "cassette from the front drive to the side drive."
	@echo ""


clean:
	rm -f vmb.exe 
	rm -rf fromdec cassette

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
