/*
 * sys.c
 */

#ifndef lint
static	char	*sccsid = "@(#)sys.c	1.7	(ULTRIX)	3/23/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1986 by			*
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
/*	sys.c	6.2	83/09/23	*/

/*
 * Modification History
 *
 * 02-Jul-85 -dunnuck
 *	defined saveCSR as int in order to fix a load problem
 *
 * 14-Mar-85 -tresvik
 *	Increased max unit select from 31 to 63 for VAX8600
 *
 * 11-Dec-84 -reilly
 *	Fixed the problem with that handled disk partitioning did not work
 *	with tapes.
 *
 * 11-Dec-84 -p.keilty
 *	Added count - i to the return of read. Where i is 
 *	some number less than count on error.
 *
 * 29-Nov-84 -tresvik
 *	Added SMALLSYS option.  If this option is enabled chunks of code
 *	which are not used in bootxx are eliminated.
 *	This is necessary since most of the bootxx images exceeded the 
 *	15 block limit.
 *
 * 03-Nov-84 -reilly
 * 001 - Added the rsblk routine that willbe used by any of the driver's
 *	 that need the partitio table info.
 *
 * 15-Nov-84 -p.keilty
 *	Added file->i_error check to read routine. This will terminate
 *	the read.
 *
 * 25-Sep-84 -tresvik
 *
 *	Reset file->i_flgs upon file open errors.  This prevents file
 *	slot usage for a file which did not open.
 */


#include "../h/param.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../h/dir.h"
#include "saio.h"

ino_t	dlook();
long	saveCSR;

struct dirstuff {
	int loc;
	struct iob *io;
};

static
openi(n, io)
	register struct iob *io;
{
	register struct dinode *dp;
	int cc;

	io->i_offset = 0;
	io->i_bn = fsbtodb(&io->i_fs, itod(&io->i_fs, n)) + io->i_boff;
	io->i_cc = io->i_fs.fs_bsize;
	io->i_ma = io->i_buf;
	cc = devread(io);
	dp = (struct dinode *)io->i_buf;
	G_TO_I(&io->i_ino)->di_ic = dp[itoo(&io->i_fs, n)].di_ic;
	return (cc);
}

static
find(path, file)
	register char *path;
	struct iob *file;
{
	register char *q;
	char c;
	int n;

	if (path==NULL || *path=='\0') {
		printf("null path\n");
		return (0);
	}

	if (openi((ino_t) ROOTINO, file) < 0) {
		printf("can't read root inode\n");
		return (0);
	}
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while(*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';

		if ((n = dlook(path, file)) != 0) {
			if (c == '\0')
				break;
			if (openi(n, file) < 0)
				return (0);
			*q = c;
			path = q;
			continue;
		} else {
			printf("%s not found\n", path);
			return (0);
		}
	}
	return (n);
}

static daddr_t
sbmap(io, bn)
	register struct iob *io;
	daddr_t bn;
{
	register struct gnode *ip;
	int i, j, sh;
	daddr_t nb, *bap;

	ip = &io->i_ino;
	if (bn < 0) {
		printf("bn negative\n");
		return ((daddr_t)0);
	}

	/*
	 * blocks 0..NDADDR are direct blocks
	 */
	if(bn < NDADDR) {
		nb = G_TO_I(ip)->di_db[bn];
		return (nb);
	}

	/*
	 * addresses NIADDR have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(&io->i_fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0) {
		printf("bn ovf %D\n", bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch the first indirect block address from the inode
	 */
	nb = G_TO_I(ip)->di_ib[NIADDR - j];
	if (nb == 0) {
		printf("bn void %D\n",bn);
		return ((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= NIADDR; j++) {
		if (blknos[j] != nb) {
			io->i_bn = fsbtodb(&io->i_fs, nb) + io->i_boff;
			io->i_ma = b[j];
			io->i_cc = io->i_fs.fs_bsize;
			if (devread(io) != io->i_fs.fs_bsize) {
				if (io->i_error)
					errno = io->i_error;
				printf("bn %D: read error\n", io->i_bn);
				return ((daddr_t)0);
			}
			blknos[j] = nb;
		}
		bap = (daddr_t *)b[j];
		sh /= NINDIR(&io->i_fs);
		i = (bn / sh) % NINDIR(&io->i_fs);
		nb = bap[i];
		if(nb == 0) {
			printf("bn void %D\n",bn);
			return ((daddr_t)0);
		}
	}
	return (nb);
}

static ino_t
dlook(s, io)
	char *s;
	register struct iob *io;
{
	register struct direct *dp;
	register struct gnode *ip;
	struct dirstuff dirp;
	int len;

	if (s == NULL || *s == '\0')
		return (0);
	ip = &io->i_ino;
	if ((ip->g_mode&GFMT) != GFDIR) {
		printf("not a directory\n");
		return (0);
	}
	if (ip->g_size == 0) {
		printf("zero length directory\n");
		return (0);
	}
	len = strlen(s);
	dirp.loc = 0;
	dirp.io = io;
	for (dp = readdir(&dirp); dp != NULL; dp = readdir(&dirp)) {
		if(dp->d_ino == 0)
			continue;
		if (dp->d_namlen == len && !strcmp(s, dp->d_name))
			return (dp->d_ino);
	}
	return (0);
}

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register struct dirstuff *dirp;
{
	register struct direct *dp;
	register struct iob *io;
	daddr_t lbn, d;
	int off;

	io = dirp->io;
	for(;;) {
		if (dirp->loc >= io->i_ino.g_size)
			return (NULL);
		off = blkoff(&io->i_fs, dirp->loc);
		if (off == 0) {
			lbn = lblkno(&io->i_fs, dirp->loc);
			d = sbmap(io, lbn);
			if(d == 0)
				return NULL;
			io->i_bn = fsbtodb(&io->i_fs, d) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = blksize(&io->i_fs, &io->i_ino, lbn);
			if (devread(io) < 0) {
				errno = io->i_error;
				printf("bn %D: read error\n", io->i_bn);
				return (NULL);
			}
		}
		dp = (struct direct *)(io->i_buf + off);
		dirp->loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}

lseek(fdesc, addr, ptr)
	int fdesc, ptr;
	off_t addr;
{
	register struct iob *io;

	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		errno = EOFFSET;
		return (-1);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs & F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	io->i_offset = addr;
	io->i_bn = addr / DEV_BSIZE;
	io->i_cc = 0;
	return (0);
}

getc(fdesc)
	int fdesc;
{
	register struct iob *io;
	register struct fs *fs;
	register char *p;
	int c, lbn, off, size, diff;


#ifndef SMALLSYS
	if (fdesc >= 0 && fdesc <= 2)
		return (getchar());
#endif SMALLSYS
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	p = io->i_ma;
	if (io->i_cc <= 0) {
		if ((io->i_flgs & F_FILE) != 0) {
			diff = io->i_ino.g_size - io->i_offset;
			if (diff <= 0)
				return (-1);
			fs = &io->i_fs;
			lbn = lblkno(fs, io->i_offset);
			io->i_bn = fsbtodb(fs, sbmap(io, lbn)) + io->i_boff;
			off = blkoff(fs, io->i_offset);
			size = blksize(fs, &io->i_ino, lbn);
		} else {
			io->i_bn = io->i_offset / DEV_BSIZE;
			off = 0;
			size = DEV_BSIZE;
		}
		io->i_ma = io->i_buf;
		io->i_cc = size;
		if (devread(io) < 0) {
			errno = io->i_error;
			return (-1);
		}
		if ((io->i_flgs & F_FILE) != 0) {
			if (io->i_offset - off + size >= io->i_ino.g_size)
				io->i_cc = diff + off;
			io->i_cc -= off;
		}
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = (unsigned)*p++;
	io->i_ma = p;
	return (c);
}

/* does this port?
getw(fdesc)
	int fdesc;
{
	register w,i;
	register char *cp;
	int val;

	for (i = 0, val = 0, cp = &val; i < sizeof(val); i++) {
		w = getc(fdesc);
		if (w < 0) {
			if (i == 0)
				return (-1);
			else
				return (val);
		}
		*cp++ = w;
	}
	return (val);
}
*/
int	errno;

read(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i;
	register struct iob *file;

	errno = 0;
#ifndef SMALLSYS
	if (fdesc >= 0 & fdesc <= 2) {
		i = count;
		do {
			*buf = getchar();
		} while (--i && *buf++ != '\n');
		return (count - i);
	}
#endif SMALLSYS
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_READ) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs & F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		file->i_bn = file->i_boff + (file->i_offset / DEV_BSIZE);
		i = devread(file);
		file->i_offset += count;
		if (i < 0)
			errno = file->i_error;
		return (i);
	} else {
		if (file->i_offset+count > file->i_ino.g_size)
			count = file->i_ino.g_size - file->i_offset;
		if ((i = count) <= 0)
			return (0);
		do {
			*buf++ = getc(fdesc+3);
		} while ((--i) && (file->i_error == 0)); /*stop read on err*/
		return (count - i);
	}
}

#ifndef SMALLSYS
write(fdesc, buf, count)
	int fdesc, count;
	char *buf;
{
	register i;
	register struct iob *file;

	errno = 0;
	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(*buf++);
		return (count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_WRITE) == 0) {
		errno = EBADF;
		return (-1);
	}
	file->i_cc = count;
	file->i_ma = buf;
	file->i_bn = file->i_boff + (file->i_offset / DEV_BSIZE);
	i = devwrite(file);
	file->i_offset += count;
	if (i < 0)
		errno = file->i_error;
	return (i);
}
#endif SMALLSYS

int	openfirst = 1;

open(str, how)
	char *str;
	int how;
{
	register char *cp;
	int i;
	register struct iob *file;
	register struct devsw *dp;
	int tmp_boff;				/* 001 */
	int fdesc;
	long atol();

	if (openfirst) {
		for (i = 0; i < NFILES; i++)
			iob[i].i_flgs = 0;
		openfirst = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
	_stop("No more file slots");
gotfile:
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;

	for (cp = str; *cp && *cp != '('; cp++)
			;
	if (*cp != '(') {
		printf("Bad device\n");
		file->i_flgs = 0;
		errno = EDEV;
		return (-1);
	}
	*cp++ = '\0';
	for (dp = devsw; dp->dv_name; dp++) {
		if (!strcmp(str, dp->dv_name))
			goto gotdev;
	}
	printf("Unknown device\n");
	file->i_flgs = 0;
	errno = ENXIO;
	return (-1);
gotdev:
	*(cp-1) = '(';
	file->i_ino.g_dev = dp-devsw;
	file->i_unit = *cp++ - '0';
	if (*cp >= '0' && *cp <= '9')
		file->i_unit = file->i_unit * 10 + *cp++ - '0';
	if (file->i_unit < 0 || file->i_unit > 63) {
		printf("Bad unit specifier\n");
		file->i_flgs = 0;
		errno = EUNIT;
		return (-1);
	}
	if (*cp++ != ',') {
badoff:
		printf("Missing offset specification\n");
		file->i_flgs = 0;
		errno = EOFFSET;
		return (-1);
	}
	file->i_boff = atol(cp);
	for (;;) {
		if (*cp == ')')
			break;
		if (*cp++)
			continue;
		goto badoff;
	}
#ifndef SMALLSYS
	/*
	 *	If we are opening a tape we don't to the disk partitioning
	 *	stuff
	 */
	if ( devsw[file->i_ino.g_dev].dv_flags != B_TAPE ) {

		/*
	 	 *	Before we can actually open up the device we first must
	 	 *	try to read the partition table if nay exists
	 	 */

		tmp_boff = file->i_boff;
		file->i_boff = 0;

		/*
	 	 *	Open up the "a" partition
	 	 */
		devopen(file);
		file->i_ma = (char *)(&file->i_fs);
		file->i_cc = SBSIZE;
		file->i_bn = SBLOCK;
		file->i_offset = 0;

		/*
	 	 *	Read the superblock that may contain the partition
		 *  	tables
	 	 */
		if (devread(file) >= 0) {
			/*
		 	 *	Now check to see if partition table exists
		 	 */
			if ( ptinfo(file,tmp_boff) < 0 ) {
				/*
			 	 *	Partition table does not exist so just
			 	 *	open the device that user has specified
			 	 */
				file->i_boff = tmp_boff;
				devopen(file);
			}
		}
	}
	else
		/*
		 *	We come here if we are a tape device
		 */
#endif SMALLSYS
		devopen(file);

	if (*++cp == '\0') {
		file->i_flgs |= how+1;
		file->i_cc = 0;
		file->i_offset = 0;
		return (fdesc+3);
	}
	file->i_ma = (char *)(&file->i_fs);
	file->i_cc = SBSIZE;
	file->i_bn = SBLOCK + file->i_boff;
	file->i_offset = 0;
	if (devread(file) < 0) {
		errno = file->i_error;
		file->i_flgs = 0;
		printf("super block read error\n");
		return (-1);
	}
	if ((i = find(cp, file)) == 0) {
		file->i_flgs = 0;
		errno = ESRCH;
		return (-1);
	}
	if (how != 0) {
		printf("Can't write files yet.. Sorry\n");
		file->i_flgs = 0;
		errno = EIO;
		return (-1);
	}
	if (openi(i, file) < 0) {
		file->i_flgs = 0;
		errno = file->i_error;
		return (-1);
	}
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how+1);
	return (fdesc+3);
}

close(fdesc)
	int fdesc;
{
	struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	if ((file->i_flgs&F_FILE) == 0)
		devclose(file);
	file->i_flgs = 0;
	return (0);
}

#ifndef SMALLSYS
ioctl(fdesc, cmd, arg)
	int fdesc, cmd;
	char *arg;
{
	register struct iob *file;
	int error = 0;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES ||
	    ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0) {
		errno = EBADF;
		return (-1);
	}
	switch (cmd) {

	case SAIOHDR:
		file->i_flgs |= F_HDR;
		break;

	case SAIOCHECK:
		file->i_flgs |= F_CHECK;
		break;

	case SAIOHCHECK:
		file->i_flgs |= F_HCHECK;
		break;

	case SAIONOBAD:
		file->i_flgs |= F_NBSF;
		break;

	case SAIODOBAD:
		file->i_flgs &= ~F_NBSF;
		break;

	case SAIOECCLIM:
		file->i_flgs |= F_ECCLM;
		break;

	case SAIOECCUNL:
		file->i_flgs &= ~F_ECCLM;
		break;

	case SAIOSEVRE:
		file->i_flgs |= F_SEVRE;
		break;

	case SAIONSEVRE:
		file->i_flgs &= ~F_SEVRE;
		break;

	default:
		error = devioctl(file, cmd, arg);
		break;
	}
	if (error < 0)
		errno = file->i_error;
	return (error);
}
#endif SMALLSYS

exit()
{
	_stop("Exit called");
}

_stop(s)
	char *s;
{
	int i;

	for (i = 0; i < NFILES; i++)
		if (iob[i].i_flgs != 0)
			close(i);
	printf("%s\n", s);
	_rtt();
}

trap(ps)
	int ps;
{
	printf("Trap %o\n", ps);
	for (;;)
		;
}

#ifndef SMALLSYS
/*
 *	Called by any of the disk driver's that need the partition table
 *	info
 */
ptinfo(file, boff)
	register struct iob *file;
	int boff;
{
	struct pt *pt;

	/*
	 *	Check to see if the block that was read in is a superblock
	 */
	if ( file->i_fs.fs_magic == FS_MAGIC ) {

		/*
		 *	Get the possible partition table
		 */
		pt = (struct pt *)&file->i_un.dummy[SBSIZE - sizeof(struct pt)];

		/*
		 *	Id the a real partition table
		 */
		if ( pt->pt_magic == PT_MAGIC ) {

			/*
			 *	We have a real partition table so now
			 *	set the driver's part tbl
			 */
			file->i_boff = pt->pt_part[boff].pi_blkoff;
			return(0);
		}
		else
			return(-1);
	}
	else
		return(-1);
}
#endif SMALLSYS
