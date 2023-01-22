/* getpcomp() -
 *	fetch a path component
*/

char	*index();

char *getpcomp(outstr,begin)
char *outstr, *begin;
{
	int	outlen;
	char	*p;

	strcpy( outstr, begin );
	if( p = index( begin, '/' ) ) /* we have a valid slash */
	{
		*(outstr + (p++ - begin) ) = '\0';
		return(p);
	}
	else	/* last component */
		return(begin+strlen(begin));
}
