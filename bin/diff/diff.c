#ifndef lint
static	char	*sccsid = "@(#)diff.c	1.4	(ULTRIX)	9/30/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
#include "diff.h"
/*
 * diff - driver and subroutines
 *
-------------
	Modification History
	~~~~~~~~~~~~~~~~~~~~
GT001	Greg Tarsa, 4-Nov-83.
	- Corrected an error text to match documentation.
*/

char	diff[] = DIFF;
char	diffh[] = DIFFH;
char	pr[] = PR;

main(argc, argv)
	int argc;
	char **argv;
{
	register char *argp;

	ifdef1 = "FILE1"; ifdef2 = "FILE2";
	status = 2;
	diffargv = argv;
	argc--, argv++;
	while (argc > 2 && argv[0][0] == '-') {
		argp = &argv[0][1];
		argv++, argc--;
		while (*argp) switch(*argp++) {

#ifdef notdef
		case 'I':
			opt = D_IFDEF;
			wantelses = 0;
			continue;
		case 'E':
			opt = D_IFDEF;
			wantelses = 1;
			continue;
		case '1':
			opt = D_IFDEF;
			ifdef1 = argp;
			*--argp = 0;
			continue;
#endif
		case 'D':
			/* -Dfoo = -E -1 -2foo */
			wantelses = 1;
			ifdef1 = "";
			/* fall through */
#ifdef notdef
		case '2':
#endif
			opt = D_IFDEF;
			ifdef2 = argp;
			*--argp = 0;
			continue;
		case 'e':
			opt = D_EDIT;
			continue;
		case 'f':
			opt = D_REVERSE;
			continue;
		case 'b':
			bflag = 1;
			continue;
		case 'c':
			opt = D_CONTEXT;
			if (isdigit(*argp)) {
				context = atoi(argp);
				while (isdigit(*argp))
					argp++;
				if (*argp) {
					fprintf(stderr,
					    "diff: -c: bad count\n");
					done();
				}
				argp = "";
			} else
				context = 3;
			continue;
		case 'h':
			hflag++;
			continue;
		case 'S':
			if (*argp == 0) {
				fprintf(stderr, "diff: use -Sstart\n");
				done();
			}
			start = argp;
			*--argp = 0;		/* don't pass it on */
			continue;
		case 'r':
			rflag++;
			continue;
		case 's':
			sflag++;
			continue;
		case 'l':
			lflag++;
			continue;
		default:
			fprintf(stderr, "diff: -%s: unknown option\n",
			    --argp);
			done();
		}
	}
	if (argc != 2) {
		fprintf(stderr, "diff: two filename arguments required\n");
		done();
	}
	file1 = argv[0];
	file2 = argv[1];
	if (hflag && opt) {
		fprintf(stderr,
		    "diff: -h doesn't support -e, -f, -c, or -D\n");
		done();
	}
	if (!strcmp(file1, "-"))
		stb1.st_mode = S_IFREG;
	else if (stat(file1, &stb1) < 0) {
		fprintf(stderr, "diff: ");
		perror(file1);
		done();
	}
	if (!strcmp(file2, "-"))
		stb2.st_mode = S_IFREG;
	else if (stat(file2, &stb2) < 0) {
		fprintf(stderr, "diff: ");
		perror(file2);
		done();
	}
	if ((stb1.st_mode & S_IFMT) == S_IFDIR &&
	    (stb2.st_mode & S_IFMT) == S_IFDIR) {
		diffdir(argv);
	} else
		diffreg();
	done();
}

char *
savestr(cp)
	register char *cp;
{
	register char *dp = malloc(strlen(cp)+1);

	if (dp == 0) {
		fprintf(stderr, "diff: ran out of memory\n");
		done();
	}
	strcpy(dp, cp);
	return (dp);
}

min(a,b)
	int a,b;
{

	return (a < b ? a : b);
}

max(a,b)
	int a,b;
{

	return (a > b ? a : b);
}

done()
{
	unlink(tempfile);
	exit(status);
}

char *
talloc(n)
{
	register char *p;
	p = malloc((unsigned)n);
	if(p!=NULL)
		return(p);
	noroom();
}

char *
ralloc(p,n)	/*compacting reallocation */
char *p;
{
	register char *q;
	char *realloc();
	free(p);
	free(dummy);
	dummy = malloc(1);
	q = realloc(p, (unsigned)n);
	if(q==NULL)
		noroom();
	return(q);
}

noroom()
{
	fprintf(stderr, "diff: files too big, try -h\n");
	done();
}
