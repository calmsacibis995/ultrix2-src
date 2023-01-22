#ifndef lint
static char *sccsid = "@(#)ufs_syscalls.c	1.6	ULTRIX	10/3/86";
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
 *		Modification History
 *
 * 11 Sep 86 -- koehler
 *	fixed a link bug -- changed the namei interface
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 	Stephen Reilly, 09-Sep-85
 *	Modified to handle the new 4.3BSD namei code.
 *
 *	Stephen Reilly, 07-Aug-85
 * 003- Fixed a stale inode cache problem that was occuring with the
 *	rmdir syscall.
 *
 *	Larry Cohen, 13-April-85
 *		If open BLKINUSE and openi returns EALREADY then wait
 *		wait for line to unblock again.
 *		Also call f_ops ioctl if open FBLKANDSET
 *
 * 	Larry Cohen, 4-April-85
 * 002- Changes to support open block if in use capability.
 *
 *									*
 * 15 Mar 85 -- funding							*
 *	Added named pipe support (re. System V named pipes)		*
 *
 * 26 Oct 84 -- jrs
 *	Add support for namei cacheing.  Major changes to rename()
 *
 *	Stephen Reilly, 11-Jul-84
 * 001- There is a race condition between unlink and rename syscalls
 *	that can cause a panic with the message "freeing free inode
 *	panic". This fix came from Kirk Mckusick.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/file.h"
#include "../h/stat.h"
#include "../h/gnode.h"
#include "../ufs/fs.h"
#include "../h/buf.h"
#include "../h/proc.h"
#include "../h/quota.h"
#include "../h/uio.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/mount.h"
#include "../h/ioctl.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

extern struct gnode *ufs_galloc();
/*
 * link system call
 * gp is the gnode to be linked to, ndp is the nameidata point used
 * to get gp and which may be trashed. linkname is the new linkname.
 */
ufs_link(source_gp, target_ndp)
	register struct	gnode *source_gp;
	register struct nameidata *target_ndp;
{
	u.u_error = direnter(source_gp, target_ndp);
	if (u.u_error) {
		ufs_glock(source_gp);
		source_gp->g_nlink--;
		source_gp->g_flag |= GCHG;
		ufs_gunlock(source_gp);
	}
	ufs_grele(source_gp);
}

/*
 * Unlink system call.
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
ufs_unlink(gp, ndp)
	register struct gnode *gp;
	register struct nameidata *ndp;
{
	if (dirremove(ndp)) {
		gp->g_nlink--;
		gp->g_flag |= GCHG;
		gp->g_gennum++;
	}
#ifdef GFSDEBUG
	if(GFS[10])
		cprintf("ufs_unlink: gp 0x%x (%d) nlink %d u.u_error %d\n",
		gp, gp->g_number, gp->g_nlink, u.u_error);
#endif
}

/*
 * XXX Chase -- ugly temporary hack to fix rename failures.
 * ssd and tsd are the starting directories for namei's on source
 * and target respectively.  These are not needed for the local case
 * because both source and target pathnames are relative to the (same)
 * current directory.  Arguments to NFS rename operation are dirs and
 * single-component pathnames, so the source and target "pathnames"
 * may not be relative to the same directory.  Since there may be up
 * to four calls to namei for a single rename operation, u.u_cdir
 * has to change in the middle of ufs_rename in order to properly
 * handle an NFS rename operation with the old interface.
 *
 * Let's find a more elegant solution to this problem someday.
 */

ufs_rename(gp, ssd, source_ndp, tsd, target_ndp, flag)
	register struct gnode *gp;
	struct gnode *ssd, *tsd;
	register struct nameidata *source_ndp;
	register struct nameidata *target_ndp;
	int flag;
{
	register struct gnode *dp;
	register struct gnode *target_gp;
	struct dirtemplate dirbuf;
	register int doingdirectory = 0;
	int oldparent = 0, newparent = 0;
	int error = 0;

	dp = source_ndp->ni_pdir;
	
	/* 
	 *lock the parent because we may make changes if there is a 
	 * directory move
	 */
	
	if ((gp->g_mode&GFMT) == GFDIR) {
		register struct direct *d;

		d = &source_ndp->ni_dent;
#ifdef GFSDEBUG
		if(GFS[16])
			cprintf("ufs_rename: source gp 0x%x (%d) '%s' is dir\n",
			gp, gp->g_number, d->d_name);
#endif
		/*
		 * Avoid ".", "..", and aliases of "." for obvious reasons.
		 */
		if ((d->d_namlen == 1 && d->d_name[0] == '.') ||
		    (d->d_namlen == 2 && bcmp(d->d_name, "..", 2) == 0) ||
		    (dp == gp) || (gp->g_flag & GRENAME)) {
			gput(dp);
			if (dp == gp)
				ufs_grele(gp);
			else
				gput(gp);
			u.u_error = EINVAL;
			return;
		}
		gp->g_flag |= GRENAME;
		oldparent = dp->g_number;
		doingdirectory++;
	}
	gput(dp);

	/*
	 * 1) Bump link count while we're moving stuff
	 *    around.  If we crash somewhere before
	 *    completing our work, the link count
	 *    may be wrong, but correctable.
	 */
	gp->g_nlink++;
	gp->g_flag |= GCHG;
#ifdef GFSDEBUG
	if(GFS[16])
		cprintf("ufs_rename: updating target\n");
#endif
	ufs_gupdat(gp, &time, &time, 1, (struct ucred *) 0);
	ufs_gunlock(gp);

	/* this should really be done in gfs */
	u.u_cdir = tsd;
	target_gp = GNAMEI(target_ndp);
	if(u.u_error) {
#ifdef GFSDEBUG
		if(GFS[16])
			cprintf("ufs_rename: target '%s' error %d\n",
			target_ndp->ni_dirp, u.u_error);
#endif
		gp->g_nlink--;
		gp->g_flag |= GCHG;
		ufs_grele(gp);
		return;
	}
	dp = target_ndp->ni_pdir;
	
	/*
	 * If ".." must be changed (ie the directory gets a new
	 * parent) then the source directory must not be in the
	 * directory hierarchy above the target, as this would
	 * orphan everything below the source directory. Also
	 * the user must have write permission in the source so
	 * as to be able to change "..". We must repeat the call 
	 * to namei, as the parent directory is unlocked by the
	 * call to checkpath().
	 */
	if (oldparent != dp->g_number)
		newparent = dp->g_number;
	if (doingdirectory && newparent) {
		if (access(gp, GWRITE))
			goto bad;
		do {
			dp = target_ndp->ni_pdir;
			if (target_gp != NULL)
				gput(target_gp);
			u.u_error = checkpath(gp, dp, flag);
			if (u.u_error)
				goto out;
			u.u_cdir = tsd;
			target_gp = GNAMEI(target_ndp);
			if (u.u_error) {
				error = u.u_error;
				goto out;
			}
		} while (dp != target_ndp->ni_pdir);
	}
	/*
	 * 2) If target doesn't exist, link the target
	 *    to the source and unlink the source. 
	 *    Otherwise, rewrite the target directory
	 *    entry to reference the source gnode and
	 *    expunge the original entry's existence.
	 */
	if (target_gp == NULL) {
#ifdef GFSDEBUG
		if(GFS[16])
			cprintf("ufs_rename: target doesn't exist\n");
#endif
		if (dp->g_dev != gp->g_dev) {
			error = EXDEV;
			goto bad;
		}
		/*
		 * Account for ".." in new directory.
		 * When source and destination have the same
		 * parent we don't fool with the link count.
		 */
		if (doingdirectory && newparent) {
			dp->g_nlink++;
			dp->g_flag |= GCHG;
			ufs_gupdat(dp, &time, &time, 1, (struct ucred *) 0);
		}
#ifdef GFSDEBUG
		if(GFS[16])
			cprintf("ufs_rename: doing direnter on gp 0x%x (%d) '%s'\n",
			gp, gp->g_number, target_ndp->ni_dirp);
#endif
		error = direnter(gp, target_ndp);
		if (error)
			goto out;
	} else {
		if (target_gp->g_dev != dp->g_dev || target_gp->g_dev != gp->g_dev) {
			error = EXDEV;
			goto bad;
		}
		/*
		 * Short circuit rename(foo, foo).
		 */
		if (target_gp->g_number == gp->g_number)
			goto bad;
		/*
		 * Target must be empty if a directory
		 * and have no links to it.
		 * Also, insure source and target are
		 * compatible (both directories, or both
		 * not directories).
		 */
		if ((target_gp->g_mode&GFMT) == GFDIR) {
			if (!dirempty(target_gp, dp->g_number) || target_gp->g_nlink > 2) {
				error = ENOTEMPTY;
				goto bad;
			}
			if (!doingdirectory) {
				error = ENOTDIR;
				goto bad;
			}
			cacheinval(dp);
		} else if (doingdirectory) {
			error = EISDIR;
			goto bad;
		}
		dirrewrite(dp, gp, target_ndp);
		if (u.u_error) {
			error = u.u_error;
			goto bad1;
		}
		/*
		 * Adjust the link count of the target to
		 * reflect the dirrewrite above.  If this is
		 * a directory it is empty and there are
		 * no links to it, so we can squash the gnode and
		 * any space associated with it.  We disallowed
		 * renaming over top of a directory with links to
		 * it above, as the remaining link would point to
		 * a directory without "." or ".." entries.
		 */
		target_gp->g_nlink--;
		if (doingdirectory) {
			if (--target_gp->g_nlink != 0)
				panic("rename: linked directory");
			ufs_gtrunc(target_gp, (u_long)0, (struct ucred *) 0);
		}
		target_gp->g_flag |= GCHG;
		gput(target_gp);
		target_gp = NULL;
	}

	/*
	 * 3) Unlink the source.
	 */
	source_ndp->ni_nameiop = DELETE | LOCKPARENT;	
	u.u_cdir = ssd;
	target_gp = GNAMEI(source_ndp);
	if (target_gp != NULL) {
		dp = source_ndp->ni_pdir;
	} else {
		dp = NULL;
	}
	/*
	 * Insure that the directory entry still exists and has not
	 * changed while the new name has been entered. If the source is
	 * a file then the entry may have been unlinked or renamed. In
	 * either case there is no further work to be done. If the source
	 * is a directory then it cannot have been rmdir'ed; its link
	 * count of three would cause a rmdir to fail with ENOTEMPTY.
	 * The GRENAME flag insures that it cannot be moved by another
	 * rename.
	 */
	if (target_gp != gp) {
		if (doingdirectory)
			panic("rename: lost dir entry");
	} else {
		/*
		 * If the source is a directory with a
		 * new parent, the link count of the old
		 * parent directory must be decremented
		 * and ".." set to point to the new parent.
		 */
		if (doingdirectory && newparent) {
			dp->g_nlink--;
			dp->g_flag |= GCHG;
			error = rdwri(UIO_READ, target_gp, (caddr_t)&dirbuf,
				sizeof (struct dirtemplate), (off_t)0, 1,
				(int *)0);
			if (error == 0) {
				if (dirbuf.dotdot_namlen != 2 ||
					dirbuf.dotdot_name[0] != '.' ||
					dirbuf.dotdot_name[1] != '.') {
					printf("rename: mangled dir\n");
				} else {
					dirbuf.dotdot_ino = newparent;
					(void) rdwri(UIO_WRITE, target_gp,
					    (caddr_t)&dirbuf,
					    sizeof (struct dirtemplate),
					    (off_t)0, 1, (int *)0);
					cacheinval(dp);
				}
			}
		}
		if (dirremove(source_ndp)) {
			target_gp->g_nlink--;
			target_gp->g_flag |= GCHG;
		}
		target_gp->g_flag &= ~GRENAME;
		if (error == 0)		/* conservative */
			error = u.u_error;
	}
	if (dp)
		gput(dp);
	if (target_gp)
		gput(target_gp);
	ufs_grele(gp);
	if (error)
		u.u_error = error;
	return;

bad:
	gput(dp);
bad1:
	if (target_gp)
		gput(target_gp);
out:
	gp->g_nlink--;
	gp->g_flag |= GCHG;
	ufs_grele(gp);
	if (error)
		u.u_error = error;
}

/*
 * Make a new file.
 */
struct gnode *
ufs_maknode(mode, ndp)
	register int mode;
	register struct nameidata *ndp;
{
	register struct gnode *gp;
	register struct gnode *pdir = ndp->ni_pdir;
	register gno_t gpref;

#ifdef GFSDEBUG
	if(GFS[17])
		cprintf("ufs_maknod: ndp 0x%x, ni_pdir 0x%x\n", ndp,
		ndp->ni_pdir);
#endif
	if ((mode & GFMT) == GFDIR)
		gpref = dirpref(FS(pdir));
	else
		gpref = pdir->g_number;
	gp = ufs_galloc(pdir, gpref, mode);
	if (gp == NULL) {
		gput(pdir);
		return (NULL);
	}
#ifdef QUOTA
	if (gp->g_dquot != NODQUOT)
		panic("maknode: dquot");
#endif
	gp->g_flag |= GACC|GUPD|GCHG;
	gp->g_mode = mode & ~u.u_cmask;
	gp->g_nlink = 1;
	gp->g_uid = u.u_uid;
	gp->g_gid = pdir->g_gid;
	if (gp->g_mode & GSGID && !groupmember(gp->g_gid))
		gp->g_mode &= ~GSGID;
#ifdef QUOTA
	gp->g_dquot = inoquota(gp);
#endif

	/*
	 * Make sure gnode goes to disk before directory entry.
	 */
	ufs_gupdat(gp, &time, &time, 1, (struct ucred *) 0);
	u.u_error = direnter(gp, ndp);
	if (u.u_error) {
		/*
		 * Write error occurred trying to update directory
		 * so must deallocate the gnode.
		 */
		gp->g_nlink = 0;
		gp->g_flag |= GCHG;
		gput(gp);
		return (NULL);
	}
	return (gp);
}

/*
 * A virgin directory (no blushing please).
 */
struct dirtemplate mastertemplate = {
	0, 12, 1, ".",
	0, DIRBLKSIZ - 12, 2, ".."
};

struct gnode *
ufs_mkdir(dp, name, mode)
	register struct gnode *dp;
	register int mode;
	register char *name;
{
        register struct nameidata *ndp = &u.u_nd;
        register struct gnode *gp;
	struct dirtemplate dirtemplate;

	mode &= 0777;
	mode |= GFDIR;
	/*
	 * Must simulate part of maknode here
	 * in order to acquire the gnode, but
	 * not have it entered in the parent
	 * directory.  The entry is made later
	 * after writing "." and ".." entries out.
	 */
	gp = ufs_galloc(dp, dirpref(FS(dp)), mode);
	if (gp == NULL) {
		gput(dp);
		return(NULL);
	}
#ifdef QUOTA
	if (gp->g_dquot != NODQUOT)
		panic("mkdir: dquot");
#endif
	gp->g_flag |= GACC|GUPD|GCHG;
	gp->g_mode = mode & ~u.u_cmask;
	gp->g_nlink = 2;
	gp->g_uid = u.u_uid;
	gp->g_gid = dp->g_gid;
#ifdef QUOTA
	gp->g_dquot = inoquota(gp);
#endif
	ufs_gupdat(gp, &time, &time, 1, (struct ucred *) 0);

	/*
	 * Bump link count in parent directory
	 * to reflect work done below.  Should
	 * be done before reference is created
	 * so reparation is possible if we crash.
	 */
	dp->g_nlink++;
	dp->g_flag |= GCHG;
	ufs_gupdat(dp, &time, &time, 1, (struct ucred *) 0);

	/*
	 * Initialize directory with "."
	 * and ".." from static template.
	 */
	dirtemplate = mastertemplate;
	dirtemplate.dot_ino = gp->g_number;
	dirtemplate.dotdot_ino = dp->g_number;
	u.u_error = rdwri(UIO_WRITE, gp, (caddr_t)&dirtemplate,
		sizeof (dirtemplate), (off_t)0, 1, (int *)0);
	if (u.u_error) {
		dp->g_nlink--;
		dp->g_flag |= GCHG;
		gp->g_nlink = 0;
		gp->g_flag |= GCHG;
		gput(dp);
		gput(gp);
		/*
		 * No need to do an explicit itrunc here,
		 * grele will do this for us because we set
		 * the link count to 0.
		 */
		return(NULL);
	}
	
	if (DIRBLKSIZ > FS(gp)->fs_fsize)
		panic("mkdir: blksize");     /* XXX - should grow with bmap() */
	else
		gp->g_size = DIRBLKSIZ;
	/*
	 * Directory all set up, now
	 * install the entry for it in
	 * the parent directory.
	 */
	u.u_error = direnter(gp, ndp);
	dp = NULL;
	if (u.u_error) {
		ndp->ni_nameiop = LOOKUP | NOCACHE;
		ndp->ni_dirp = name;
		dp = GNAMEI(ndp);
		if (dp) {
			dp->g_nlink--;
			dp->g_flag |= GCHG;
		}
	}
	GUNLOCK(gp);
	return(gp);
}


ufs_rmdir(gp, ndp)
	register struct gnode *gp;
	register struct nameidata *ndp;
{
        register struct gnode *dp;

	dp = ndp->ni_pdir;
	/*
	 * No rmdir "." please.
	 */
	if (dp == gp) {
		GRELE(dp);
		gput(gp);
		u.u_error = EINVAL;
		return;
	}
	if ((gp->g_mode&GFMT) != GFDIR) {
		u.u_error = ENOTDIR;
		goto out;
	}
	/*
	 * Don't remove a mounted on directory.
	 */
	if (gp->g_dev != dp->g_dev) {
		u.u_error = EBUSY;
		goto out;
	}
	/*
	 * Verify the directory is empty (and valid).
	 * (Rmdir ".." won't be valid since
	 *  ".." will contain a reference to
	 *  the current directory and thus be
	 *  non-empty.)
	 */
#ifdef GFSDEBUG
	if(GFS[13])
		cprintf("ufs_rmdir: gp 0x%x (%d) links %d size %d\n", gp, gp->g_number,
		gp->g_nlink, gp->g_size);
#endif
	if (gp->g_nlink != 2 || !dirempty(gp, dp->g_number)) {
		u.u_error = ENOTEMPTY;
		goto out;
	}
	/*
	 * Delete reference to directory before purging
	 * gnode.  If we crash in between, the directory
	 * will be reattached to lost+found,
	 */
	if (dirremove(ndp) == 0)
		goto out;
	dp->g_nlink--;
	dp->g_flag |= GCHG;
	cacheinval(dp);
	gput(dp);
	dp = NULL;
	/*
	 * Truncate gnode.  The only stuff left
	 * in the directory is "." and "..".  The
	 * "." reference is inconsequential since
	 * we're quashing it.  The ".." reference
	 * has already been adjusted above.  We've
	 * removed the "." reference and the reference
	 * in the parent directory, but there may be
	 * other hard links so decrement by 2 and
	 * worry about them later.
	 */
	gp->g_nlink -= 2;
	gp->g_gennum++;
	ufs_gtrunc(gp, (u_long)0, (struct ucred *) 0);
	cacheinval(gp);
out:
	if (dp)
		gput(dp);
	gput(gp);
}

ufs_seek(gp, where)
	struct gnode *gp;
	int where;
{
	/*
	 * this is a dummy routine because gfs semantics tell us that
	 * if an operation is not supported, there is no function
	 * in the mount ops.
	 */
	return(0);
}
