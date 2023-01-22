

#ifndef lint
static	char	*sccsid = "@(#)repl.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	Replace each occurrence of `old' with `new' in `str'.
	Return `str'.
*/

repl(str,old,new)
char *str;
char old,new;
{
	return(trnslat(str, &old, &new, str));
}
