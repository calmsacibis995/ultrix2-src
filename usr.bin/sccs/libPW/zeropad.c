

#ifndef lint
static	char	*sccsid = "@(#)zeropad.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	Replace initial blanks with '0's in `str'.
	Return `str'.
*/

char *zeropad(str)
char *str;
{
	register char *s;

	for (s=str; *s == ' '; s++)
		*s = '0';
	return(str);
}
