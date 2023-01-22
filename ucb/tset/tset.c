#ifndef lint
static	char	*sccsid = "@(#)tset.c	1.6	ULTRIX	10/3/86";
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

/*
 * Modification History
 *
 * 06-Jun-86  Miriam Amos
 *		Clean up tset - remove unnecessary code, ifdefs, etc.
 *
 */


/*
 * static char sccsid[] = "@(#)tset.c	1.6 (Berkeley) 8/11/83";
 */

/*
**  TSET -- set terminal modes
**
**	This program does sophisticated terminal initialization.
**	I recommend that you include it in your .start_up or .login
**	file to initialize whatever terminal you are on.
**
**	There are several features:
**
**	A special file or sequence (as controlled by the ttycap file)
**	is sent to the terminal.
**
**	Mode bits are set on a per-terminal_type basis (much better
**	than UNIX itself).  This allows special delays, automatic
**	tabs, etc.
**
**	Erase and Kill characters can be set to whatever you want.
**	Default is to change erase to control-H on a terminal which
**	can overstrike, and leave it alone on anything else.  Kill
**	is always left alone unless specifically requested.  These
**	characters can be represented as "^X" meaning control-X;
**	X is any character.
**
**	Terminals which are dialups or plugboard types can be aliased
**	to whatever type you may have in your home or office.  Thus,
**	if you know that when you dial up you will always be on a
**	TI 733, you can specify that fact to tset.  You can represent
**	a type as "?type".  This will ask you what type you want it
**	to be -- if you reply with just a newline, it will default
**	to the type given.
**
**	The current terminal type can be queried.
**
**	Usage:
**		tset [-] [-EC] [-eC] [-kC] [-s] [-h] [-u] [-r]
**			[-m [ident] [test baudrate] :type]
**			[-Q] [-I] [-S] [type]
**
**		In systems with environments, use:
**			eval `tset -s ...`
**		Actually, this doesn't work in old csh's.
**		Instead, use:
**			tset -s ... > tset.tmp
**			source tset.tmp
**			rm tset.tmp
**		or:
**			set noglob
**			set term=(`tset -S ....`)
**			setenv TERM $term[1]
**			setenv TERMCAP "$term[2]"
**			unset term
**			unset noglob
**
**	Positional Parameters:
**		type -- the terminal type to force.  If this is
**			specified, initialization is for this
**			terminal type.
**
**	Flags:
**		- -- report terminal type.  Whatever type is
**			decided on is reported.  If no other flags
**			are stated, the only affect is to write
**			the terminal type on the standard output.
**		-r -- report to user in addition to other flags.
**		-EC -- set the erase character to C on all terminals
**			except those which cannot backspace (e.g.,
**			a TTY 33).  C defaults to control-H.
**		-eC -- set the erase character to C on all terminals.
**			C defaults to control-H.  If neither -E or -e
**			are specified, the erase character is set to
**			control-H if the terminal can both backspace
**			and not overstrike (e.g., a CRT).  If the erase
**			character is NULL (zero byte), it will be reset
**			to '#' if nothing else is specified.
**		-kC -- set the kill character to C on all terminals.
**			Default for C is control-X.  If not specified,
**			the kill character is untouched; however, if
**			not specified and the kill character is NULL
**			(zero byte), the kill character is set to '@'.
**		-iC -- reserved for setable interrupt character.
**		-qC -- reserved for setable quit character.
**		-m -- map the system identified type to some user
**			specified type. The mapping can be baud rate
**			dependent. This replaces the old -d, -p flags.
**			(-d type  ->  -m dialup:type)
**			(-p type  ->  -m plug:type)
**			Syntax:	-m identifier [test baudrate] :type
**			where: ``identifier'' is whatever is found in
**			/etc/ttys for this port, (abscence of an identifier
**			matches any identifier); ``test'' may be any combination
**			of  >  =  <  !  @; ``baudrate'' is as with stty(1);
**			``type'' is the actual terminal type to use if the
**			mapping condition is met. Multiple maps are scanned
**			in order and the first match prevails.
**		-n -- If the new tty driver from UCB is available, this flag
**			will activate the new options for erase and kill
**			processing. This will be different for printers
**			and crt's. For crts, if the baud rate is < 1200 then
**			erase and kill don't remove characters from the screen.
**		-s -- output setenv commands for TERM.  This can be
**			used with
**				`tset -s ...`
**			and is to be prefered to:
**				setenv TERM `tset - ...`
**			because -s sets the TERMCAP variable also.
**		-S -- Similar to -s but outputs 2 strings suitable for
**			use in csh .login files as follows:
**				set noglob
**				set term=(`tset -S .....`)
**				setenv TERM $term[1]
**				setenv TERMCAP "$term[2]"
**				unset term
**				unset noglob
**		-Q -- be quiet.  don't output 'Erase set to' etc.
**		-I -- don't do terminal initialization (is & if
**			strings).
**		-v -- On virtual terminal systems, don't set up a
**			virtual terminal.  Otherwise tset will tell
**			the operating system what kind of terminal you
**			are on (if it is a known terminal) and fix up
**			the output of -s to use virtual terminal sequences.
**
**	Files:
**		/etc/ttys
**			contains a terminal id -> terminal type
**			mapping; used when any user mapping is specified,
**			or the environment doesn't have TERM set.
**		/etc/termcap
**			a terminal_type -> terminal_capabilities
**			mapping.
**
**	Return Codes:
**		-1 -- couldn't open ttycap.
**		1 -- bad terminal type, or standard output not tty.
**		0 -- ok.
**
**	Defined Constants:
**		DIALUP -- the type code for a dialup port.
**		PLUGBOARD -- the type code for a plugboard port.
**		BACKSPACE -- control-H, the default for -e.
**		CTRL('X') -- control-X, the default for -k.
**		OLDERASE -- the system default erase character.
**		OLDKILL -- the system default kill character.
**		FILEDES -- the file descriptor to do the operation
**			on, nominally 1 or 2.
**		STDOUT -- the standard output file descriptor.
**		GTTYN -- defines file containing generalized ttynames
**			and compiles code to look there.
**
**	Requires:
**		Routines to handle ttys, and ttycap.
**
**	Trace Flags:
**		none
**
**	Diagnostics:
**		Bad flag
**			An incorrect option was specified.
**		Too few args
**			more command line arguments are required.
**		Unexpected arg
**			wrong type of argument was encountered.
**		Cannot open ...
**			The specified file could not be openned.
**		Type ... unknown
**			An unknown terminal type was specified.
**		Erase set to ...
**			Telling that the erase character has been
**			set to the specified character.
**		Kill set to ...
**			Ditto for kill
**		Erase is ...    Kill is ...
**			Tells that the erase/kill characters were
**			wierd before, but they are being left as-is.
**		Not a terminal
**			Set if FILEDES is not a terminal.
**
**	Compilation Instructions:
**		cc -n -O tset.c -ltermlib
**		mv a.out tset
**		chown bin tset
**		chmod 4755 tset
**
**
*/

#define curerase mode.sg_erase
#define curkill mode.sg_kill
#define olderase oldmode.sg_erase
#define oldkill oldmode.sg_kill

#define GTTYN		"/etc/ttys"

#include <ttyent.h>
#include <sgtty.h>
#include <stdio.h>
#include <signal.h>

#define YES		1
#define NO		0

#undef CTRL
#define CTRL(x)	(x ^ 0100)
#define BACKSPACE	(CTRL('H'))
#define CHK(val, dft)	(val<=0 ? dft : val)
#define isdigit(c)	(c >= '0' && c <= '9')
#define isalnum(c)	(c > ' ' && !(index("<@=>!:|\177", c)) )
#define OLDERASE	'#'
#define OLDKILL	'@'

#define FILEDES		2	/* do gtty/stty on this descriptor */
#define STDOUT		1	/* output of -s/-S to this descriptor */

#define USAGE	"usage: tset [-] [-nrsIQS] [-eC] [-kC] [-m [ident][test speed]:type] [type]\n"

#define DIALUP		"dialup"
#define PLUGBOARD	"plugboard"
#define DEFTYPE		"unknown"

#define NOTTY		0

/*
 * Baud Rate Conditionals
 */
#define ANY		0
#define GT		1
#define EQ		2
#define LT		4
#define GE		(GT|EQ)
#define LE		(LT|EQ)
#define NE		(GT|LT)
#define ALL		(GT|EQ|LT)

#define NMAP		10

struct	map {
	char *Ident;
	char Test;
	char Speed;
	char *Type;
} map[NMAP];

struct map *Map = map;

/* This should be available in an include file */
struct
{
	char	*string;
	int	speed;
	int	baudrate;
} speeds[] = {
	"0",	B0,	0,
	"50",	B50,	50,
	"75",	B75,	75,
	"110",	B110,	110,
	"134",	B134,	134,
	"134.5",B134,	134,
	"150",	B150,	150,
	"200",	B200,	200,
	"300",	B300,	300,
	"600",	B600,	600,
	"1200",	B1200,	1200,
	"1800",	B1800,	1800,
	"2400",	B2400,	2400,
	"4800",	B4800,	4800,
	"9600",	B9600,	9600,
	"exta",	EXTA,	19200,
	"extb",	EXTB,	38400,
	0,
};

char	Erase_char;	/* new erase character */
char	Kill_char;	/* new kill character */
char	Specialerase;	/* set => Erase_char only on terminals with backspace */

char	*Ttyid = NOTTY;	/* terminal identifier */
char	*TtyType;	/* type of terminal */
char	*DefType;	/* default type if none other computed */
char	*NewType;	/* mapping identifier based on old flags */
int	Mapped;		/* mapping has been specified */
int	DoSetenv;	/* output setenv commands */
int	BeQuiet;	/* be quiet */
int	NoInit;		/* don't output initialization string */
int	IsReset;	/* invoked as reset */
int	Report;		/* report current type */
int	Ureport;	/* report to user */
int	RepOnly;	/* report only */
int	CmndLine;	/* output full command lines (-s option) */
int	Ask;		/* ask user for termtype */
int	DoVirtTerm = YES;	/* Set up a virtual terminal */
int	PadBaud;	/* Min rate of padding needed */

#define CAPBUFSIZ	1024
char	Capbuf[CAPBUFSIZ];	/* line from /etc/termcap for this TtyType */
char	*Ttycap;	/* termcap line from termcap or environ */

char	Aliasbuf[128];
char	*Alias[16];

struct delay
{
	int	d_delay;
	int	d_bits;
};

#include "tset.delays.h"

struct sgttyb	mode;
struct sgttyb	oldmode;


main(argc, argv)
int	argc;
char	*argv[];
{
	char		buf[256];
	char		termbuf[32];
	auto char	*bufp;
	register char	*p;
	char		*command;
	register int	i;
	int		Break;
	int		Not;
	char		*nextarg();
	char		*mapped();
	extern char	*rindex();
	extern char	*getenv();
	char		*stypeof();
	extern char	*ttyname();
	extern char	*tgetstr();
	char		bs_char;
	int		csh;
	int		settle;
	int		setmode();
	extern		prc();
	extern char	PC;
	extern short	ospeed;
	int		lmode;
	int		ldisc;

	ioctl(FILEDES, TIOCLGET, &lmode);
	ioctl(FILEDES, TIOCGETD, &ldisc);

	if (gtty(FILEDES, &mode) < 0)
	{
		prs("Not a terminal\n");
		exit(1);
	}
	bmove(&mode, &oldmode, sizeof mode);
	ospeed = mode.sg_ospeed & 017;
	signal(SIGINT, setmode);
	signal(SIGQUIT, setmode);
	signal(SIGTERM, setmode);

	if (command = rindex(argv[0], '/'))
		command++;
	else
		command = argv[0];
	if (sequal(command, "reset") )
	{
	/*
	 * reset the teletype mode bits to a sensible state.
	 * Copied from the program by Kurt Shoens & Mark Horton.
	 * Very useful after crapping out in raw.
	 */
		struct tchars tbuf;
		struct ltchars ltc;

		if (ldisc == NTTYDISC)
		{
			ioctl(FILEDES, TIOCGLTC, &ltc);
			ltc.t_suspc = CHK(ltc.t_suspc, CTRL('Z'));
			ltc.t_dsuspc = CHK(ltc.t_dsuspc, CTRL('Y'));
			ltc.t_rprntc = CHK(ltc.t_rprntc, CTRL('R'));
			ltc.t_flushc = CHK(ltc.t_flushc, CTRL('O'));
			ltc.t_werasc = CHK(ltc.t_werasc, CTRL('W'));
			ltc.t_lnextc = CHK(ltc.t_lnextc, CTRL('V'));
			ioctl(FILEDES, TIOCSLTC, &ltc);
		}
		ioctl(FILEDES, TIOCGETC, &tbuf);
		tbuf.t_intrc = CHK(tbuf.t_intrc, CTRL('?'));
		tbuf.t_quitc = CHK(tbuf.t_quitc, CTRL('\\'));
		tbuf.t_startc = CHK(tbuf.t_startc, CTRL('Q'));
		tbuf.t_stopc = CHK(tbuf.t_stopc, CTRL('S'));
		tbuf.t_eofc = CHK(tbuf.t_eofc, CTRL('D'));
		/* brkc is left alone */
		ioctl(FILEDES, TIOCSETC, &tbuf);
		mode.sg_flags &= ~(RAW |CBREAK |VTDELAY|ALLDELAY);
		mode.sg_flags |= XTABS|ECHO|CRMOD|ANYP;
		curerase = CHK(curerase, OLDERASE);
		curkill = CHK(curkill, OLDKILL);
		BeQuiet = YES;
		IsReset = YES;
	}
	else if (argc == 2 && sequal(argv[1], "-"))
		RepOnly = YES;
	argc--;

	/* scan argument list and collect flags */
	while (--argc >= 0)
	{
		p = *++argv;
		if (*p == '-')
		{
			if (*++p == NULL)
				Report = YES; /* report current terminal type */
			else while (*p) switch (*p++)
			{

			  case 'n':
				ldisc = NTTYDISC;
				if (ioctl(FILEDES, TIOCSETD, &ldisc)<0)
					fatal("ioctl ", "new");
				continue;

			  case 'r':	/* report to user */
				Ureport = YES;
				continue;

			  case 'E':	/* special erase: operate on all but TTY33 */
				Specialerase = YES;
				/* explicit fall-through to -e case */

			  case 'e':	/* erase character */
				if (*p == NULL)
					Erase_char = -1;
				else
				{
					if (*p == '^' && p[1] != NULL)
						Erase_char = CTRL(*++p);
					else
						Erase_char = *p;
					p++;
				}
				continue;

			  case 'k':	/* kill character */
				if (*p == NULL)
					Kill_char = CTRL('X');
				else
				{
					if (*p == '^' && p[1] != NULL)
						Kill_char = CTRL(*++p);
					else
						Kill_char = *p;
					p++;
				}
				continue;

			  case 'd':	/* dialup type */
				NewType = DIALUP;
				goto mapold;

			  case 'p':	/* plugboard type */
				NewType = PLUGBOARD;

mapold:				Map->Ident = NewType;
				Map->Test = ALL;
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				Map->Type = p;
				Map++;
				Mapped = YES;
				p = "";
				continue;

			  case 'm':	/* map identifier to type */
				/* This code is very loose. Almost no
				** syntax checking is done!! However,
				** illegal syntax will only produce
				** weird results.
				*/
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				if (isalnum(*p))
				{
					Map->Ident = p;	/* identifier */
					while (isalnum(*p)) p++;
				}
				else
					Map->Ident = "";
				Break = NO;
				Not = NO;
				while (!Break) switch (*p)
				{
					case NULL:
						p = nextarg(argc--, argv++);
						continue;

					case ':':	/* mapped type */
						*p++ = NULL;
						Break = YES;
						continue;

					case '>':	/* conditional */
						Map->Test |= GT;
						*p++ = NULL;
						continue;

					case '<':	/* conditional */
						Map->Test |= LT;
						*p++ = NULL;
						continue;

					case '=':	/* conditional */
					case '@':
						Map->Test |= EQ;
						*p++ = NULL;
						continue;
					
					case '!':	/* invert conditions */
						Not = ~Not;
						*p++ = NULL;
						continue;

					case 'B':	/* Baud rate */
						p++;
						/* intentional fallthru */
					default:
						if (isdigit(*p) || *p == 'e')
						{
							Map->Speed = baudrate(p);
							while (isalnum(*p) || *p == '.')
								p++;
						}
						else
							Break = YES;
						continue;
				}
				if (Not)	/* invert sense of test */
				{
					Map->Test = (~(Map->Test))&ALL;
				}
				if (*p == NULL)
				{
					p = nextarg(argc--, argv++);
				}
				Map->Type = p;
				p = "";
				Map++;
				Mapped = YES;
				continue;

			  case 's':	/* output setenv commands */
				DoSetenv = YES;
				CmndLine = YES;
				continue;

			  case 'S':	/* output setenv strings */
				DoSetenv = YES;
				CmndLine = NO;
				continue;

			  case 'Q':	/* be quiet */
				BeQuiet = YES;
				continue;

			  case 'I':	/* no initialization */
				NoInit = YES;
				continue;

			  case 'A':	/* Ask user */
				Ask = YES;
				continue;
			
			  case 'v':	/* no virtual terminal */
				DoVirtTerm = NO;
				continue;

			  default:
				*p-- = NULL;
				fatal("Bad flag -", p);
			}
		}
		else
		{
			/* terminal type */
			DefType = p;
		}
	}

	if (DefType)
	{
		if (Mapped)
		{
			Map->Ident = "";	/* means "map any type" */
			Map->Test = ALL;	/* at all baud rates */
			Map->Type = DefType;	/* to the default type */
		}
		else
			TtyType = DefType;
	}

	/*
	 * Get rid of $TERMCAP, if it's there, so we get a real
	 * entry from /etc/termcap.  This prevents us from being
	 * fooled by out of date stuff in the environment, and
	 * makes tabs work right on CB/Unix.
	 */
	bufp = getenv("TERMCAP");
	if (bufp && *bufp != '/')
		strcpy(bufp-8, "NOTHING");	/* overwrite only "TERMCAP" */
	/* get current idea of terminal type from environment */
	if (!Mapped && TtyType == 0)
		TtyType = getenv("TERM");

	/* determine terminal id if needed */
	if (!RepOnly && Ttyid == NOTTY && TtyType == 0)
		Ttyid = ttyname(FILEDES);

	/* If still undefined, look at /etc/ttytype */
	if (TtyType == 0)
	{
		TtyType = stypeof(Ttyid);
	}

	/* If still undefined, use DEFTYPE */
	if (TtyType == 0)
	{
		TtyType = DEFTYPE;
	}

	/* check for dialup or other mapping */
	if (Mapped)
		TtyType = mapped(TtyType);

	/* TtyType now contains a pointer to the type of the terminal */
	/* If the first character is '?', ask the user */
	if (TtyType[0] == '?')
	{
		Ask = YES;
		TtyType++;
		if (TtyType[0] == '\0')
			TtyType = DEFTYPE;
	}
	if (Ask)
	{
		prs("TERM = (");
		prs(TtyType);
		prs(") ");
		flush();

		/* read the terminal.  If not empty, set type */
		i = read(2, termbuf, sizeof termbuf - 1);
		if (i > 0)
		{
			if (termbuf[i - 1] == '\n')
				i--;
			termbuf[i] = '\0';
			if (termbuf[0] != '\0')
				TtyType = termbuf;
		}
	}

	/* get terminal capabilities */
	if (!(Alias[0] && isalias(TtyType))) {
		switch (tgetent(Capbuf, TtyType))
		{
		  case -1:
			prs("Cannot find termcap\n");
			flush();
			exit(-1);

		  case 0:
			prs("Type ");
			prs(TtyType);
			prs(" unknown\n");
			flush();
			if (DoSetenv)
			{
				TtyType = DEFTYPE;
				tgetent(Capbuf, TtyType);
			}
			else
				exit(1);
		}
	}
	Ttycap = Capbuf;

	if (!RepOnly)
	{
		/* determine erase and kill characters */
		if (Specialerase && !tgetflag("bs"))
			Erase_char = 0;
		bufp = buf;
		p = tgetstr("kb", &bufp);
		if (p == NULL || p[1] != '\0')
			p = tgetstr("bc", &bufp);
		if (p != NULL && p[1] == '\0')
			bs_char = p[0];
		else if (tgetflag("bs"))
			bs_char = BACKSPACE;
		else
			bs_char = 0;
		if (Erase_char == 0 && !tgetflag("os") && curerase == OLDERASE)
		{
			if (tgetflag("bs") || bs_char != 0)
				Erase_char = -1;
		}
		if (Erase_char < 0)
			Erase_char = (bs_char != 0) ? bs_char : BACKSPACE;

		if (curerase == 0)
			curerase = OLDERASE;
		if (Erase_char != 0)
			curerase = Erase_char;

		if (curkill == 0)
			curkill = OLDKILL;
		if (Kill_char != 0)
			curkill = Kill_char;

		/* set modes */
		PadBaud = tgetnum("pb");	/* OK if fails */
		for (i=0; speeds[i].string; i++)
			if (speeds[i].baudrate == PadBaud) {
				PadBaud = speeds[i].speed;
				break;
			}
		setdelay("dC", CRdelay, CRbits, &mode.sg_flags);
		setdelay("dN", NLdelay, NLbits, &mode.sg_flags);
		setdelay("dB", BSdelay, BSbits, &mode.sg_flags);
		setdelay("dF", FFdelay, FFbits, &mode.sg_flags);
		setdelay("dT", TBdelay, TBbits, &mode.sg_flags);
		if (tgetflag("UC") || (command[0] & 0140) == 0100)
			mode.sg_flags |= LCASE;
		else if (tgetflag("LC"))
			mode.sg_flags &= ~LCASE;
		mode.sg_flags &= ~(EVENP | ODDP | RAW);
		mode.sg_flags &= ~CBREAK;
		if (tgetflag("EP"))
			mode.sg_flags |= EVENP;
		if (tgetflag("OP"))
			mode.sg_flags |= ODDP;
		if ((mode.sg_flags & (EVENP | ODDP)) == 0)
			mode.sg_flags |= EVENP | ODDP;
		mode.sg_flags |= CRMOD | ECHO | XTABS;
		if (tgetflag("NL"))	/* new line, not line feed */
			mode.sg_flags &= ~CRMOD;
		if (tgetflag("HD"))	/* half duplex */
			mode.sg_flags &= ~ECHO;
		if (tgetflag("pt"))	/* print tabs */
			mode.sg_flags &= ~XTABS;
		if (ldisc == NTTYDISC)
		{
			lmode |= LCTLECH;	/* display ctrl chars */
			if (tgetflag("hc"))
			{	/** set printer modes **/
				lmode &= ~(LCRTBS|LCRTERA|LCRTKIL);
				lmode |= LPRTERA;
			}
			else
			{	/** set crt modes **/
				if (!tgetflag("os"))
				{
					lmode &= ~LPRTERA;
					lmode |= LCRTBS;
					if (mode.sg_ospeed >= B1200)
						lmode |= LCRTERA|LCRTKIL;
				}
			}
		}
		ioctl(FILEDES, TIOCLSET, &lmode);

		/* get pad character */
		bufp = buf;
		if (tgetstr("pc", &bufp) != 0)
			PC = buf[0];

		/* output startup string */
		if (!NoInit)
		{
			if (oldmode.sg_flags&(XTABS|CRMOD))
			{
				oldmode.sg_flags &= ~(XTABS|CRMOD);
				setmode(-1);
			}
			if (settabs()) {
				settle = YES;
				flush();
			}
			bufp = buf;
			if (tgetstr(IsReset? "rs" : "is", &bufp) != 0)
			{
				tputs(buf, 0, prc);
				settle = YES;
				flush();
			}
			bufp = buf;
			if (tgetstr(IsReset? "rf" : "if", &bufp) != 0)
			{
				cat(buf);
				settle = YES;
			}
			if (settle)
			{
				prc('\r');
				flush();
				sleep(1);	/* let terminal settle down */
			}
		}


		setmode(0);	/* set new modes, if they've changed */

		/* set up environment for the shell we are using */
		/* (this code is rather heuristic, checking for $SHELL */
		/* ending in the 3 characters "csh") */
		csh = NO;
		if (DoSetenv)
		{
			char *sh;

			if ((sh = getenv("SHELL")) && (i = strlen(sh)) >= 3)
			{
				if ((csh = sequal(&sh[i-3], "csh")) && CmndLine)
					write(STDOUT, "set noglob;\n", 12);
			}
			if (!csh)
				/* running Bourne shell */
				write(STDOUT, "export TERMCAP TERM;\n", 21);
		}
	}

	/* report type if appropriate */
	if (DoSetenv || Report || Ureport)
	{
		/* if type is the short name, find first alias (if any) */
		makealias(Ttycap);
		if (sequal(TtyType, Alias[0]) && Alias[1]) {
			TtyType = Alias[1];
		}

		if (DoSetenv)
		{
			if (csh)
			{
				if (CmndLine)
					write(STDOUT, "setenv TERM ", 12);
				write(STDOUT, TtyType, strlen(TtyType));
				write(STDOUT, " ", 1);
				if (CmndLine)
					write(STDOUT, ";\n", 2);
			}
			else
			{
				write(STDOUT, "TERM=", 5);
				write(STDOUT, TtyType, strlen(TtyType));
				write(STDOUT, ";\n", 2);
			}
		}
		else if (Report)
		{
			write(STDOUT, TtyType, strlen(TtyType));
			write(STDOUT, "\n", 1);
		}
		if (Ureport)
		{
			prs("Terminal type is ");
			prs(TtyType);
			prs("\n");
			flush();
		}

		if (DoSetenv)
		{
			if (csh)
			{
				if (CmndLine)
					write(STDOUT, "setenv TERMCAP '", 16);
			}
			else
				write(STDOUT, "TERMCAP='", 9);
			wrtermcap(Ttycap);
			if (csh)
			{
				if (CmndLine)
				{
					write(STDOUT, "';\n", 3);
					write(STDOUT, "unset noglob;\n", 14);
				}
			}
			else
				write(STDOUT, "';\n", 3);
		}
	}

	if (RepOnly)
		exit(0);

	/* tell about changing erase and kill characters */
	reportek("Erase", curerase, olderase, OLDERASE);
	reportek("Kill", curkill, oldkill, OLDKILL);
	exit(0);
}

/*
 * Set the hardware tabs on the terminal, using the ct (clear all tabs),
 * st (set one tab) and ch (horizontal cursor addressing) capabilities.
 * This is done before if and is, so they can patch in case we blow this.
 */
settabs()
{
	char caps[100];
	char *capsp = caps;
	char *clear_tabs, *set_tab, *set_column, *set_pos;
	char *tg_out, *tgoto();
	int columns, lines, c;

	clear_tabs = tgetstr("ct", &capsp);
	set_tab = tgetstr("st", &capsp);
	set_column = tgetstr("ch", &capsp);
	if (set_column == 0)
		set_pos = tgetstr("cm", &capsp);
	columns = tgetnum("co");
	lines = tgetnum("li");

	if (clear_tabs && set_tab) {
		prc('\r');	/* force to be at left margin */
		tputs(clear_tabs, 0, prc);
	}
	if (set_tab) {
		for (c=8; c<columns; c += 8) {
			/* get to that column. */
			tg_out = "OOPS";	/* also returned by tgoto */
			if (set_column)
				tg_out = tgoto(set_column, 0, c);
			if (*tg_out == 'O' && set_pos)
				tg_out = tgoto(set_pos, c, lines-1);
			if (*tg_out != 'O')
				tputs(tg_out, 1, prc);
			else {
				prc(' '); prc(' '); prc(' '); prc(' ');
				prc(' '); prc(' '); prc(' '); prc(' ');
			}
			/* set the tab */
			tputs(set_tab, 0, prc);
		}
		prc('\r');
		return 1;
	}
	return 0;
}

setmode(flag)
int	flag;
/* flag serves several purposes:
 *	if called as the result of a signal, flag will be > 0.
 *	if called from terminal init, flag == -1 means reset "oldmode".
 *	called with flag == 0 at end of normal mode processing.
 */
{
	struct sgttyb *ttymode;

	if (flag < 0)	/* unconditionally reset oldmode (called from init) */
		ttymode = &oldmode;
	else if (!bequal(&mode, &oldmode, sizeof mode))
		ttymode = &mode;
	else		/* don't need it */
	ttymode = (struct sgttyb *)0;
	
	if (ttymode)
	{
		ioctl(FILEDES, TIOCSETN, ttymode);     /* don't flush */
	}
	if (flag > 0)	/* trapped signal */
		exit(1);
}

reportek(name, new, old, def)
char	*name;
char	old;
char	new;
char	def;
{
	register char	o;
	register char	n;
	register char	*p;
	char		buf[32];
	char		*bufp;

	if (BeQuiet)
		return;
	o = old;
	n = new;

	if (o == n && n == def)
		return;
	prs(name);
	if (o == n)
		prs(" is ");
	else
		prs(" set to ");
	bufp = buf;
	if (tgetstr("kb", &bufp) > 0 && n == buf[0] && buf[1] == NULL)
		prs("Backspace\n");
	else if (n == 0177)
		prs("Delete\n");
	else
	{
		if (n < 040)
		{
			prs("Ctrl-");
			n ^= 0100;
		}
		p = "x\n";
		p[0] = n;
		prs(p);
	}
	flush();
}




setdelay(cap, dtab, bits, flags)
char		*cap;
struct delay	dtab[];
int		bits;
int		*flags;
{
	register int	i;
	register struct delay	*p;
	extern short	ospeed;

	/* see if this capability exists at all */
	i = tgetnum(cap);
	if (i < 0)
		i = 0;
	/* No padding at speeds below PadBaud */
	if (PadBaud > ospeed)
		i = 0;

	/* clear out the bits, replace with new ones */
	*flags &= ~bits;

	/* scan dtab for first entry with adequate delay */
	for (p = dtab; p->d_delay >= 0; p++)
	{
		if (p->d_delay >= i)
		{
			p++;
			break;
		}
	}

	/* use last entry if none will do */
	*flags |= (--p)->d_bits;
}


prs(s)
char	*s;
{
	while (*s != '\0')
		prc(*s++);
}


char	OutBuf[256];
int	OutPtr;

prc(c)
char	c;
{
	OutBuf[OutPtr++] = c;
	if (OutPtr >= sizeof OutBuf)
		flush();
}

flush()
{
	if (OutPtr > 0)
		write(2, OutBuf, OutPtr);
	OutPtr = 0;
}


cat(file)
char	*file;
{
	register int	fd;
	register int	i;
	char		buf[BUFSIZ];

	fd = open(file, 0);
	if (fd < 0)
	{
		prs("Cannot open ");
		prs(file);
		prs("\n");
		flush();
		return;
	}

	while ((i = read(fd, buf, BUFSIZ)) > 0)
		write(FILEDES, buf, i);

	close(fd);
}



bmove(from, to, length)
char	*from;
char	*to;
int	length;
{
	register char	*p, *q;
	register int	i;

	i = length;
	p = from;
	q = to;

	while (i-- > 0)
		*q++ = *p++;
}



bequal(a, b, len)	/* must be same thru len chars */
char	*a;
char	*b;
int	len;
{
	register char	*p, *q;
	register int	i;

	i = len;
	p = a;
	q = b;

	while ((*p == *q) && --i > 0)
	{
		p++; q++;
	}
	return ((*p == *q) && i >= 0);
}

sequal(a, b)	/* must be same thru NULL */
char	*a;
char	*b;
{
	register char *p = a, *q = b;

	while (*p && *q && (*p == *q))
	{
		p++; q++;
	}
	return (*p == *q);
}

makealias(buf)
char	*buf;
{
	register int i;
	register char *a;
	register char *b;

	Alias[0] = a = Aliasbuf;
	b = buf;
	i = 1;
	while (*b && *b != ':') {
		if (*b == '|') {
			*a++ = NULL;
			Alias[i++] = a;
			b++;
		}
		else
			*a++ = *b++;
	}
	*a = NULL;
	Alias[i] = NULL;
}

isalias(ident)	/* is ident same as one of the aliases? */
char	*ident;
{
	char **a = Alias;

	if (*a)
		while (*a)
			if (sequal(ident, *a))
				return(YES);
			else
				a++;
	return(NO);
}

char *
stypeof(ttyid)
char	*ttyid;
{
	static char	typebuf[50];
	static char	namebuf[50];
	register char	*PortType;
	register char	*PortName;
	register char	*TtyId;
	register char	*p;
	register FILE	*f;
	register struct ttyent *t;

	if (ttyid == NOTTY)
		return (DEFTYPE);
	f = fopen(GTTYN, "r");
	if (f == NULL)
		return (DEFTYPE);

	/* split off end of name */
	TtyId = ttyid;
	while (*ttyid)
		if (*ttyid++ == '/')
			TtyId = ttyid;

	/* scan the file */
	if (t = getttynam(TtyId)) {
		strcpy(typebuf, t->ty_type);
		p = PortType = typebuf;
		while (*p && isalnum(*p))
			p++;
		*p++ = NULL;


		strcpy(namebuf, t->ty_name);
		PortName = p = namebuf;
		/* put NULL at end of name */
		while (*p && isalnum(*p))
			p++;
		*p = NULL;

		/* check match on port name */
		if (sequal(PortName, TtyId))	/* found it */
		{
			/* get aliases from termcap entry */
			if (Mapped && tgetent(Capbuf, PortType) > 0) {
				makealias(Capbuf);
				if (sequal(Alias[0], PortType) && Alias[1])
					PortType = Alias[1];
			}
			return(PortType);
		}
	}
	return (DEFTYPE);
}

/*
 * routine to output the string for the environment TERMCAP variable
 */
#define	WHITE(c)	(c == ' ' || c == '\t')
char delcap[128][2];
int ncap = 0;

wrtermcap(bp)
char *bp;
{
	char buf[CAPBUFSIZ];
	char *p = buf;
	char *tp;
	char *putbuf();
	int space, empty;

	/* discard names with blanks */
/** May not be desireable ? **/
	while (*bp && *bp != ':') {
		if (*bp == '|') {
			tp = bp+1;
			space = NO;
			while (*tp && *tp != '|' && *tp != ':') {
				space = (space || WHITE(*tp) );
				tp++;
			}
			if (space) {
				bp = tp;
				continue;
			}
		}
		*p++ = *bp++;
	}
/**/

	while (*bp) {
		switch (*bp) {
		case ':':	/* discard empty, cancelled  or dupl fields */
			tp = bp+1;
			empty = YES;
			while (*tp && *tp != ':') {
				empty = (empty && WHITE(*tp) );
				tp++;
			}
			if (empty || cancelled(bp+1)) {
				bp = tp;
				continue;
			}
			break;

		case ' ':	/* no spaces in output */
			p = putbuf(p, "\\040");
			bp++;
			continue;

		case '!':	/* the shell thinks this is history */
			p = putbuf(p, "\\041");
			bp++;
			continue;

		case ',':	/* the shell thinks this is history */
			p = putbuf(p, "\\054");
			bp++;
			continue;

		case '"':	/* no quotes in output */
			p = putbuf(p, "\\042");
			bp++;
			continue;

		case '\'':	/* no quotes in output */
			p = putbuf(p, "\\047");
			bp++;
			continue;

		case '`':	/* no back quotes in output */
			p = putbuf(p, "\\140");
			bp++;
			continue;

		case '\\':
		case '^':	/* anything following is OK */
			*p++ = *bp++;
		}
		*p++ = *bp++;
		;
	}
	*p++ = ':';	/* we skipped the last : with the : lookahead hack */
	write (STDOUT, buf, p-buf);
}

cancelled(cap)
char	*cap;
{
	register int i;

	for (i = 0; i < ncap; i++)
	{
		if (cap[0] == delcap[i][0] && cap[1] == delcap[i][1])
			return (YES);
	}
	/* delete a second occurrance of the same capability */
	delcap[ncap][0] = cap[0];
	delcap[ncap][1] = cap[1];
	ncap++;
	return (cap[2] == '@');
}

char *
putbuf(ptr, str)
char	*ptr;
char	*str;
{
	char buf[20];

	while (*str) {
		switch (*str) {
		case '\033':
			ptr = putbuf(ptr, "\\E");
			str++;
			break;
		default:
			if (*str <= ' ') {
				sprintf(buf, "\\%03o", *str);
				ptr = putbuf(ptr, buf);
				str++;
			} else
				*ptr++ = *str++;
		}
	}
	return (ptr);
}


baudrate(p)
char	*p;
{
	char buf[8];
	int i = 0;

	while (i < 7 && (isalnum(*p) || *p == '.'))
		buf[i++] = *p++;
	buf[i] = NULL;
	for (i=0; speeds[i].string; i++)
		if (sequal(speeds[i].string, buf))
			return (speeds[i].speed);
	return (-1);
}

char *
mapped(type)
char	*type;
{
	extern short	ospeed;
	int	match;

	Map = map;
	while (Map->Ident)
	{
		if (*(Map->Ident) == NULL || sequal(Map->Ident, type) || isalias(Map->Ident))
		{
			match = NO;
			switch (Map->Test)
			{
				case ANY:	/* no test specified */
				case ALL:
					match = YES;
					break;
				
				case GT:
					match = (ospeed > Map->Speed);
					break;

				case GE:
					match = (ospeed >= Map->Speed);
					break;

				case EQ:
					match = (ospeed == Map->Speed);
					break;

				case LE:
					match = (ospeed <= Map->Speed);
					break;

				case LT:
					match = (ospeed < Map->Speed);
					break;

				case NE:
					match = (ospeed != Map->Speed);
					break;
			}
			if (match)
				return (Map->Type);
		}
		Map++;
	}
	/* no match found; return given type */
	return (type);
}


char *
nextarg(argc, argv)
int	argc;
char	*argv[];
{
	if (argc <= 0)
		fatal ("Too few args: ", *argv);
	if (*(*++argv) == '-')
		fatal ("Unexpected arg: ", *argv);
	return (*argv);
}

fatal (mesg, obj)
char	*mesg;
char	*obj;
{
	prs (mesg);
	prs (obj);
	prc ('\n');
	prs (USAGE);
	flush();
	exit(1);
}
