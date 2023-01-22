#ifndef lint
/* static char *sccsid = "@(#)ln03of.c	4.12 (Berkeley) 7/16/83";
*/
static char *sccsid = "@(#)ln03of.c	1.4	ULTRIX	3/3/87";
#endif

/*
 *	Letter Quality Printers filter for ln03 looking like lqp
 *
 * 	filter which reads the output of nroff and converts lines
 *	with ^H's to overwritten lines.  Thus this works like 'ul'
 *	but is much better: it can handle more than 2 overwrites
 *	and it is written with some style.
 *	modified by kls to use register references instead of arrays
 *	to try to gain a little speed.
 *
 * 	Passes through escape and control sequences.
 *
 *	Sends control chars to change to landscape mode for pages wider
 *	than 80 columns.  Also changes font and pitch for this case in
 *	order to get 66 lines per page in landscape mode.
 *
 *	Special logic for nroff ESC9 (partial line feed): this is
 * 	converted to ln03 sequence for partial line feed.  Eventually
 *	this kluge should be replaced by an output package for nroff
 *	which knows about ln03 output.	
 *
 */

#include <stdio.h>
#include <signal.h>

/*************************************************************************/
/* added for escape sequence pass through 				 */

#define ESC	  '\033'	/* escape sequence introducer */
#define BSLH	  '\134'	/* back slash */
#define UCP	  '\120'	/* upper case P */
#define PLD	  '\113'	/* upper case K = ln03of partial line feed */
#define NINE	  '\071'	/* 9: nroff ESC9 = partial line feed */
#define escend(x) ((x!='\120')&&(x!='\133')&&(x>='\100')&&(x<='\176'))
int   	escflg =  0;		/* escape sequence flag, 1 = in progress */
int	lstchr;		
/*************************************************************************/

#define MAXWIDTH  132
#define MAXREP    10

char	buf[MAXREP][MAXWIDTH];
int	maxcol[MAXREP] = {-1};
int	lineno;
int	width = 80;	/* default line length */
int	length = 66;	/* page length */
int	indent;		/* indentation length */
int	npages = 1;
int	literal;	/* print control characters */
char	*name;		/* user's login name */
char	*host;		/* user's machine name */
char	*acctfile;	/* accounting information file */

main(argc, argv) 
	int argc;
	char *argv[];
{
	register FILE *p = stdin, *o = stdout;
	register int i, col;
	register char *cp;
	int done, linedone, maxrep;
	char ch, *limit;


	while (--argc) {
		if (*(cp = *++argv) == '-') {
			switch (cp[1]) {
			case 'n':		/* collect login name */
				argc--;
				name = *++argv;
				break;

			case 'h':		/* collect host name */
				argc--;
				host = *++argv;
				break;

			case 'w':		/* collect page width */
				if ((i = atoi(&cp[2])) > 0 && i <= MAXWIDTH)
					width = i;

				if (width > 80) {	/* switch to landscape mode */
					fprintf(o,"\033[15m");    /* change font */
					fprintf(o,"\033[7 J");    /* wide extended A4 page format */	
					fprintf(o,"\033[66t");    /* 66 lines/page */
					fprintf(o,"\033[8 L");    /* vertical pitch = 12 lines/30mm */
				}
				break;

			case 'l':		/* collect page length */	
				length = atoi(&cp[2]);
				break;

			case 'i':		/* collect indent */
				indent = atoi(&cp[2]);
				break;

			case 'c':		/* print control chars */
				literal++;
				break;
			}
		} else
			acctfile = cp;
	}

	for (cp = buf[0], limit = buf[MAXREP]; cp < limit; *cp++ = ' ');
	done = 0;
	
	escflg = 0;		/* is escape/control sequence in progress? */
	while (!done) {
		col = indent;
		maxrep = -1;
		linedone = 0;
		while (!linedone) {
			ch = getc(p);
			if (((escflg==0)&&(ch==ESC))||escflg)
				eschdl(o,ch);	/* deal with escape character */
			else 
				switch (ch) {
				case EOF:
					linedone = done = 1;
					ch = '\n';
					break;
	
				case '\f':		/* new page on form feed */
					lineno = length;
				case '\n':		/* new line */
					if (maxrep < 0)
						maxrep = 0;
					linedone = 1;
					break;
	
				case '\b':		/* backspace */
					if (--col < indent)
						col = indent;
					break;
	
				case '\r':		/* carriage return */
					col = indent;
					break;
	
				case '\t':		/* tab */
					col = ((col - indent) | 07) + indent + 1;
					break;
	
				case '\031':		/* end media */
					/*
				 	* lpd needs to use a different filter to
				 	* print data so stop what we are doing and
				 	* wait for lpd to restart us.
				 	*/
					if ((ch = getchar()) == '\1') {
						fflush(stdout);
						kill(getpid(), SIGSTOP);
						break;
					} else {
						ungetc(ch, stdin);
						ch = '\031';
					}
	
				default:		/* everything else */
					if (col >= width || !literal && ch < ' ') {
						col++;
						break;
					}
					cp = &buf[0][col];
					for (i = 0; i < MAXREP; i++) {
						if (i > maxrep)
							maxrep = i;
						if (*cp == ' ') {
							*cp = ch;
							if (col > maxcol[i])
								maxcol[i] = col;
							break;
						}
						cp += MAXWIDTH;
					}
					col++;
					break;
				}
			}

		/* print out lines */
		for (i = 0; i <= maxrep; i++) {
			for (cp = buf[i], limit = cp+maxcol[i]; cp <= limit;) {
				putc(*cp, o);
				*cp++ = ' ';
			}
			if (i < maxrep)
				putc('\r', o);
			else
				putc(ch, o);
			if (++lineno >= length) {
				npages++;
				lineno = 0;
				if (length < 66)
					putchar('\f');  /* FF for length < 66 */
			}
			maxcol[i] = -1;
		}
	}
	if (lineno) {		/* be sure to end on a page boundary */
		putchar('\f');
		npages++;
	fprintf(o,"\033\143");	/* reset printer defaults */
	fflush(o);		/* make sure reset goes out */
	sleep(6);		/* some printers eat lines during reset so wait */
	}
	if (name && acctfile && access(acctfile, 02) >= 0 &&
	    freopen(acctfile, "a", stdout) != NULL) {
		printf("%7.2f\t%s:%s\n", (float)npages, host, name);
	}
	exit(0);
}
/****************************************************************/
/*								*/
/*	eschdl - escape sequence handler			*/
/*								*/
/*      This routine intercepts escape sequences for the purpose*/
/*	of pass through.					*/
/*								*/
/****************************************************************/
eschdl(o,c)
int c;
FILE  *o;
{
int j,chr;
if(escflg==0)
	{		/* set escflg=1 => ready to receive 2nd seqchar*/
	escflg=1;
	}
else	switch(escflg)
		{
		case 1:		/* second character of escseq 		*/
			switch(c)
				{
				case NINE:
					putc(ESC,o);
					putc(PLD,o);
					escflg=0;
					break;
  				case UCP:
					escflg=2; /*ctrl str pass thru mode=8 */
					lstchr=c;
					putc(ESC,o);
					putc(c,o);
					break;
				default:
					escflg=3;  /* set seq pass thru mode*/
					putc(ESC,o);
					putc(c,o);
					break;
				}
			break;
		case 2:		/* ctrl string pass through mode       	*/
			if((lstchr==ESC) && (c==BSLH))
				{
				escflg=0;
				lstchr=0;
				}
			else lstchr=c;	/* save it for next pass */
			putc(c,o);
			break;
		case 3:
			if(escend(c))
				escflg=0;/* turn off esc handler if at end  */
			putc(c,o);
			break;
		}
return(0);
}
