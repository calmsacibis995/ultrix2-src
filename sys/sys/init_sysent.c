#ifndef lint
static char *sccsid = "@(#)init_sysent.c	1.15	ULTRIX	12/16/86";
#endif

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
 *   Modification history:
 *
 * 15-Jul-86 -- rich
 *	added adjtime
 *
 * 11 June 86 -- Chase
 *	Added getdomainname and setdomainname system calls
 *
 * 02-Apr-86 -- jrs
 *	Added third param on whether call is mp safe
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 *	07 Oct 85 -- reilly
 *	Added the ustat syscall.
 *
 *	25 Jun 85 -- depp
 *	Moved shmsys due to conflict with Berkeley.  Reserved 160-170
 *	for future Ultrix use, with 160-162 for {plock, lockf, ustat}
 *
 *	 4 Jun 85 -- depp
 *	Added uname system call for System V
 *
 *	22 Feb 85 -- depp
 *	Added in new system calls for System V IPC
 *
 *
 */

/*
 * System call switch table.
 */

#include "../h/param.h"
#include "../h/systm.h"

int	nosys();

/* 1.1 processes and protection */
int	sethostid(),gethostid(),sethostname(),gethostname(),getpid();
int	getdomainname(), setdomainname();
int	fork(),rexit(),execv(),execve(),wait();
int	getuid(),setreuid(),getgid(),getgroups(),setregid(),setgroups();
int	getpgrp(),setpgrp();

/* 1.2 memory management */
int	plock();
int	sbrk(),sstk();
int	getpagesize(),smmap(),mremap(),munmap(),mprotect(),madvise(),mincore();

/* 1.3 signals */
int	sigvec(),sigblock(),sigsetmask(),sigpause(),sigstack();
int	kill(), killpg();

/* 1.4 timing and statistics */
int	gettimeofday(),settimeofday();
int	getitimer(),setitimer();

/* 1.5 descriptors */
int	getdtablesize(),dup(),dup2(),close();
int	select(),getdopt(),setdopt(),fcntl(),flock();
int	adjtime();

/* 1.6 resource controls */
int	getpriority(),setpriority(),getrusage(),getrlimit(),setrlimit();
int	setquota(),qquota();

/* 1.7 system operation support */
int	umount(),smount(),swapon();
int	sync(),reboot(),sysacct();

/* 2.1 generic operations */
int	read(),write(),readv(),writev(),ioctl();

/* 2.2 file system */
int	chdir(),chroot();
int	mkdir(),rmdir();
int	creat(),open(),mknod(),unlink(),stat(),fstat(),lstat();
int	chown(),fchown(),chmod(),fchmod(),utimes();
int	link(),symlink(),readlink(),rename();
int	lseek(),truncate(),ftruncate(),saccess(),fsync();

/* 2.3 communications */
int	socket(),bind(),listen(),accept(),connect();
int	socketpair(),sendto(),send(),recvfrom(),recv();
int	sendmsg(),recvmsg(),shutdown(),setsockopt(),getsockopt();
int	getsockname(),getpeername(),pipe();

int	umask();		/* XXX */

/* 2.4 processes */
int	ptrace();

/* 2.5 terminals */

#ifdef COMPAT
/* emulations for backwards compatibility */
#define	compat(n, name, safe)	n, o/**/name, safe

int	owait();		/* now receive message on channel */
int	otime();		/* now use gettimeofday */
int	ostime();		/* now use settimeofday */
int	oalarm();		/* now use setitimer */
int	outime();		/* now use utimes */
int	opause();		/* now use sigpause */
int	onice();		/* now use setpriority,getpriority */
int	oftime();		/* now use gettimeofday */
int	osetpgrp();		/* ??? */
int	otimes();		/* now use getrusage */
int	ossig();		/* now use sigvec, etc */
int	ovlimit();		/* now use setrlimit,getrlimit */
int	ovtimes();		/* now use getrusage */
int	osetuid();		/* now use setreuid */
int	osetgid();		/* now use setregid */
int	ostat();		/* now use stat */
int	ofstat();		/* now use fstat */
#else
#define	compat(n, name, safe)	0, nosys, 0
#endif

/* 2.6 System V IPC stuff */
int msgctl(),msgget(),msgrcv(),msgsnd();
int semctl(),semget(),semop();
int smsys();
int uname();
int ustat();

/* GFS items */
int getmnt(), getdirentries();

/* NFS items */
#ifdef NFS
int nfs_biod(), nfs_getfh(), nfs_svc();
#endif

/* BEGIN JUNK */
#ifdef vax
int	resuba();
#ifdef TRACE
int	vtrace();
#endif
#endif
int	profil();		/* 'cuz sys calls are interruptible */
int	vhangup();		/* should just do in exit() */
int	vfork();		/* awaiting fork w/ copy on write */
int	obreak();		/* awaiting new sbrk */
int	ovadvise();		/* awaiting new madvise */
/* END JUNK */

struct sysent sysent[] = {
	0, nosys, 0,			/*   0 = indir */
	1, rexit, 0,			/*   1 = exit */
	0, fork, 0,			/*   2 = fork */
	3, read, 0,			/*   3 = read */
	3, write, 0,			/*   4 = write */
	3, open, 0,			/*   5 = open */
	1, close, 0,			/*   6 = close */
	compat(0,wait, 0),		/*   7 = old wait */
	2, creat, 0,			/*   8 = creat */
	2, link, 0,			/*   9 = link */
	1, unlink, 0,			/*  10 = unlink */
	2, execv, 0,			/*  11 = execv */
	1, chdir, 0,			/*  12 = chdir */
	compat(0,time, 0),		/*  13 = old time */
	3, mknod, 0,			/*  14 = mknod */
	2, chmod, 0,			/*  15 = chmod */
	3, chown, 0,			/*  16 = chown; now 3 args */
	1, obreak, 0,			/*  17 = old break */
	compat(2,stat, 0),		/*  18 = old stat */
	3, lseek, 0,			/*  19 = lseek */
	0, getpid, 1,			/*  20 = getpid */
	5, smount, 0,			/*  21 = mount */
	1, umount, 0,			/*  22 = umount */
	compat(1,setuid, 0),		/*  23 = old setuid */
	0, getuid, 1,			/*  24 = getuid */
	compat(1,stime, 0),		/*  25 = old stime */
	4, ptrace, 0,			/*  26 = ptrace */
	compat(1,alarm, 0),		/*  27 = old alarm */
	compat(2,fstat, 0),		/*  28 = old fstat */
	compat(0,pause, 0),		/*  29 = opause */
	compat(2,utime, 0),		/*  30 = old utime */
	0, nosys, 0,			/*  31 = was stty */
	0, nosys, 0,			/*  32 = was gtty */
	2, saccess, 0,			/*  33 = access */
	compat(1,nice, 0),		/*  34 = old nice */
	compat(1,ftime, 0),		/*  35 = old ftime */
	0, sync, 0,			/*  36 = sync */
	2, kill, 0,			/*  37 = kill */
	2, stat, 0,			/*  38 = stat */
	compat(2,setpgrp, 0),		/*  39 = old setpgrp */
	2, lstat, 0,			/*  40 = lstat */
	2, dup, 0,			/*  41 = dup */
	0, pipe, 0,			/*  42 = pipe */
	compat(1,times, 0),		/*  43 = old times */
	4, profil, 0,			/*  44 = profil */
	0, nosys, 0,			/*  45 = nosys */
	compat(1,setgid, 0),		/*  46 = old setgid */
	0, getgid, 1,			/*  47 = getgid */
	compat(2,ssig, 0),		/*  48 = old sig */
	0, nosys, 0,			/*  49 = reserved for USG */
	0, nosys, 0,			/*  50 = reserved for USG */
	1, sysacct, 0,			/*  51 = turn acct off/on */
	0, nosys, 0,			/*  52 = old set phys addr */
	0, nosys, 0,			/*  53 = old lock in core */
	3, ioctl, 0,			/*  54 = ioctl */
	1, reboot, 0,			/*  55 = reboot */
	0, nosys, 0,			/*  56 = old mpxchan */
	2, symlink, 0,			/*  57 = symlink */
	3, readlink, 0,			/*  58 = readlink */
	3, execve, 0,			/*  59 = execve */
	1, umask, 1,			/*  60 = umask */
	1, chroot, 0,			/*  61 = chroot */
	2, fstat, 0,			/*  62 = fstat */
	0, nosys, 0,			/*  63 = used internally */
	1, getpagesize, 1,		/*  64 = getpagesize */
	5, mremap, 0,			/*  65 = mremap */
	0, vfork, 0,			/*  66 = vfork */
	0, read, 0,			/*  67 = old vread */
	0, write, 0,			/*  68 = old vwrite */
	1, sbrk, 0,			/*  69 = sbrk */
	1, sstk, 0,			/*  70 = sstk */
	6, smmap, 0,			/*  71 = mmap */
	1, ovadvise, 0,			/*  72 = old vadvise */
	2, munmap, 0,			/*  73 = munmap */
	3, mprotect, 0,			/*  74 = mprotect */
	3, madvise, 0,			/*  75 = madvise */
	1, vhangup, 0,			/*  76 = vhangup */
	compat(2,vlimit, 0),		/*  77 = old vlimit */
	3, mincore, 0,			/*  78 = mincore */
	2, getgroups, 1,		/*  79 = getgroups */
	2, setgroups, 1,		/*  80 = setgroups */
	1, getpgrp, 0,			/*  81 = getpgrp */
	2, setpgrp, 0,			/*  82 = setpgrp */
	3, setitimer, 0,		/*  83 = setitimer */
	0, wait, 0,			/*  84 = wait */
	1, swapon, 0,			/*  85 = swapon */
	2, getitimer, 0,		/*  86 = getitimer */
	2, gethostname, 1,		/*  87 = gethostname */
	2, sethostname, 0,		/*  88 = sethostname */
	0, getdtablesize, 1,		/*  89 = getdtablesize */
	2, dup2, 0,			/*  90 = dup2 */
	2, getdopt, 1,			/*  91 = getdopt */
	3, fcntl, 0,			/*  92 = fcntl */
	5, select, 0,			/*  93 = select */
	2, setdopt, 1,			/*  94 = setdopt */
	1, fsync, 0,			/*  95 = fsync */
	3, setpriority, 0,		/*  96 = setpriority */
	3, socket, 0,			/*  97 = socket */
	3, connect, 0,			/*  98 = connect */
	3, accept, 0,			/*  99 = accept */
	2, getpriority, 0,		/* 100 = getpriority */
	4, send, 0,			/* 101 = send */
	4, recv, 0,			/* 102 = recv */
	0, nosys, 0,			/* 103 = old socketaddr */
	3, bind, 0,			/* 104 = bind */
	5, setsockopt, 0,		/* 105 = setsockopt */
	2, listen, 0,			/* 106 = listen */
	compat(2,vtimes, 0),		/* 107 = old vtimes */
	3, sigvec, 0,			/* 108 = sigvec */
	1, sigblock, 0,			/* 109 = sigblock */
	1, sigsetmask, 0,		/* 110 = sigsetmask */
	1, sigpause, 0,			/* 111 = sigpause */
	2, sigstack, 0,			/* 112 = sigstack */
	3, recvmsg, 0,			/* 113 = recvmsg */
	3, sendmsg, 0,			/* 114 = sendmsg */
#ifdef TRACE
	2, vtrace, 0,			/* 115 = vtrace */
#else
	0, nosys, 0,			/* 115 = nosys */
#endif
	2, gettimeofday, 0,		/* 116 = gettimeofday */
	2, getrusage, 1,		/* 117 = getrusage */
	5, getsockopt, 0,		/* 118 = getsockopt */
#ifdef vax
	1, resuba, 0,			/* 119 = resuba */
#else
	0, nosys, 0,			/* 119 = nosys */
#endif
	3, readv, 0,			/* 120 = readv */
	3, writev, 0,			/* 121 = writev */
	2, settimeofday, 0,		/* 122 = settimeofday */
	3, fchown, 0,			/* 123 = fchown */
	2, fchmod, 0,			/* 124 = fchmod */
	6, recvfrom, 0,			/* 125 = recvfrom */
	2, setreuid, 0,			/* 126 = setreuid */
	2, setregid, 1,			/* 127 = setregid */
	2, rename, 0,			/* 128 = rename */
	2, truncate, 0,			/* 129 = truncate */
	2, ftruncate, 0,		/* 130 = ftruncate */
	2, flock, 0,			/* 131 = flock */
	0, nosys, 0,			/* 132 = nosys */
	6, sendto, 0,			/* 133 = sendto */
	2, shutdown, 0,			/* 134 = shutdown */
	5, socketpair, 0,		/* 135 = socketpair */
	2, mkdir, 0,			/* 136 = mkdir */
	1, rmdir, 0,			/* 137 = rmdir */
	2, utimes, 0,			/* 138 = utimes */
	0, nosys, 0,			/* 139 = used internally */
	2, adjtime, 0,			/* 140 = adjtime */
	3, getpeername, 0,		/* 141 = getpeername */
	2, gethostid, 1,		/* 142 = gethostid */
	2, sethostid, 1,		/* 143 = sethostid */
	2, getrlimit, 1,		/* 144 = getrlimit */
	2, setrlimit, 0,		/* 145 = setrlimit */
	2, killpg, 0,			/* 146 = killpg */
	0, nosys, 0,			/* 147 = nosys */
	2, setquota, 0,			/* 148 = quota */
	4, qquota, 0,			/* 149 = qquota */
	3, getsockname, 0,		/* 150 = getsockname */
	3, msgctl, 0,			/* 151 = msgctl */
	2, msgget, 0,			/* 152 = msgget */
	5, msgrcv, 0,			/* 153 = msgrcv */
	4, msgsnd, 0,			/* 154 = msgsnd */
	4, semctl, 0,			/* 155 = semctl */
	3, semget, 0,			/* 156 = semget */
	3, semop, 0,			/* 157 = semop */
	1, uname, 1,			/* 158 = uname */
	4, smsys, 0,			/* 159 = shared memory */
	1, plock, 0,			/* 160 = plock */
	0, nosys, 0,			/* 161 = lockf (future) */
	2, ustat, 0,			/* 162 = ustat */
	3, getmnt, 0,			/* 163 = getmnt */
	4, getdirentries, 0,		/* 164 = getdirentries */
#ifdef NFS
	0, nfs_biod, 0,			/* 165 = NFS block I/O daemon */
	2, nfs_getfh, 0,		/* 166 = NFS get file handle */
	1, nfs_svc, 0,			/* 167 = NFS server daemon */
#else
	0, nosys, 0,
	0, nosys, 0,
	0, nosys, 0,
#endif
	0, nosys, 0,			/* 168 = old nfs_mount */
	2, getdomainname, 1,		/* 169 = getdomainname */
	2, setdomainname, 0,		/* 170 = setdomainname */
};
int	nsysent = sizeof (sysent) / sizeof (sysent[0]);
