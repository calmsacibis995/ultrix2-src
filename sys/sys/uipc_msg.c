#ifndef lint
static	char	*sccsid = "@(#)uipc_msg.c	1.5	(ULTRIX)	11/25/85";
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
/**/
/*
 *
 *   File name:
 *
 *	uipc_msg.h
 *
 *
 *   Source file description:
 *
 *	This file contains system calls and associated support functions to 
 *	implement System V IPC.
 *
 *   System Calls
 *
 *	msgctl 		System call to provide control functions to the
 *			application's message queues.
 *
 *	msgget 		System call to get a message queue for write or read.
 *
 *	msgrcv 		System call to receive a message.
 *
 *	msgsnd 		System call to send a message.
 *
 *
 *   Functions:
 *
 *	msgconv 	Convert a user supplied message queue id into a ptr to a
 *			msqid_ds structure.
 *
 *	msgfree  	Free up space and message header, relink pointers on q,
 *			and wakeup anyone waiting for resources.
 *
 *	msginit 	Called by main(main.c) to initialize message queues.
 *
 *
 *   Usage:
 *
 *
 *   Compile:
 *
 *
 *   Modification history:
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 14 Oct 85 -- reilly
 *	Modified the user.h file
 *
 * 29 Jul 85 -- depp
 *	Removed "curpri = PMSG" statements as they are not needed and
 *	they interfere with multiprocessor scheduling.  I'm not sure
 *	why they were in the code to begin with, but they are in System V
 *	v2r2.
 *
 * 23 Jul 85 -- depp
 *	Fixed minor bug in msgfree, a set of {} braces missing.
 *
 * 22 Feb 85 -- depp
 *	New file in system
 *
 */

/*
**	Inter-Process Communication Message Facility.
*/

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/seg.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/errno.h"
#include "../h/map.h"
#include "../h/ipc.h"
#include "../h/msg.h"
#include "../h/systm.h"

extern struct map	msgmap[];	/* msg allocation map */
extern struct msqid_ds	msgque[];	/* msg queue headers */
extern struct msg	msgh[];		/* message headers */
extern struct msginfo	msginfo;	/* message parameters */
extern char ipcmsgbuf[];		/* message buffer */
struct msg		*msgfp;		/* ptr to head of free header list */
extern time_t		time;		/* system idea of date */

struct msqid_ds		*ipcget(),
			*msgconv();

/* Convert bytes to msg segments. */
#define	btoq(X)	((X + msginfo.msgssz - 1) / msginfo.msgssz)

/*
**	msgconv - Convert a user supplied message queue id into a ptr to a
**		msqid_ds structure.
*/

struct msqid_ds *
msgconv(id)
register int	id;
{
	register struct msqid_ds	*qp;	/* ptr to associated q slot */

	qp = &msgque[id % msginfo.msgmni];
	if((qp->msg_perm.mode & IPC_ALLOC) == 0 ||
		id / msginfo.msgmni != qp->msg_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(qp);
}

/*
**	msgctl - Msgctl system call.
*/

msgctl()
{
	register struct a {
		int		msgid,
				cmd;
		struct msqid_ds	*buf;
	}		*uap = (struct a *)u.u_ap;
	struct msqid_ds			ds;	/* queue work area */
	register struct msqid_ds	*qp;	/* ptr to associated q */

	if((qp = msgconv(uap->msgid)) == NULL)
		return;
	u.u_r.r_val1 = 0;
	switch(uap->cmd) {
	case IPC_RMID:
		if(u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			&& !suser())
			return;
		while(qp->msg_first)
			msgfree(qp, NULL, qp->msg_first);
		qp->msg_cbytes = 0;
		if(uap->msgid + msginfo.msgmni < 0)
			qp->msg_perm.seq = 0;
		else
			qp->msg_perm.seq++;
		if(qp->msg_perm.mode & MSG_RWAIT)
			wakeup(&qp->msg_qnum);
		if(qp->msg_perm.mode & MSG_WWAIT)
			wakeup(qp);
		qp->msg_perm.mode = 0;
		return;
	case IPC_SET:
		if(u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			 && !suser())
			return;
		if(copyin(uap->buf, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		if(ds.msg_qbytes > qp->msg_qbytes && !suser())
			return;
		qp->msg_perm.uid = ds.msg_perm.uid;
		qp->msg_perm.gid = ds.msg_perm.gid;
		qp->msg_perm.mode = (qp->msg_perm.mode & ~0777) |
			(ds.msg_perm.mode & 0777);
		qp->msg_qbytes = ds.msg_qbytes;
		qp->msg_ctime = time;
		return;
	case IPC_STAT:
		if(ipcaccess(&qp->msg_perm, MSG_R))
			return;
		if(copyout(qp, uap->buf, sizeof(*qp))) {
			u.u_error = EFAULT;
			return;
		}
		return;
	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
**	msgfree - Free up space and message header, relink pointers on q,
**	and wakeup anyone waiting for resources.
*/

msgfree(qp, pmp, mp)
register struct msqid_ds	*qp;	/* ptr to q of mesg being freed */
register struct msg		*mp,	/* ptr to msg being freed */
				*pmp;	/* ptr to mp's predecessor */
{
	/* Unlink message from the q. */
	if(pmp == NULL)
		qp->msg_first = mp->msg_next;
	else
		pmp->msg_next = mp->msg_next;
	if(mp->msg_next == NULL)
		qp->msg_last = pmp;
	qp->msg_qnum--;
	if(qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~MSG_WWAIT;
		wakeup(qp);
	}

	/* Free up message text. */
	if(mp->msg_ts)	{
		rmfree(msgmap, btoq(mp->msg_ts), mp->msg_spot + 1);
		if (msginfo.msgwnt) {
		    msginfo.msgwnt = 0;
		    wakeup(&msginfo.msgwnt);
		}
	}

	/* Free up header */
	mp->msg_next = msgfp;
	if(msgfp == NULL)
		wakeup(&msgfp);
	msgfp = mp;
}

/*
**	msgget - Msgget system call.
*/

msgget()
{
	register struct a {
		long	key;
		int	msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				s;	/* ipcget status return */

	if((qp = ipcget(uap->key, uap->msgflg, msgque, msginfo.msgmni, 
	  sizeof(*qp), &s)) == NULL) 
		return;

	if(s) {
		/* This is a new queue.  Finish initialization. */
		qp->msg_first = qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_qbytes = msginfo.msgmnb;
		qp->msg_lspid = qp->msg_lrpid = 0;
		qp->msg_stime = qp->msg_rtime = 0;
		qp->msg_ctime = time;
	}
	u.u_r.r_val1 = qp->msg_perm.seq * msginfo.msgmni + (qp - msgque);
}

/*
**	msginit - Called by main(main.c) to initialize message queues.
*/

msginit()
{
	register int		i;	/* loop control */
	register struct msg	*mp;	/* ptr to msg begin linked */

	rminit(msgmap, msginfo.msgseg, 1, "msgmap", msginfo.msgmap);
	for(i = 0, mp = msgfp = msgh;++i < msginfo.msgtql;mp++)
		mp->msg_next = mp + 1;
}

/*
**	msgrcv - Msgrcv system call.
*/

msgrcv()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		long		msgtyp;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;
	register struct msg		*mp,	/* ptr to msg on q */
					*pmp,	/* ptr to mp's predecessor */
					*smp,	/* ptr to best msg on q */
					*spmp;	/* ptr to smp's predecessor */
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				sz;	/* transfer byte count */

	if((qp = msgconv(uap->msqid)) == NULL)
		return;
	if(ipcaccess(&qp->msg_perm, MSG_R))
		return;
	if(uap->msgsz < 0) {
		u.u_error = EINVAL;
		return;
	}
	smp = spmp = NULL;
findmsg:
	pmp = NULL;
	mp = qp->msg_first;
	if(uap->msgtyp == 0)
		smp = mp;
	else
		for(;mp;pmp = mp, mp = mp->msg_next) {
			if(uap->msgtyp > 0) {
				if(uap->msgtyp != mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
				break;
			}
			if(mp->msg_type <= -uap->msgtyp) {
				if(smp && smp->msg_type <= mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
			}
		}
	if(smp) {
		if(uap->msgsz < smp->msg_ts)
			if(!(uap->msgflg & MSG_NOERROR)) {
				u.u_error = E2BIG;
				return;
			} else
				sz = uap->msgsz;
		else
			sz = smp->msg_ts;
		u.u_error = copyout(&smp->msg_type, uap->msgp, 
		  sizeof(smp->msg_type));
		if(u.u_error)
			return;
		if(sz) {
			ndp->ni_segflg = UIO_USERSPACE;
			u.u_error = copyout(ipcmsgbuf+
			 (msginfo.msgssz*smp->msg_spot),
			 (caddr_t)uap->msgp + sizeof(smp->msg_type), sz);
			if(u.u_error)
				return;
		}
		u.u_r.r_val1 = sz;
		qp->msg_cbytes -= smp->msg_ts;
		qp->msg_lrpid = u.u_procp->p_pid;
		qp->msg_rtime = time;

#ifdef notdef	-- not required -- depp
		curpri = PMSG;
#endif notdef

		msgfree(qp, spmp, smp);
		return;
	}
	if(uap->msgflg & IPC_NOWAIT) {
		u.u_error = ENOMSG;
		return;
	}
	qp->msg_perm.mode |= MSG_RWAIT;
	sleep(&qp->msg_qnum, PMSG ) ;
	if(msgconv(uap->msqid) == NULL) {
		u.u_error = EIDRM;
		return;
	}
	goto findmsg;
}

/*
**	msgsnd - Msgsnd system call.
*/

msgsnd()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	register struct msg		*mp;	/* ptr to allocated msg hdr */
	register int			cnt,	/* byte count */
					spot;	/* msg pool allocation spot */
	long				type;	/* msg type */

	if((qp = msgconv(uap->msqid)) == NULL)
		return;
	if(ipcaccess(&qp->msg_perm, MSG_W))
		return;
	if((cnt = uap->msgsz) < 0 || cnt > msginfo.msgmax) {
		u.u_error = EINVAL;
		return;
	}
	u.u_error = copyin(uap->msgp, &type, sizeof(type));
	if(u.u_error)
		return;
	if(type < 1) {
		u.u_error = EINVAL;
		return;
	}
getres:
	/* Be sure that q has not been removed. */
	if(msgconv(uap->msqid) == NULL) {
		u.u_error = EIDRM;
		return;
	}

	/* Allocate space on q, message header, & buffer space. */
	if(cnt + qp->msg_cbytes > qp->msg_qbytes) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		qp->msg_perm.mode |= MSG_WWAIT;
		if(sleep(qp, PMSG | PCATCH)) {
			u.u_error = EINTR;
			qp->msg_perm.mode &= ~MSG_WWAIT;
			wakeup(qp);
			return;
		}
		goto getres;
	}
	if(msgfp == NULL) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		sleep(&msgfp, PMSG );
		goto getres;
	}
	if(cnt && (spot = rmalloc(msgmap, btoq(cnt))) == NULL) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		msginfo.msgwnt++;
		sleep(&msginfo.msgwnt, PMSG);
		goto getres;
	}

	/* Everything is available, copy in text and put msg on q. */
	if(cnt) {
		u.u_error = copyin( (caddr_t)uap->msgp + sizeof(type), 
		  ipcmsgbuf + (msginfo.msgssz * --spot), cnt );
		if(u.u_error) {
			rmfree(msgmap, btoq(cnt), spot + 1);
			if (msginfo.msgwnt) {
			    msginfo.msgwnt = 0;
			    wakeup(&msginfo.msgwnt);
			}
			return;
		}
	}
	qp->msg_qnum++;
	qp->msg_cbytes += cnt;
	qp->msg_lspid = u.u_procp->p_pid;
	qp->msg_stime = time;
	mp = msgfp;
	msgfp = mp->msg_next;
	mp->msg_next = NULL;
	mp->msg_type = type;
	mp->msg_ts = cnt;
	mp->msg_spot = cnt ? spot : -1;
	if(qp->msg_last == NULL)
		qp->msg_first = qp->msg_last = mp;
	else {
		qp->msg_last->msg_next = mp;
		qp->msg_last = mp;
	}
	if(qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;

#ifdef notdef	-- not required		-- depp
		curpri = PMSG;
#endif notdef

		wakeup(&qp->msg_qnum);
	}
	u.u_r.r_val1 = 0;
}
