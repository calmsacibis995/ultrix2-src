#ifndef lint
static char *sccsid = "@(#)kern_acct.c	1.6	ULTRIX	10/3/86";
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

/************************************************************************
 *
 *			Modification History
 *
 *	Koehler 11 Sep 86
 *	  gfs namei interface change
 *
 *	Stephen Reilly, 09-Sept-85
 *	  Modified to handle the new 4.3BSD namei code.
 *
 *
 ***********************************************************************/
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode.h"
#include "../h/mount.h"
#include "../h/buf.h"
#include "../h/kernel.h"
#include "../h/acct.h"
#include "../h/uio.h"
#include "../h/kmalloc.h"

/*
 * SHOULD REPLACE THIS WITH A DRIVER THAT CAN BE READ TO SIMPLIFY.
 */
struct	gnode *acctp;
struct	gnode *savacctp;

/*
 * Perform process accounting functions.
 */
sysacct()
{
	register struct gnode *gp;
	register struct a {
		char	*fname;
	} *uap = (struct a *)u.u_ap;
	register struct nameidata *ndp = &u.u_nd;

	if (suser()) {
		if (savacctp) {
			acctp = savacctp;
			savacctp = NULL;
		}
		if (uap->fname==NULL) {
			if (gp = acctp) {
				GRELE(gp);
				acctp = NULL;
			}
			return;
		}
 		ndp->ni_nameiop = LOOKUP | FOLLOW;

		if((ndp->ni_dirp = km_alloc(MAXPATHLEN, KM_SLEEP)) == NULL) {
			return;
		}
 		if(u.u_error = copyinstr(uap->fname, ndp->ni_dirp, MAXPATHLEN,
		(u_int *) 0)) {
			km_free(ndp->ni_dirp, MAXPATHLEN);
			return;
		}

 		gp = gfs_namei(ndp);

		km_free(ndp->ni_dirp, MAXPATHLEN);
		if(gp == NULL)
			return;
		if((gp->g_mode & GFMT) != GFREG) {
			u.u_error = EACCES;
			gput(gp);
			return;
		}
		if (ISREADONLY(gp->g_mp)) {
			u.u_error = EROFS;
			gput(gp);
			return;
		}
		if (acctp && (acctp->g_number != gp->g_number ||
		    acctp->g_dev != gp->g_dev))
			GRELE(acctp);
		acctp = gp;
		gfs_unlock(gp);
	}
}

int	acctsuspend = 2;	/* stop accounting when < 2% free space left */
int	acctresume = 4;		/* resume when free space risen to > 4% */

struct	acct acctbuf;
/*
 * On exit, write a record on the accounting file.
 */
acct()
{
	register int i;
	register struct gnode *gp;
	register struct fs_data *fs_data;
	off_t siz;
	register struct acct *ap = &acctbuf;
	double itime, million = 1000000.0;

	if (savacctp) {
		fs_data = GGETFSDATA(savacctp->g_mp);
		if(acctresume * fs_data->fd_btot / 100 <= fs_data->fd_bfreen) {
			acctp = savacctp;
			savacctp = NULL;
			printf("Accounting resumed\n");
		}
	}
	if ((gp = acctp) == NULL)
		return;
	fs_data = GGETFSDATA(acctp->g_mp);
	if(acctsuspend * fs_data->fd_btot / 100 > fs_data->fd_bfreen) {
		savacctp = acctp;
		acctp = NULL;
		printf("Accounting suspended\n");
		return;
	}
	gfs_lock(gp);
	for (i = 0; i < sizeof (ap->ac_comm); i++)
		ap->ac_comm[i] = u.u_comm[i];
	/* RR - 001
	 * Changed fields are ac_utime, ac_stime, ac_etime, ac_mem, ac_io
	 * The decision here is to keep the accounting records small (44 bytes)
	 * yet maintain accuracy. The first three times (user, sys, elapsed)
	 * need to be accurate fractions. The mem usage and io are also
	 * floats for accuracy, i.e., we won't overflow, we get fractional usage
	 * of memory and we save bytes over doubles. Note that we also threw
	 * out the routine compress for the 13 bit pseudo-fraction and
	 * 3 bit exponent
	 */
#define tm_to_dbl(tmbuf) (double)tmbuf.tv_sec+(tmbuf.tv_usec/million)
	ap->ac_utime = tm_to_dbl(u.u_ru.ru_utime);
	ap->ac_stime = tm_to_dbl(u.u_ru.ru_stime);
	ap->ac_etime = time.tv_sec - u.u_start;
	ap->ac_btime = u.u_start;
	ap->ac_uid = u.u_ruid;
	ap->ac_gid = u.u_rgid;
	ap->ac_mem = 0.0;
	itime = tm_to_dbl(u.u_ru.ru_utime) + tm_to_dbl(u.u_ru.ru_stime);
	if (itime > 0.01) {
		ap->ac_mem = (u.u_ru.ru_ixrss  + u.u_ru.ru_idrss +
				u.u_ru.ru_ismrss + u.u_ru.ru_isrss) / itime;
	}
	ap->ac_io = u.u_ru.ru_inblock + u.u_ru.ru_oublock;
	if (u.u_ttyp)
		ap->ac_tty = u.u_ttyd;
	else
		ap->ac_tty = NODEV;
	ap->ac_flag = u.u_acflag;
	siz = gp->g_size;
	u.u_error = 0;				/* XXX */
	u.u_error =
	    rdwri(UIO_WRITE, gp, (caddr_t)ap, sizeof (acctbuf), siz,
		1, (int *)0);
	if (u.u_error)
		GTRUNC(gp, (u_long)siz, u.u_cred);
	gfs_unlock(gp);
}
