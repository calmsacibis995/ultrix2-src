#ifndef lint
static	char	*sccsid = "@(#)IIw_right.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# undef MAXNAME
# include	"../equel/constants.h"
# include	"IIglobals.h"



/*
**  IIw_right -- Write down to the Quel parser a string
**	for a target list of anything but a tupret.
**
**	Parameters:
**		string -- a string which contains the target list
**			of a quel statement, where values from C variables
**			to be plugged in are flagged by the construct
**			'%<ingres_type>" a la printf(). String is left 
**			unchanged. 
**			To escape the '%' mechanism use '%%'.
**
**		argv -- a vector of pointers to
**			variables from which the values flagged by the '%'
**			mechanism are taken.
**
**	Usage:
**		argv [0] = &double_raise;
**		IIw_right("dom1 = i.ifield * (1+%f8)", argv);
**
**	Required By:
**		all the parametrized statements except "tupret_p".
**
**	Error numbers:
**		1004 -- bad type in parametrized statement.
*/



IIw_right(string, argv)
char	*string;
char	**argv;
{
	register char	*b_st, *e_st;
	register char	**av;
	int		type;
	char		*IIitos();

	if (IIdebug)
		printf("ent IIw_right : string \"%s\"\n",
		string);
	av = argv;
	for (b_st = e_st = string; *e_st; )
	{
		if (*e_st != '%')
		{
			e_st++;
			continue;
		}

		/* provide '%%' escape mechanism */
		if (e_st [1] == '%')
		{
			e_st [1] = '\0';
			IIwrite(b_st);
			e_st [1] = '%';
			b_st = e_st = &e_st [2];
			continue;
		}
		*e_st = '\0';
		IIwrite(b_st);
		*e_st++ = '%';

		switch (*e_st)
		{

		  case 'f' :
			switch (*++e_st)
			{

			  case '8' :
				IIcvar(*av, 4, 8);
				break;

			  case '4' :
				IIcvar(*av, 2, 4);
				break;

			  default :
				goto error_label;
			}
			av++;
			break;

		  case 'i' :
			switch (*++e_st)
			{

			  case '4' :
				IIcvar(*av, 2, 4);
				break;

			  case '2' :
				IIcvar(*av, 1, 2);
				break;

			  case '1' :
				IIcvar(*av, 5, 1);

			  default :
				goto error_label;
			}
			av++;
			break;

		  case 'c' :
			IIcvar(*av++, 3, 0);
			break;
		}
		b_st = ++e_st;
	}
	IIwrite(b_st);
	return;


error_label :
	IIerror(1004, 1, &string); 
	IIerrflag = 1004;
	/* make sure that part already written down will 
	 * cause an error, and don't print it.
	 *	The old IIprint_err is restored in IIerror()
	 */
	IIwrite(",");
	IIo_print = IIprint_err;
	IIprint_err = IIno_err;
}
