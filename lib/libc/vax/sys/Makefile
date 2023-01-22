#  @(#)Makefile	1.15	ULTRIX	12/16/86
#
#************************************************************************
#*									*
#*			Copyright (c) 1986 by				*
#*		Digital Equipment Corporation, Maynard, MA		*
#*			All rights reserved.				*
#*									*
#*   This software is furnished under a license and may be used and	*
#*   copied  only  in accordance with the terms of such license and	*
#*   with the  inclusion  of  the  above  copyright  notice.   This	*
#*   software  or  any	other copies thereof may not be provided or	*
#*   otherwise made available to any other person.  No title to and	*
#*   ownership of the software is hereby transferred.			*
#*									*
#*   This software is  derived	from  software	received  from	the	*
#*   University    of	California,   Berkeley,   and	from   Bell	*
#*   Laboratories.  Use, duplication, or disclosure is	subject  to	*
#*   restrictions  under  license  agreements  with  University  of	*
#*   California and with AT&T.						*
#*									*
#*   The information in this software is subject to change  without	*
#*   notice  and should not be construed as a commitment by Digital	*
#*   Equipment Corporation.						*
#*									*
#*   Digital assumes no responsibility for the use  or	reliability	*
#*   of its software on equipment which is not supplied by Digital.	*
#*									*
#************************************************************************
#
# vax/sys Makefile  4.19 (Berkeley) 7/31/83
#
# Modification history
#
# 008 - Chet Juszczak
#	Renamed nfs_async_daemon to nfs_biod.
#
# 007 - Richard Hyde
#	Added adjtime system call. 15-Jul-86
#
# 006 - Jeff Chase		12-June-86
#	Added getdomainname and setdomainname
#
# 005-	Ricky Palmer,		15-Mar-86
#	Added getdirentries, getmnt, nfs_async_daemon, nfs_getfh, and nfs_svc
#	for gfs/nfs effort. V2.0
#
# 004-  Greg Depp		17-Mar-86
#	Added mprotect module
#
# 003-  Stephen L. Reilly,	07-Oct-85
#	Added ustat module
#

OBJS=	Ovfork.o Ovadvise.o \
	accept.o access.o acct.o adjtime.o \
	bind.o brk.o \
	cerror.o chdir.o chmod.o chown.o chroot.o close.o connect.o \
	creat.o \
	dup.o dup2.o \
	execl.o execle.o exect.o execv.o execve.o _exit.o \
	fchmod.o fchown.o fcntl.o flock.o fork.o fstat.o fsync.o ftruncate.o \
	getdirentries.o getdomainname.o getdtablesize.o getegid.o geteuid.o getgid.o \
	getgroups.o gethostid.o gethostname.o getmnt.o getitimer.o getrlimit.o \
	getpagesize.o getpeername.o getpgrp.o getpid.o getppid.o \
	getpriority.o getrusage.o getsockname.o getsockopt.o \
	gettimeofday.o getuid.o \
	ioctl.o \
	kill.o killpg.o \
	link.o listen.o lseek.o lstat.o \
	mkdir.o mknod.o mount.o mprotect.o \
	msgctl.o msgget.o msgrcv.o msgsnd.o\
	nfs_biod.o nfs_getfh.o nfs_svc.o \
	open.o \
	pipe.o profil.o ptrace.o \
	quota.o \
	read.o readlink.o readv.o reboot.o recv.o recvfrom.o \
	recvmsg.o rename.o rmdir.o \
	sbrk.o select.o \
	semctl.o semget.o semop.o \
	send.o sendmsg.o sendto.o setdomainname.o setregid.o setgroups.o \
	sethostid.o sethostname.o setitimer.o setquota.o setrlimit.o \
	setpgrp.o setpriority.o setsockopt.o settimeofday.o setreuid.o \
	shmat.o shmctl.o shmdt.o shmget.o shmsys.o plock.o \
	shutdown.o sigblock.o sigpause.o \
	sigsetmask.o sigstack.o sigvec.o socket.o socketpair.o stat.o \
	swapon.o symlink.o sync.o syscall.o \
	truncate.o \
	umask.o umount.o uname.o unlink.o ustat.o utimes.o \
	vhangup.o \
	wait.o wait3.o write.o writev.o

.c.o:
	/lib/cpp -E -DPROF  $*.c | ${AS} -o $*.o
	-ld -x -r -o profiled/$*.o $*.o
	/lib/cpp -E  $*.c | ${AS} -o $*.o
	-ld -x -r -o gfloat/$*.o $*.o
	/lib/cpp -E  $*.c | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out $*.o

syslib syslib_p: ${OBJS}
	@echo "building profiled syslib"
	@cd profiled; ar cru syslib ${OBJS}
	@echo "building gfloat syslib"
	@cd gfloat; ar cru syslib ${OBJS}
	@echo "buiding normal syslib_p"
	@ar cru syslib ${OBJS}

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		/lib/cpp -E `basename $$i .o`.c | ${AS} -o $$i; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal syslib"
	ar cru syslib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f syslib profiled/syslib gfloat/syslib \
	      ${OBJS} profiled/*.o gfloat/*.o

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
