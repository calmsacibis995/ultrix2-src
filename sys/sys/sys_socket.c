#ifndef lint
static	char	*sccsid = "@(#)sys_socket.c	1.7	(ULTRIX)	3/23/86";
#endif lint

/***********************************************************************
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
 **********************************************************************/

/************************************************************************
 *			Modification History				*
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *									*
 * 09/16/85 -- Larry Cohen						*
 * 		Add 43bsd alpha tape changes  				*
 *									*
 * 16 Apr 85 -- depp							*
 *	Added code to "soo_rw" routine that will properly handle	*
 *	FNDELAY if the file/socket is a named pipe.  This fix will	*
 *	now cause the process not to block in a normal blocking		*
 *	situation and if FNDELAY is set.				*
 *									*
 * 15 Mar 85 -- funding							*
 *	Added named pipe support (re. System V named pipes)		*
 *									*
 *	David L. Ballenger, 28-Nov-1984					*
 * 001	Add fixes so that fstat() calls on pipes will set up		*
 *	st_blksize. This will cause I/O on pipes to be buffered.	*
 *									*
 **********************************************************************/
/*	sys_socket.c	6.1	83/07/29	*/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/ioctl.h"
#include "../h/uio.h"
#include "../h/stat.h"
#include "../h/gnode.h"

#include "../net/if.h"
#include "../net/route.h"

int	soo_rw(), soo_ioctl(), soo_select(), soo_close();
struct	fileops socketops =
    { soo_rw, soo_ioctl, soo_select, soo_close };

soo_rw(fp, rw, uio)
	struct file *fp;
	enum uio_rw rw;
	struct uio *uio;
{
	int soreceive(), sosend();

	if(fp->f_type == DTYPE_PORT){
		register int result, savestate;

		if(rw == UIO_READ){
			savestate = ((struct gnode *)(fp->f_data))
				-> g_rso->so_state;
			if(fp->f_flag & FNDELAY)
				((struct gnode *)(fp->f_data))
				-> g_rso->so_state |= SS_NBIO;
			result=soreceive(((struct gnode *)(fp->f_data))
				-> g_rso, 0, uio, 0, 0);
			((struct gnode *)(fp->f_data))
				-> g_rso->so_state = savestate;
		}
		else{
			savestate = ((struct gnode *)(fp->f_data))
				-> g_wso->so_state;
			if(fp->f_flag & FNDELAY)
				((struct gnode *)(fp->f_data))
				-> g_wso->so_state |= SS_NBIO;
			result = sosend(((struct gnode *)(fp->f_data))
				-> g_wso, 0, uio, 0, 0);
			((struct gnode *)(fp->f_data))
				-> g_wso->so_state = savestate;
		}

		return(result);
	}

	return (
	    (*(rw==UIO_READ?soreceive:sosend))
	      ((struct socket *)fp->f_data, 0, uio, 0, 0));
}

soo_ioctl(fp, cmd, data)
	struct file *fp;
	int cmd;
	register caddr_t data;
{
	register struct socket *so = (struct socket *)fp->f_data;

	if(fp->f_type == DTYPE_PORT){
		if(fp->f_flag & FREAD)
			so = ((struct gnode *)(fp->f_data))->g_rso;
		else
			so = ((struct gnode *)(fp->f_data))->g_wso;
	}
	switch (cmd) {

	case FIONBIO:
		if (*(int *)data)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return (0);

	case FIOASYNC:
		if (*(int *)data)
			so->so_state |= SS_ASYNC;
		else
			so->so_state &= ~SS_ASYNC;
		return (0);

	case FIONREAD:
		*(int *)data = so->so_rcv.sb_cc;
		return (0);

	case SIOCSPGRP:
		so->so_pgrp = *(int *)data;
		return (0);

	case SIOCGPGRP:
		*(int *)data = so->so_pgrp;
		return (0);

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		return (0);
	}
	/*
	 * Interface/routing/protocol specific ioctls:
	 * interface and routing ioctls should have a
	 * different entry since a socket's unnecessary
	 */
#define	cmdbyte(x)	(((x) >> 8) & 0xff)
	if (cmdbyte(cmd) == 'i')
		return (ifioctl(so, cmd, data));
	if (cmdbyte(cmd) == 'r')
		return (rtioctl(cmd, data));
	return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
	    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));
}

soo_select(fp, which)
	struct file *fp;
	int which;
{
	register struct socket *so = (struct socket *)fp->f_data;
	register int s = splnet();

	if(fp->f_type == DTYPE_PORT){
		if(fp->f_flag & FREAD)
			so = ((struct gnode *)(fp->f_data))->g_rso;
		else
			so = ((struct gnode *)(fp->f_data))->g_wso;
	}
	switch (which) {

	case FREAD:
		if (soreadable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_rcv);
		break;

	case FWRITE:
		if (sowriteable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_snd);
		break;
	}
	splx(s);
	return (0);
}

/*ARGSUSED*/
soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{

#ifdef lint
	so = so;
#endif
	bzero((caddr_t)ub, sizeof (*ub));
	return ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,	/*DLB001*/
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0));
}

soo_close(fp)
	struct file *fp;
{
	int error = 0;
	
	if (fp->f_data)
		error = soclose((struct socket *)fp->f_data);
	fp->f_data = 0;
	return (error);
}
