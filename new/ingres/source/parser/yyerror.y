/*
 *		@(#)yyerror.y	1.2	(ULTRIX)	1/22/85
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
**   YYERROR -- the routine which error routine yacc calls
**
**	Version:
**		@(#)yyerror.y	7.1	2/5/81
*/

yyerror(errmessage)
char	*errmessage;
{
#	ifdef	xPTR1
	tTfp(60, 6, "yyerror: an error from yacc itself\n");
#	endif

	if (sequal(errmessage, "syntax error"))
		par_error(SYMERR, WARN, 0);
	else
		par_error(YOVRFLOW, WARN, 0);
}

/*
**   PAR_ERROR -- the error routine for the parser
**
**	Par_error sends its arguments to print_error(), the front-end
**	error processor.  If result = FATAL then reset() is called,
**	otherwise, the error is assumed to be recoverable, and parsing
**	continues.
**
**	Parameters:
**		num -- the number of the error (2000 - 2999).
**		result -- if = FATAL then reset, otherwise return.
**		a, b, c -- arbitrary arguments to the error routine.
**		
**	Returns:
**		if result is not FATAL
**
**	Requires:
**		print_error() -- of the monitor
**		
**	Called By:
**		parser routines
**		scanner routines
**
**	Written:
**		1979 (jiw)
**		13 feb 1980 modified for monpar (jiw)
*/

par_error(num, result, a, b, c)
int	num;
int	result;
char	*a, *b, *c;
{
	char		buff[30];
	register char	*buf;

	extern short	yyerrflag;
	extern int	Err_current;
	extern int	Err_fnd;
	extern int	Opflag;
	extern int	yyline;

	resetp();

	buf = buff;

#	ifdef	xPTR1
	tTfp(60, 7, "par_error: %d, (fatal = %d), a, b, c.\n", num, result, a, b, c);
#	endif

	yyerrflag = 3;		/* tell yyparse that an error has been found */

	/* 
	**	if Err_current is true at this point,
	**	it is the second error found in the statement.
	**	Thus for the simple error recovery currently
	**	used no other message should be printed.
	*/

	if (Err_current)
	{
		if (result != FATAL)
			return;
		else
		{
#			ifdef	xPTR1
			tTfp(60, 9, "par_error: a non recoverable error\n");
#			endif

			endgo();

			error(0);
		}
	}

#	ifdef	xPTR1
	tTfp(60, 8, "par_error: first error.\n");
#	endif

	Err_fnd += 1;		/* check syntax of remainder */
	Err_current = 1;	/* error was found in this statement */

	if (num == SYMERR || num == NXTCMDERR)
	{
		/* syntax error */
		a = buf;
		b = 0;
		switch (Lastok.toktyp)
		{
		  case I2CONST:
			itoa(*(short *)Lastok.tok, buf);
			break;

		  case I4CONST:
			smove(locv(*(long *)Lastok.tok), buf);
			break;

		  case F4CONST:
			ftoa(*(float *)Lastok.tok, buf, 10, 3, 'n');
			break;

		  case F8CONST:
			ftoa(*(double *)Lastok.tok, buf, 10, 3, 'n');
			break;

		  case SCONST:
			smove(Lastok.tok, buf);
			break;

		  case 0:
			a = "EOF";
			break;

		  default:
			syserr("bad Lastok format");
		}
		num += Opflag;	/* choosing correct error */
	}

	if (result != FATAL)
		error(num, iocv(yyline), a, b, c, 0);
	else
	{
#		ifdef	xPTR1
		tTfp(60, 9, "par_error: a non recoverable error\n");
#		endif

		error(num, iocv(yyline), a, b, c, 0);
	}
}
neederr(errnum)
int	errnum;
{
	par_error(errnum, WARN, 0);
}
