

#ifndef lint
static	char	*sccsid = "@(#)imatch.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	initial match
	if `prefix' is a prefix of `string' return 1
	else return 0
*/

imatch(prefix,string)
register char *prefix, *string;
{
	while (*prefix++ == *string++)
		if (*prefix == 0)
			return(1);
	return(0);
}
