#ifndef lint
static	char *sccsid = "@(#)cat.c	1.5	(ULTRIX)	9/30/85";
#endif

/*
 *  cat	- conCATenate and print

    Calling format: cat [-e] [-n] [-u] [-v] [-s] [-b] [-t]
	-e print a dollarsign ($) at the end of every line.
	-n number lines
	-u unbuffered output (ordinarily 1024-byte blocks if not terminal)
	-v show non-printables as ^x (or M-x if > 0177), except newline and tab
	-s reduce multiple blank lines to a single blank line
	-b same as -n, but don't number (or count) blank lines
	-t same as -v, but show tabs as well.

    a '-' as the input file will cause input to be requested from stdin.
-------------
	Modification History
	~~~~~~~~~~~~~~~~~~~~
GT001	Greg Tarsa, 24-Oct-83.
	- Added header and made -e its own option and not a suboption of -v.

	R. Rodriguez 	add fast cat using read/write if no options
*/
/************************************************************************
 *									*
 *			Copyright (c) 1983 by				*
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>		/* to get MAXBSIZE */

int	bflg, eflg, nflg, sflg, tflg, vflg;
int	spaced, col, lno, inline;

main(argc, argv)
char **argv;
{
	int fflg = 0;
	register FILE *fi;
	register c;
	int dev, ino = -1;
	struct stat statb;

	lno = 1;
	for( ; argc>1 && argv[1][0]=='-'; argc--,argv++) {
		switch(argv[1][1]) {
		case 0:
			break;
		case 'u':
			setbuf(stdout, (char *)NULL);
			continue;
		case 'n':
			nflg++;
			continue;
		case 'b':
			bflg++;
			nflg++;
			continue;
		case 'v':
			vflg++;
			continue;
		case 's':
			sflg++;
			continue;
		case 'e':
			eflg++;
/* GT001		vflg++;		make -e it's own option */
			continue;
		case 't':
			tflg++;
			vflg++;
			continue;
		}
		break;
	}
	if (fstat(fileno(stdout), &statb) == 0) {
		statb.st_mode &= S_IFMT;
		if (statb.st_mode!=S_IFCHR && statb.st_mode!=S_IFBLK) {
			dev = statb.st_dev;
			ino = statb.st_ino;
		}
	}
	if (argc < 2) {
		argc = 2;
		fflg++;
	}
	while (--argc > 0) {
		if (fflg || (*++argv)[0]=='-' && (*argv)[1]=='\0')
			fi = stdin;
		else {
			if ((fi = fopen(*argv, "r")) == NULL) {
				perror(*argv);
				continue;
			}
		}
		if (fstat(fileno(fi), &statb) == 0) {
			if ((statb.st_mode & S_IFMT) == S_IFREG &&
			    statb.st_dev==dev && statb.st_ino==ino) {
				fprintf(stderr, "cat: input %s is output\n",
				   fflg?"-": *argv);
				fclose(fi);
				continue;
			}
		}
		if (nflg||sflg||vflg||eflg)	/* GT001: add eflg */
			copyopt(fi);
		else {	/* No options so do it FAST! */
			register int	n, nwritten, offset;
			char stdbuf[MAXBSIZE];
			/*
	 		 * Note that on some systems (V7), very large writes
			 * to a pipe return less than the requested size of the
			 * write. In this case, multiple writes are required.
	 		 */
			while ((n=read(fileno(fi),stdbuf,sizeof(stdbuf))) > 0) {
				offset = 0;
				do {
					nwritten = write(fileno(stdout),
							&stdbuf[offset],n);
					if (nwritten <= 0) {
						perror("cat: write error");
						exit(2);
					}
					offset += nwritten;
				} while ((n -= nwritten) > 0);
			}
			if (n < 0) {
				perror("cat: read error");
				return (1);
			}
		}
		if (fi!=stdin)
			fclose(fi);
	}
	if (ferror(stdout))
		fprintf(stderr, "cat: output write error\n");
	return(0);
}

/*
    copyopt

    Implement copy options.  Called if any major option is
    set.
*/
copyopt(f)
	register FILE *f;
{
	register int c;

top:
	c = getc(f);
	if (c == EOF)
		return;
	if (c == '\n') {
		if (inline == 0) {
			if (sflg && spaced)
				goto top;
			spaced = 1;
		}
		if (nflg && bflg==0 && inline == 0)
			printf("%6d\t", lno++);
		if (eflg)
			putchar('$');
		putchar('\n');
		inline = 0;
		goto top;
	}
	if (nflg && inline == 0)
		printf("%6d\t", lno++);
	inline = 1;
	if (vflg) {
		if (tflg==0 && c == '\t')
			putchar(c);
		else {
			if (c > 0177) {
				printf("M-");
				c &= 0177;
			}
			if (c < ' ')
				printf("^%c", c+'@');
			else if (c == 0177)
				printf("^?");
			else
				putchar(c);
		}
	} else
		putchar(c);
	spaced = 0;
	goto top;
}
