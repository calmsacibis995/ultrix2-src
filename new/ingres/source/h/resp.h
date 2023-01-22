/*
 *		@(#)resp.h	1.1	(ULTRIX)	1/8/85
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

/*
**  RESP.H -- response blocks
**
**	Version:
**		@(#)resp.h	7.1	2/5/81
*/

struct resp
{
	short	resp_resp;	/* the function value */
	char	resp_active;	/* > 0 if in use */
	long	resp_time;	/* elapsed time */
	long	resp_tups;	/* count of tuples touched */
	long	resp_pread;	/* pages read */
	long	resp_pwrit;	/* pages written */
	/* PARM	resp_rval;	/* the module return value */
};

extern struct resp	Resp;	/* the current response */
