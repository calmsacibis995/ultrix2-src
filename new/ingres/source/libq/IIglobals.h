/*
 *		@(#)IIglobals.h	1.1	(ULTRIX)	1/8/85
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
**  IIGLOBALS.H -- Equel run-time library globals
**
**	In this file are defined the global variables,
**	and manifest constants used in the Equel run-time
**	routines. All globals must start with "II".
**
**	Version:
**		@(#)IIglobals.h	7.2	10/27/81
*/

# include	<pipes.h>
# include	<proc.h>
# include	<resp.h>
# include	<pv.h>

# define	opSHORT		1
# define	opFLOAT		2
# define	opSTRING	3
# define	opDOUBLE	4
# define	opCHAR		5
# define	opLONG		6
# ifdef PDP
# define	opINT		opSHORT
# else PDP
# define	opINT		opLONG
# endif PDP

# define	EQUEL		'&'
# define	ERRDELIM	'~'

struct resp	IIresp;			/* response structure */
char		*IIproc_name;		/* file name */
int		IIline_no;		/* line no */
int		IIdebug;		/* debug flag */
int		IIingpid;		/* process id of ingres */
char		*IIoptn [9];		/* ingres options */
int		IIin_retrieve;		/* set if inside a retrieve */
int		IIndomains;		/* number of doamins in this retrieve */
int		IIdomains;
int		IInxtdomain;		/* index into source field of buffer */
long		IItupcnt;		/* # tuples which satified last 
					 * update query 
					 */
int		IInewqry;		/* set to indicate that IIwrites have
					 * already set buffers 
					 */
int		IIw_down, IIr_down;	/* pipe descriptors for 
					 * parser communcation 
					 */
int		IIerrflag;		/* error flag. 
					 * Set in IIerror, cleared in IIsync 
					 */
extern char	IIPathname[];		/* initialized by 
					 * IIgetpath() [IIingres.c] 
					 */
int		(*IIprint_err)();	/* wether or not error messgs should
					 * be printed
					 */
int		IIret_err();		/* returns its integer arg for 
					 * (*IIprint_err)()
					 */
int		IIno_err();		/* returns 0 for (*IIprint_err)() */
int		(*IIo_print)();		/* a one value stack for 
					 * temporarily turned off printing of
					 * errors. Done in [IIw_left.c & 
					 * IIw_right.c].
					 */
char		*IImainpr;		/* "/usr/bin/ingres.c" usually, 
					 * [ingres.c]
					 */
int		IISyncs[];		/* Interrupt expectance vector */

/* buffer structure of get(III) */
struct iob
{
	int		fildes;
	int		nleft;
	char		*nextp;
	char		buff[512];
};

struct retsym
{
	char		type;
	char		len;
	char		*addr;
};
struct retsym	IIretsym[MAXDOM];	/* retrieve table */
struct retsym	IIr_sym;		/* type, length fields used
					 * by new equel in IIn_ret and IIn_get
					 */

int	IIinput;	/* File descriptor from which current input is read,
			 * is either IIr_down, or IIr_front
			 */

pb_t	IIpb;		/* the data pipe buffer */
