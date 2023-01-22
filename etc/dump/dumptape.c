# ifndef lint
static char *sccsid = "@(#)dumptape.c	1.10	(ULTRIX)	10/16/86";
# endif not lint

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

/* ------------------------------------------------------------------------
 * Modification History: /usr/src/etc/dump/dumptape.c
 *
 * 16 Oct 86 -- fries
 *	Added code to properly rewind/ not rewind tape devices.
 *	(based on whether the rewind/no-rewind device was specified)
 *
 * 18 Sep 86 -- fries
 *	Modified code to open files correctly.
 *
 *  9 Sep 86 -- fries
 *	Modified code to signal device open or closed using device_open
 *	flag.
 *
 *  9 May 86 -- fries
 *	Added code to support EOT handling.
 *
 * 28 Apr 86 -- lp
 *	Added n-buffered support (which includes read across tape 
 *	blocks for improved input performance). Fixed eot code
 *	to work with n-buffered.
 *
 * 10 Mar 86 -- fries
 *	Added code to not report tape rewinding if it is already at
 *	EOT mark.
 *
 * 13 Feb 86 -- fries
 *	modified messages to report disk device if output device is
 *	a disk device.
 *
 * 22 Apr 85 -- dlb
 *	Change declaration of dump specific rewind() function to 
 *	rewind_tape() to avoid conflict with declaration of rewind()
 *	in <stdio.h>.
 *
 * 28 Feb 85 -- afd
 *	It is necessary to minimize the number of concurrent processes on
 *	the MicroVAX when dumping to rx50's.  The flag "uvaxrx50" is set
 *	if the B flag (dumping to floppies) is specified.  When "uvaxrx50"
 *	is set, processes kill their parent process just before they
 *	fork; the master process waits until the last child signals it
 *	to exit.  Thus there are never more than 3 dump processes active
 *	at a time.  All modifications for this change are restricted to
 *	3 files: dump.h, dumpmain.c, dumptape.c   and are noted by a
 *	comment containing the word: "uVAXrx50".
 *	
 * 18 Dec 84 -- reilly
 *	Replaced a for loop with a structure copy.  This is a performance
 *	enhancement.
 *
 * 23 Apr 84 -- jmcg
 *	To aid in dumping to fixed-size devices, like floppy or cartridge
 *	disks, added a -B flag that specifies the size of the device in
 *	blocks.  This size will override the other size calcualations.
 *	[This is a band-aid; fixing dump to truly understand end-of-media,
 *	and fixing the rest of the system to consistently signal it, was
 *	left for the future.]
 *
 * 23 Apr 84 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		dumptape.c	1.7 (Berkeley) 5/8/83
 *
 * ------------------------------------------------------------------------
 */

#include "dump.h"
#include <sys/errno.h>
#define MAXASYNC 8

struct atblock {		/* Pointer to malloc'd buffers for I/O */
	char tblock[TP_BSIZE];
};
struct atblock *fblock[MAXASYNC];

int	writesize;		/* Size of malloc()ed buffer for tape */
int	trecno = 0;		/* present # of tape record */
extern int devtyp;		/* device type(Tape or disk ?)  */
extern int ntrec;		/* blocking factor on tape     */
				/* # of tape records per block */
int	maxrasync=0;		/* Read ahead possible */
int	maxwasync=MAXASYNC; 	/* How many write buffers we use */
int	dowasync=0;		/* Non-blocking writes possible */
int 	curbuf=0;		/* Current buffer we're using */

extern int uvaxrx50;		/* uVAXrx50: dump is on uvax to rx50's */
int	blockcount = 0;		/* blocks written on current tape */
struct mtop mt_com;
struct mtop mt_rew = {MTREW, 1};

/*
 * Allocate the buffer for tape operations.
 * ---------------------------------------
 *
 * Depends on global variable ntrec, set from 'b' option in command line.
 * Returns 1 if successful, 0 if failed.
 *
 * For later kernel performance improvement, this buffer should be allocated
 * on a page boundary.
 */
alloctape()
{
	int i, pf, mpf;
	char *mtemp;

	writesize = ntrec * TP_BSIZE;
	mtemp = (char *)malloc(writesize * MAXASYNC);
	if(mtemp == (char *)0)
		return(0);

	/* Clumsy way to page align the buffer - malloc should do this */
	mtemp = (char *) ((unsigned int)mtemp + 
		(1024 - ((unsigned int)mtemp % 1024)));

	for(i=0; i<MAXASYNC; i++) {
		fblock[i] = (struct atblock *)mtemp;
		mtemp += writesize;
	}
	return (fblock[MAXASYNC-1] != NULL);
}

/* Update tape record number, if tape record # > or = to the    */
/* # of blocks to write, then flush tape write buffer to tape   */
taprec(dp)
	char *dp;
{
	register i;

	*(union u_spcl *)(&fblock[curbuf][trecno]) = *(union u_spcl *)dp; 

	trecno++;	/* Bump # of tape block counter */
	spcl.c_tapea++;	/* update record # in header	*/
	if(trecno >= ntrec)	/* If blocking factor met... */
		flusht();	/* Output data to tape	     */
}

#ifdef notdef
int nbufdeep = 0; /* # of read ahead blocks going */
#endif

/* Read blocks from disk and move them to the tape as long as there */
/* are blocks available to move onto the tape			    */
dmpblk(blkno, size)
	daddr_t blkno;
	int size;
{
	int avail, tpblks, dblkno;

	if (size % TP_BSIZE != 0)
		msg("bad size to dmpblk: %d\n", size);
	avail = ntrec - trecno;
	dblkno = fsbtodb(sblock, blkno);
	for (tpblks = size / TP_BSIZE; tpblks > avail; ) {
	if(maxwasync && (curbuf < maxwasync - 1)) {
	  bread(dblkno, (char *)&fblock[curbuf][trecno], TP_BSIZE * tpblks, 1);
	  	spcl.c_tapea += tpblks;
	  	flusht();
		if(trecno != 0) { /* Implied eom */
		dblkno += avail * (TP_BSIZE / DEV_BSIZE);
		tpblks -= avail;
		goto fini;
		} else
		trecno = tpblks - avail;
		return;
	}
	  bread(dblkno, (char *)&fblock[curbuf][trecno], TP_BSIZE * avail,1);
		trecno += avail;
		spcl.c_tapea += avail;
		flusht();
		dblkno += avail * (TP_BSIZE / DEV_BSIZE);
		tpblks -= avail;
		avail = ntrec - trecno;
	}
fini:
	bread(dblkno, (char *)&fblock[curbuf][trecno], TP_BSIZE * tpblks,0);
	trecno += tpblks;
	spcl.c_tapea += tpblks;
	if(trecno >= ntrec)
		flusht();
}

/* Flush Tape data to Drive */
int	nogripe = 0;
int 	wpending[MAXASYNC];

flusht()
{
	register i, si;
	daddr_t d;
	extern int errno;

	errno = trecno = 0;

	if (write(to, (char *)&fblock[curbuf][0], writesize) != writesize){
harderr:
		if (devtyp == PIP){
			msg("Error while writing to Standard Output\n");
			msg("Cannot recover\n");
			dumpabort();
			/* NOTREACHED */
		}
		if (errno == ENOSPC){
		   eom_flag = 1;
		}
		else{
			if (devtyp == TAP){
			   msg("Tape write error on tape %d\n", tapeno);
			   broadcast("TAPE ERROR!\n");
			}
			else{
			   msg("Disk write error on disk %d\n", tapeno);
			   broadcast("DISK ERROR!\n");
			}
			if (query("Do you want to restart?")){
				if (devtyp == TAP){
				   msg("This tape will rewind.  After it is rewound,\n");
				   msg("replace the faulty tape with a new one;\n");
				}
				else
				   msg("Replace this disk with a new one;\n");

				msg("this dump volume will be rewritten.\n");
				/*
				 *	Temporarily change the tapeno identification
				 */
				tapeno--;
				nogripe = 1;
				close_rewind();
				nogripe = 0;
				tapeno++;
				Exit(X_REWRITE);
			} else {
				dumpabort();
				/*NOTREACHED*/
			}
		}
	}
	asize += writesize/density;
	asize += 7;
	blockswritten += ntrec;
	blockcount += ntrec;
	if((eom_flag)||(devblocks && ((devblocks - blockcount) < ntrec))){
		if(dowasync) {
			int k;
			char *tmp;
			for(k=0; k<maxwasync; k++) {
			int p;
			tmp = (char *)&fblock[k][0];
			errno = 0;
			p = wpending[k];
			if(ioctl(to, FIONBDONE, &tmp) != writesize) {
				if(errno == ENOSPC) {
					wpending[k]=1;
				}
			} else
				wpending[k]=0;
			}
			k = 0;
		 	(void) ioctl(to, FIONBUF, &k);
			if(devtyp == TAP)
			  wpending[curbuf]=1;
			if(++curbuf >= maxwasync)
				curbuf = 0;
			k = 0;
			while(k++ < maxwasync) {
			/* write these synchronously */
				errno = 0;
				if(wpending[curbuf]){
				 	mt_com.mt_op    = MTCSE;
					mt_com.mt_count = 1;
					if(ioctl(to, MTIOCTOP, &mt_com) < 0)
						perror("write past eom fail");
				errno = 0;
				if(write(to,(char *)&fblock[curbuf][0], 
					writesize) != writesize)
					printf("wrt err %d\n", errno);
				}
				wpending[curbuf]=0;
				if(++curbuf >= maxwasync)
					curbuf = 0;
			}
			} else if(devtyp == TAP){
				   mt_com.mt_op = MTCSE;
				   mt_com.mt_count = 1;
				   if(ioctl(to, MTIOCTOP, &mt_com) < 0)
				   	perror("write past eom fail");
				if (write(to, (char *)&fblock[curbuf][0], 
					writesize) != writesize)
					goto harderr;
			}

		close_rewind();

		/* uVAXrx50: Added to minimize the # of concurrent processes */
		if (uvaxrx50)
			kill(lparentpid, SIGTERM);
		otape(); /* Child returns */
		eom_flag = errno = 0;
		goto cont;
	}

	if(maxwasync) {
		char *tmp;
		wpending[curbuf]++;
		if(++curbuf >= maxwasync)
			curbuf = 0;
		if(dowasync && wpending[curbuf]) {
			errno = 0;
			tmp = (char *)&fblock[curbuf][0];
			if(ioctl(to, FIONBDONE, &tmp) != writesize) {
				printf("iofail %d %d\n", curbuf, ino);
				if(errno != EINVAL)
					goto harderr;
			}
			wpending[curbuf]=0;
		}
	}
cont: timeest();	/* print % of dump completed & elapsed time */
}

/* Rewinds mag tape and waits for completion */
void rewind_tape()
{
	int	secs;
	int f;

	if (devtyp == PIP)
		return;
#ifdef DEBUG
	msg("Waiting 10 seconds to rewind.\n");
	sleep(10);
#else
	/*
	 *	It takes about 3 minutes, 25secs to rewind 2300' of tape
	 */
	/* Perform device ioctl to determine if tape is presently */
	/* at the BOT mark				          */
	if ((devtyp == TAP) && ( strncmp("/dev/rmt", tape, 8) == 0))
	     msg("Tape rewinding\n", secs);
	
	if(dowasync) { /* Turn off n-buffering */
		int k;
		for(k=0; k<maxwasync; k++) {
			char *tmp;
			tmp = (char *)&fblock[k][0];
			(void) ioctl(to, FIONBDONE, &tmp);
		}
		(void) ioctl(to, FIONBUF, &k);
		dowasync = 0;
	}
	close(to);
	device_open = 0;
#endif
}

/* Rewind Tape, wait til done and request a new tape */
close_rewind()
{
	if (devtyp == PIP)
		return;

	/* Rewind the no-rewind device */
	if((devtyp == TAP) && (strncmp(tape, "/dev/nrmt", 9) == 0))
	  ioctl(to, MTIOCTOP, &mt_rew);

	blockcount = 0;
	rewind_tape();
	if (!nogripe){
		if (devtyp == TAP){
		   msg("Change Tapes: Mount tape #%d\n", tapeno+1);
		   broadcast("CHANGE TAPES!\7\7\n");
		}
		else{
		   msg("Change Disks: Mount disk #%d\n", tapeno+1);
		   broadcast("CHANGE DISKS!\7\7\n");
		}
	}
	do{
		if (devtyp == TAP){
		   if (query ("Is the new tape mounted and ready to go?"))
		      break;
		}
		else
		   if (query ("Is the new disk mounted and ready to go?"))
		      break;
		if (query ("Do you want to abort?")){
			dumpabort();
			/*NOTREACHED*/
		}
	} while (1);
}

/*
 *	We implement taking and restoring checkpoints on
 *	the tape level.
 *	When each tape is opened, a new process is created by forking; this
 *	saves all of the necessary context in the parent.  The child
 *	continues the dump; the parent waits around, saving the context.
 *	If the child returns X_REWRITE, then it had problems writing that tape;
 *	this causes the parent to fork again, duplicating the context, and
 *	everything continues as if nothing had happened.
 */
otape()
{
	int	parentpid;
	int	childpid;
	int	status;
	int	waitpid;
	int	sig_ign_parent();
	int	interrupt();

	parentpid = getpid();
	lparentpid = parentpid;		/* uVAXrx50 */

	/* uVAXrx50: Added to minimize the # of concurrent processes */
	if (uvaxrx50 && parentpid != mpid)
	signal(SIGTERM, SIG_DFL);	/* allow processes to exit */

    restore_check_point:
	signal(SIGINT, interrupt);
	/*
	 *	All signals are inherited...
	 */
	childpid = fork();
	if (childpid < 0){
		msg("Context save fork fails in parent %d\n", parentpid);
		Exit(X_ABORT);
	}
	if (childpid != 0){
		/*
		 *	PARENT:
		 *	save the context by waiting
		 *	until the child doing all of the work returns.
		 *	don't catch the interrupt 
		 */

		/* uVAXrx50: Added to minimize the # of concurrent processes */
		if (uvaxrx50 && getpid() == mpid)
			{
			/* EMT will signal master pid for normal exit,
			 * QUIT will siganl master pid for abort exit.
			 */
			signal(SIGEMT, exitmaster);
			signal(SIGQUIT, abortmaster);
			};

		signal(SIGINT, SIG_IGN);
#ifdef TDEBUG
		msg("Tape: %d; parent process: %d child process %d\n",
			tapeno+1, parentpid, childpid);
#endif TDEBUG
		for (;;){
			waitpid = wait(&status);
			if (waitpid != childpid){
				msg("Parent %d waiting for child %d has another child %d return\n",
					parentpid, childpid, waitpid);
			} else
				break;
		}
		if (status & 0xFF){
			/* uVAXrx50: Added to minimize the # of concurrent processes */
			if (uvaxrx50 == 0 || status != SIGTERM)	/* uVAXrx50 */
				msg("Child %d returns LOB status %o\n",
					childpid, status&0xFF);
		}
		status = (status >> 8) & 0xFF;
#ifdef TDEBUG
		switch(status){
			case 0:
				msg("Child %d terminated \n", childpid);
				break;
			case X_FINOK:
				msg("Child %d finishes X_FINOK\n", childpid);
				break;
			case X_ABORT:
				msg("Child %d finishes X_ABORT\n", childpid);
				break;
			case X_REWRITE:
				msg("Child %d finishes X_REWRITE\n", childpid);
				break;
			default:
				msg("Child %d finishes unknown %d\n", childpid,status);
				break;
		}
#endif TDEBUG
		switch(status){
			/* uVAXrx50: Added to minimize the # of concurrent processes */
			/* Child was killed. If master, wait to be signaled */ 
			case 0:
				if (uvaxrx50) {
					if (getpid() == mpid) {
						sleep(mpid,PZERO);
					/* In case of accidental wakeup */
						for (;;) ;
					}
					Exit(X_FINOK);
				} else {
					msg("Bad return code from dump: %d\n", status);
					Exit(X_ABORT);
				};
			case X_FINOK:
				Exit(X_FINOK);
			case X_ABORT:
				Exit(X_ABORT);
			case X_REWRITE:
				goto restore_check_point;
			default:
				msg("Bad return code from dump: %d\n", status);
				Exit(X_ABORT);
		}
		/*NOTREACHED*/
	} else {	/* we are the child; just continue */
#ifdef TDEBUG
		sleep(4);	/* allow time for parent's message to get out */
		msg("Child on Tape %d has parent %d, my pid = %d\n",
			tapeno+1, parentpid, getpid());
#endif
around:
		do{
			if (devtyp == PIP)
				to = 1;
			else
			if (usecreate)
				to = open(tape,O_RDWR|O_CREAT, 0666);
			else
				to = statchk(tape,O_RDWR); /* disk or tape */


			/* If device open error... */
			if (to < 0){
			if(usecreate){
			  if(!query("Can not create file. Do you want to retry the create?"))
			    dumpabort();
			}
			else if (to == -3)dumpabort();

			} else break;
		} while (1);
		{
			int maxcnt = MAXASYNC;
			if(to != 1) {
				if(ioctl(to, FIONBUF, &maxcnt) >= 0) {
					maxwasync = maxcnt;
					dowasync++;
				}
			}
#ifdef notdef
			if(!maxrasync) {
				maxcnt = 2;
				if(ioctl(fi, FIONBUF, &maxcnt) < 0)
					maxrasync = 0;
				else
					maxrasync = maxcnt;
			}
			printf("async status r %d w %d %d\n", maxrasync, maxwasync, dowasync);
#endif
		}

		asize = 0;
		tapeno++;		/* current tape sequence */
		newtape++;		/* new tape signal */
		spcl.c_volume++;
		spcl.c_type = TS_TAPE;
		spclrec();
		if (tapeno > 1){
			if (devtyp == TAP)
			   msg("Tape %d begins with blocks from ino %d\n",
				tapeno, ino);
			else
			   msg("Disk %d begins with blocks from ino %d\n",
				tapeno, ino);
		}
	}
}

/*
 *	uVAXrx50: Added to minimize the # of concurrent processes.
 *	Catches SIGEMT signal for master pid.  Call Exit with normal
 *	exit status.
 */

exitmaster()
{
	Exit(X_FINOK);
}


/*
 *	uVAXrx50: Added to minimize the # of concurrent processes.
 *	Catches SIGQUIT signal for master pid.  Call Exit with abort
 *	exit status.
 */

abortmaster()
{
	Exit(X_ABORT);
}

/*
 *	The parent still catches interrupts, but does nothing with them
 */
sig_ign_parent()
{
	msg("Waiting parent receives interrupt\n");
	signal(SIGINT, sig_ign_parent);
}

dumpabort()
{
	msg("The ENTIRE dump is aborted.\n");

	/* uVAXrx50: Added to minimize the # of concurrent processes */
	if (uvaxrx50)
		kill(mpid, SIGQUIT);	/* signal master pid to exit */

	Exit(X_ABORT);
}

Exit(status)
{
#ifdef TDEBUG
	msg("pid = %d exits with status %d\n", getpid(), status);
#endif TDEBUG
	exit(status);
}
