#ifndef lint
static	char	*sccsid = "@(#)savecore.c	1.9		5/22/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
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
 *	pete keilty 12-Feb-87
 *	Added printf stating checking for dump. Bar
 *
 *	Paul Shaughnessy, 22-May-86
 *	Added saving of the u_area support to save_core. This
 *	required writing a new procedure save_uarea, which
 *	users raw i/o for accesses to the dump device.
 *
 *	Barb Glover, 05-May-86
 *	Changed savecore to use raw i/o to solve "I/O error" problem,
 *	because of file system read past the end of the partition.
 *	Also, append to end of elbuffer file.
 *
 *	Paul Shaughnessy, 09-Apr-86
 *	Changed initialization of namelist values because of the
 *	addition on the include file a.out.h. Added partial
 *	crash dump support, including modifying the namelist
 *	entry for elbuf. Also added a print statement to print
 *	out when saving core.
 *
 *	Barbara Glover, 05-Mar-86
 *	Store elbuf base addr at beg. of elbuffer file
 *	Added include of errlog.h for DUMPSIZE
 *
 *	Barbara Glover, 19-Feb-86
 *	Added save of error log bufffer; added -e switch to
 *	save error log buffer(only).
 *
 *	Stephen Reilly, 22-Apr-85
 * 001- Added the -c switch to enable the user to clear the dump flag
 *	on the dump device.
 *
 ***********************************************************************/

/*
 * savecore
 */
#include <stdio.h>
#include <a.out.h>
#include <stab.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/user.h>
#include "/usr/include/sys/../ufs/fs.h"
#include <sys/errlog.h>
#include <sys/types.h>
#include <sys/un.h>
#include <elcsd.h>
#include <machine/pte.h>

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)

#define BLOCK 0
#define RAW 1

#define eq(a,b) (!strcmp(a,b))
#ifdef vax
#define ok(number) ((number)&0x7fffffff)
#else
#define ok(number) (number)
#endif

#define SHUTDOWNLOG "/usr/adm/shutdownlog"

#define X_DUMPDEV	0
#define X_DUMPLO	1
#define X_TIME		2
#define	X_DUMPSIZE	3
#define X_VERSION	4
#define X_PANICSTR	5
#define	X_DUMPMAG	6
#define X_FULLDUMPMAG	7
#define	X_PARTDUMPMAG	8
#define	X_ELBUF		9
#define X_ELBUF_OFF	10
#define X_SBR		11
#define X_CPU		12
#define NUM_IN_TAB	13
#define RAWBUFSZ	2048

struct nlist nlsystem[NUM_IN_TAB + 1];
struct nlist nl[NUM_IN_TAB +1];

char	*system;
char	*dirname;			/* directory to save dumps in */
char	*ddname;			/* name of dump device */
char	*find_dev();
dev_t	dumpdev;			/* dump device */
time_t	dumptime;			/* time the dump was taken */
int	dumplo;				/* where dump starts on dumpdev */
int	dumpsize;			/* amount of memory dumped */
int	dumpmag;			/* magic number in dump */
int	full_dumpmag;			/* full dump magic number */
int	partial_dumpmag;		/* partial dump magic number */
int	kerrsize;			/* Size of error log buffer in pages */
int	partial_dump;			/* Switch for partial or full dump */
int	elbuf_offset;			/* pg offset of elbuf (partial dump) */
int	old_bounds;			/* temp hold for original bounds */
time_t	now;				/* current date */
char	*path();
char 	*malloc();
char	*ctime();
char	vers[80];
char	core_vers[80];
char	panic_mesg[80];
int	panicstr;
off_t	lseek();
off_t	Lseek();
int	cflag;			/* 001 Used for the -c switch */
int	eflag = 0;
char rbuf[RAWBUFSZ];		/* buffer for raw i/o */
main(argc, argv)
	char **argv;
	int argc;
{
	argv++;			/* 001 */
	argc--;			/* 001 */

	/*
	 * Did the user specify the -c switch, if so then setup the
	 * arg pointers for any other parameter.
	 */
	if ( !strcmp("-c",*argv) ) {
		argc--;
		argv++;
		cflag++;
	}
	/*
	 * Did the user specify the -e switch, if so then setup the
	 * arg pointers for any other parameter.
	 */
	if ( !strcmp("-e",*argv) ) {
		argc--;
		argv++;
		eflag++;
	}

	if (!eflag && argc != 1 && argc != 2) {
		fprintf(stderr, "usage: savecore [-c] [-e] dirname [ system ]\n");
		exit(1);
	}
	dirname = argv[0];
	if (argc == 2)
		system = argv[1];
	if (access(dirname, 2) < 0) {
		perror(dirname);
		exit(1);
	}

	/*
	 * Initialize the name list tables.
	 */
	init_nlist();
	read_kmem();
	printf("savecore: checking for dump...");
	if (dump_exists()) {
		/*
		 * if cflag set then clear the dump flag
		 */
		if ( cflag )
			clear_dump(); 

		if (partial_dump)
			printf("partial dump exists\n");
		else
			printf("dump exists\n");
		(void) time(&now);
		check_kmem();
		log_entry();
		if (get_crashtime() && check_space()) {
			if (eflag) {
				printf("saving elbuf\n");
				save_elbuf();
				clear_dump();
			}
			else {
				printf("saving core\n");
				save_core();
				printf("saving elbuf\n");
				save_elbuf();
				clear_dump(); 
			}
		} else {
			printf("crashtime or space problem\n");
			exit(1);
		}
	}
	else
		printf("dump does not exist\n");
	return 0;
}

init_nlist()
{
	nl[X_DUMPDEV].n_un.n_name = nlsystem[X_DUMPDEV].n_un.n_name =
		"_dumpdev";
	nl[X_DUMPLO].n_un.n_name = nlsystem[X_DUMPLO].n_un.n_name =
		"_dumplo";
	nl[X_TIME].n_un.n_name = nlsystem[X_TIME].n_un.n_name =
		"_time";
	nl[X_DUMPSIZE].n_un.n_name = nlsystem[X_DUMPSIZE].n_un.n_name =
		"_dumpsize";
	nl[X_VERSION].n_un.n_name = nlsystem[X_VERSION].n_un.n_name =
		"_version";
	nl[X_PANICSTR].n_un.n_name = nlsystem[X_PANICSTR].n_un.n_name =
		"_panicstr";
	nl[X_DUMPMAG].n_un.n_name = nlsystem[X_DUMPMAG].n_un.n_name =
		"_dumpmag";
	nl[X_FULLDUMPMAG].n_un.n_name = nlsystem[X_FULLDUMPMAG].n_un.n_name =
		"_full_dumpmag";
	nl[X_PARTDUMPMAG].n_un.n_name = nlsystem[X_PARTDUMPMAG].n_un.n_name =
		"_partial_dumpmag";
	nl[X_ELBUF].n_un.n_name = nlsystem[X_ELBUF].n_un.n_name =
		"_elbuf";
	nl[X_ELBUF_OFF].n_un.n_name = nlsystem[X_ELBUF_OFF].n_un.n_name =
		"_elbuf_offset";
	nl[X_SBR].n_un.n_name = nlsystem[X_SBR].n_un.n_name =
		"_Sysmap";
	nl[X_CPU].n_un.n_name = nlsystem[X_CPU].n_un.n_name =
		"_cpudata";
	nl[NUM_IN_TAB].n_un.n_name = nlsystem[NUM_IN_TAB].n_un.n_name =
		"" ;
}

int
dump_exists()
{
	register int dumpfd;
	int word;
	int seektot,seekval,seekrem;
	char *rptr;

	dumpfd = Open(ddname, 0);
	seektot = (off_t) (dumplo + ok(nlsystem[X_DUMPMAG].n_value));
	seekval = (seektot / NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(dumpfd, (off_t) seekval, 0);
	Read(dumpfd, rbuf, round(seekrem + 4));
	close(dumpfd);
	
	rptr = rbuf + seekrem;
	word = *(int *)rptr;
	if (word == full_dumpmag)
 		/* If full dump, set partial_dump to false */
 		partial_dump = 0;
	else {
		if (word == partial_dumpmag)
			/* If partial dump, set partial_dump to true */
			partial_dump = 1;
		else	
			/* invalid dump magic number */
			return(0);
	}
	return(1);	/* dump exists */
}

clear_dump()
{
	register int dumpfd;
	int zero = 0;

	dumpfd = Open(ddname, 1);
	Lseek(dumpfd, (off_t)(dumplo + ok(nlsystem[X_DUMPMAG].n_value)), 0);
	Write(dumpfd, (char *)&zero, sizeof zero);
	close(dumpfd);
}

char *
find_dev(dev, type, raw)
	register dev_t dev;
	register int type;
	int raw;
{
	register DIR *dfd = opendir("/dev");
	struct direct *dir;
	struct stat statb;
	static char devname[MAXPATHLEN + 1];
	char *dp;

	strcpy(devname, "/dev/");
	while ((dir = readdir(dfd))) {
		strcpy(devname + 5, dir->d_name);
		if (stat(devname, &statb)) {
			perror(devname);
			continue;
		}
		if ((statb.st_mode&S_IFMT) != type)
			continue;
		if (dev == statb.st_rdev) {
			closedir(dfd);
			dp = (char *)malloc(strlen(devname)+2);

			if (raw) {
				strcpy(dp, "/dev/r");
				strcat(dp, &devname[5]);
			}
			else {
				strcpy(dp, devname);
			}
			return dp;
		}
	}
	closedir(dfd);
	fprintf(stderr, "savecore: Can't find device %d,%d\n",
		major(dev), minor(dev));
	exit(1);
	/*NOTREACHED*/
}

read_kmem()
{
	int kmem;
	FILE *fp;
	register char *cp;

	nlist("/vmunix", nl);
	if (nl[X_DUMPDEV].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: dumpdev not in namelist\n");
		exit(1);
	}
	if (nl[X_DUMPLO].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: dumplo not in namelist\n");
		exit(1);
	}
	if (nl[X_TIME].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: time not in namelist\n");
		exit(1);
	}
	if (nl[X_DUMPSIZE].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: dumpsize not in namelist\n");
		exit(1);
	}
	if (nl[X_VERSION].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: version not in namelist\n");
		exit(1);
	}
	if (nl[X_PANICSTR].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: panicstr not in namelist\n");
		exit(1);
	}
	if (nl[X_DUMPMAG].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: dumpmag not in namelist\n");
		exit(1);
	}
	if (nl[X_FULLDUMPMAG].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: full_dumpmag not in namelist\n");
		exit(1);
	}
	if (nl[X_PARTDUMPMAG].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: part_dumpmag not in namelist\n");
		exit(1);
	}
	if (nl[X_ELBUF].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: elbuf not in namelist\n");
		exit(1);
	}
	if (nl[X_ELBUF_OFF].n_value == 0) {
		fprintf(stderr, "savecore: /vmunix: elbuf_offset not in namelist\n");
		exit(1);
	}
	kmem = Open("/dev/kmem", 0);
	Lseek(kmem, (long)nl[X_DUMPDEV].n_value, 0);
	Read(kmem, (char *)&dumpdev, sizeof (dumpdev));
	Lseek(kmem, (long)nl[X_DUMPLO].n_value, 0);
	Read(kmem, (char *)&dumplo, sizeof (dumplo));
	Lseek(kmem, (long)nl[X_DUMPMAG].n_value, 0);
	Read(kmem, (char *)&dumpmag, sizeof (dumpmag));
	Lseek(kmem, (long)nl[X_FULLDUMPMAG].n_value, 0);
	Read(kmem, (char *)&full_dumpmag, sizeof (full_dumpmag));
	Lseek(kmem, (long)nl[X_PARTDUMPMAG].n_value, 0);
	Read(kmem, (char *)&partial_dumpmag, sizeof (partial_dumpmag));
	dumplo *= NBPG;
	ddname = find_dev(dumpdev, S_IFBLK, RAW); 

	if (system) 
		nlist(system, nlsystem);
	else
		nlist("/vmunix", nlsystem);
	if ((fp = fdopen(kmem, "r")) == NULL) {
		fprintf(stderr, "savecore: Couldn't fdopen kmem\n");
		exit(1);
	}
	fseek(fp, (long)nl[X_VERSION].n_value, 0);
	fgets(vers, sizeof vers, fp);
	fclose(fp);
}

check_kmem()
{
	int seektot, seekval, seekrem;
	char *rptr;
	int dumpfd;

	dumpfd = Open(ddname, 0);
	seektot = (off_t)(dumplo+ok(nlsystem[X_VERSION].n_value));
	seekval = (seektot / NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(dumpfd, (off_t)seekval, 0);
	Read(dumpfd, rbuf, round(sizeof(core_vers) + seekrem)); 
	rptr = rbuf + seekrem;
	strcpy(core_vers, rptr, sizeof(core_vers));
	close(dumpfd);

	if (!eq(vers, core_vers))
		fprintf(stderr,
		   "savecore: Warning: vmunix version mismatch:\n\t%sand\n\t%s",
		   vers, core_vers);

	dumpfd = Open(ddname, 0);
	seektot = (off_t)(dumplo + ok(nlsystem[X_PANICSTR].n_value));
	seekval = (seektot / NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(dumpfd, (off_t)seekval, 0);
	Read(dumpfd, rbuf, round(sizeof(panicstr) + seekrem));
	rptr = rbuf + seekrem;
	panicstr = *(int *)rptr;

	if (panicstr) {
	    close(dumpfd);
	    dumpfd = Open(ddname, 0);
	    seektot = (off_t)(dumplo + ok(panicstr));
	    seekval = (seektot / NBPG) * NBPG;
	    seekrem = seektot % NBPG;
	    Lseek(dumpfd, (off_t)seekval, 0);
	    Read(dumpfd, rbuf, round(sizeof(panic_mesg) + seekrem));
	    rptr = rbuf + seekrem;
	    strcpy(panic_mesg, rptr, sizeof(panic_mesg));
	}
	close(dumpfd);
}

get_crashtime()
{
	int dumpfd;
	int seektot, seekval, seekrem;
	char *rptr;
	time_t clobber = (time_t)0;

	if (system)
		return (1);
	dumpfd = Open(ddname, 0);
	seektot = (off_t)(dumplo + ok(nlsystem[X_TIME].n_value));
	seekval = (seektot / NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(dumpfd, (off_t) seekval, 0);
	Read(dumpfd, rbuf, round(seekrem + sizeof(dumptime)));
	rptr = rbuf + seekrem;
	dumptime = *(int *)rptr;
	close(dumpfd);
	if (dumptime == 0)
		return (0);
	printf("System went down at %s", ctime(&dumptime));
	if (dumptime < now - LEEWAY || dumptime > now + LEEWAY) {
		printf("Dump time is unreasonable\n");
		return (0);
	}
	return (1);
}

char *
path(file)
	char *file;
{
	register char *cp = (char *)malloc(strlen(file) + strlen(dirname) + 2);

	(void) strcpy(cp, dirname);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}

check_space()
{
	struct stat dsb;
	register char *ddev;
	int dfd, spacefree;
	struct fs fs;
	struct fs *fsptr;
	int seektot, seekval, seekrem;
	char *rptr;

	if (stat(dirname, &dsb) < 0) {
		perror(dirname);
		exit(1);
	}
	ddev = find_dev(dsb.st_dev, S_IFBLK, BLOCK);
	dfd = Open(ddev, 0); 
	seektot = (long)(SBLOCK * DEV_BSIZE);
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(dfd, (long) seekval, 0);
	Read(dfd, rbuf, round(sizeof(fs) + seekrem));
	rptr = rbuf + seekrem;
	fsptr = (struct fs *)rbuf;
	close(dfd);
	spacefree = fsptr->fs_cstotal.cs_nbfree * fsptr->fs_bsize / 1024;
	if (read_number("minfree") > spacefree) {
		fprintf(stderr,
		   "savecore: Dump omitted, not enough space on device\n");
		return (0);
	}
	if (fsptr->fs_cstotal.cs_nbfree * fsptr->fs_frag + fsptr->fs_cstotal.cs_nffree <
	    fsptr->fs_dsize * fsptr->fs_minfree / 100)
		fprintf(stderr,
			"Dump performed, but free space threshold crossed\n");
	return (1);
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(path(fn), "r")) == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

save_core()
{
	register int n;
	char buffer[32*NBPG];
	register char *cp = buffer;
	register int ifd, ofd, bounds;
	register FILE *fp;
	register blks_out;			/* count of blocks written */
	int num_to_print;			/* After this many pages are
						   written, print a message */
	int seektot, seekval, seekrem;
	char *rptr;
	short fstread = 1;

	old_bounds = bounds = read_number("bounds");
	ifd = Open(system?system:"/vmunix", 0);
	sprintf(cp, "vmunix.%d", bounds);
	ofd = Create(path(cp), 0644);
	while((n = Read(ifd, cp, BUFSIZ)) > 0)
		Write(ofd, cp, n);
	close(ifd);
	close(ofd);

	ifd = Open(ddname, 0);
	seektot = (off_t)(dumplo + ok(nlsystem[X_DUMPSIZE].n_value));
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t) seekval, 0);
	Read(ifd, rbuf, round(seekrem + sizeof(dumpsize)));
	rptr = rbuf + seekrem;
	dumpsize = *(int *)rptr;

	/* calculate num_to_print to print a message every 20% */

	num_to_print = (dumpsize / 32) * 0.2;
	blks_out = 0;

	sprintf(cp, "vmcore.%d", bounds);
	ofd = Create(path(cp), 0644);

	seektot = (off_t)dumplo;
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t)seekval, 0);
	printf("Saving %d bytes of image in vmcore.%d\n", NBPG*dumpsize,
		bounds);

	while (dumpsize > 0) {
	    n = Read(ifd, cp, (dumpsize > 32 ? 32 : dumpsize) * NBPG);
    	    if (fstread) {
	 	cp += seekrem;
		Write(ofd, cp, n-seekrem);
		dumpsize -= (n-seekrem)/NBPG;
		fstread = 0;
	    }
	    else {
		if (n == (32 * NBPG)) {
		    Write(ofd, cp, n);
		}
		else {	/* last read */
		    if (seekrem) {
			Write(ofd, cp, (n - NBPG) + seekrem);
		    }
		    else {
			Write(ofd, cp, n);
		    }
		}
		dumpsize -= n/NBPG;
	    }
    	    blks_out++;
	    if ((blks_out % num_to_print == 0) && (dumpsize > 32)) {
		    printf("saving core, %d bytes remaining\n",dumpsize * NBPG);
		    blks_out = 0;
	    }
	}
	close(ifd);
	close(ofd);
	/*
	 * If a partial dump is being saved, call save_uarea which will
	 * extract the user area from the core image and modify the pte's
	 * associated with the area.
	 */
	if (partial_dump)
		save_uarea();
	fp = fopen(path("bounds"), "w");
	fprintf(fp, "%d\n", bounds+1);
	fclose(fp);
}

save_uarea()
{
	int ifd, iofd;		/* file descriptors */
	register int u_addr;	/* location in core where uarea resides */
	int dumpsize;		/* size of dump */

	struct pte_offset {
	int	off_0_8:9,	/* bits 0 - 8  */
		off_9_29:21,	/* bits 9 - 29 */
		off_30_31:2;	/* bits 30, 31 */
	};
	union uoffset {
		int off_int;
		struct pte_offset off_str;
	} uval;
	union uoffset pte_addr;		/* the pte of the pte of the uarea */
	union uoffset pte_addr_hold;	/* temp hold for above */

	struct user u_area;
	struct pte  phys_pte;

	int kerrsize;		/* error logger buffer size */
	int phys_addr_pte;	/* physical address of pte */
	char buffer[32*NBPG];
	register char *cp = buffer;
	register int ind;
	register int addr;	/* page number where u area is in core image */
	int seektot, seekval, seekrem;
	char *rptr;

#define convert(addr, base) (((((long)addr&~0xc00001ff)>>7)+ \
		((long)base&~0x00000003))&~0x00000003)

	ifd = Open(ddname, 0);
	seektot = (off_t)(dumplo + ok(nlsystem[X_DUMPSIZE].n_value));
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t) seekval, 0);
	Read(ifd, rbuf, round(seekrem + sizeof(dumpsize)));
	rptr = rbuf + seekrem;
	dumpsize = *(int *)rptr;

	seektot = (off_t)(dumplo + ok(nlsystem[X_ELBUF_OFF].n_value));
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t) seekval, 0);
	Read(ifd, rbuf, round(seekrem + sizeof(elbuf_offset)));
	rptr = rbuf + seekrem;
	elbuf_offset = *(int *)rptr;
	/*
	 * Calculate size of error logger buffer, which is at end of core
	 * image.
	 */
	kerrsize = EL_DUMPSIZE / NBPG;
	if ((EL_DUMPSIZE % NBPG) != 0)
		kerrsize++;
	if (elbuf_offset)
		kerrsize++;
	/*
	 * Calculate where uarea resides in the core image.
	 */
	u_addr = dumplo + ((dumpsize - (kerrsize + UPAGES)) * NBPG);
	/*
	 * Read it in. u area always starts on a page boundry.
	 */
	seektot = (off_t)u_addr;
	Lseek(ifd, (off_t) seektot, 0);
	Read(ifd, (char *)&u_area, sizeof(u_area));
	close(ifd);
	/*
	 * Set pte_addr_hold = system virtual address of pte of uarea.
	 */
	pte_addr_hold.off_int = convert(0x80000000 - (UPAGES * NBPG), 
			 	 (long)u_area.u_pcb.pcb_p1br);
	/*
	 * Set phys_addr_pte = physical addr of pte of above.
	 */
	phys_addr_pte = convert(pte_addr_hold.off_int, 
				(long)ok(nlsystem[X_SBR].n_value));
	/*
	 * Read from core image, pte of pte where uarea resides physically
	 */
	sprintf(cp, "vmcore.%d", old_bounds);
	iofd = Open(path(cp), 2);
	Lseek(iofd, (off_t)phys_addr_pte, 0);
	Read(iofd, (char *)&phys_pte, sizeof(phys_pte));
	/*
	 * Set pte_addr = address of pte where uarea resides physically
	 */
	pte_addr.off_str.off_30_31 = 0;
	pte_addr.off_str.off_9_29 = phys_pte.pg_pfnum;
	pte_addr.off_str.off_0_8  = pte_addr_hold.off_str.off_0_8;
	/*
	 * Read in one pte of u area
	 */
	Lseek(iofd, (off_t)pte_addr.off_int, 0);
	Read(iofd, (char *)&phys_pte, sizeof(phys_pte));
	/*
	 * Reset pointer to beginning of pte's.
	 */
	Lseek(iofd, (off_t)pte_addr.off_int, 0);
	/*
	 * Modify all u area pte to be valid, and reflect new location
	 * in core file.
	 */
	addr = (u_addr - dumplo) / 512;
	for (ind = 0; ind < UPAGES; ind++) {
		phys_pte.pg_v = 1;
		phys_pte.pg_pfnum = addr++;
		Write(iofd, (char *)&phys_pte, sizeof(phys_pte));
	}
	/*
	 * Make physical address of pcb entry in cpudata, point to
	 * the new location of the pcb.
	 */
	Lseek(iofd, (off_t)ok(nlsystem[X_CPU].n_value), 0);
	phys_pte.pg_pfnum = (u_addr - dumplo) / 512;
	Write(iofd, (char *)&phys_pte, sizeof(phys_pte));
	close(iofd);
}

char *days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};

log_entry()
{
	FILE *fp;
	struct tm *tm, *localtime();

	tm = localtime(&now);
	fp = fopen("/usr/adm/shutdownlog", "a");
	if (fp == 0)
		return;
	fseek(fp, 0L, 2);
	fprintf(fp, "%02d:%02d  %s %s %2d, %4d.  Reboot", tm->tm_hour,
		tm->tm_min, days[tm->tm_wday], months[tm->tm_mon],
		tm->tm_mday, tm->tm_year + 1900);
	if (panicstr)
		fprintf(fp, " after panic: %s\n", panic_mesg);
	else
		putc('\n', fp);
	fclose(fp);
}

/*
 * Versions of std routines that exit on error.
 */

Open(name, rw)
	char *name;
	int rw;
{
	int fd;

	if ((fd = open(name, rw)) < 0) {
		perror(name);
		exit(1);
	}
	return fd;
}

Read(fd, buff, size)
	int fd, size;
	char *buff;
{
	int ret;

	if ((ret = read(fd, buff, size)) < 0) {
		perror("read");
		exit(1);
	}
	return ret;
}

off_t
Lseek(fd, off, flag)
	int fd, flag;
	long off;
{
	long ret;

	if ((ret = lseek(fd, off, flag)) == -1L) {
		perror("lseek");
		exit(1);
	}
	return ret;
}

Create(file, mode)
	char *file;
	int mode;
{
	register int fd;

	if ((fd = creat(file, mode)) < 0) {
		perror(file);
		exit(1);
	}
	return fd;
}

Write(fd, buf, size)
	int fd, size;
	char *buf;
{

	if (write(fd, buf, size) < size) {
		perror("write");
		exit(1);
	}
}

struct elcfg elcfg;

save_elbuf()
{
	int ifd, ofd;
	int kerrsize_old;
	register int n;
	char buffer[32*NBPG];
	register char *cp = buffer;
	unsigned long elbufaddr;
	int new_loc;		      /* New location of elbuf (partial dump) */
	char svelbuf[MAX_PATH + 10];  /* store elbuffer pathname */
	extern int errno;
	int seektot, seekval, seekrem;
	char *rptr;
	short fstread = 1;

	/* Now dump error log buffer into a seperate file */

	kerrsize = EL_DUMPSIZE / NBPG;
	if ((EL_DUMPSIZE % NBPG) != 0)
		kerrsize++;

	kerrsize_old = kerrsize;

	ifd = Open(ddname, 0);

	seektot = (off_t)(dumplo + ok(nlsystem[X_DUMPSIZE].n_value));
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t) seekval, 0);
	Read(ifd, rbuf, round(seekrem + sizeof(dumpsize)));
	rptr = rbuf + seekrem;
	dumpsize = *(int *)rptr;

	seektot = (off_t)(dumplo + ok(nlsystem[X_ELBUF_OFF].n_value));
	seekval = (seektot/NBPG) * NBPG;
	seekrem = seektot % NBPG;
	Lseek(ifd, (off_t) seekval, 0);
	Read(ifd, rbuf, round(seekrem + sizeof(elbuf_offset)));
	rptr = rbuf + seekrem;
	elbuf_offset = *(int *)rptr;

	/*
	 * If elbuf was not on a page boundry, then the number of pages
	 * that was saved at the end of memory was increased by one.
	 */

	if (elbuf_offset)
		kerrsize++;

	if (partial_dump) {
		seektot = dumplo + (((dumpsize-kerrsize) * NBPG) + elbuf_offset);
		seekval = (seektot/NBPG) * NBPG;
		seekrem = seektot % NBPG;
		Lseek(ifd, (off_t) seekval, 0);
	}
	else {
		seektot = (off_t)(dumplo + ok(nlsystem[X_ELBUF].n_value));
		seekval = (seektot/NBPG) * NBPG;
		seekrem = seektot % NBPG;
		Lseek(ifd, (off_t) seekval, 0);
	}

	/*
	 * Dump the kernel error message buffer into the file
	 * elbuffer. The pathname of the file is determined by
	 * calling the subroutine rdcfg, which reads in the
	 * error logger config file.
	 */

	rdcfg();

	sprintf(svelbuf, "%s%s", elcfg.elpath, "/elbuffer");
	ofd = open(svelbuf, O_WRONLY|O_CREAT|O_APPEND, 0644);
	if (ofd < 0) {
		if (errno != ENOENT) {
			fprintf(stderr, "savecore: error opening %s\n",svelbuf);
	    		fprintf(stderr,"savecore: elbuf not saved\n");
			return(-1);
		}
		else {	/* try single user directory */
			sprintf(svelbuf, "%s%s", elcfg.supath, "/elbuffer");
			ofd = open(svelbuf, O_WRONLY|O_CREAT, 0644);
			if (ofd < 0) {
				fprintf(stderr, "savecore: fatal error opening %s\n", svelbuf);
	    			fprintf(stderr,"savecore: elbuf not saved\n");
				return(-1);
			}
		}
	}
			
	printf("Saving %d bytes of image in %s\n", NBPG*kerrsize_old, svelbuf);

	/* First tack on address of elbufaddr */
	elbufaddr = nlsystem[X_ELBUF].n_value; /* don't use ok macro */

	Write(ofd, &elbufaddr, sizeof(elbufaddr));
	while (kerrsize_old > 0) {
            n = Read(ifd, cp, (kerrsize_old > 32 ? 32 : kerrsize_old)*NBPG);
	    if (fstread) {	/* first read */
		cp += seekrem;
		Write(ofd, cp, n-seekrem);
		kerrsize_old -= (n-seekrem)/NBPG;
		fstread = 0;
	    }
	    else {
		if (n == (32 * NBPG)) {
		    Write(ofd, cp, n);
		}
		else {	/* last read */
		    if (seekrem) {
			Write(ofd, cp, (n - NBPG) + seekrem);
		    }
		    else {
			Write(ofd, cp, n);
		    }
		}
		kerrsize_old -= n/NBPG;
	    }
	}
	if (partial_dump)
	  if (!eflag)
	    if ((dumpsize * NBPG) <
		(ok(nlsystem[X_ELBUF].n_value) + (kerrsize * NBPG))) {
		/*
		 * If a partial dump is being performed, and
		 * the -e flag was not in the parameter list.
		 * Also, if elbuf was not saved in its original
		 * location and only resides at the end of the dump,
		 * then modify the elbuf nlist entry.
		 */
		sprintf(cp, "vmunix.%d", old_bounds);
		if (mod_nlist(path(cp), "_elbuf", new_loc - dumplo) != 0)
			fprintf(stderr, "savecore: elbuf namelist unchanged\n");
	    }
	close(ifd);
	close(ofd);
}

/*
 * If this routine is called, then a partial dump must be in progress.
 * If a partial dump is being performed, then the contents of the error
 * logger buffer (elbuf) are saved at the very end of the core image.
 * The name list entry for these in vmunix.bounds must be updated to
 * reflect the change.
 */

mod_nlist(core, nlist_name, new_nvalue)
	char *core;
	char *nlist_name;
	int new_nvalue;
{
	struct nlist *p, *q;
	int n, i, nreq;
	int fd;
	off_t sa;		/* symbol address */
	off_t ss;		/* start of strings */
	struct exec buf;
	struct nlist buffer;	/* to allocate space for q */
	struct nlist buffer1;	/* to allocate space for p */
	char *names;
	int maxlen;
	int ind;		/* loop variable */
	int num_symbols;	/* number of symbols */
	int ret_code = -1;

	fd = open(core, 2);
	if (fd < 0)
		return(-1);
	if (read(fd,(char *)&buf, sizeof (buf)) != sizeof (buf)) {
		close(fd);
		return(-1);
	}
	if (N_BADMAG(buf) || buf.a_syms <= 0) {
		close(fd);
		return(-1);
	}
	sa = N_SYMOFF(buf);
	ss = sa + buf.a_syms;
	/* seek to end of symbols */
	if (ss != lseek(fd, ss, 0)) {
		close(fd);
		return(-1);
	}
	if (read(fd,(char *)&maxlen,sizeof(int)) != sizeof(int)) {
		close(fd);
		return(-1);
	}
	n = maxlen;
	if (n <= 0) {
		/* no strings ?? */
		close(fd);
		return(-1);
	}
	names = malloc(n);			/* 4 bytes + strings */
	if (names <= 0) {
		/* no mem */
		close(fd);
		return(-1);
	}
	n -= sizeof(int);
	/* read strings */
	if (read(fd,names+sizeof(int),n) != n) {
		close(fd);
		return(-1);
	}
	if (sa != lseek(fd,sa,0)) {
		close(fd);
		return(-1);
	}
	num_symbols = buf.a_syms / sizeof(struct nlist);
	q = &buffer;
	p = &buffer1;
	p->n_un.n_name = nlist_name;
	for (ind = 0; ind < num_symbols; ind++) {
		char *nambuf;
		if (read(fd, q, sizeof(struct nlist)) != sizeof(struct nlist)) {
			fprintf(stderr, "savecore: fatal read error\n");
			close(fd);
			return(ret_code);
		}
		if (q->n_un.n_strx == 0 || q->n_type & N_STAB)
			continue;
		nambuf = &names[q->n_un.n_strx];
		i = 0;
		while (p->n_un.n_name[i]) {
			if (p->n_un.n_name[i] != nambuf[i]) goto cont;
			i++;
		}
		if (nambuf[i])		    /* not at end of string */
			continue;
		q->n_value = new_nvalue | 0x80000000;
		if (sa != lseek(fd,sa,0)) {
			close(fd);
			return(ret_code);
		}
		for (i = 0; i < ind; i++)
			lseek(fd, sizeof(struct nlist), 1);
		if (write(fd, q, sizeof(struct nlist)) != sizeof(struct nlist))
			fprintf(stderr, "savecore: write of elbuf failed\n");
		else
			ret_code = 0;
		close(fd);
		return(ret_code);

cont:		continue;
	}
	fprintf(stderr, "savecore: _elbuf not found in symbol table\n");
	close(fd);
	return(ret_code);
}

int rdcfg()
{
	int i, j;
	FILE *cfp;
	char *cp, *cp2;
	char line[256];
	int dataflg = 0;

	cfp = fopen("/etc/elcsd.conf","r");
	if (cfp == NULL) {
	    fprintf(stderr,"savecore: error opening elcsd.conf\n");
	    fprintf(stderr,"savecore: elbuf not saved\n");
	    return(-1);
	}
	i = 0;
	while (fgets(line,sizeof(line),cfp) != NULL) {
	    cp = line;
	    if (*cp == '#' && dataflg == 0)
		continue;
	    if (*cp == '}')
	        break;
	    if (*cp == '{') {
		dataflg++;
		continue;
	    }
	    if (dataflg > 0) {
	        while (*cp == ' ' || *cp == '\t')
		    cp++;
	        if (*cp == '#') {
	            if (i < 1) {
		        fprintf(stderr,"savecore: error reading elcsd.conf\n");
	    		fprintf(stderr,"savecore: elbuf not saved\n");
			(void)fclose(cfp);
	                return(-1);
		    }
	            else {
		        line[0] = '\0';
		        fct(i,line);
		        i++;
		        continue;
		    }
	        }
	        cp2 = cp;
	        while (*cp2 != ' ' && *cp2 != '\t' &&
		       *cp2 != '\n' && *cp2 != '#') 
		    cp2++;
	        *cp2 = '\0';
	        fct(i,cp);
	        i++;
	    }
	}
	(void)fclose(cfp);
}

fct(i,cp)
int i;
char *cp;
{
	int num;

	switch (i) {
	case 0:
	    num = atoi(cp);
	    elcfg.status = num;
	    break;
	case 1:
	    num = atoi(cp);
	    num *= NBPG;
	    elcfg.limfsize = num;
	    break;
	case 2:
	    (void)strcpy(elcfg.elpath,cp);
	    break;
	case 3:
	    (void)strcpy(elcfg.bupath,cp);
	    break;
	case 4:
	    (void)strcpy(elcfg.supath,cp);
	    break;
	case 5:
	    (void)strcpy(elcfg.rhpath,cp);
	    break;
	case 6:
	    (void)strcpy(elcfg.rhostn,cp);
	    break;
	}
}
round(num)
int num;
{
	int val, rem;
	val = num/NBPG;
	rem = num % NBPG;
	if (rem > 0) {
		val++;
	}
	val *= NBPG;
	if (val > RAWBUFSZ) {
		fprintf(stderr, "Internal error: rbuf too small\n");
		exit(-1);
	}
	return(val);
}
