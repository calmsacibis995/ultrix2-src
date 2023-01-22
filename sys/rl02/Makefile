#	@(#)Makefile	1.2	(ULTRIX)	4/18/85
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
#   The information in this software is subject to change  without	*
#   notice  and should not be construed as a commitment by Digital	*
#   Equipment Corporation.						*
#									*
#   Digital assumes no responsibility for the use  or  reliability	*
#   of its software on equipment which is not supplied by Digital.	*
#									*
#***********************************************************************

#
# Update a VAX8600 RL02 console disk
# 

# Modification History
#
# 28-Mar-85 --tresvik
#	added copy.com
#
# 14-Mar-85 --tresvik
#	Created
#
UTILS=	boot copy rabads
BOOTS=	copy.com defboo.com anyboo.com rasboo.com ramboo.com hpsboo.com \
	hpmboo.com
CONSDEV=/dev/crl

update:
	/etc/arff rmvf ${CONSDEV} ${UTILS} ${BOOTS}

install: 
	cp anyboo.com defboo.com
	/etc/arff rmvf ${CONSDEV} ${UTILS} ${BOOTS}
clean:
	rm -f ${UTILS}
