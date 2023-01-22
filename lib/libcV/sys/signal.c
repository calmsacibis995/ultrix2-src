#ifndef lint
static	char	*sccsid = "@(#)signal.c	1.1	(ULTRIX)	3/31/85";
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
/************************************************************************
 *			Modification History				*
 *									*
 *	David L Ballenger, 30-Mar-1985					*
 * 0001	Change definitions of signal handlers from void (*()) to	*
 *	int (*()).  							*
 *									*
 ************************************************************************/

/*
	signal -- system call emulation for 4.2BSD (VAX version)

	last edit:	13-Jan-1984	D A Gwyn

	NOTE:  Although this module is VAX-specific, it should be
	possible to adapt it to other fairly clean implementations of
	4.2BSD.  The difficulty lies in avoiding the automatic restart
	of certain system calls when the signal handler returns.  I use
	here a trick first described by Donn Seeley of UCSD Chem. Dept.
*/

#include	<errno.h>
#include	<signal.h>

extern int	_sigvec();
extern long	sigsetmask();

extern		etext;


typedef int	bool;			/* Boolean data type */
#define	false	0
#define	true	1

static int	(*handler[NSIG])() =	/* "current handler" memory */
	{
	BADSIG				/* initially, unknown state */
	};
static bool	inited = false;		/* for initializing above */

typedef struct
	{
	int		(*sv_handler)();/* signal handler */
	long		sv_mask;	/* signal mask to apply */
	bool		sv_onstack;	/* take on signal stack */
	}	sigvec;			/* for _sigvec() */

static int	catchsig();
static int	ret_eintr();


int	(*
signal( sig, func )			/* returns previous handler */
	)()
	register int	sig;		/* signal affected */
	register int	(*func)();	/* new handler */
	{
	register int	(*retval)();	/* previous handler value */
	sigvec		oldsv;		/* previous state */
	sigvec		newsv;		/* state being set */

	if ( func >= (int (*)())&etext
       /* && func != SIG_DFL && func != SIG_IGN */
	   )	{
		errno = EFAULT;
		return BADSIG;		/* error */
		}

	/* cancel pending signals */
	newsv.sv_handler = SIG_IGN;
	newsv.sv_mask = 0L;
	newsv.sv_onstack = false;
	if ( _sigvec( sig, &newsv, &oldsv ) != 0 )
		return BADSIG;		/* error */

	/* C language provides no good way to initialize handler[] */
	if ( !inited )			/* once only */
		{
		register int	i;

		for ( i = 1; i < NSIG; ++i )
			handler[i] = BADSIG;	/* initialize */

		inited = true;
		}

	/* the first time for this sig, get state from the system */
	if ( (retval = handler[sig - 1]) == BADSIG )
		retval = oldsv.sv_handler;

	handler[sig - 1] = func;	/* keep track of state */

	if ( func == SIG_DFL )
		newsv.sv_handler = SIG_DFL;
	else if ( func != SIG_IGN )
		newsv.sv_handler = catchsig;	/* actual sig catcher */

	if ( func != SIG_IGN		/* sig already being ignored */
	  && _sigvec( sig, &newsv, (sigvec *)0 ) != 0
	   )
		return BADSIG;		/* error */

	return retval;			/* previous handler */
	}


/* # bytes to skip at the beginning of C ret_eintr() function code: */
#define	OFFSET	2			/* for VAX .word reg_mask */

/* PC will be pointing at a syscall if it is to be restarted: */
typedef unsigned char	opcode;		/* one byte long */
#define	SYSCALL		((opcode)0xBC)	/* VAX CHMK instruction */
#define	IMMEDIATE	((opcode)0x8F)	/* VAX immediate addressing */
/* restartable syscall codes: */
/* syscall codes < 64 are assembled in "literal" mode */
#define	READ		((opcode)3)
#define	WRITE		((opcode)4)
#define	IOCTL		((opcode)54)
/* syscall codes > 63 are assembled in "immediate" mode; there is
   actually another 0 byte after the syscall code too */
#define	WAIT		((opcode)84)
#define	READV		((opcode)120)
#define	WRITEV		((opcode)121)

typedef struct				/* VAX-specific format */
	{
	bool		sc_onstack;	/* sigstack state to restore */
	long		sc_mask;	/* signal mask to restore */
	long		sc_sp;		/* sp to restore */
	opcode		*sc_pc;		/* pc to retore */
	long		sc_ps;		/* psl to restore */
	}	sigcontext;		/* interrupted context */


/*ARGSUSED*/
static int
catchsig( sig, code, scp )		/* signal interceptor */
	register int		sig;	/* signal number */
	long			code;	/* code for SIGILL, SIGFPE */
	register sigcontext	*scp;	/* -> interrupted context */
	{
	register int		(*uhandler)();	/* user handler */
	register opcode		*pc;	/* for snooping instructions */
	sigvec			oldsv;	/* previous state */
	sigvec			newsv;	/* state being set */

	/* at this point, sig is blocked */

	uhandler = handler[sig - 1];

	/* UNIX System V usually wants the state reset to SIG_DFL */
	if ( sig != SIGILL && sig != SIGTRAP /* && sig != SIGPWR */ )
		{
		handler[sig - 1] = newsv.sv_handler = SIG_DFL;
		newsv.sv_mask = 0L;
		newsv.sv_onstack = false;
		(void)_sigvec( sig, &newsv, &oldsv );
		}

	(void)sigsetmask( scp->sc_mask );	/* restore old mask */

	/* at this point, sig is not blocked, usually have SIG_DFL;
	   a longjmp may safely be taken by the user signal handler */

	(*uhandler)( sig );		/* user signal handler */

	/* must now avoid restarting certain system calls */
	pc = scp->sc_pc;
	if ( *pc++ == SYSCALL
	  && (*pc == READ || *pc == WRITE || *pc == IOCTL
	   || *pc++ == IMMEDIATE
	   && (*pc == WAIT || *pc == READV || *pc == WRITEV)
	     )
	   )
		scp->sc_pc = (opcode *)((char *)ret_eintr + OFFSET);

	/* return here restores interrupted context */
	}


static int
ret_eintr()				/* substitute for system call */
{
	errno = EINTR;
	return -1;
}
