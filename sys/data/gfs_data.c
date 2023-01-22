#ifndef lint
static char *sccsid = "@(#)gfs_data.c	1.7	ULTRIX	10/3/86";
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

/***********************************************************************
 *
 *		Modification History
 *
 * 15 Sep 86 -- bglover
 *	Add back declaration of nfs_biod()
 *
 * 11 Sep 86 -- koehler
 *	moved the mounting of root into here
 *
 ***********************************************************************/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/mount.h"
#include "../h/user.h"
#include "../h/errno.h"

/* include the rest of the sfs's here */
#include "../h/fs_types.h"

#ifndef BINARY

init_fs() {
	int n_fs;
	extern struct mount *ufs_mount();
#ifdef NFS
	extern struct mount *nfs_mount();
#endif

	bzero((caddr_t)mount_sfs,NUM_FS*sizeof(struct _sfs));

	MOUNTFS(GT_ULTRIX) = ufs_mount;
#ifdef NFS
	MOUNTFS(GT_NFS) = nfs_mount;
#endif
	/* include the rest of the mount functions here */
}

#ifndef NFS
dnlc_init() {
}

nfs_biod() {
}

nfs_mount() {
}

nfs_async_daemon() {
}

nfs_getfh() {
}

nfs_svc() {
}
#endif NFS

mount_root(mp)
	struct mount *mp;
{

#ifdef NFS
	dnlc_init();
#endif
	mp = (struct mount *)ufs_mount((caddr_t) 0, "/", 0, mp, (caddr_t) 0);
	if (mp == 0) {
		if (u.u_error == EROFS) {
			printf("root file system is read only");
		}
		else if (u.u_error==ENODEV) {
			printf("root file system device is offline");
			asm("halt");
		}
		else {
			printf("no root file system");
			asm("halt");
		}
	}
	mp->m_fstype =  GT_ULTRIX;
#ifdef ROOTSYNC
	mp->m_flags |= M_SYNC;
#endif
}
#endif !BINARY
