#ifndef lint
static	char	*sccsid = "@(#)prompt.c	1.1	(ULTRIX)	1/8/85";
#endif lint

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

# include	"monitor.h"
# include	<ingres.h>
# include	<aux.h>




/*
**  OUTPUT PROMPT CHARACTER
**
**	The prompt is output to the standard output.  It will not be
**	output if -ss mode is set or if we are not at a newline.
**
**	The parameter is printed out if non-zero.
**
**	Uses trace flag 14
*/

prompt(msg)
char	*msg;
{
	if (!Prompt || GiveEof)
		return;
	if (Nodayfile >= 0)
	{
		if (msg)
			printf("\07%s\n", msg);
		printf("* ");
	}
	fflush(stdout);
}


/*
**  PROMPT WITH CONTINUE OR GO
*/

cgprompt()
{
	prompt(Notnull ? "continue" : "go");
}
