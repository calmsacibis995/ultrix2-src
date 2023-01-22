/*
 *		@(#)monitor.h	1.1	(ULTRIX)	1/8/85
 */

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

/*
**  MONITOR.H -- globals for the interactive terminal monitor
**
**	Version:
**		@(#)monitor.h	7.1	2/5/81
*/

# include	<stdio.h>
# include	<useful.h>

/* various global names and strings */
char		Qbname[30];	/* pathname of query buffer */
extern char	*Fileset;	/* unique string */
extern char	*Pathname;	/* pathname of INGRES root */

/* flags */
char		Nodayfile;	/* suppress dayfile/prompts */
				/* 0 - print dayfile and prompts
				** 1 - suppress dayfile but not prompts
				** -1 - supress dayfile and prompts
				*/
char		Userdflag;	/* same: user flag */
				/*  the Nodayfile flag gets reset by include();
				**  this is the flag that the user actually
				**  specified (and what s/he gets when in
				**  interactive mode.			*/
char		Autoclear;	/* clear query buffer automatically if set */
char		Notnull;	/* set if the query is not null */
char		Prompt;		/* set if a prompt is needed */
char		Nautoclear;	/* if set, disables the autoclear option */
char		Phase;		/* set if in processing phase */

/* query buffer stuff */
FILE		*Qryiop;	/* the query buffer */
char		Newline;	/* set if last character was a newline */

/* other stuff */
int		Xwaitpid;	/* pid to wait on - zero means none */
int		Error_id;	/* the error number of the last err */

/* \include support stuff */
FILE		*Input;		/* current input file */
int		Idepth;		/* include depth */
char		Oneline;	/* deliver EOF after one line input */
bool		GiveEof;	/* if set, return EOF on next getch */

/* commands to monitor */
# define	C_APPEND	1
# define	C_BRANCH	2
# define	C_CHDIR		3
# define	C_EDIT		4
# define	C_GO		5
# define	C_INCLUDE	6
# define	C_MARK		7
# define	C_LIST		8
# define	C_PRINT		9
# define	C_QUIT		10
# define	C_RESET		11
# define	C_TIME		12
# define	C_EVAL		13
# define	C_WRITE		14
# define	C_SHELL		15
# define	C_SYSTRACE	16
# define	C_SYSRESET	17

/* stuff for querytrap facility */
extern FILE	*Trapfile;
