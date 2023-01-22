#ifndef lint
static	char *sccsid = "@(#)df.c	1.5	(ULTRIX)	4/8/86";
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

#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ustat.h>
#include <strings.h>

/*
 * df
 */
struct	fs_data *mountbuffer;
#define MSIZE (NMOUNT*sizeof(struct fs_data))

int	iflag;

char	*strcpy();

main(argc, argv)
	int argc;
	char **argv;
{
	register int i,used,ret,did_one;
	register struct	fs_data *fd;
	struct stat sbuf;
	register struct stat *sbp = &sbuf;
	char temp[BUFSIZ];
	int loc;
	char *malloc();
	
	while (argc > 1 && argv[1][0]=='-') {
		switch (argv[1][1]) {

		case 'i':
			iflag++;
			break;

		default:
			fprintf(stderr, "usage: df [ -i ] [ filsys... ]\n");
			exit(0);
		}
		argc--, argv++;
	}
	mountbuffer = (struct fs_data *) malloc(MSIZE);
	ret = getmnt(&loc,mountbuffer,MSIZE);
	if (ret < 0) {
		perror("getmnt");
		fprintf(stderr, "%s: cannot get mount info\n",argv[0]);
		exit(1);
	}
	/*
	 * print top part of title.
	 */
	printf("Filesystem    total    kbytes  kbytes  percent");
	if (iflag)
		printf(" inodes  inodes percent");
	printf("\n");
	/*
	 * print second part of titles.
	 */
	printf("   node       kbytes    used    free   used  ");
	if (iflag)
		printf("   used    free   used");
	printf("  Mounted on\n");
	if (argc <= 1) {
		/* print them all */
		for (fd=mountbuffer; fd < &mountbuffer[ret]; fd++) {
			print_df(fd);
		}
		exit(0);
	}
	for (i=1; i<argc; i++) {
	
		if(stat(argv[i], sbp) == -1) {
			sscanf(temp, "cannot stat %s ", argv[i]);
			perror (temp);
			exit(1);
		}
		did_one = 0;
		/* try rdev for special file */
		for (fd=mountbuffer; fd < &mountbuffer[ret]; fd++) {
			if ( fd->fd_dev == sbp->st_rdev) {
				print_df(fd);
				did_one = 1;
				break;
			}
		}
		if (!did_one) {	/* try dev for regular file */
			for (fd=mountbuffer; fd < &mountbuffer[ret]; fd++) {
				if ( fd->fd_dev == sbp->st_dev) {
					print_df(fd);
					did_one = 1;
					break;
				}
			}
		}
		if (!did_one) {
			fprintf(stderr,"Can't find %s in mount table\n",
								argv[i]);
			exit(2);
		}
	}
	exit(0);
}

print_df(fd)
	register struct fs_data *fd;
{
	register int used;
	used = fd->fd_btot - fd->fd_bfree;
	printf("%-12.12s", fd->fd_devname);
	printf("%8d%8d%8d", fd->fd_btot, used, (int) fd->fd_bfreen > 0 ? fd->fd_bfreen : 0);
	printf("%6.0f%%", fd->fd_btot == 0 ? 0.0 : used /
		(double)(fd->fd_btot - (fd->fd_bfree - fd->fd_bfreen)) * 100.0);
	if (iflag) {
		used = fd->fd_gtot - fd->fd_gfree;
		printf(" %8ld%8ld%6.0f%%", used, fd->fd_gfree, fd->fd_gtot == 0
			? 0.0 : used / (double)fd->fd_gtot * 100.0);
	} else 
		printf("  ");
	printf("  %s\n", fd->fd_path);
}
