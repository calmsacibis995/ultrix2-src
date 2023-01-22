#ifndef lint
static	char	*sccsid = "@(#)branch.c	1.2	(ULTRIX)	4/23/85";
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
/************************************************************************
 *			Modification History				*
 *									*
 *	David L Ballenger, 16-Apr-1985					*
 * 0001	Don't check rewind() for a return value, since it doesn't 	*
 *	return one.
 *									*
 ************************************************************************/

# include	"monitor.h"
# include	<ingres.h>
# include	<aux.h>




/*
**  BRANCH
**
**	The "filename" following the \b op must match the "filename"
**	which follows some \k command somewhere in this same file.
**	The input pointer is placed at that point if possible.  If
**	the label does not exist, an error is printed and the next
**	character read is an EOF.
**
**	Trace Flags:
**		33
*/

branch()
{
	register char	c;
	register int	i;
	extern char	getch();

#	ifdef xMTR2
	if (tTf(33, -1))
		printf(">>branch: ");
#	endif

	/* see if conditional */
	while ((c = getch()) > 0)
		if (c != ' ' && c != '\t')
			break;
	if (c == '?')
	{
		/* got a conditional; evaluate it */
		Oneline = TRUE;
		macinit(getch, 0, 0);
		i = expr();

		if (i <= 0)
		{
			/* no branch */
#			ifdef xMTR2
			if (tTf(33, 0))
				printf("no branch\n");
#			endif
			getfilenm();
			return;
		}
	}
	else
	{
		ungetc(c, Input);
	}

	/* get the target label */
	if (branchto(getfilenm()) == 0)
		if (branchto(macro("{default}")) == 0)
		{
			GiveEof = TRUE;
			printf("Cannot branch\n");
		}
	return;
}


branchto(label)
char	*label;
{
	char		target[100];
	register char	c;

	smove(label, target);
	rewind(Input);		/*dlb0001*/

	/* search for the label */
	while ((c = getch()) > 0)
	{
		if (c != '\\')
			continue;
		if (getescape(0) != C_MARK)
			continue;
		if (sequal(getfilenm(), target))
			return;
	}
}
