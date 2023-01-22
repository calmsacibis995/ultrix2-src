#ifndef lint
static	char	*sccsid = "@(#)ufs_gnodeops.c	1.13	(ULTRIX)	3/3/87";
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
 *		Modification History					*
 *
 * 29 Jan 87 -- chet
 *	add new arg to bdwrite() calls.
 *
 * 15 Dec 86 -- depp
 *	Returned to rdwri() the munhashing of text blocks on write.
 *	This caused the "panic: chgd c_page".
 *	
 * 04 Dec 86 -- prs
 *	Fixed code in ufs_open. Code now closes the file is an
 *	attempt was made to open with write mode a write locked
 *	device. Also ufs_open will return after the open call to
 *	the driver if the NDELAY flag is set.
 *
 * 23 Oct 86 -- chet
 *	implemented IO_SYNC operation in ufs_rwgp(); made sure
 *	that gnode was updated before return.
 *
 * 23 Oct 86 -- prs
 *	Added code to return EROFS in ufs_open, if an attempt is being
 *	made to open a write locked block or character special file for
 *	writing.
 *
 * 11 Sep 86 -- koehler
 *	introduced bmap function, made changes to fsdata so that it
 *	returned the correct data
 *
 * 11 Mar 86 -- lp
 *	Added n-buffered hooks to rwip & ioctl routines.
 *
 * 23 Dec 85 -- Shaughnessy
 *	Added code to set/reset the syncronous write flag in
 *	the inode. Also made sure inode was updated immediately after
 *	syncronous write is performed. 
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 16 Apr 85 -- depp
 *	Fixed routine "openi"; It didn't return error correctly when
 *	routine "openp" was called
 *									*
 * 15 Mar 85 -- funding							*
 *	Added named pipe support (re. System V named pipes)		*
 *									*
 * 	I added some fixes from berkeley to fix the inode reference
 * count from going negative.
 * Rich
 *
 *	Modified 25-Oct-84 -- jrs
 *	Add various bug fixes
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode_common.h"	
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../h/proc.h"
#include "../ufs/fs.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/file.h"
#include "../h/text.h"
#include "../h/uio.h"
#include "../h/ioctl.h"
#include "../h/devio.h"
#include "../h/tty.h"
#include "../h/cmap.h"
#include "../h/stat.h"
#include "../h/kernel.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

ufs_rwgp(gp, uio, rw, ioflag, cred)
	register struct gnode *gp;
	register struct uio *uio;
	enum uio_rw rw;
	int ioflag;
	struct ucred *cred;
{
	dev_t dev = (dev_t)gp->g_rdev;
	struct buf *bp;
	struct fs *fs;
	daddr_t lbn, bn;
	register int n, on, type;
	int size;
	long bsize;
	extern int mem_no;
	int error = 0;

	type = gp->g_mode & GFMT;
	if (type == GFCHR) {
		if (rw == UIO_READ) {
			if(cdevsw[major(dev)].d_strat && (gp->g_flag&ASYNC)) {
				/* check for n-buffered I/O */
				error = aphysio(cdevsw[major(dev)].d_strat,
					dev, B_READ, uio);
				if(error == ENOBUFS)
				error = (*cdevsw[major(dev)].d_read)(dev, uio);
			 } else
				error = (*cdevsw[major(dev)].d_read)(dev, uio);
		} else {
			gp->g_flag |= GUPD|GCHG;
			if(cdevsw[major(dev)].d_strat && (gp->g_flag&ASYNC)) {
				/* check for n-buffered I/O */
				error = aphysio(cdevsw[major(dev)].d_strat,
					dev, B_WRITE, uio);
				if(error == ENOBUFS)
				error = (*cdevsw[major(dev)].d_write)(dev, uio);
			} else
				error = (*cdevsw[major(dev)].d_write)(dev, uio);
		}
		return (error);
	}
	if (uio->uio_resid == 0)
		return (0);

	if ((gp->g_mode & GFMT) != GFBLK) {
		dev = gp->g_dev;
		fs = FS(gp);
		bsize = fs->fs_bsize;
	} else
		bsize = BLKDEV_IOSIZE;
	do {
		lbn = uio->uio_offset / bsize;
		on = uio->uio_offset % bsize;
		n = MIN((unsigned)(bsize - on), uio->uio_resid);
		if (type != GFBLK) {
			if (rw == UIO_READ) {
				register int diff = gp->g_size - uio->uio_offset;
				if (diff <= 0)
					return (0);
				if (diff < n)
					n = diff;
			}
			bn = ufs_bmap(gp, (int) lbn,
				rw==UIO_WRITE ? B_WRITE : B_READ,
				(int)(on+n), (ioflag & IO_SYNC));
			if (u.u_error || rw == UIO_WRITE && (long)bn<0)
				return (u.u_error);
			if (rw == UIO_WRITE && uio->uio_offset + n > gp->g_size &&
			   (type == GFDIR || type == GFREG || type == GFLNK))
				gp->g_size = uio->uio_offset + n;


			size = blksize(fs, gp, lbn);
		} else {
			bn = lbn * (BLKDEV_IOSIZE/DEV_BSIZE);
			rablock = bn + (BLKDEV_IOSIZE/DEV_BSIZE);
			rasize = size = bsize;
		}
		if (rw == UIO_READ) {
			if ((long)bn<0) {
				bp = geteblk(size);
				clrbuf(bp);
			} else if (gp->g_lastr + 1 == lbn)
				bp = breada(dev, bn, size, rablock, rasize, 
				(struct gnode *) NULL);
			else
				bp = bread(dev, bn, size,
				(struct gnode *) NULL);
			gp->g_lastr = lbn;
		} else {
			int i, count;
			struct text *xp = gp->g_textp;
			extern struct cmap *mfind();

			count = howmany(size, DEV_BSIZE);
			for (i = 0; i < count; i += CLSIZE)
				if (mfind(dev, bn + i, gp))
					munhash(dev, bn + i, gp);
			if (xp) {
				/* sanity check the structs */
				if (xp->x_gptr != gp) {
					printf("ufs_rwgp: messed up gp, xp");
					printf("gp %X xp %X\n",gp,xp);
					panic("ufs_rwgp: messed up gp, xp");
				}
				if (xp->x_count > 1 || (gp->g_flag & GSVTX)) {
					printf("textp = %X gp = %X\n",xp,gp);
					panic("ufs_rwgp: illegal text reuse");
				}
				xuntext(xp);
			}
			if (n == bsize) 
				bp = getblk(dev, bn, size,
				(struct gnode *) NULL);
			else
				bp = bread(dev, bn, size,
				(struct gnode *) NULL);
		}
		n = MIN(n, size - bp->b_resid);
		if (bp->b_flags & B_ERROR) {
			error = EIO;
			brelse(bp);
			goto bad;
		}
		u.u_error =
		    uiomove(bp->b_un.b_addr+on, n, rw, uio);
		if (rw == UIO_READ) {
			if (n + on == bsize || uio->uio_offset == gp->g_size)
				bp->b_flags |= B_AGE;
			brelse(bp);
		} else {

			/*
			 * If writing a directory, or performing syncronous
			 * writes, then call bwrite to write synchronously.
			 */

			if (
			    ((gp->g_mode & GFMT) == GFDIR) ||
			    (ioflag & IO_SYNC)
			   )
				bwrite(bp);
			else if (n + on == bsize) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else
				bdwrite(bp, (struct gnode *)0);
			gp->g_flag |= GUPD|GCHG;
			if (u.u_ruid != 0)
				gp->g_mode &= ~(GSUID|GSGID);
		}
	} while (u.u_error == 0 && uio->uio_resid > 0 && n != 0);

	/*
	 * If doing a synchronous write,
	 * get gnode out to disk now.
	 */
	if (
	    (rw == UIO_WRITE) &&
	    (ioflag & IO_SYNC) &&
	    (gp->g_flag & (GUPD|GCHG))
	   )
		(void) ufs_gupdat(gp, &time, &time, 1, (struct ucred *) 0);

	if (error == 0)				/* XXX */
		error = u.u_error;		/* XXX */
bad:
	return (error);
}


ufs_fcntl(gp, cmd, arg, flag, cred)
	register struct gnode *gp;
	register int cmd;
	register caddr_t arg;
	register int flag;
	struct ucred *cred;
{
	register int fmt = gp->g_mode & GFMT;
	dev_t dev;

	switch (fmt) {
		case GFREG:
		case GFDIR:
			if (cmd == FIONREAD) {
				*(off_t *)arg = gp->g_size - flag;
				return (0);
			}
			if (cmd == FIONBIO || cmd == FIOASYNC ||
			    cmd == FIOSINUSE || cmd == FIOCINUSE) /* XXX */
				return (0);			/* XXX */
			/* fall into ... */
		default:
			return (ENOTTY);
		case GFCHR:
			dev = gp->g_rdev;
			u.u_r.r_val1 = 0;

			if ((u.u_procp->p_flag&SOUSIG) == 0 &&
			setjmp(&u.u_qsave)) {
				u.u_eosys = RESTARTSYS;
				return (0);
			}
			return ((*cdevsw[major(dev)].d_ioctl)(dev, cmd, arg,
			    flag));
	}
}

ufs_select(gp, rw, cred)
	register struct gnode *gp;
	register int rw;
	struct ucred *cred;
{ 
	switch (gp->g_mode & GFMT) {

	default:
		return (1);		/* XXX */

	case GFCHR:
		return
		    ((*cdevsw[major(gp->g_rdev)].d_select)(gp->g_rdev, rw));
	}
}

ufs_close(gp, flag)
	register struct gnode *gp;
	register int flag;
{
	register struct mount *mp;
	dev_t dev;

	dev = (dev_t)gp->g_rdev; 	
#ifdef GFSDEBUG
	if(GFS[0])
		cprintf("ufs_close: gp 0x%x mode 0x%x dev 0x%x\n", gp, gp->g_mode, dev);
#endif
	switch (gp->g_mode & GFMT) {
		case GFCHR:
#ifdef GFSDEBUG
			if(GFS[0])
				cprintf("ufs_close: GFCHR\n");
#endif
			(*cdevsw[major(dev)].d_close)(dev, flag);
			break;

		case GFBLK:
			/*
			 * We don't want to really close the device if
			 * it is mounted
			 */
			
			/* we now have a function for this, change it */
		
			/* MOUNT TABLE SHOULD HOLD INODE */
			GETMP(mp, dev);
			if((mp != NULL) && (mp != (struct mount *) MSWAPX))
				return;
			(*bdevsw[major(dev)].d_close)(dev, flag);
			break;
		default:
			return;
	}
}

ufs_open(gp, mode)
	register struct gnode *gp;
	register int mode;
{
	dev_t dev = (dev_t)gp->g_rdev;
	register int maj = major(dev);
	struct devget devget;
	int status, stat;

#ifdef GFSDEBUG
	if(GFS[0])
		cprintf ("ufs_open: gp 0x%x, rdev 0x%x\n", gp, gp->g_rdev);
#endif
	switch (gp->g_mode&GFMT) {
		case GFCHR:
#ifdef GFSDEBUG
			if(GFS[0])
				cprintf("ufs_open: FCHR maj 0%o dev 0%o\n",
				maj, dev);
#endif
			if ((u_int)maj >= nchrdev)
				return (ENXIO);
			status = (*cdevsw[maj].d_open)(dev, mode);
			/*
			 * If an opening a charater special file with
			 * write privilages, verify it is not write
			 * locked.
			 */
			if ((status==0) && (mode&FWRITE) && !(mode&FNDELAY)) {
				stat=(*cdevsw[maj].d_ioctl)(dev,DEVIOCGET,&devget,0);
				if (stat == 0)
					if (devget.stat & DEV_WRTLCK) {
						(*cdevsw[maj].d_close)(dev, 0);
						if (devget.category == DEV_TAPE)
							return(EIO);
						else
							return(EROFS);
					}
			}
			return (status);
		case GFBLK:
#ifdef GFSDEBUG
			if(GFS[0])
				cprintf("ufs_open: FBLK maj 0%o dev 0%o\n",
				maj, dev);
#endif
			if ((u_int)maj >= nblkdev)
				return (ENXIO);
			status = (*bdevsw[maj].d_open)(dev, mode);
			/*
			 * If an opening a block special file with
			 * write privilages, verify it is not write
			 * locked.
			 */
			if ((status==0) && (mode&FWRITE) && !(mode&FNDELAY)) {
				stat=(*bdevsw[maj].d_ioctl)(dev,DEVIOCGET,&devget,0);
				if (stat == 0)
					if (devget.stat & DEV_WRTLCK) {
						(*cdevsw[maj].d_close)(dev, 0);
						return(EROFS);
					}
			}
			return (status);
		case GFPORT:
#ifdef GFSDEBUG
			if(GFS[0])
				cprintf("ufs_open: FPORT maj 0%o dev 0%o\n",
				maj, dev);
#endif
			return(openp(gp, mode));
			break;
	}
	return (0);
}

ufs_symlink(ndp, target_name)
	register struct nameidata *ndp;
	register char *target_name;
{
	register int len;
	struct uio auio;
	struct iovec aiov;
	register struct gnode *gp;
	struct gnode *ufs_maknode();
	
#ifdef GFSDEBUG
	if(GFS[17])
		cprintf("ufs_symlink: ndp 0x%x ni_pdir 0x%x\n", ndp,
		ndp->ni_pdir);
#endif
	if((gp = ufs_maknode(GFLNK | 0777, ndp)) == NULL) 
		return;

	len = strlen(target_name);
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = target_name;
	aiov.iov_len = auio.uio_resid = len;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_offset = 0;
	
	u.u_error = ufs_rwgp(gp, &auio, UIO_WRITE, 0,  u.u_cred);
	gput(gp);
}


ufs_readlink(gp, auio)
	register struct gnode *gp;
	register struct uio *auio;
{
	u.u_error = ufs_rwgp(gp, auio, UIO_READ, 0, u.u_cred);
}


ufs_stat(gp, sb)
	register struct gnode *gp;
	register struct stat *sb;
{
	ufs_gtimes(gp, &time, &time);
	/*
	 * Copy from gnode table
	 */
	sb->st_dev = gp->g_dev;
	sb->st_ino = gp->g_number;
	sb->st_mode = gp->g_mode;
	sb->st_nlink = gp->g_nlink;
	sb->st_uid = gp->g_uid;
	sb->st_gid = gp->g_gid;
	sb->st_rdev = (dev_t)gp->g_rdev;
	sb->st_size = gp->g_size;
	sb->st_atime = gp->g_atime.tv_sec;
	sb->st_spare1 = gp->g_atime.tv_usec;
	sb->st_mtime = gp->g_mtime.tv_sec;
	sb->st_spare2 = gp->g_mtime.tv_usec;
	sb->st_ctime = gp->g_ctime.tv_sec;
	sb->st_spare3 = gp->g_ctime.tv_usec;
	/* this doesn't belong here */
	if ((gp->g_mode & GFMT) == GFBLK)
		sb->st_blksize = BLKDEV_IOSIZE;
	else if ((gp->g_mode & GFMT) == GFCHR)
		sb->st_blksize = MAXBSIZE;
	else
		sb->st_blksize = FS(gp)->fs_bsize;
	sb->st_blocks = gp->g_blocks;
	sb->st_spare4[0] = sb->st_spare4[1] = 0;
	return (0);
}

struct fs_data *
ufs_getfsdata(mp)
	register struct mount *mp;
{
	register struct fs_data *fs_data = mp->m_fs_data;
	register struct fs *fs;
	
	fs = (struct fs *) mp->m_bufp->b_un.b_fs;
#ifdef GFSDEBUG
	if(GFS[14])
		cprintf("ufs_getfsdata: mp 0x%x fs 0x%x\n", mp, fs);
#endif
	fs_data->fd_gtot = fs->fs_ncg * fs->fs_ipg;
	fs_data->fd_gfree = fs->fs_cstotal.cs_nifree;
	fs_data->fd_btot = fs->fs_dsize * fs->fs_fsize / FSDUNIT;
	fs_data->fd_bfree = (fs->fs_cstotal.cs_nbfree * fs->fs_frag +
	fs->fs_cstotal.cs_nffree) * fs->fs_fsize / FSDUNIT;
	fs_data->fd_otsize = fs->fs_bsize;
	fs_data->fd_mtsize = MAXBSIZE;
	fs_data->fd_bfreen = freespace(fs, fs->fs_minfree) *
	fs->fs_fsize / FSDUNIT;
	fs_data->fd_dev = mp->m_dev;
#ifdef GFSDEBUG
	if(GFS[14]) {
		cprintf("ufs_getfsdata:\n");
		cprintf("\tgtot %d gfree %d btot %d bfree %d\n",
		fs_data->fd_gtot, fs_data->fd_gfree, fs_data->fd_btot,
		fs_data->fd_bfree);
		cprintf("\totsize %d mtsize %d bfreen %d\n", fs_data->fd_otsize,
		fs_data->fd_mtsize, fs_data->fd_bfreen);
	}
#endif
	return(fs_data);
}


ufs_getdirent(gp, auio, cred)
	register struct gnode *gp;
	register struct uio *auio;
	register struct ucred *cred;
{
	u.u_error = ufs_rwgp(gp, auio, UIO_READ, 0, cred);
}
