/*
 * fsmrg.c
 */
#ifndef lint
static	char	*sccsid = "@(#)fsmrg.c	1.1  (ULTRIX)        3/20/86";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * This utility opens the kernel image specified, locates two
 * variables (mdsize and memdev).  If present, `mdsize' blocks of the
 * specified file system image are copied into the kernel image at
 * `memdev'. It is used to install a memory file system for use in the
 * Ultrix-32 standalone environment.
 */
#include <stdio.h>
#include <sys/param.h>
#include <a.out.h>
#include <sys/buf.h>

struct nlist nmlst[3];

main(argc, argv)
	int argc;
	char **argv;
{
	struct exec x;
	int i, fs, vm;
	int mdsize;
	int memdev;
	char *buf[512];

	argc--, argv++;
	if (argc < 2) {
		fprintf(stderr, "Usage: fsmrg fs_image kernel_image\n");
		exit(1);
	}
	/*
	 * Initialize the name list
	 */
	nmlst[0].n_un.n_name = "_mdsize";
	nmlst[1].n_un.n_name = "_memdev";
	nmlst[2].n_un.n_name = "";
	nlist(argv[1], nmlst);
	if(nmlst[0].n_type == 0) {
		fprintf(stderr, "no %s namelist\n", argv[1]);
		exit(1);
	}
	if ((vm = open(argv[1], 2)) < 0) {
		fprintf(stderr, "Can't open %s\n", argv[1]);
		exit(1);
	}
	if ((fs = open(argv[0], 0)) < 0) {
		fprintf(stderr, "Can't open %s\n", argv[0]);
		exit(1);
	}
	if ((i = read(vm, (char *)&x, sizeof x)) != sizeof x) {
		fprintf(stderr, "Can't read %s\n", argv[1]);
		exit(1);
	}
	mdsize = ((int)nmlst[0].n_value); 
	mdsize &= ~0x80000000;
	mdsize += sizeof x;
	if (x.a_text % 1024)
		mdsize -= (1024 - (x.a_text % 1024));
	if ((lseek(vm, mdsize, 0)) < 0) {
		fprintf(stderr, "can't find mdsize\n");
		exit(1);
	}
	if ((read(vm, &mdsize, sizeof mdsize)) != sizeof mdsize) {
		fprintf(stderr, "can't read mdsize\n");
		exit(1);
	}
	memdev = ((int)nmlst[1].n_value); 
	memdev &= ~0x80000000;
	memdev += sizeof x;
	if (x.a_text % 1024)
		memdev -= (1024 - (x.a_text % 1024));
	lseek(vm, memdev, 0);
	fprintf(stdout, "Writing %d blocks of %s into %s\n",
		mdsize, argv[0], argv[1]);
	for (i = 0 ; i < mdsize; i ++) {
		if ((read (fs, buf, 512)) != 512) {
			fprintf(stderr, "%s: read failed\n", argv[0]);
			exit(1);
		}
		if ((write (vm, buf, 512)) != 512) {
			fprintf(stderr, "%s: write failed\n", argv[1]);
			exit(1);
		}
	}
	close(vm);
	close(fs);
}
