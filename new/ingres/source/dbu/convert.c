#ifndef lint
static	char	*sccsid = "@(#)convert.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>


/*
**	convert  converts  numeric values of one type and length
**	to a different type and length.
**
**	The source numeric can be i1,i2,i4,f4,or f8.
**	The source number will be converted to the
**	type and length specified in the destination.
**	It also  must be one of i1,i2,i4,f4, or f8.
**
**	convert returns 0 is no overflow occured,
**	  else it returns -1
*/

convert(inp, outp, sf, slen, df, dlen)
char	*inp;	/* pointer to input */
char	*outp;	/* pointer to the output area */
int	sf;	/* format of the source number */
int	slen;	/* length of the source number */
int	df;	/* format of the dest */
int	dlen;	/* length of the dest */
{
	union anytype	number;
	register union anytype	*num;
	register int	sl;
	register int	dl;

	dl = dlen;
	sl = slen;
	num = &number;
	bmove(inp, num, sl);	/* copy number into buffer */

	if (sf != df)
	{
		/* if the source and destination formats are
		   different then the source must be converted
		   to i4 if the dest is int, otherwise to f8 */

		if (df == FLOAT)
		{
			switch (sl)
			{

			  case 1:
				num->f8type = num->i1type;	/* i1 to f8 */
				break;

			  case 2:
				num->f8type = num->i2type;	/* i2 to f8 */
				break;

			  case 4:
				num->f8type = num->i4type;	/* i4 to f8 */
			}
		sl = 8;
		}
		else
		{
			/* check if float >  2**31 */
			if (sl == 8)
				num->f4type = num->f8type;	/* f8 to f4 */

			if (num->f4type > 2147483647.0 | num->f4type < -2147483648.0)
				return (-1);
			num->i4type = num->f4type;	/* f4 to i4 */
			sl = 4;
		}
	}

	/* source is now the same type as destination */
	/* convert lengths to match */

	if (sl != dl)
	{
		/* lengths don't match. convert. */
		if (df == FLOAT)
		{
			if (dl == 8)
				num->f8type = num->f4type;	/* f4 to f8 */
			else
				num->f4type = num->f8type;	/* f8 to f4 with rounding */
		}
		else
		{
			switch (dl)
			{

			  case 1:
				if (sl == 2)
				{
					if (num->i2type > 127 | num->i2type < -128)
						return (-1);
					num->i1type = num->i2type;	/* i2 to i1 */
				}
				else
				{
					if (num->i4type > 127 | num->i4type < -128)
						return (-1);
					num->i1type = num->i4type;	/* i4 to i1 */
				}
				break;

			  case 2:
				if (sl == 1)
				{
					num->i2type = num->i1type;	/* i1 to i2 */
				}
				else
				{
					if (num->i4type > 32767 | num->i4type <-32768)
						return (-1);
					num->i2type = num->i4type;	/* i4 to i2 */
				}
				break;

			  case 4:
				if (sl == 1)
					num->i4type = num->i1type;	/* i1 to i4 */
				else
					num->i4type = num->i2type;	/* i2 to i4 */
			}
		}
	}

	/* conversion is complete */
	/* copy the result into outp */

	bmove(num, outp, dl);
	return (0);
}
