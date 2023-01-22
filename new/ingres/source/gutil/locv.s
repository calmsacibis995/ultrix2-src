/*
 *		@(#)locv.s	1.1	(ULTRIX)	1/8/85
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

# @(#)locv.s	1.1	9/26/83
# C library -- long output conversion

.globl	_locv

_locv:
	.word	0x0000
	subl2	$8,sp
	cvtlp	4(ap),$11,(sp)
	editpc	$11,(sp),edpat,str
	skpc 	$' ,$11,str
	movl	r1,r0
	ret
	.data
edpat:	.byte	0xaa,0x01,0x91,0x44,0x00,0x00
str:	.space	13
