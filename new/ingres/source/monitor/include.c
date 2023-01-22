#ifndef lint
static	char	*sccsid = "@(#)include.c	1.1	(ULTRIX)	1/8/85";
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
**  INCLUDE FILE
**
**	A file name, which must follow the \i, is read and inserted
**	into the text stream at this location.  It may include all of
**	the standard control functions.  Includes may be nested.
**
**	If the parameter is 0, the file name is taken from the input;
**	otherwise it is taken directly from the parameter.  In this
**	mode, errors are not printed.
**
**	Prompts are turned off during the include.
*/

include(filename)
char	*filename;
{
	int			savendf;
	FILE			*saveinp;
	register char		*f;
	register FILE		*b;
	extern char		*getfilenm();

	f = filename;
	if (f == 0)
		f = getfilenm();
	if (sequal(f, "-"))
	{
		/* read keyboard */
		b = stdin;
	}
	else if (*f == 0)
	{
		/* back up one level (EOF on next read) */
		GiveEof = TRUE;
		return;
	}
	else
	{
		/* read file */
		if ((b = fopen(f, "r")) == NULL)
		{
			if (filename == 0)
				printf("Cannot open \"%s\"\n", f);
			return;
		}
	}

	/* check for too deep */
	if (Idepth >= 5)
	{
		printf("Include nested too deep\n");
		if (b)
			fclose(b);
		return;
	}
	Idepth++;

	/* get input from alternate file */
	savendf = Nodayfile;
	if (b == stdin)
	{
		Nodayfile = Userdflag;
		prompt("<<input>>");
	}
	else
		Nodayfile = -1;
	saveinp = Input;
	Input = b;
	monitor();

	/* done -- restore old file */
	Input = saveinp;
	Nodayfile = savendf;
	Idepth--;
}
