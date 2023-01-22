#ifndef lint
static char *sccsid = "@(#)syscalls.c	1.12	ULTRIX	12/16/86";
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
/*
 *
 *   Modification history:
 *
 * 15-Jul-86 -- rich
 *	added adjtime()
 *
 * 11 June 86 -- Chase
 *	Added the getdomainname and setdomainname system calls.
 *
 * 07 Oct 85 -- reilly
 *      Added the ustat system call.
 *
 * 25 Jun 85 -- depp
 *	Moved shmsys (smsys) system call to prevent conflict with Berkeley.
 *	Also, reserved 160-170 for future Ultrix system calls, with 
 *	160-162 reserved for {plock, lockf, ustat}.
 *
 *  4 Jun 85 -- depp
 *	Added uname system call
 *
 *  9 Apr 85 -- depp
 *	Added System V IPC system calls
 *
 */

/*
 * System call names.
 */
char *syscallnames[] = {
	"indir",		/*   0 = indir */
	"exit",			/*   1 = exit */
	"fork",			/*   2 = fork */
	"read",			/*   3 = read */
	"write",		/*   4 = write */
	"open",			/*   5 = open */
	"close",		/*   6 = close */
	"old wait",		/*   7 = old wait */
	"creat",		/*   8 = creat */
	"link",			/*   9 = link */
	"unlink",		/*  10 = unlink */
	"execv",		/*  11 = execv */
	"chdir",		/*  12 = chdir */
	"old time",		/*  13 = old time */
	"mknod",		/*  14 = mknod */
	"chmod",		/*  15 = chmod */
	"chown",		/*  16 = chown; now 3 args */
	"old sbreak",		/*  17 = old sbreak */
	"old stat",		/*  18 = old stat */
	"lseek",		/*  19 = lseek */
	"getpid",		/*  20 = getpid */
	"mount",		/*  21 = mount */
	"umount",		/*  22 = umount */
	"old setuid",		/*  23 = old setuid */
	"getuid",		/*  24 = getuid */
	"old stime",		/*  25 = old stime */
	"ptrace",		/*  26 = ptrace */
	"old alarm",		/*  27 = old alarm */
	"old fstat",		/*  28 = old fstat */
	"old pause",		/*  29 = old pause */
	"old utime",		/*  30 = old utime */
	"old stty",		/*  31 = old stty */
	"old gtty",		/*  32 = old gtty */
	"access",		/*  33 = access */
	"old nice",		/*  34 = old nice */
	"old ftime",		/*  35 = old ftime */
	"sync",			/*  36 = sync */
	"kill",			/*  37 = kill */
	"stat",			/*  38 = stat */
	"old setpgrp",		/*  39 = old setpgrp */
	"lstat",		/*  40 = lstat */
	"dup",			/*  41 = dup */
	"pipe",			/*  42 = pipe */
	"old times",		/*  43 = old times */
	"profil",		/*  44 = profil */
	"#45",			/*  45 = nosys */
	"old setgid",		/*  46 = old setgid */
	"getgid",		/*  47 = getgid */
	"old sigsys",		/*  48 = old sigsys */
	"#49",			/*  49 = reserved for USG */
	"#50",			/*  50 = reserved for USG */
	"acct",			/*  51 = turn acct off/on */
	"old phys",		/*  52 = old set phys addr */
	"old syslock",		/*  53 = old syslock in core */
	"ioctl",		/*  54 = ioctl */
	"reboot",		/*  55 = reboot */
	"old mpx",		/*  56 = old mpxchan */
	"symlink",		/*  57 = symlink */
	"readlink",		/*  58 = readlink */
	"execve",		/*  59 = execve */
	"umask",		/*  60 = umask */
	"chroot",		/*  61 = chroot */
	"fstat",		/*  62 = fstat */
	"#63",			/*  63 = used internally */
	"getpagesize",		/*  64 = getpagesize */
	"mremap",		/*  65 = mremap */
	"old vfork",		/*  66 = old vfork */
	"old vread",		/*  67 = old vread */
	"old vwrite",		/*  68 = old vwrite */
	"sbrk",			/*  69 = sbrk */
	"sstk",			/*  70 = sstk */
	"mmap",			/*  71 = mmap */
	"old vadvise",		/*  72 = old vadvise */
	"munmap",		/*  73 = munmap */
	"mprotect",		/*  74 = mprotect */
	"madvise",		/*  75 = madvise */
	"vhangup",		/*  76 = vhangup */
	"old vlimit",		/*  77 = old vlimit */
	"mincore",		/*  78 = mincore */
	"getgroups",		/*  79 = getgroups */
	"setgroups",		/*  80 = setgroups */
	"getpgrp",		/*  81 = getpgrp */
	"setpgrp",		/*  82 = setpgrp */
	"setitimer",		/*  83 = setitimer */
	"wait",			/*  84 = wait */
	"swapon",		/*  85 = swapon */
	"getitimer",		/*  86 = getitimer */
	"gethostname",		/*  87 = gethostname */
	"sethostname",		/*  88 = sethostname */
	"getdtablesize",	/*  89 = getdtablesize */
	"dup2",			/*  90 = dup2 */
	"getdopt",		/*  91 = getdopt */
	"fcntl",		/*  92 = fcntl */
	"select",		/*  93 = select */
	"setdopt",		/*  94 = setdopt */
	"fsync",		/*  95 = fsync */
	"setpriority",		/*  96 = setpriority */
	"socket",		/*  97 = socket */
	"connect",		/*  98 = connect */
	"accept",		/*  99 = accept */
	"getpriority",		/* 100 = getpriority */
	"send",			/* 101 = send */
	"recv",			/* 102 = recv */
	"socketaddr",		/* 103 = socketaddr */
	"bind",			/* 104 = bind */
	"setsockopt",		/* 105 = setsockopt */
	"listen",		/* 106 = listen */
	"old vtimes",		/* 107 = old vtimes */
	"sigvec",		/* 108 = sigvec */
	"sigblock",		/* 109 = sigblock */
	"sigsetmask",		/* 110 = sigsetmask */
	"sigpause",		/* 111 = sigpause */
	"sigstack",		/* 112 = sigstack */
	"recvmsg",		/* 113 = recvmsg */
	"sendmsg",		/* 114 = sendmsg */
#ifdef TRACE
	"vtrace",		/* 115 = vtrace */
#else
	"#115",			/* 115 = nosys */
#endif
	"gettimeofday",		/* 116 = gettimeofday */
	"getrusage",		/* 117 = getrusage */
	"getsockopt",		/* 118 = getsockopt */
	"resuba",		/* 119 = resuba */
	"readv",		/* 120 = readv */
	"writev",		/* 121 = writev */
	"settimeofday",		/* 122 = settimeofday */
	"fchown",		/* 123 = fchown */
	"fchmod",		/* 124 = fchmod */
	"recvfrom",		/* 125 = recvfrom */
	"setreuid",		/* 126 = setreuid */
	"setregid",		/* 127 = setregid */
	"rename",		/* 128 = rename */
	"truncate",		/* 129 = truncate */
	"ftruncate",		/* 130 = ftruncate */
	"flock",		/* 131 = flock */
	"portal",		/* 132 = portal */
	"sendto",		/* 133 = sendto */
	"shutdown",		/* 134 = shutdown */
	"socketpair",		/* 135 = socketpair */
	"mkdir",		/* 136 = mkdir */
	"rmdir",		/* 137 = rmdir */
	"utimes",		/* 138 = utimes */
	"getdprop",		/* 139 = getdprop */
	"adjtime",		/* 140 = adjtime*/
	"getpeername",		/* 141 = getpeername */
	"gethostid",		/* 142 = gethostid */
	"sethostid",		/* 143 = sethostid */
	"getrlimit",		/* 144 = getrlimit */
	"setrlimit",		/* 145 = setrlimit */
	"killpg",		/* 146 = killpg */
	"#147",			/* 147 = nosys */
	"setquota",		/* 148 = setquota */
	"quota",		/* 149 = quota */
	"getsockname",		/* 150 = getsockname */
	"msgctl",		/* 151 = msgctl */
	"msgget",		/* 152 = msgget */
	"msgrcv",		/* 153 = msgrcv */
	"msgsnd",		/* 154 = msgsnd */
	"semctl",		/* 155 = semctl */
	"semget",		/* 156 = semget */
	"semop",		/* 157 = semop */
	"uname",		/* 158 = uname */
	"shmsys",		/* 159 = shared memory multiplexed */
	"plock",		/* 160 = plock */
	"lockf",		/* 161 = lockf */
	"ustat",		/* 162 = ustat */
	"getmnt",		/* 163 = getmnt */
	"getdirentries",	/* 164 = getdirentries */
	"nfs_biod",		/* 165 = SUN nfs async daemon */
	"nfs_getfh",		/* 166 = SUN nfs get file handle */
	"nfs_svc",		/* 167 = SUN nfs service */
	"#168",			/* 168 = nosys */
	"getdomainname",	/* 169 = getdomainname */
	"setdomainname",	/* 170 = setdomainname */
};
