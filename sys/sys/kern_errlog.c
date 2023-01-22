


#ifndef lint
static char *sccsid = "@(#)kern_errlog.c	1.11	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *                                                                      *
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 *   File name: kern_errlog.c
 *
 *   Source file description: 
 *   	This file contains the error logging routines, used to log
 *	hardware and general kernel errors into a global kernel buffer.
 *	These errors are then read by a user-level daemon process,
 *	via the special device, /dev/errlog.
 *
 *   Functions:
 *	erropen, errclose	open/close special char device
 *	erread, errwrite	read/write special char device
 *	errioctl, errsel	ioctl/select on special char device
 *	initrtn			initialize error log buffer and ptrs
 *	ealloc			allocate error log buffer space
 *	movptr			called on ioctl(MOVPTR)
 *	schedeldaemon		set timeout for eldaemon
 *	eldaemon		do selwakeup on elprocp
 *
 *   Modification history:
 *
 *	June 9 86	bjg
 *		Added savetime to save system startup time.
 *
 *	June 3 86	bjg
 *		Added support to disable/re-enable missed error messages;
 *		reran lint.
 *
 *
 *	Apr 22 86	bjg
 *		Check asize passed in ealloc for negative value.
 *
 *	Apr 12 86	bjg
 *		Check that 0 < amt < BSIZE in erread()
 *
 *	Apr 02 86	bjg
 *		Change debug statements
 *
 *	Apr 02 86	pmk
 *		Add schedeldaemon and eldaemon routines
 *
 *	Mar 18 86	bjg
 *		Add timer in for Missed Error message - 15 minutes
 *
 *	Mar 12 86	bjg
 *		Remove vax dependent routines; put into vax/errlog.c
 *		Remove sbi routines; put into ka780.c and ka8600.c
 *
 *	Mar 05 86 	bjg
 *		Change overwrite capability for Severe errors to use
 *		of 2K buffer for Severes; add MUTEX (multiprocessor);
 *		Remove msgbuf.h; added newline to Missed Error msg;
 *		Remove elbuf and defines (put in errlog.h);
 *		Add parameter to logmck for severe
 *
 *	Feb 19 86	bjg
 *		Add sbi error logging; add check for memenable; add
 *		overwrite capability for Severe errors
 *
 *	Jan 20 86	bjg
 *		Initial Creation
 * 
 *	
 *
 */

/* Error Logging Code */


#include "../h/ioctl.h"
#include "../h/types.h"
#include "../h/param.h"
#include "../vax/cpu.h"
#include "../vax/mtpr.h"	/* to get system id */
#include "../h/uio.h"
#include "../h/file.h"
#include "../h/errno.h"
#include "../h/dir.h"
#include "../h/user.h"		/* time.h included in user.h */
#include "../h/proc.h"
#include "../h/kernel.h"	/* time defined */
#include "../h/errlog.h"
#include "../h/interlock.h"
#include "../h/cpuconf.h"


/* ALLOC will be executed at raised IPL */
#define ALLOC() \
    loc = elbuf.in; \
    elbuf.in = ptr;

#define SUCCESS 0
#define ERROR  -1
#define FULL (char *) 0

struct proc *elprocp;	/* ptr to proc struc for elcs */
int elwakeup = 0;	/* errlog wakeup flag */
int err_cnt = 0;	/* cntr for Missed Error Mesgs */
short nowarn = 0;	/* Missed Error warning not desired */
long savetime = 0;	/* Save system startup time */
int el_debug = 0;	/* debug flag */
extern long sbi_there;	/* defined in autoconf.c */

/* erropen and errclose do nothing; just satisfy caller */
erropen(dev, mode)
dev_t dev;
int mode;
{
	return(SUCCESS);
}
errclose()
{
	return(SUCCESS);
}
/*
*
*
* Function: erread(dev,uio)
*
* Function description: Pass error log buffer contents to caller; Invoked
*	by elcsd process.
*
* Arguments: 
*	dev	device (not used)
*	uio	pointer to uio structure
*
* Return value: errno value
*
* Side effects: None
*
*
*/

erread(dev,uio) 
dev_t dev;
struct uio *uio;
{
	int rval = 0;
	int s;
	int amt = 0;

	/* raise IPL */
	s = spl7();
	lock(LOCK_ERRLOG);

	if (elbuf.out == elbuf.le && elbuf.in < elbuf.out) {
		elbuf.out = EL_BEG;
	}

	if (elbuf.in < elbuf.out) {	
		amt = elbuf.le - elbuf.out;
	}
	else if (elbuf.in > elbuf.out) {
		amt = elbuf.in - elbuf.out;
	}

	if (elwakeup == 1)
		elwakeup = 0;

	/* lower IPL */
	unlock(LOCK_ERRLOG);
	splx(s);


	if ((amt > 0) && (amt < EL_BSIZE)) {	/* some data to be output */
		rval = uiomove((caddr_t)elbuf.out, amt, UIO_READ, uio);
		if (rval != 0) {
			cprintf("\n error logger read: uiomove error\n");

		} 
	}
	else if ((amt < 0) || (amt >= EL_BSIZE)) {
		cprintf("\nerrlog ptrs scribbled; reinitializing\n");
		s = spl7();
		lock(LOCK_ERRLOG);
		initrtn();
		unlock(LOCK_ERRLOG);
		splx(s);
	        rval = -1;
        } 

if (el_debug) {
		cprintf("\n return from erread:");
		cprintf("\n i is %x", elbuf.in);
		cprintf("\n o is %x", elbuf.out);
		cprintf("\n le is %x", elbuf.le);
}

	return(rval);
}
/*
*
*
* Function: errwrite(dev,uio)
*
* Function description: Insert error log packet into error log buffer; 
*	Invoked by privileged user process, via llogerr() library routine.
*
* Arguments:
*	dev	device (not used)
*	uio	pointer to uio structure
*
* Return value: errno value 
*
* Side effects: None.
*
*
*/

errwrite(dev,uio) 
dev_t dev;
struct uio *uio; 
{
	struct iovec *iov = uio->uio_iov;
	struct el_rec *ealloc();
	int rval = 0;
	register struct el_rec *elp;
	struct llog_msg {
		short class;
		char line[256];
	} llogmsg;
	int save_len;

	save_len = iov->iov_len;
	rval = uiomove((caddr_t)&llogmsg, iov->iov_len, UIO_WRITE, uio);
	if (rval != 0) {
		return(rval);
	}
	elp = ealloc(save_len, EL_PRILOW);
if (el_debug) {
	cprintf("\n return from ealloc, elp is %x", elp);
}

	/* copy buffer contents to elbuf*/

	if (elp != EL_FULL) {

		LSUBID(elp, llogmsg.class, EL_UNDEF, EL_UNDEF, EL_UNDEF,
			EL_UNDEF, EL_UNDEF);

		elp->el_body.elmsg.msg_len = save_len - (sizeof (short));
		bcopy((caddr_t)llogmsg.line, (caddr_t)elp->el_body.elmsg.msg_asc, (unsigned)(save_len - (sizeof(short))));


if (el_debug) {
		
		cprintf("\n i is %x", elbuf.in);
		cprintf("\n o is %x", elbuf.out);
		cprintf("\n le is %x", elbuf.le);
}

		EVALID(elp);
	}
	return(rval);

}
/*
*
*
* Function:errioctl(dev, com, data, f_flag)
*
* Function description: Perform special functions for elcsd
*
* Arguments: 
*	dev	device (not used)
*	com 	command type specified by user(e.g. ELMOVPTR)
*	data	argument passed by user, or loc to return value to user
*	f_flag	not used
*
* Return value: SUCCESS or ERROR
*
* Side effects: None
*
*
*/

errioctl(dev, com, data, f_flag)
dev_t dev;
int com;
caddr_t data; 
int f_flag;
{
	int rval = 0;
	int s;
	struct elparam {
		int pid;
		int sid;
	};
	struct elparam *elp;
	struct proc *p;
	extern struct proc *pfind();
	static long savepid = 0L;	/* save pid of elcs */
	static long savesid = 0L;	/* system id */

	switch(com) {

	case ELMOVPTR: 
		if (u.u_procp->p_pid == savepid) {
			rval = movptr(data);
		}
		else {
			rval = ERROR;
		}
		break;

	case ELSETPID:
		elp = (struct elparam *) data;
		savepid = (long) elp->pid;
		if (savepid > 0L) {
			p = pfind(savepid);
			if (p != (struct proc *)0) {
				elprocp = p;
				if (elbuf.in != elbuf.out) {
					schedeldaemon();
				}
				/* return sid */
				if (savesid == 0L) {
					savesid = mfpr(SID);
				}
				elp->sid = savesid;
				rval = 0;
			}
			else rval = ERROR;
		}
		else rval = ERROR;
		break;

	case ELGETPID:
		* (long *) data =  savepid;
		rval = 0;
		break;

	case ELREINIT:
		s = spl7();
		lock(LOCK_ERRLOG);
		initrtn();
		unlock(LOCK_ERRLOG);
		splx(s);
		rval = 0;
		break;

	case ELCLRPID:
		savepid = 0L;
		elprocp = (struct proc*) 0;
		rval = 0;
		break;	

	case ELWARNOFF:
		nowarn = 1;
		break;

	case ELWARNON:
		err_cnt = 0;	/* reset error count */
		nowarn = 0;
		break;

	case ELGETTIME:
		* (long *) data = savetime;
		break;
	default:
		rval = ERROR;
		break;
	}
	return(rval);
}
/*
*
*
* Function: movptr(data)
*
* Function description: Called by elcsd; update error log buffer
*	 pointer to reflect amount of data successfully writen to disk.
*	
* Arguments: 
*	data	pointer to amount of data read/written to disk.
*
* Return value: SUCCESS or ERROR
*
* Side effects: None
*
*
*/
		
movptr(data)
caddr_t data;
{
	int size;	
	int s;
	int rval = 0;
	int csize;

	size = * (int *) data;
	csize = EL_END - elbuf.out;
if (el_debug) {
	cprintf("\n movptr: SIZE is %x", size);
}
	if (size > 0 && size < csize) {
		/* raise IPL */
		s = spl7();
		lock(LOCK_ERRLOG);

		elbuf.out += size;
		if (elbuf.out == elbuf.in) {
			initrtn();
		}

		/* lower IPL */
		unlock(LOCK_ERRLOG);
		splx(s);

		if (elbuf.out == elbuf.le) {
		/* more data, possibly, at BEG */
			schedeldaemon();
		}
		rval = SUCCESS;
	}
	else {
		rval = ERROR;
	}
if (el_debug) {
	cprintf("\n Return from movptr");
	cprintf("\n in is %x", elbuf.in);
	cprintf("\n out is %x", elbuf.out);
	cprintf("\n le is %x", elbuf.le);
}
	return(rval);
	
}
/*
*
*
* Function: errsel(dev, rw)
*
* Function description:  Set return value so that select code sets 
*	select bits properly/wakes up elcsd if data in error log buffer.
*
* Arguments: 
*	dev 	device (not used)
*	rw	request type (read or write)
*
* Return value:
*
* Side effects: None
*
*
*/

errsel(dev,rw)
dev_t dev;
int rw;
{
	int rval = 0;

	switch(rw) {
	
	case FREAD: 
		if (elbuf.in != elbuf.out) {	/* kqueue not empty */
			rval = 1;
		}
		else {
			rval = 0;
		}
		break;

	case FWRITE:
		rval = EACCES; /* can't select for write */
		break;

	default:
		break;
	}
	return(rval);
}
/*
*
*
* Function: initrtn()
*
* Function description:  Initialize error log buffer pointers; called
*	when first error logged, or when user does ioctl(REINIT).
*
* Arguments: None
*
* Return value: None
*
* Side effects: None
*
* NOTE: This routine MUST be called at ipl 31, and with the
*	multiprocessor lock (LOCK_ERRLOG) locked.  This routine does
*	not raise the ipl and set the lock here, because the caller
*	may already have the ipl raised and the lock set.
*/
initrtn()
{
	extern int errtimer();

	elbuf.in = EL_BEG;
	elbuf.out = EL_BEG;
	elbuf.le = EL_END;
	elbuf.sin = EL_SBEG;
	if (err_cnt > 0) {
		err_cnt = 0;
		untimeout(errtimer, 0);
	}
}
/*
*
*
* Function: ealloc(size, pri)
*
* Function description: Allocate space in the error log buffer for the 
*	caller.  Caller must not use more space than requests.
*
* Arguments: 
*	size 	size of packet to be logged, including all header info,
*			etc. (caller should use sizeof()).
*	pri	priority of packet to be logged (severe, high, low);
*			note: severe packets will not be logged until
*			system is rebooted, but are guaranteed to be logged
*			into reserved space.
*
* Return value: pointer to location in buffer, or EL_FULL if buffer full
*
* Side effects: None
*
*
*/

struct el_rec *ealloc(asize,pri)
int asize;	/* size (in bytes) of buffer */
int pri;
{

	register char *ptr;
	register struct el_rec *elp;
	register int size;
	static seqnum = -1;
	register char *loc;
	int s;
	static count = 0;
	extern int errtimer();

if (el_debug) {
	cprintf("\nealloc: asize is %x", asize); 
}

	/* check if vm is enabled */
#ifdef vax
	if(!mfpr(MAPEN)) {
		return(EL_FULL);
	}
#endif

	if (count == 0) {
		s = spl7();
		lock(LOCK_ERRLOG);
		initrtn();
		unlock(LOCK_ERRLOG);
		splx(s);
		count++;
	}


	/* calculate total size */
	size = asize + EL_MISCSIZE;
	if ((size < EL_MISCSIZE) || (size > EL_MAXRECSIZE)) {
		return(EL_FULL);
	}

	/* Regardless of whether error gets logged, increment seq. number */
	seqnum++;
	
	if (pri != EL_PRISEVERE) { 
		
		/* raise IPL */
		s = spl7();
		lock(LOCK_ERRLOG);
		ptr = elbuf.in+size;
		
		if (elbuf.in >= elbuf.out) { /* insert bet. i and END ?*/
			if (ptr < EL_END) {
				ALLOC();
				if (ptr > elbuf.le) {
					elbuf.le = ptr;
				}
			}
			else {			/* insert bet. BEG. and o ? */
				ptr = EL_BEG + size;
				if (ptr < elbuf.out) {
					elbuf.le = elbuf.in; 
				 	elbuf.in = EL_BEG;
					ALLOC();
				}
	
				else {
					loc = FULL;
				}

			}
		}
		else {  /* in < out; insert bet. in and out ? */
			if (ptr < elbuf.out) {
				ALLOC();
			}
			else if (elbuf.out == elbuf.le) {
				if (ptr < EL_END) {
					elbuf.out = EL_BEG;
					ALLOC();
				}
			}
			else {
				loc = FULL;
			}
		}

		/* lower IPL */
		unlock(LOCK_ERRLOG);
		splx(s);
	}		/* LOG SEVERE */
	else  {   
		s = spl7();
		lock(LOCK_ERRLOG);
		ptr = elbuf.sin + size;
		if (ptr < EL_SEND) {
			loc = elbuf.sin;
			elbuf.sin = ptr;
		}
		else {
			loc = FULL;
		}
		unlock(LOCK_ERRLOG);
		splx(s);
	}


	/* Back at callers IPL */


	if (loc == FULL) {
		err_cnt++;
		if (err_cnt == 1) {
			errtimer();
		}
	}
	else  {
		elp = (struct el_rec *) loc;
		elp->elrhdr.rhdr_reclen = size;
		elp->elrhdr.rhdr_time = time.tv_sec; /*defined in kernel.h */
		elp->elrhdr.rhdr_seqnum = seqnum;
		elp->elrhdr.rhdr_valid = EL_INVALID; /*until validated */
		elp->elrhdr.rhdr_pri = pri;
	}	

if (el_debug) {
		cprintf("\n Exiting ealloc");
		cprintf("\n i is %x", elbuf.in);
		cprintf("\n o is %x", elbuf.out);
		cprintf("\n le is %x", elbuf.le);
}
	return((struct el_rec *) loc);

}
errtimer()
{
	struct timeval timeval;
	if (!nowarn && err_cnt > 0) {
            cprintf("\nErrlog Buffer Full: %d Missed Error(s)\n", err_cnt);
	    timeval.tv_sec = time.tv_sec + 900;	 /* 15 minutes */
	    timeval.tv_usec = 0;
	    timeout(errtimer, 0, hzto(&timeval));	
	}
}

schedeldaemon()
{
	int ipl, s;
	int eldaemon();

	s = splx((ipl = mfpr(IPL)) < 0x8 ? 0x8 : ipl);

	if (elwakeup == 0) {
	    elwakeup = 1;
	    timeout(eldaemon, 0, hz);
	}

	splx(s);
}

eldaemon()
{
	extern int selwakeup();

	selwakeup(elprocp, 0);
}
