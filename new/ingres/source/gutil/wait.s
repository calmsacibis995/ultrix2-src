/*
/*		@(#)wait.s	1.1	(ULTRIX)	1/8/85
/*/

/************************************************************************
/*									*
/*			Copyright (c) 1984 by				*
/*		Digital Equipment Corporation, Maynard, MA		*
/*			All rights reserved.				*
/*									*
/*   This software is furnished under a license and may be used and	*
/*   copied  only  in accordance with the terms of such license and	*
/*   with the  inclusion  of  the  above  copyright  notice.   This	*
/*   software  or  any  other copies thereof may not be provided or	*
/*   otherwise made available to any other person.  No title to and	*
/*   ownership of the software is hereby transferred.			*
/*									*
/*   This software is  derived  from  software  received  from  the	*
/*   University    of   California,   Berkeley,   and   from   Bell	*
/*   Laboratories.  Use, duplication, or disclosure is  subject  to	*
/*   restrictions  under  license  agreements  with  University  of	*
/*   California and with AT&T.						*
/*									*
/*   The information in this software is subject to change  without	*
/*   notice  and should not be construed as a commitment by Digital	*
/*   Equipment Corporation.						*
/*									*
/*   Digital assumes no responsibility for the use  or  reliability	*
/*   of its software on equipment which is not supplied by Digital.	*
/*									*
/************************************************************************/

/ C library -- wait

/	@(#)wait.s	7.1	2/5/81

/ pid = wait(&status);
/
/ pid == -1 if error
/ status indicates fate of process, if given

.globl	_wait, cerror

_wait:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	jmp	cerror
1:
	mov	r1,*4(r5)	/ status return
	mov	(sp)+,r5
	rts	pc
