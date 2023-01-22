#ifndef lint
static	char	*sccsid = "@(#)ufs_namei.c	1.8	(ULTRIX)	3/3/87";
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

/* ------------------------------------------------------------------------
 * Modification History: /sys/sys/ufs_nami.c
 *
 * 23 Oct 86 -- chet
 *	Add arg to ufs_bmap() call
 *
 * 11 Sep 86 -- koehler
 *	added support for local under remote fs
 *
 * 14 Oct 85 -- Reilly
 *	Modified a comment
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code.
 *
 * 26 Oct 84 -- jrs
 *	Add code for nami cacheing
 *	Derived from 4.2BSD, labeled:
 *		ufs_nami.c 6.11	84/07/07
 *
 * -----------------------------------------------------------------------
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../ufs/fs.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/uio.h"
#include "../h/kernel.h"
#include "../h/fs_types.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

/*
 * Convert a pathname into a pointer to a locked inode,
 * with side effects usable in creating and removing files.
 * This is a very central and rather complicated routine.
 *
 * The segflg defines whether the name is to be copied from user
 * space or kernel space.
 *
 * The flag argument is (LOOKUP, CREATE, DELETE) depending on whether
 * the name is to be (looked up, created, deleted).  If flag has
 * LOCKPARENT or'ed into it and the target of the pathname exists,
 * namei returns both the target and its parent directory locked. 
 * If the file system is not maintained in a strict tree hierarchy,
 * this can result in a deadlock situation.  When creating and
 * LOCKPARENT is specified, the target may not be ".".  When deleting
 * and LOCKPARENT is specified, the target may be ".", but the caller
 * must check to insure it does an grele and gput instead of two gputs.
 *
 * The FOLLOW flag is set when symbolic links are to be followed
 * when they occur at the end of the name translation process.
 *
 * Name caching works as follows:
 *
 *	names found by directory scans are retained in a cache
 *	for future reference.  It is managed LRU, so frequently
 *	used names will hang around.  Cache is indexed by hash value
 *	obtained from (ino,dev,name) where ino & dev refer to the
 *	directory containing name.
 *
 *	For simplicity (and economy of storage), names longer than
 *	some (small) maximum length are not cached, they occur
 *	infrequently in any case, and are almost never of interest.
 *
 *	Upon reaching the last segment of a path, if the reference
 *	is for DELETE, or NOCACHE is set (rewrite), and the
 *	name is located in the cache, it will be dropped.
 *
 *	We must be sure never to enter the name ".." into the cache
 *	because of the extremely kludgey way that rename() alters
 *	".." in a situation like
 *		mv a/x b/x
 *	where x is a directory, and x/.. is the ".." in question.
 *
 * Overall outline of namei:
 *
 *	copy in name
 *	get starting directory
 * dirloop:
 *	check accessibility of directory
 * dirloop2:
 *	copy next component of name to ndp->ni_dent
 *	handle degenerate case where name is null string
 *	look for name in cache, if found, then if at end of path
 *	and deleting or creating, drop it, else to haveino
 *	search for name in directory, to found or notfound
 * notfound:
 *	if creating, return locked directory, leaving info on avail. slots
 *	else return error
 * found:
 *	if at end of path and deleting, return information to allow delete
 *	if at end of path and rewriting (create and LOCKPARENT), lock target
 *	  inode and return info to allow rewrite
 *	if .. and on mounted filesys, look in mount table for parent
 *	if not at end, if neither creating nor deleting, add name to cache
 * haveino:
 *	if symbolic link, massage name in buffer and continue at dirloop
 *	if more components of name, do next level at dirloop
 *	return the answer as locked inode
 *
 * NOTE: (LOOKUP | LOCKPARENT) currently returns the parent inode,
 *	 but unlocked.
 */

/*
 * GFS expects sfs's to gput the pdir when passed and that the
 * returned gnode is locked and has the reference count incremented
 */

struct gnode *
ufs_namei(ndp)
	register struct nameidata *ndp;
{
	register char *cp;		/* pointer into pathname argument */
/* these variables refer to things which must be freed or unlocked */
	register struct gnode *dp = 0;	/* the directory we are searching */
	register struct nch *ncp;	/* cache slot for entry */
	register struct mount *mp;	/* file system that directory is in */
	register struct buf *bp = 0;	/* a buffer of directory entries */
	register struct direct *ep;	/* the current directory entry */
	struct gnode *pmgp;
	int entryoffsetinblock;		/* offset of ep in bp's buffer */
/* these variables hold information about the search for a slot */
	enum {NONE, COMPACT, FOUND} slotstatus;
	int slotoffset = -1;		/* offset of area with free space */
	int slotsize;			/* size of area at slotoffset */
	int slotfreespace;		/* amount of space free in slot */
	int slotneeded;			/* size of the entry we're seeking */
/* */
	int numdirpasses;		/* strategy for directory search */
	int endsearch;			/* offset to end directory search */
	int prevoff;			/* ndp->ni_offset of previous entry */
	struct gnode *pdp;		/* saved dp during symlink work */
	int i;
	int lockparent;
	int docache;
	int makeentry;			/* != 0 if name to be added to cache */
	unsigned hash;			/* value of name hash for entry */
	union nchash *nhp;		/* cache chain head for entry */
	int isdotdot;			/* != 0 if current name is ".." */
	int flag;			/* op ie, LOOKUP, CREATE, or DELETE */
	off_t enduseful;		/* pointer past last used dir slot */
	int nomount;
	
	lockparent = ndp->ni_nameiop & LOCKPARENT;
	nomount = ndp->ni_nameiop & NOMOUNT;
	docache = (ndp->ni_nameiop & NOCACHE) ^ NOCACHE;
	flag = ndp->ni_nameiop &~ (LOCKPARENT|NOCACHE|FOLLOW|NOMOUNT);
	docache = docache & (nomount == 0 & flag == CREATE);	
	if (flag == DELETE || lockparent)
		docache = 0;

	dp = ndp->ni_pdir;
	cp = ndp->ni_cp;
	while(*cp && (*cp == '/'))
		cp++;
		
#ifdef GFSDEBUG
	if(GFS[1])
		cprintf("ufs_namei: pdir %x (%d) count %d flags 0x%x\n", dp,
		dp->g_number, dp->g_count, dp->g_flag);
#endif
	ndp->ni_endoff = 0;

	/*
	 * We come to dirloop to search a new directory.
	 * The directory must be locked so that it can be
	 * gput, and fs must be already set to dp->i_fs.
	 */
dirloop:
	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->g_mode&GFMT) != GFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if (access(dp, GEXEC))
		goto bad;

dirloop2:
	/*
	 * Copy next component of name to ndp->ni_dent.
	 */
	hash = 0;
	for (i = 0; *cp != 0 && *cp != '/'; cp++) {
		if (i >= MAXNAMLEN) {
			u.u_error = ENAMETOOLONG;
			goto bad;
		}
		if (*cp & 0200)
			if ((*cp&0377) == ('/'|0200) || flag != DELETE) {
				u.u_error = EINVAL;
				goto bad;
			}
		ndp->ni_dent.d_name[i++] = *cp;
		hash += (unsigned char)*cp * i;
	}
	ndp->ni_dent.d_namlen = i;
	ndp->ni_dent.d_name[i] = '\0';
	isdotdot = (i == 2 &&
		ndp->ni_dent.d_name[0] == '.' && ndp->ni_dent.d_name[1] == '.');
	makeentry = 1;
	if (*cp == '\0' && docache == 0)
		makeentry = 0;
#ifdef GFSDEBUG
	if(GFS[1])
		cprintf ("ufs_namei: looking for '%s'\n", ndp->ni_dent.d_name);
#endif
	/*
	 * Check for degenerate name (e.g. / or "")
	 * which is a way of talking about a directory,
	 * e.g. like "/." or ".".
	 */
	if (ndp->ni_dent.d_name[0] == '\0') {
		if (flag != LOOKUP || lockparent) {
			u.u_error = EISDIR;
			goto bad;
		}
		dp->g_flag &= ~GINCOMPLETE;
		dp->g_rdev = NULL;
		ndp->ni_cp = cp;
		return (dp);
	}

	/*
	 * We now have a segment name to search for, and a directory to search.
	 *
	 * Before tediously performing a linear scan of the directory,
	 * check the name cache to see if the directory/name pair
	 * we are looking for is known already.  We don't do this
	 * if the segment name is long, simply so the cache can avoid
	 * holding long names (which would either waste space, or
	 * add greatly to the complexity).
	 */
	if (ndp->ni_dent.d_namlen > NCHNAMLEN) {
		nchstats.ncs_long++;
		makeentry = 0;
	} else {
		nhp = &nchash[NHASH(hash, dp->g_number, dp->g_dev)];
		for (ncp = nhp->nch_forw; ncp != (struct nch *)nhp;
		    ncp = ncp->nc_forw) {
			if (ncp->nc_ino == dp->g_number &&
			    ncp->nc_dev == dp->g_dev &&
			    ncp->nc_nlen == ndp->ni_dent.d_namlen &&
			    !bcmp(ncp->nc_name, ndp->ni_dent.d_name,
				ncp->nc_nlen))
				break;
		}

		if (ncp == (struct nch *)nhp) {
			nchstats.ncs_miss++;
			ncp = NULL;
		} else {
			if (ncp->nc_id != ncp->nc_ip->g_id) {
				nchstats.ncs_falsehits++;
			} else if (!makeentry) {
				nchstats.ncs_badhits++;
			} else {

					/*
					 * move this slot to end of LRU
					 * chain, if not already there
					 */
				if (ncp->nc_nxt) {
						/* remove from LRU chain */
					*ncp->nc_prev = ncp->nc_nxt;
					ncp->nc_nxt->nc_prev = ncp->nc_prev;

						/* and replace at end of it */
					ncp->nc_nxt = NULL;
					ncp->nc_prev = nchtail;
					*nchtail = ncp;
					nchtail = &ncp->nc_nxt;
				}

				/*
				 * Get the next inode in the path.
				 * See comment above other `GUNLOCK' code for
				 * an explaination of the locking protocol.
				 */
				pmgp = pdp;
				pdp = dp;
				if (!isdotdot || dp != u.u_rdir)
					dp = ncp->nc_ip;
				if (dp == NULL)
					panic("ufs_namei: null cache ino");
				if (pdp == dp) {
					dp->g_count++;
				} else if (isdotdot) {
					ufs_gunlock(pdp);
					ggrab(dp);
					if(nomount && (dp->g_number == ROOTINO)) {
						ufs_gunlock(dp);
						dp = dp->g_mp->m_gnodp ?
						dp->g_mp->m_gnodp : rootdir;
						ufs_glock(dp);
						dp->g_count++;
						pdp = pmgp;
						ufs_glock(pdp);
						goto gothash;
					}
				} else {
					ggrab(dp);
					ufs_gunlock(pdp);
					if(dp->g_mp->m_fstype != GT_ULTRIX) {
#ifdef GFSDEBUG
						if(GFS[1])
							cprintf("ufs_namei: dp 0x%x (%d), new fs type\n",
							dp, dp->g_number);
#endif
						ndp->ni_cp = cp;
						ndp->ni_pdir = dp;
						ufs_grele(dp);
						(void)GLOCK(dp);
						dp->g_flag |= GINCOMPLETE;

						/*
						 * cant have a special device
						 * as a mount point
						 */

						dp->g_rdev = NULL;
#ifdef GFSDEBUG
						if(GFS[1])
							cprintf("ufs_namei: (0) different fs type gno 0x%x (%d)\n",
							dp, dp->g_number);
#endif
						return(dp);
					}
					if(nomount && (dp->g_number == ROOTINO)) {
						ufs_gunlock(dp);
						dp = dp->g_mp->m_gnodp ?
						dp->g_mp->m_gnodp : rootdir;
						ufs_glock(dp);
						dp->g_count++;
						pdp = pmgp;
						ufs_glock(pdp);
						goto gothash;
					}

				}
				
				/*
				 * Verify that the inode that we got
				 * did not change while we were waiting
				 * for it to be locked.
				 */
gothash:			if (ncp->nc_id != ncp->nc_ip->g_id) {
					gput(dp);
					ufs_glock(pdp);
					dp = pdp;
					nchstats.ncs_falsehits++;
				} else {
					ndp->ni_dent.d_ino = dp->g_number;
					/* ni_dent.d_reclen is garbage ... */
					nchstats.ncs_goodhits++;
					goto haveino;
				}
			}

			/*
			 * Last component and we are renaming or deleting,
			 * the cache entry is invalid, or otherwise don't
			 * want cache entry to exist.
			 */

				/* remove from LRU chain */
			*ncp->nc_prev = ncp->nc_nxt;
			if (ncp->nc_nxt)
				ncp->nc_nxt->nc_prev = ncp->nc_prev;
			else
				nchtail = ncp->nc_prev;

				/* remove from hash chain */
			remque(ncp);

				/* insert at head of LRU list (first to grab) */
			ncp->nc_nxt = nchhead;
			ncp->nc_prev = &nchhead;
			nchhead->nc_prev = &ncp->nc_nxt;
			nchhead = ncp;

				/* and make a dummy hash chain */
			ncp->nc_forw = ncp;
			ncp->nc_back = ncp;

			ncp = NULL;
		}
	}

	/*
	 * Suppress search for slots unless creating
	 * file and at end of pathname, in which case
	 * we watch for a place to put the new file in
	 * case it doesn't already exist.
	 */
	slotstatus = FOUND;
	if (flag == CREATE && *cp == 0) {
		slotstatus = NONE;
		slotfreespace = 0;
		slotneeded = DIRSIZ(&ndp->ni_dent);
	}
	/*
	 * If this is the same directory that this process
	 * previously searched, pick up where we last left off.
	 * We cache only lookups as these are the most common
	 * and have the greatest payoff. Caching CREATE has little
	 * benefit as it usually must search the entire directory
	 * to determine that the entry does not exist. Caching the
	 * location of the last DELETE has not reduced profiling time
	 * and hence has been removed in the interest of simplicity.
	 */
	if (flag != LOOKUP || dp->g_number != u.u_ncache.nc_inumber ||
	    dp->g_dev != u.u_ncache.nc_dev) {
		ndp->ni_offset = 0;
		numdirpasses = 1;
	} else {
		if ((dp->g_flag & GCHG) || dp->g_ctime.tv_sec >=
		u.u_ncache.nc_time) {
			if (u.u_ncache.nc_prevoffset > dp->g_size)
				u.u_ncache.nc_prevoffset = 0;
			else
				u.u_ncache.nc_prevoffset &= ~(DIRBLKSIZ - 1);
			u.u_ncache.nc_time = time.tv_sec;
		}
		ndp->ni_offset = u.u_ncache.nc_prevoffset;
		entryoffsetinblock = blkoff(FS(dp), ndp->ni_offset);
		if (entryoffsetinblock != 0) {
			bp = blkatoff(dp, ndp->ni_offset, (char **)0);
			if (bp == 0)
				goto bad;
		}
		numdirpasses = 2;
		nchstats.ncs_2passes++;
	}
	endsearch = roundup(dp->g_size, DIRBLKSIZ);
	enduseful = 0;

searchloop:
	while (ndp->ni_offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (blkoff(FS(dp), ndp->ni_offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			bp = blkatoff(dp, ndp->ni_offset, (char **)0);
			if (bp == 0)
				goto bad;
			entryoffsetinblock = 0;
		}

		/*
		 * If still looking for a slot, and at a DIRBLKSIZE
		 * boundary, have to start looking for free space again.
		 */
		if (slotstatus == NONE &&
		    (entryoffsetinblock&(DIRBLKSIZ-1)) == 0) {
			slotoffset = -1;
			slotfreespace = 0;
		}

		/*
		 * Get pointer to next entry.
		 * Full validation checks are slow, so we only check
		 * enough to insure forward progress through the
		 * directory. Complete checks can be run by patching
		 * "dirchk" to be true.
		 */
		ep = (struct direct *)(bp->b_un.b_addr + entryoffsetinblock);
		if (ep->d_reclen <= 0 ||
		    dirchk && dirbadentry(ep, entryoffsetinblock)) {
			dirbad(dp, ndp->ni_offset, "mangled entry");
			i = DIRBLKSIZ - (entryoffsetinblock & (DIRBLKSIZ - 1));
			ndp->ni_offset += i;
			entryoffsetinblock += i;
			continue;
		}

		/*
		 * If an appropriate sized slot has not yet been found,
		 * check to see if one is available. Also accumulate space
		 * in the current block so that we can determine if
		 * compaction is viable.
		 */
		if (slotstatus != FOUND) {
			int size = ep->d_reclen;

			if (ep->d_ino != 0)
				size -= DIRSIZ(ep);
			if (size > 0) {
				if (size >= slotneeded) {
					slotstatus = FOUND;
					slotoffset = ndp->ni_offset;
					slotsize = ep->d_reclen;
				} else if (slotstatus == NONE) {
					slotfreespace += size;
					if (slotoffset == -1)
						slotoffset = ndp->ni_offset;
					if (slotfreespace >= slotneeded) {
						slotstatus = COMPACT;
						slotsize = ndp->ni_offset +
						      ep->d_reclen - slotoffset;
					}
				}
			}
		}

		/*
		 * Check for a name match.
		 */
		if (ep->d_ino) {

			if (ep->d_namlen == ndp->ni_dent.d_namlen &&
			!bcmp(ndp->ni_dent.d_name, ep->d_name, ep->d_namlen)) {
#ifdef GFSDEBUG
				if(GFS[1])
					cprintf ("ufs_namei: name match gno %d\n", ep->d_ino);

#endif
				goto found;
			}
		}
		prevoff = ndp->ni_offset;
		ndp->ni_offset += ep->d_reclen;
		entryoffsetinblock += ep->d_reclen;
		if (ep->d_ino)
			enduseful = ndp->ni_offset;
	}
/* notfound: */
	/*
	 * If we started in the middle of the directory and failed
	 * to find our target, we must check the beginning as well.
	 */
	if (numdirpasses == 2) {
		numdirpasses--;
		ndp->ni_offset = 0;
		endsearch = u.u_ncache.nc_prevoffset;
		goto searchloop;
	}
	/*
	 * If creating, and at end of pathname and current
	 * directory has not been removed, then can consider
	 * allowing file to be created.
	 */
	if (flag == CREATE && *cp == 0 && dp->g_nlink != 0) {
		/*
		 * Access for write is interpreted as allowing
		 * creation of files in the directory.
		 */
		if (access(dp, GWRITE))
			goto bad;
		/*
		 * Return an indication of where the new directory
		 * entry should be put.  If we didn't find a slot,
		 * then set ndp->ni_count to 0 indicating that the new
		 * slot belongs at the end of the directory. If we found
		 * a slot, then the new entry can be put in the range
		 * [ndp->ni_offset .. ndp->ni_offset + ndp->ni_count)
		 */
		if (slotstatus == NONE) {
			ndp->ni_offset = roundup(dp->g_size, DIRBLKSIZ);
			ndp->ni_count = 0;
			enduseful = ndp->ni_offset;
		} else {
			ndp->ni_offset = slotoffset;
			ndp->ni_count = slotsize;
			if (enduseful < slotoffset + slotsize)
				enduseful = slotoffset + slotsize;
		}
		ndp->ni_endoff = roundup(enduseful, DIRBLKSIZ);
		dp->g_flag |= GUPD|GCHG;
		if (bp)
			brelse(bp);
		/*
		 * We return with the directory locked, so that
		 * the parameters we set up above will still be
		 * valid if we actually decide to do a direnter().
		 * We return NULL to indicate that the entry doesn't
		 * currently exist, leaving a pointer to the (locked)
		 * directory inode in ndp->ni_pdir.
		 */
		dp->g_flag &= ~GINCOMPLETE;
		ndp->ni_pdir = dp;
		ndp->ni_cp = cp;
		return (NULL);
	}
	u.u_error = ENOENT;
	goto bad;
found:
	if (numdirpasses == 2)
		nchstats.ncs_pass2++;
	/*
	 * Check that directory length properly reflects presence
	 * of this entry.
	 */
	if (entryoffsetinblock + DIRSIZ(ep) > dp->g_size) {
		dirbad(dp, ndp->ni_offset, "i_size too small");
		dp->g_size = entryoffsetinblock + DIRSIZ(ep);
		dp->g_flag |= GUPD|GCHG;
	}

	/*
	 * Found component in pathname.
	 * If the final component of path name, save information
	 * in the cache as to where the entry was found.
	 */
	if (*cp == '\0' && flag == LOOKUP) {
		u.u_ncache.nc_prevoffset = ndp->ni_offset;
		u.u_ncache.nc_inumber = dp->g_number;
		u.u_ncache.nc_dev = dp->g_dev;
		u.u_ncache.nc_time = time.tv_sec;
	}
	/*
	 * Save directory entry's inode number and reclen in ndp->ni_dent,
	 * and release directory buffer.
	 */
	ndp->ni_dent.d_ino = ep->d_ino;
	ndp->ni_dent.d_reclen = ep->d_reclen;
	brelse(bp);
	bp = NULL;

	/*
	 * If deleting, and at end of pathname, return
	 * parameters which can be used to remove file.
	 * If the lockparent flag isn't set, we return only
	 * the directory (in ndp->ni_pdir), otherwise we go
	 * on and lock the inode, being careful with ".".
	 */
	if (flag == DELETE && *cp == 0) {
		/*
		 * Write access to directory required to delete files.
		 */
		if (access(dp, GWRITE))
			goto bad;
		ndp->ni_pdir = dp;		/* for dirremove() */
		/*
		 * Return pointer to current entry in ndp->ni_offset,
		 * and distance past previous entry (if there
		 * is a previous entry in this block) in ndp->ni_count.
		 * Save directory inode pointer in ndp->ni_pdir for dirremove().
		 */
		if ((ndp->ni_offset&(DIRBLKSIZ-1)) == 0)
			ndp->ni_count = 0;
		else
			ndp->ni_count = ndp->ni_offset - prevoff;
		if (lockparent) {
			if (dp->g_number == ndp->ni_dent.d_ino)
				dp->g_count++;
			else {
#ifdef GFSDEBUG
				if(GFS[1])
					cprintf("ufs_namei: (1) dp 0x%x (%d) pdp 0x%x (%d)\n",
					dp, ((int)dp & 0xc0000000) ? -1 : dp->g_number,
					ndp->ni_pdir, ((int)ndp->ni_pdir & 0xc000000) ?
					-1 : ndp->ni_pdir->g_number);
#endif
				dp = ufs_gget(dp->g_dev, dp->g_mp,
				ndp->ni_dent.d_ino, nomount);
				if (dp == NULL) {
					gput(ndp->ni_pdir);
					goto bad;
				}
				if(dp->g_mp->m_fstype != GT_ULTRIX) {
					ndp->ni_cp = cp;
					gput(ndp->ni_pdir);
					ndp->ni_pdir = dp;
					dp->g_flag |= GINCOMPLETE;

					/*
					 * cant have a special device
					 * as a mount point
					 */

					dp->g_rdev = NULL;
#ifdef GFSDEBUG
					if(GFS[1])
						cprintf("ufs_namei: (1) different fs type gno 0x%x (%d)\n",
						dp, dp->g_number);
#endif
					return(dp);
				}
				/*
				 * If directory is "sticky", then user must own
				 * the directory, or the file in it, else he
				 * may not delete it (unless he's root). This
				 * implements append-only directories.
				 */
				if ((ndp->ni_pdir->g_mode & GSVTX) &&
				    u.u_uid != 0 &&
				    u.u_uid != ndp->ni_pdir->g_uid &&
				    dp->g_uid != u.u_uid) {
					gput(ndp->ni_pdir);
					u.u_error = EPERM;
					goto bad;
				}
			}
		}
		dp->g_flag &= ~GINCOMPLETE;
		ndp->ni_cp = cp;
		/* should be optimized */
		if(((dp->g_mode & GFMT) == GFCHR) ||
		((dp->g_mode & GFMT) == GFBLK)) {
			dp->g_rdev = G_TO_I(dp)->di_rdev;
		} else
			dp->g_rdev = NULL;
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf("ufs_namei: (2) dp 0x%x (%d)\n", dp, dp->g_number);
#endif
		return (dp);
	}

	/*
	 * Special handling for ".." allowing chdir out of mounted
	 * file system: indirect .. in root inode to reevaluate
	 * in directory file system was mounted on.
	 */
	if (isdotdot) {
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf("ufs_namei: dot dot, u.u_rdir 0x%x\n",
			u.u_rdir);
#endif
		if (dp == u.u_rdir) {
			ndp->ni_dent.d_ino = dp->g_number;
			makeentry = 0;
		} else if (ndp->ni_dent.d_ino == ROOTINO &&
		   dp->g_number == ROOTINO) {
			if(dp->g_dev == rootdev) {
				while(*cp == '/')
					cp++;
				ndp->ni_pdir = dp = rootdir;
				ndp->ni_cp = cp;
				dp->g_flag &= ~GINCOMPLETE;
				if(*cp)
					goto dirloop2;
				return(dp);
			}
			mp = dp->g_mp;
			gput(dp);
			dp = mp->m_gnodp;
			dp->g_count++;
			cp -= 2;     /* back over .. */
			gfs_lock(dp);
			if(dp->g_mp->m_fstype != GT_ULTRIX) {
				ndp->ni_pdir = dp;
				ndp->ni_cp = cp;
				dp->g_flag |= GINCOMPLETE;
				dp->g_rdev = NULL;
#ifdef GFSDEBUG
				if(GFS[1])
					cprintf("ufs_namei: (3) different fs type dp 0x%x (%d)\n",
					dp, dp->g_number);
#endif
				return(dp);
			}
			goto dirloop2;
		}
	}

	/*
	 * If rewriting (rename), return the inode and the
	 * information required to rewrite the present directory
	 * Must get inode of directory entry to verify it's a
	 * regular file, or empty directory.  
	 */
	if ((flag == CREATE && lockparent) && *cp == 0) {
		if (access(dp, GWRITE))
			goto bad;
		ndp->ni_pdir = dp;		/* for dirrewrite() */
		/*
		 * Careful about locking second inode. 
		 * This can only occur if the target is ".". 
		 */
		if (dp->g_number == ndp->ni_dent.d_ino) {
			u.u_error = EISDIR;		/* XXX */
			goto bad;
		}
		dp = gfs_gget(dp->g_dev, dp->g_mp, ndp->ni_dent.d_ino, nomount);
		if (dp == NULL) {
			gput(ndp->ni_pdir);
			goto bad;
		}
		if(dp->g_mp->m_fstype != GT_ULTRIX) {
			ndp->ni_cp = cp;
			gput(ndp->ni_pdir);
			ndp->ni_pdir = dp;
			dp->g_flag |= GINCOMPLETE;
			dp->g_rdev = NULL;
#ifdef GFSDEBUG
			if(GFS[1])
				cprintf("ufs_namei: (4) different fs type dp 0x%x (%d)\n",
				dp, dp->g_number);
#endif
			return(dp);
		}

		dp->g_flag &= ~GINCOMPLETE;
		ndp->ni_cp = cp;
		if(((dp->g_mode & GFMT) == GFCHR) ||
		((dp->g_mode & GFMT) == GFBLK)) {
			dp->g_rdev = G_TO_I(dp)->di_rdev;
		} else
			dp->g_rdev = NULL;
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf("ufs_namei: (5) dp 0x%x (%d)\n", dp, dp->g_number);
#endif
		return (dp);
	}

	/*
	 * Check for symbolic link, which may require us to massage the
	 * name before we continue translation.  We do not `gput' the
	 * directory because we may need it again if the symbolic link
	 * is relative to the current directory.  Instead we save it
	 * unlocked as "pdp".  We must get the target inode before unlocking
	 * the directory to insure that the inode will not be removed
	 * before we get it.  We prevent deadlock by always fetching
	 * inodes from the root, moving down the directory tree. Thus
	 * when following backward pointers ".." we must unlock the
	 * parent directory before getting the requested directory.
	 * There is a potential race condition here if both the current
	 * and parent directories are removed before the `iget' for the
	 * inode associated with ".." returns.  We hope that this occurs
	 * infrequently since we cannot avoid this race condition without
	 * implementing a sophisticated deadlock detection algorithm.
	 * Note also that this simple deadlock detection scheme will not
	 * work if the file system has any hard links other than ".."
	 * that point backwards in the directory structure.
	 */
	pdp = dp;
	if (isdotdot) {
		ufs_gunlock(pdp);	/* race to get the inode */
	
		dp = gfs_gget(dp->g_dev, dp->g_mp, ndp->ni_dent.d_ino,
		nomount);
		
		if (dp == NULL)
			goto bad2;
		
		if(dp->g_mp->m_fstype != GT_ULTRIX) {
			gput(ndp->ni_pdir);
			ndp->ni_cp = cp;
			ndp->ni_pdir = dp;
			dp->g_flag |= GINCOMPLETE;
			dp->g_rdev = NULL;
#ifdef GFSDEBUG
			if(GFS[1])
				cprintf("ufs_namei: (6) different fs type dp 0x%x (%d)\n",
				dp, dp->g_number);
#endif
			return(dp);
		}
	} else if (dp->g_number == ndp->ni_dent.d_ino) {
		dp->g_count++;	/* we want ourself, ie "." */
	} else {
		dp = ufs_gget(dp->g_dev, dp->g_mp, ndp->ni_dent.d_ino,
		nomount);
		ufs_gunlock(pdp);
		if (dp == NULL)
			goto bad2;
		if(dp->g_mp->m_fstype != GT_ULTRIX) {
#ifdef GFSDEBUG
			if(GFS[1])
				cprintf("ufs_name: (7) different fs type dp 0x%x (%d)\n",
				dp, dp->g_number);
#endif
			(void)GRELE(pdp);
			ndp->ni_cp = cp;
			ndp->ni_pdir = dp;
			dp->g_flag |= GINCOMPLETE;
			dp->g_rdev = NULL;
			return(dp);
		}
	}

	/*
	 * insert name into cache if appropriate
	 */
	if (makeentry) {
		if (ncp != NULL)
			panic("ufs_namei: duplicating cache");

			/*
			 * free the cache slot at head of lru chain
			 */
		if (ncp = nchhead) {
				/* remove from lru chain */
			*ncp->nc_prev = ncp->nc_nxt;
			if (ncp->nc_nxt)
				ncp->nc_nxt->nc_prev = ncp->nc_prev;
			else
				nchtail = ncp->nc_prev;

				/* remove from old hash chain */
			remque(ncp);

				/* grab the inode we just found */
			ncp->nc_ip = dp;

				/* fill in cache info */
			ncp->nc_ino = pdp->g_number;	/* parents inum */
			ncp->nc_dev = pdp->g_dev;	/* & device */
			ncp->nc_idev = dp->g_dev;	/* our device */
			ncp->nc_id = dp->g_id;		/* identifier */
			ncp->nc_nlen = ndp->ni_dent.d_namlen;
			bcopy(ndp->ni_dent.d_name, ncp->nc_name, ncp->nc_nlen);

				/* link at end of lru chain */
			ncp->nc_nxt = NULL;
			ncp->nc_prev = nchtail;
			*nchtail = ncp;
			nchtail = &ncp->nc_nxt;

				/* and insert on hash chain */
			insque(ncp, nhp);
		}
	}

haveino:
	mp = dp->g_mp;

	/*
	 * Check for symbolic link
	 */
	if ((dp->g_mode & GFMT) == GFLNK &&
	    ((ndp->ni_nameiop & FOLLOW) || *cp == '/')) {
		u_int pathlen = strlen(cp) + 1;

		if (dp->g_size + pathlen >= MAXPATHLEN - 1) {
			u.u_error = ENAMETOOLONG;
			goto bad2;
		}
		if (++ndp->ni_slcnt > MAXSYMLINKS) {
			u.u_error = ELOOP;
			goto bad2;
		}
		ovbcopy(cp, ndp->ni_dirp + dp->g_size, pathlen);
		u.u_error =
		    rdwri(UIO_READ, dp, ndp->ni_dirp, (int)dp->g_size,
			0, 1, (int *)0);
		if (u.u_error)
			goto bad2;
		cp = ndp->ni_dirp;
		gput(dp);
		if (*cp == '/') {
			ufs_grele(pdp);
			while (*cp == '/')
				cp++;
			if ((dp = u.u_rdir) == NULL)
				dp = rootdir;
			ufs_glock(dp);
			dp->g_count++;
		} else {
			dp = pdp;
			ufs_glock(dp);
		}
		mp = dp->g_mp;
		goto dirloop;
	}

	/*
	 * Not a symbolic link.  If more pathname,
	 * continue at next component, else return.
	 */
	
	/* 
	 * for nomount, if we are at a mount point and there is more
	 * pathname to translate, we need to terminate the name translation,
	 * set u.u_error and return
	 */
	
	if(*cp && nomount && (dp->g_flag & GMOUNT)) {
#ifdef GFSDEBUG
		if(GFS[1])
			cprintf("ufs_namei: nomount dp 0x%x (%d) pdp 0x%x (%d) flags 0%o\n",
			dp, dp->g_number, pdp, pdp->g_number, pdp->g_flag);
#endif
		u.u_error = ENOENT;
		goto bad2;
	}
	if (*cp == '/') {
		while (*cp == '/')
			cp++;
		gfs_grele(pdp);
		goto dirloop;
	}
	if (lockparent)
		ndp->ni_pdir = pdp;
	else
		gfs_grele(pdp);
	dp->g_flag &= ~GINCOMPLETE;
	ndp->ni_cp = cp;
	if(((dp->g_mode & GFMT) == GFCHR) ||
	((dp->g_mode & GFMT) == GFBLK)) {
		dp->g_rdev = G_TO_I(dp)->di_rdev;
	} else
		dp->g_rdev = NULL;
#ifdef GFSDEBUG
	if(GFS[1])
		cprintf("ufs_namei: (8) dp 0x%x (%d)\n", dp, dp->g_number);
#endif
	return (dp);
bad2:
	ufs_grele(pdp);
bad:
	if (bp)
		brelse(bp);


	if (dp) {
		dp->g_flag &= ~GINCOMPLETE;		
		gput(dp);
	}
	ndp->ni_cp = cp;
	return (NULL);
}


dirbad(gp, offset, how)
	struct gnode *gp;
	off_t offset;
	char *how;
{

	printf("%s: bad dir ino %d at offset %d: %s\n",
	    FS(gp)->fs_fsmnt, gp->g_number, offset, how);
}

/*
 * Do consistency checking on a directory entry:
 *	record length must be multiple of 4
 *	record length must not be non-negative
 *	entry must fit in rest of its DIRBLKSIZ block
 *	record must be large enough to contain entry
 *	name is not longer than MAXNAMLEN
 *	name must be as long as advertised, and null terminated
 */
dirbadentry(ep, entryoffsetinblock)
	register struct direct *ep;
	int entryoffsetinblock;
{
	register int i;

	if ((ep->d_reclen & 0x3) != 0 || ep->d_reclen <= 0 ||
	    ep->d_reclen > DIRBLKSIZ - (entryoffsetinblock & (DIRBLKSIZ - 1)) ||
	    ep->d_reclen < DIRSIZ(ep) || ep->d_namlen > MAXNAMLEN)
		return (1);
	for (i = 0; i < ep->d_namlen; i++)
		if (ep->d_name[i] == '\0')
			return (1);
	return (ep->d_name[i]);
}

/*
 * Write a directory entry after a call to namei, using the parameters
 * which it left in the u. area.  The argument gp is the inode which
 * the new directory entry will refer to.  The u. area field ndp->ni_pdir is
 * a pointer to the directory to be written, which was left locked by
 * namei.  Remaining parameters (ndp->ni_offset, ndp->ni_count) indicate
 * how the space for the new entry is to be gotten.
 */
direnter(gp, ndp)
	struct gnode *gp;
	register struct nameidata *ndp;
{
	register struct direct *ep, *nep;
	register struct gnode *dp = (struct gnode *)ndp->ni_pdir;
	struct buf *bp;
	int loc, spacefree, error = 0;
	u_int dsize;
	int newentrysize;
	char *dirbuf;

	ndp->ni_dent.d_ino = gp->g_number;
	newentrysize = DIRSIZ(&ndp->ni_dent);
	if (ndp->ni_count == 0) {
		/*
		 * If ndp->ni_count is 0, then namei could find no space in the
		 * directory. In this case ndp->ni_offset will be on a directory
		 * block boundary and we will write the new entry into a fresh
		 * block.
		 */
		if (ndp->ni_offset&(DIRBLKSIZ-1))
			panic("wdir: newblk");
		ndp->ni_dent.d_reclen = DIRBLKSIZ;
		error = rdwri(UIO_WRITE, dp, (caddr_t)&ndp->ni_dent,
		    newentrysize, ndp->ni_offset, 1, (int *)0);
		if (DIRBLKSIZ > FS(dp)->fs_fsize)
			panic("wdir: blksize"); /* XXX - should grow w/bmap() */
		else
			dp->g_size = roundup(dp->g_size, DIRBLKSIZ);
		gput(dp);
		return (error);
	}

	/*
	 * If ndp->ni_count is non-zero, then namei found space for the new
	 * entry in the range ndp->ni_offset to ndp->ni_offset + ndp->ni_count.
	 * in the directory.  To use this space, we may have to compact
	 * the entries located there, by copying them together towards
	 * the beginning of the block, leaving the free space in
	 * one usable chunk at the end.
	 */

	/*
	 * Increase size of directory if entry eats into new space.
	 * This should never push the size past a new multiple of
	 * DIRBLKSIZE.
	 *
	 * N.B. - THIS IS AN ARTIFACT OF 4.2 AND SHOULD NEVER HAPPEN.
	 */
	if (ndp->ni_offset + ndp->ni_count > dp->g_size)
		dp->g_size = ndp->ni_offset + ndp->ni_count;

	/*
	 * Get the block containing the space for the new directory
	 * entry.  Should return error by result instead of u.u_error.
	 */
	bp = blkatoff(dp, ndp->ni_offset, (char **)&dirbuf);
	if (bp == 0) {
		gput(dp);
		return (u.u_error);
	}

	/*
	 * Find space for the new entry.  In the simple case, the
	 * entry at offset base will have the space.  If it does
	 * not, then namei arranged that compacting the region
	 * ndp->ni_offset to ndp->ni_offset+ndp->ni_count would yield the space.
	 */
	ep = (struct direct *)dirbuf;
	dsize = DIRSIZ(ep);
	spacefree = ep->d_reclen - dsize;
	for (loc = ep->d_reclen; loc < ndp->ni_count; ) {
		nep = (struct direct *)(dirbuf + loc);
		if (ep->d_ino) {
			/* trim the existing slot */
			ep->d_reclen = dsize;
			ep = (struct direct *)((char *)ep + dsize);
		} else {
			/* overwrite; nothing there; header is ours */
			spacefree += dsize;	
		}
		dsize = DIRSIZ(nep);
		spacefree += nep->d_reclen - dsize;
		loc += nep->d_reclen;
		bcopy((caddr_t)nep, (caddr_t)ep, dsize);
	}
	/*
	 * Update the pointer fields in the previous entry (if any),
	 * copy in the new entry, and write out the block.
	 */
	if (ep->d_ino == 0) {
		if (spacefree + dsize < newentrysize)
			panic("wdir: compact1");
		ndp->ni_dent.d_reclen = spacefree + dsize;
	} else {
		if (spacefree < newentrysize)
			panic("wdir: compact2");
		ndp->ni_dent.d_reclen = spacefree;
		ep->d_reclen = dsize;
		ep = (struct direct *)((char *)ep + dsize);
	}
	bcopy((caddr_t)&ndp->ni_dent, (caddr_t)ep, (u_int)newentrysize);
	bwrite(bp);
	dp->g_flag |= GUPD|GCHG;
	if (ndp->ni_endoff && ndp->ni_endoff < dp->g_size)
		ufs_gtrunc(dp, (u_long) ndp->ni_endoff, (struct ucred *) 0);
	gput(dp);
	return (error);
}

/*
 * Remove a directory entry after a call to namei, using the
 * parameters which it left in the u. area.  The u. entry
 * ni_offset contains the offset into the directory of the
 * entry to be eliminated.  The ni_count field contains the
 * size of the previous record in the directory.  If this
 * is 0, the first entry is being deleted, so we need only
 * zero the inode number to mark the entry as free.  If the
 * entry isn't the first in the directory, we must reclaim
 * the space of the now empty record by adding the record size
 * to the size of the previous entry.
 */
dirremove(ndp)
	register struct nameidata *ndp;
{
	register struct gnode *dp = (struct gnode *)ndp->ni_pdir;
	register struct buf *bp;
	struct direct *ep;

	if (ndp->ni_count == 0) {
		/*
		 * First entry in block: set d_ino to zero.
		 */
		ndp->ni_dent.d_ino = 0;
		(void) rdwri(UIO_WRITE, dp, (caddr_t)&ndp->ni_dent,
		    (int)DIRSIZ(&ndp->ni_dent), ndp->ni_offset, 1, (int *)0);
	} else {
		/*
		 * Collapse new free space into previous entry.
		 */
		bp = blkatoff(dp, (int)(ndp->ni_offset - ndp->ni_count),
			(char **)&ep);
/************************************************************
#ifdef GFSDEBUG
		if(GFS[13])
			cprintf("dirremove: bp 0x%x\n", bp);
#endif
*********************************************/
		if (bp == 0)
			return (0);
		ep->d_reclen += ndp->ni_dent.d_reclen;
		bwrite(bp);
		dp->g_flag |= GUPD|GCHG;
	}
	return (1);
}

/*
 * Rewrite an existing directory entry to point at the inode
 * supplied.  The parameters describing the directory entry are
 * set up by a call to namei.
 */
dirrewrite(dp, gp, ndp)
	struct gnode *dp, *gp;
	struct nameidata *ndp;
{

	ndp->ni_dent.d_ino = gp->g_number;
	u.u_error = rdwri(UIO_WRITE, dp, (caddr_t)&ndp->ni_dent,
		(int)DIRSIZ(&ndp->ni_dent), ndp->ni_offset, 1, (int *)0);
	gput(dp);
}

/*
 * Return buffer with contents of block "offset"
 * from the beginning of directory "gp".  If "res"
 * is non-zero, fill it in with a pointer to the
 * remaining space in the directory.
 */
struct buf *
blkatoff(gp, offset, res)
	register struct gnode *gp;
	register off_t offset;
	char **res;
{
	register struct fs *fs = FS(gp);
	register daddr_t lbn = lblkno(fs, offset);
	register struct buf *bp;
	register daddr_t bn;
	int bsize = blksize(fs, gp, lbn);

	bn = ufs_bmap(gp, (int)lbn, B_READ, bsize, 0, 0);
	if (u.u_error)
		return (0);
	if (bn == (daddr_t)(-1)) {
		dirbad(gp, offset, "hole in dir");
		return (0);
	}
	bp = bread(gp->g_dev, bn, bsize, (struct gnode *) NULL);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return (0);
	}
	if (res)
		*res = bp->b_un.b_addr + blkoff(fs, offset);
	return (bp);
}

/*
 * Check if a directory is empty or not.
 * Inode supplied must be locked.
 *
 * Using a struct dirtemplate here is not precisely
 * what we want, but better than using a struct direct.
 *
 * NB: does not handle corrupted directories.
 */
dirempty(gp, parentino)
	register struct gnode *gp;
	gno_t parentino;
{
	register off_t off;
	struct dirtemplate dbuf;
	register struct direct *dp = (struct direct *)&dbuf;
	int error, count;
#define	MINDIRSIZ (sizeof (struct dirtemplate) / 2)

	for (off = 0; off < gp->g_size; off += dp->d_reclen) {
		error = rdwri(UIO_READ, gp, (caddr_t)dp, MINDIRSIZ,
 		    off, 1, &count);
		/*
		 * Since we read MINDIRSIZ, residual must
		 * be 0 unless we're at end of file.
		 */
#ifdef GFSDEBUG
		if(GFS[13])
			cprintf("dirempty: error %d count %d\n", error, count);
#endif
		if (error || count != 0) 
			return (0);
		if (dp->d_reclen <= 0)
			return (0);
		/* skip empty entries */
		if (dp->d_ino == 0)
			continue;
		/* accept only "." and ".." */

#ifdef GFSDEBUG
		if(GFS[13] && dp->d_namlen)
			cprintf("dirempty: namelen %d name '%s'\n",
			dp->d_namlen, dp->d_name);
#endif
		if (dp->d_namlen > 2) 
			return (0);
		if (dp->d_name[0] != '.') 
			return (0);

		/*
		 * At this point d_namlen must be 1 or 2.
		 * 1 implies ".", 2 implies ".." if second
		 * char is also "."
		 */
		if (dp->d_namlen == 1)
			continue;
		if (dp->d_name[1] == '.' && dp->d_ino == parentino)
			continue;
		return (0);
	}
	return (1);
}

/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always gput() before returning.
 */
/* this routine is ok for it's purposes.  it should eventually go away
 * since it is only used by rename
 */
checkpath(source, target, flag)
	struct gnode *source, *target;
	int flag;
{
	struct dirtemplate dirbuf;
	register struct gnode *gp;
	int error = 0;

	gp = target;
	if (gp->g_number == source->g_number) {
		error = EEXIST;
		goto out;
	}
	if (gp->g_number == ROOTINO)
		goto out;

	for (;;) {
		if ((gp->g_mode&GFMT) != GFDIR) {
			error = ENOTDIR;
			break;
		}
		error = rdwri(UIO_READ, gp, (caddr_t)&dirbuf,
			sizeof (struct dirtemplate), (off_t)0, 1, (int *)0);
		if (error != 0)
			break;
		if (dirbuf.dotdot_namlen != 2 ||
		    dirbuf.dotdot_name[0] != '.' ||
		    dirbuf.dotdot_name[1] != '.') {
			error = ENOTDIR;
			break;
		}
		if (dirbuf.dotdot_ino == source->g_number) {
			error = EINVAL;
			break;
		}
		if (dirbuf.dotdot_ino == ROOTINO)
			break;
		gput(gp);
		gp = (struct gnode *)ufs_gget(gp->g_dev,
			 gp->g_mp, dirbuf.dotdot_ino, flag);
		if (gp == NULL) {
			error = u.u_error;
			break;
		}
	}

out:
	if (error == ENOTDIR)
		printf("checkpath: .. not a directory\n");
	if (gp != NULL)
		gput(gp);
	return (error);
}
