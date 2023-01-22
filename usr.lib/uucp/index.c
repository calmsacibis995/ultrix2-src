#ifndef lint
static char sccsid[] = "@(#)index.c	1.4 (decvax!larry) 3/6/84";
#endif

#include <stdio.h>


/*******
 *	char *
 *	index(str, c)	return pointer to character c
 *	char c, *str;
 *
 *	return codes:
 *		NULL  -  character not found
 *		pointer  -  pointer to character
 */

char *
index(str, c)
char c, *str;
{
	for (; *str != '\0'; str++) {
		if (*str == c)
			return(str);
	}

	return(NULL);
}
