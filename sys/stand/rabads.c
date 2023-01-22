#ifndef lint
static	char	*sccsid = "@(#)rabads.c	1.11	(ULTRIX)	3/23/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1983, 1986 by			*
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
/* Modification History
 * 
 * 12-04-85  Darrell Dunnuck
 *	Fixed a bug that caused garbage to be written into the 
 *	replacement block.  Also, removed any reference to RA82.
 *	RA82 is still supported, but not refered to.
 *
 * 10-10-85  Darrell Dunnuck
 *	Added changes to implement BBR algorithm ECO#:DSDF14-13.
 *
 * 07-26-85  Darrell Dunnuck
 *	Increased the block size accessed by the RQDX1, and allowed
 *	the RQDX3 to be accessed as the other processors.  Also
 *	changed the init command so that whenever a forced error is 
 *	detected, or rewritten after a replacement that you increment
 *	to the next block.
 *
 * 07-11-85  Mark Parenti
 *	Move fix in dipset so BI systems work right.
 *	Fix definition of saveCSR once again.
 *
 * 07-02-85  Darrell Dunnuck
 *	Changed dipset so that the #ifndef MVAX would work right.
 *
 * 06-26-85  Mark Parenti
 *
 *	Change defintion of saveCSR to fix loading problem
 *
 * 06-20-85  Mark Parenti
 *
 *	Added support for BDA.
 *
 * 11-16-84  Darrell Dunnuck
 *
 *	Made changes so that ratest.c would run on VAXes.
/*
 * ULTRIX stand-alone DSA Disk Initialization and Test
 *
 * Darrell Dunnuck
 *	Added routines to write and read disk sectors in order to
 *	verify that the this program is doing the operations
 *	according to spec.
 *
 * Fred Canter
 *	For RX50, reads all blocks and flags any bad ones.
 *	For RD51/RD52 reads all blocks and rewrites any bad
 *	blocks to force controller initiated replacement.
 *	For RA60/RA80/RA81, reads all blocks and uses host initiated
 *	replacement to revector any bad blocks.
 *
 * SECRET STUFF:
 *
 *	This program has the following undocumented commands/features,
 *	used mainly for debugging:
 *
 * 1.	The "u" command is commented out. This was an unsuccessful
 *	attempt to unreplace a block. I soon discovered that could
 *	not be done.
 *
 * 2.	The "t all" feature of the table command causes all entries
 *	in the RCT to be printed, not just the ones that represent
 *	replaced blocks. Also allows RCT copy # selection.
 *
 * 3.	The "f" command (requires a simple password) causes the
 *	specified block to be written with the "force error"
 *	indicator. This is used for debugging.
 *
 * 4.	The "q" command (requires a simple password) causes the 
 *	specified disk block (512 bytes) to be dumped on hex and
 *	character format to the terminal.
 *
 * 5.	The "w" command (requires a simple passwork) allows writing
 * 	a disk block (512 bytes).
 *
 *
 * NOTE:
 *	This program consists, for the most part, of straight-line
 *	unoptimized code! Its only saving grace is that it works, at
 *	least as far as I can tell so far! -- Fred Canter 8/5/84
 *
 */

char	buf[512*32];	/* disk read and replacement process buffer */

#include "../h/param.h"
#include "../h/gnode.h"

#include "../vax/nexus.h"
#include "../vax/cpu.h"
#include "../vax/mtpr.h"

#include "../vaxbi/bireg.h"

#include "sa_defs.h"
#include "saio.h"
#include "ra_saio.h"

#define	RD	0
#define	WRT	1
#define	RW	2
#define	ERR	0
#define	NOERR	1
#define	YES	1
#define	NO	0
#define	HELP	2

/*
 * Bad block replacement strategy flags
 */
#define	BB_COOP	0	/* Use MSCP cooperative replacement */
#define	BB_RWRT	1	/* (rd51/rd52) - rewriting a block replaces it */
#define	BB_NONE	2	/* RX50 - no bad block replacement */

struct dkinfo {
	char	*dki_type;	/* type name of disk */
	char	*dki_name;	/* ULTRIX-11 disk name */
	daddr_t	dki_size;	/* size of entire volume in blocks */
	int	dki_chunk;	/* number of sectors per read */
	int	dki_flag;	/* replacement strategy to use */
				/* if set, just rewrite block to replace it. */
} dkinfo[] = {
	"ra60",	"ra",	400176,		32, BB_COOP,
	"ra80",	"ra",	236964,		31, BB_COOP,
	"ra81",	"ra",	891072,		32, BB_COOP,
	"rx50",	"ra",	800,		10, BB_NONE,
	"rd51",	"ra",	21600,		18, BB_RWRT,
	"rd52",	"ra",	60480,		18, BB_RWRT,
	"rd53", "ra",	138672,		18, BB_RWRT,
	"rc25",	"ra",	50902,		31, BB_COOP,
	0,
};


/*
 * Bad block replacement process definitions.
 */
				/* Compare Modifiers */
#define MD_CMP	040000		/* Compare */
#define MD_ERR	010000		/* Force Error */
#define MD_SEC	01000		/* Suppress Error Correction */
#define MD_SREC	0400		/* Suppress Error Recovery */
#define MD_PRI	01		/* Primary replacement block */

				/* End Message Flags */
#define	EF_BBR	0200		/* Bad block Reported */
#define	EF_BBU	0100		/* Bad Block Unreported */

				/* Status or Event Codes */
#define	ST_MSK	037		/* Status/event code mask */
#define	ST_SUC	0		/* Success (includes subcode "normal") */
#define	ST_DAT	010		/* Data Error */
#define ST_EDC	020		/* EDC error */
#define MD_HCE	0110		/* Header Compare Error */

				/* Revector Control Info flags */
#define	RCI_VP	02		/* Volume write protect flag */
#define	RCI_FE	0200		/* Forced error flag */
#define	RCI_BR	020000		/* Bad RBN flag */
#define	RCI_P2	040000		/* Phase 2 flag */
#define	RCI_P1	0100000		/* Phase 1 flag */

				/* Block read check return values */
#define	BR_NBBR	0		/* no bad block reported */
#define	BR_BBR	1		/* bad block reported */
#define	BR_CECC	2		/* correctable ECC error */
#define	BR_UECC	3		/* uncorrectable ECC error */
#define	BR_NDE	4		/* non data error */
#define	BR_FEM	5		/* block written with "force error" modifier */
#define	BR_DEB	6		/* data error with bad block reported */


#define printd	if(badsdebug)printf
#define printd1	if(badsdebug > 1)printf
#define printd2	if(badsdebug > 2)printf
#define printd3 if(badsdebug > 3)printf
#define printd4 if(badsdebug > 4)printf

int	main_loop = 0;
int	badsdebug = 2;
/*
 * Variables used by the replacement process.
 */

#ifdef	DEBUG
int	rp_bps;			/* interrupt replacement process at step ? */
#endif DEBUG

daddr_t	rp_lbn;			/* LBN of logical block being replaced */
daddr_t	rp_rbn;			/* RBN (Replacement Block Number) */
int	rp_didit;		/* Says block actually replaced */
int	rp_hblk;		/* RCT block of primary RBN descriptor */
int	rp_hoff;		/* RCT block offset of primary RBN descriptor */
int	rp_blk;			/* Current RCT block being searched */
int	rp_off;			/* Current RCT block offset */
int	rp_oblk;		/* RCT block number of OLD RBN */
int	rp_ooff;		/* RCT block offset of OLD RBN */
int	rp_delta;		/* RCT block search offset delta (ping-pong) */

int	rp_irs;			/* Initial read of bad LBN successful */
int	rp_wcs7;		/* Step 7, write-compare successful */
int	rp_wcs8;		/* Step 8, write-compare successful */
int	rp_prev;		/* LBN was previously replaced */
daddr_t	rp_orbn;		/* If previously replaced, OLD RBN */
daddr_t rp_orbd;		/* old RBN descriptor, in case replace fsils */
int	rp_pri;			/* RBN is primary replacement block */
int	rp_sec;			/* RBN is non-primary replacement block */
int	rp_l;			/* Number of LBNs per track */
int	rp_r;			/* Number of RBNs per track */
int	rp_dirty;		/* Phase (P1 | P2) of interrupted replacement */
int	scan_size;		/* size in blocks of the scan */

/*
 * Replacement process buffer pointers,
 * segment the read/write buffer for use
 * by the replacement process.
 */

struct rct_rci *bp_rci = (struct rct_rci *)&buf[0]; /* RCT blk 0 - replacement cntrl info */
char *bp_ir = &buf[512];	  /* RCT blk 1 - image of bad LBN */
union rctb *bp_rct1 = (union rctb *)&buf[1024]; /* RCT blk F - first block being searched */
union rctb *bp_rct2 = (union rctb *)&buf[1536]; /* RCT blk S - second block (if needed) */
char *bp_tp = &buf[2048];	/* test data pattern ( 0165555 0133333 ) */
char *bp_tmp = &buf[2560];	/* scratch buffer */
char *tbp_ir;			/* temporary pointer used in rp_s7 */

/*
 * Structure of RCT (Revector Control Table) block 0.
 * Used during bad block replacement.
 */

struct	rct_rci {
	short	rt_vsn[4];	/* volume serial number */
	short	rt_stat;	/* replacement process status word */
	short	rt_rsvd;	/* reserved ??? */
	daddr_t rt_lbn;		/* LBN being replaced */
	daddr_t	rt_rbn;		/* RBN - replacement block number */
	daddr_t rt_brb;		/* bad RBN - RBN being replaced */
	short	rt_pad[244];	/* bad to 512 byte block */
};

/*
 * Structure used to access an RCT block
 * containing 128 RBN descriptors. The
 * order (swap hi & lo words) must be reversed
 * because of the way the V7 C compiler stores longs.
 *
 * NOTE: The above statement about the V7 C compiler does not hold true
 * for the Ultrix-32 C compiler.
 */

#define	UNALLOCATED	0x0	/* Unallocated RBN */
#define PRIMARY		0x2	/* Primary RBN */
#define NON_PRIMARY	0x3	/* Non-primary RBN */
#define UNUSABLE	0x4	/* Unusable RBN */
#define ALT_UNUSABLE	0x5	/* Alternate unusable RBN */
#define NULL_ENTRY	0x8	/* Null netry - no corresponding RBN sector */

union	rctb {
	daddr_t	rbnd_l;		/* Access descriptor as a long */
	short	rbnd_w[2];	/* Access descriptor as two shorts */
	struct {
		unsigned	rct_lbn:28,
				rct_code:4;
	}rbnd_bits;

};

#define TABLE		0
#define	STATUS		1
#define INIT		2
#define REPLACE		3
#define FRCE		4
#define VERBOSE 	5
#define GETBLOCK	6
#define CMDMOD		7
#define WRITEBLOCK	8

/*
 * External variables found in the MSCP disk driver (udabads.c).
 * Used to modifier driver operation for bad block
 * replacement and get error status info from the driver.
 */

#define	RP_WRT	1
#define	RP_RD	2
#define	RP_REP	3
#define RP_AC	4

extern struct udadevice *udaddr;/* address of udadevice structure */
extern struct uda *ud_ubaddr;	/* Unibus address of uda structure */
extern daddr_t	ra_rbn;		/* RABADS RBN for udabads.c */
extern int	ra_badc;	/* RABADS command flag in udabads.c */
extern int	ra_badm;	/* RABADS command modifier in udabads.c */
extern int	ra_ctid;	/* MSCP controller type ID */
extern char	*ra_dct;	/* Controller type name string */
extern int	ra_stat;	/* MSCP status/event code */
extern int	ra_ecode;	/* MSCP end code */
extern int	ra_eflags;	/* MSCP end message flags */
extern struct	ra_drv	ra_drv[MAXDRIVE];	/* Drive type info */

extern	int	uda_offline;	/* UDA offine flag */
extern	int	ra_offline[];	/* drive offline flag */
extern	u_short	udastd[];	/* array of controller CSRs */
extern	char	can_bda;	/* flag --  1 = can be a BDA */
extern long	saveCSR;	/* boot CSR */
extern	int	bad_lbn;	/* bad LBN from ACCESS end message */
extern	int	command_modifier; /* Control variable for changing the modifier
				   * field in the mscp command message packet.
				   * Used for debuging.
				   */

struct dkinfo *dip;
daddr_t	sbn, us, nblk, nbc, rbn, nb;
daddr_t	bn;
int	repcnt, badcnt, rsize;
int fd, rcnt;
int	ask;		/* flag so table doesn't ask for a return */
int	allflag;
int	rwfem;
int	force;
int	continu();
int	atol();
int	blocktest();	/* writes and reads blocks in replacement step 7 */
int	blockno();
int	unit;
int	nodeid = 0;	/* node ID on BI systems */
int	csraddr;	/* new csr address */
int	cpu_type;	/* cpu type */
char	line[80];
char	fn[30];		/* file spec i.e., hp(0,0) */
char	cmd[20];	/* command name */
char	dt[20];		/* disk type rm03, etc */
char	dn[10];		/* drive number */
char	ctn[2];		/* ascii string for controller number */
char	csr[10];	/* ascii string for controller CSR */
int	ubanum;		/* UBA number */
int	firstblk;	/* flag for linear rct searching */
int	off_bottom;	/* descriptor in RCT greater that 127 */
int	off_top;	/* descriptor in RCT less than zero */
int	first_level;	/* have completed first level search of the RCT block */
int	nondataerr;	/* got a non-data error reading the block in step 7 */
int	bbr_recur;	/* BBR recursion counter */
int	goforit = 0;	/* for turning on and off debug */
char	*dtp;

char	*help[] =
{
	"",
	"To correct typing mistakes, press <DELETE> to erase one character",
	"or <CTRL/U> to erase the entire line.",
	"",
	"To execute a command, type the first letter of the command then",
	"press <RETURN>. The program may prompt for additional information.",
	"",
	"The valid RABADS commands are:",
	"",
	"help    - Print this help message.",
	"",
	"exit    - Exit from the RABADS program.",
	"",
	"drives  - List the disks that can be initialized with RABADS.",
	"",
	"status  - Print the status and geometry of the specified disk.",
	"",
	"table   - Print the RCT (Revector Control Table) for a disk.",
	"",
	"init    - Do a read scan of the disk and replace any bad blocks.",
	"",
	"replace - Force replacement of a disk block.",
	"",
	0
};

char	*fewarn[] =
{
	"\7\7\7",
	"The block was written with the \"forced error\" indicator",
	"because data read from the block to be replaced con-",
	"tained uncorrected ECC errors. Moreover, the data is",
	"\"best guess\" and should be considered corrupted.",
	"This \"best guess\" data is written with the \"forced",
	"error\" indicator set to indicate to you that it is",
	"corrupted.",
	"",
	"When the block is subsequently read, an error is",
	"reported even if the read succeeds. This error probably",
	"indicates that the block has already been replaced.",
	"It may also indicate that a previous attempt to replace",
	"the block failed for some reason. In either case, the",
	"recommended procedure is to rewrite the block then",
	"reread it and only replace the block if the reread",
	"fails.",
	"",
	"                ****** CAUTION ******",
	"",
	"Successfully rewriting the block will indeed clear",
	"the \"forced error\". However, the data in the block",
	"still must be considered corrupted. Therefore, rewrite",
	"the block here only if data integrity is no longer",
	"needed. To maintain data integrity, restore the data",
	"from backup. This restoring will rewrite the data and",
	"clear the \"forced error\" indicator.",
	"",
	0
};

static char	*cmd_mesg[] = {
	"table", "status", "init", "replace", "force", "verbose",
	"getblock", "cmdmod", "writeblock"
};
char	*bdwarn =
  "\n\n\7\7\7DISK SHOULD NOT BE USED: \
  \n    (has unreplaced bad blocks or uncleared \"forced error\" blocks!)\n";
char	*pwstr = "qwerty";
char	*feb_nc1 =
  "was written with \"forced error\" - NOT CLEARED!\n";
char	*feb_nc2 =
  "(Data in this block must considered invalid!)\n";
char	*ndemsg =
  "(NON DATA ERROR: suspect faulty disk hardware or bad media!)\n";
char	*rb_uecc =
  "(Uncorrectable ECC error in replacement block!)\n";
char	*rb_bad =
  "(Hardware says replacement block bad, retry disk initialization!)\n";

int argflag = 0;	/* 0=interactive, 1=called by sdload */
			/* disabled for now...		*/
int	rctcopy;

main()
{
	struct rct_rci *rtp;
	register int i, j, q;
	register char *p;
	int b, s, cnt, k;
	int finished;		/* flag used to exit this program */
	int found, rctend, rctrw;
	int rep_count;

	scan_size = (20 * 1024);
	finished = 0;
	ubanum = 0;
	printf("\n\nULTRIX DSA Disk Initialization Program\n");
	if(argflag) 	/* called by SDLOAD, can only do init command */
		goto do_i;
retry:
	while (!finished) {
		ask = 1;
		force = 0;
		allflag = 0;
		rctcopy = 0;
		printf("\nrabads <help exit drives status table init replace>: ");
		getstring(cmd);
		/*
		 *  Get rid of any  ^S or ^Q s that the terminal sends.
		for (b = 0; ((cmd[b] == 0x13) || (cmd[b] == 0x11)); b++);
		cmd[0] = cmd[b];
		 */

		switch(cmd[0]) {
		case 'b':
			printf("\nPassword: ");
			getstring(line);
			if(strcmp(line, pwstr) != 0)
				printf("\nSorry - reserved for Digital!\n");
			else
				beatblock(); break;
		case 'v':
			printf("\nPassword: ");
			getstring(line);
			if(strcmp(line, pwstr) != 0)
				printf("\nSorry - reserved for Digital!\n");
			else
				verbose(&badsdebug);
			break;
		case 'c':
			printf("\nPassword: ");
			getstring(line);
			if(strcmp(line, pwstr) != 0)
				printf("\nSorry - reserved for Digital!\n");
			else
				if(continu(CMDMOD) == 0)
					chgthreshold(&command_modifier);
			break;
		case 'g':
			printf("\nPassword: ");
			getstring(line);
			if(strcmp(line, pwstr) != 0)
				printf("\nSorry - reserved for Digital!\n");
			else
				if(continu(GETBLOCK) == 0)
					getblk();
			break;
		case 'w':
			printf("\nPassword: ");
			getstring(line);
			if(strcmp(line, pwstr) != 0)
				printf("\nSorry - reserved for Digital!\n");
			else
				if(continu(WRITEBLOCK) == 0)
					writeblk(); break;
		case 'f':
			set_forced_error();
			break;
		case 'h':
			for(i=0; help[i]; i++)
				printf("\n%s", help[i]);
			break;
		case 'e':
			_stop("Exiting rabads");
		case 'd':
			prtdsk();
			break;
		case 's':
			goto do_s;
		case 't':
			if(strcmp("t all", cmd) == 0)
				allflag++;	/* print entire RCT */
			if(strcmp("t go", cmd) == 0){
				ask = 0;	/* don't stop to ask */
				allflag++;	/* print entire RCT */
			}
			goto do_t;
		case 'r':
			goto do_r;
		case 'i':
			goto do_i;
		case 0:
			break;
		default:
			printf("\n(%s) - not a valid command!\n", cmd);
			break;
		}
	}
	_stop("Exiting rabads -- default");
	exit(NORMAL);		/* original Fred code */
do_s:
	if(dipset() < 0)
		goto retry;
	dskopen(NOERR, RD);
	if(continu(STATUS) < 0){
		if(fd >= 0)
			close_fd(fd);
		goto retry;
		}
	if(fd >= 0)
		close_fd(fd);
	printf("\nDSA controller: %s at micro-code revision %d",
		ra_dct, ra_ctid & 0xf);
	printf("\nUnit %d: ", unit);

#define F_to_C(x,q)     ((x) >> (q*5+7) & 0x1f ? ( ( ((x) >>( q*5 + 7)) & 0x1f) + 'A' - 1): ' ')
		/* this mess decodes the Media type identifier */
	if (F_to_C(ra_drv[unit].ra_mediaid,2) == 'R')
		printf("%c%c%d"
			,F_to_C(ra_drv[unit].ra_mediaid,2)
			,F_to_C(ra_drv[unit].ra_mediaid,1)
			,ra_drv[unit].ra_mediaid & 0x7f);
	else
		printf("UNKNOWN or NONEXISTENT");
	printf(" status =");
	if(ra_drv[unit].ra_online)
		printf(" ONLINE");
	else
		printf(" OFFLINE");
	printf("\nLogical blocks per track\t= %d",
		ra_drv[unit].ra_trksz);
	printf("\nReplacement blocks per track\t= %d",
		ra_drv[unit].ra_rbns);
	printf("\nNumber of RCT copies\t\t= %d", ra_drv[unit].ra_ncopy);
	printf("\nSize of each RCT copy in blocks\t= %d",
		ra_drv[unit].ra_rctsz);
	printf("\nHost area size in blocks\t= %d\n",
		ra_drv[unit].ra_dsize);
	goto retry;
do_u_x:
	close_fd(fd);
	goto retry;
do_r:
	if(dipset() < 0)
		goto retry;
	if(dip->dki_flag == BB_NONE) {
		printf("\nCannot replace blocks on an %s disk!\n",
			dip->dki_type);
		goto retry;
	}
	dskopen(NOERR, RW);
	if(continu(REPLACE) < 0) {
		if(fd >= 0)
			close_fd(fd);
		goto retry;
		}
	if(fd < 0) {
		printf("\nCan't open %s!\n", fn);
		goto retry;
	}
	if(rp_dirty)
		goto rp_s1;	/* finish up interrupted replacement */
	if(ra_drv[unit].ra_rctsz == 0) {
		printf("\nDisk does not have a Revector Control Table!");
		goto do_u_x;
	}
do_r_bn:
	printf("\nBlock number: ");
	getstring(line);
	if(line[0] == '\0')
		goto do_r_bn;
	bn = atol(line);
	if((bn < 0) || (bn >= ra_drv[unit].ra_dsize)) {
		printf("\nBlock number %d out of range\n", bn);
		goto do_r_bn;
	}
	if(dip->dki_flag == BB_RWRT) {	/* cntlr will replace block if bad */
		lseek(fd, (long)(bn*512), 0);
		ra_badc = RP_RD;
		read(fd, (char *)buf, 512);
	}
	printf("\nBlock %d read check: ", bn);
	s = brchk(bn);	/* read check block - see if really bad */
	if((s == BR_BBR) || (s == BR_NBBR))
		printf("SUCCEEDED - ");
	else
		printf("FAILED - ");
	switch(s) {
	case BR_BBR:
	case BR_DEB:
		printf("bad block reported\n");
		break;
	case BR_NBBR:
		printf("no bad block reported\n");
		if(dip->dki_flag == BB_RWRT) {
			printf("\n\7\7\7Disk controller replaces bad blocks,");
			printf(" no way to force replacement!\n");
			goto do_u_x;
		}
		printf("\nReally replace this block (y or n) <n> ? ");
		if((yes(NO) & YES) == YES)
			break;
		else
			goto do_u_x;
	case BR_CECC:
	case BR_UECC:
		printf("data error\n");
		break;
	case BR_NDE:
		printf("non data error\n");
		printf("\n** - block cannot be replaced!\n");
		goto do_u_x;
	case BR_FEM:
		printf("block written with \"force error\" indicator\n");
		for(i=0; fewarn[i]; i++)
			printf("\n%s", fewarn[i]);
		printf("\nRewrite the block (y or n) <n> ? ");
		if((yes(NO) & YES) == NO)
			goto do_u_x;
		lseek(fd, (long)(bn*512), 0);
		ra_badc = RP_WRT;
		if(write(fd, (char *)bp_tmp, 512) != 512) {
			printf("\n\7\7\7BLOCK REWRITE FAILED!\n");
			goto do_u_x;
		}
		lseek(fd, (long)(bn*512), 0);
		ra_badc = RP_RD;
		if(dip->dki_flag == BB_RWRT)
			goto do_r_rq;
		if(read(fd, (char *)bp_tmp, 512) == 512)
			printf("\nREREAD SUCCEEDED: block not replaced!\n");
		else
			printf("\nREREAD FAILED: retry the replace command!\n");
		goto do_u_x;
do_r_rq:
		if(read(fd, (char *)bp_tmp, 512) != 512) {
			printf("\n\7\7\7Disk controller failed ");
			printf("to replace bad block!\n");
		} else
			printf("\nDisk controller replaced block!\n");
		goto do_u_x;
	}
	if(dip->dki_flag == BB_COOP) {
		printf("\nBLOCK: %d - replacement ", bn);
		force++;	/* force replacement (see rp_s8:) */
		rp_lbn = bn;
		goto rp_s1;
	}
	goto do_u_x;
do_t:
	if(dipset() < 0)
		goto retry;
	dskopen(NOERR, RD);
	if(continu(TABLE) < 0){
		if(fd >= 0)
			close_fd(fd);
		goto retry;
		}
	if(fd < 0) {
		printf("\nCan't open %s!\n", fn);
		goto retry;
	}
	s = ra_drv[unit].ra_rctsz;
	if(s == 0) {
		printf("\nDisk does not have a Revector Control Table!");
		goto do_t_x;
	}
	if(allflag == 0)	/* if "t all" ask which copy */
		goto do_t_n;
	printf("\nRCT copy < ");
	k = ra_drv[unit].ra_ncopy;
	for(i=0; i<k; i++)
		printf("%d ", i+1);
	printf(">: ");
	getstring(line);
	rctcopy = atoi(line);
	if((rctcopy < 0) || (rctcopy > k))
		rctcopy = 0;
	if(rctmcr(0, bp_rci) < 0)
		goto do_t_err;
	rtp = bp_rci;
	printf("\nRCT block 0: revector control information.\n");
	printf("\nStatus bits: P1=%d ", rtp->rt_stat&RCI_P1 ? 1 : 0);
	printf("P2=%d ", rtp->rt_stat&RCI_P2 ? 1 : 0);
	printf("BR=%d ", rtp->rt_stat&RCI_BR ? 1 : 0);
	printf("FE=%d ", rtp->rt_stat&RCI_FE ? 1 : 0);
	printf("VP=%d ", rtp->rt_stat&RCI_VP ? 1 : 0);
	printf("\nBad LBN\t= %d", rtp->rt_lbn);
	printf("\nRBN\t= %d", rtp->rt_rbn);
	printf("\nOld RBN\t= %d\n", rtp->rt_brb);
	printf("\nHex dump of RCT block 1 <y or n> ? ");
	getstring(line);
	if(line[0] != 'y')
		goto do_t_n;
	if(rctmcr(1, bp_ir) < 0)
		goto do_t_err;
	dumpblk(bp_ir);
	if (ask) {
		if(prfm())
			goto do_t_x;
	}
do_t_n:
	cnt = -1;
	for(i=2; i<s; i++) {
		if(rctmcr(i, bp_rct1) < 0) {
do_t_err:
			printf("\nCANNOT READ REVECTOR CONTROL TABLE: ");
			printf("RCT multicopy read failed!\n");
			goto do_t_x;
		}
		for(j=0; j<128; j++) {
			if((cnt == 16) || (cnt == -1)) {
				if(cnt == 16)
					if (ask) 
						if(prfm())
							goto do_t_x;
				cnt = 0;
				printf("\nBlock\tOffset\tLBN\tCODE");
				printf("\n-----\t------\t---\t----");
			}
			k = (bp_rct1[j].rbnd_bits.rct_code);
			switch(k) {
			case 02:
			case 03:
			case 04:
			case 05:
				break;
			case 07:
				if(((ra_ctid >> 4) & 0x1f) == RQDX1) {
					if(allflag)
						break;
					else
						goto do_t_x;
				} else
					continue;
			case 010:
				goto do_t_x;
			default:
				if(allflag)
					break;
				else
					continue;
			}
			cnt++;
			printf("\n%d\t%d\t%d\t", i, j,
				bp_rct1[j].rbnd_bits.rct_lbn);
			switch(k) {
			case UNALLOCATED:
				printf("Unallocated replacement block");
				break;
			case PRIMARY:
				printf("Allocated - primary RBN");
				break;
			case NON_PRIMARY:
				printf("Allocated - non-primary RBN");
				break;
			case UNUSABLE:
			case ALT_UNUSABLE:
				printf("Unusable replacement block");
				break;
			case NULL_ENTRY:
				printf("Null entry");
				break;
			default:
				printf("Unknown code (%o)", k);
				break;
			}
		}
	}
do_t_x:
	close_fd(fd);
	goto retry;
do_i:
	if(dipset() < 0)
		goto retry;
	dskopen(ERR, RW);
	if(continu(INIT) < 0)
		goto retry;
	if(rp_dirty) {
		printf("\nInitialization aborted!\n");
		goto do_u_x;
	}
	us = ra_drv[unit].ra_dsize;
	printf("\nStarting block number < 0 >: ");
	getstring(line);
	if(line[0] == '\0')
		sbn = 0L;
	else {
		sbn = atol(line);
		if((sbn < 0) || (sbn >= us)) {
			printf("\nBad starting block!\n");
	do_i_ex:
			if(argflag)
				exit(FATAL);
			else
				goto retry;
		}
	}
	printf("\nNumber of blocks to check < %d >: ", us - sbn);
	getstring(line);
	if(line[0] == '\0')
		nblk = us - sbn;
	else
		nblk = atol(line);
	if(nblk <= 0) {
		printf("\nBad # of blocks!\n");
		goto do_i_ex;
	}
	if(nblk > (us - sbn)) {
		nblk = (us - sbn);
		printf("\nToo many blocks, truncating to %d blocks!\n", nblk);
	}
do_i_rw:
	if(dip->dki_flag != BB_NONE) {
		printf("\nRewrite blocks written with \"forced error\" ");
		printf("(? for help) (y or n) <n> ? ");
		rwfem = yes(NO);
		if(rwfem == HELP) {
			for(i=0; fewarn[i]; i++)
				printf("\n%s", fewarn[i]);
			goto do_i_rw;
		}
	}
	printf("\nscanning...\n");
	badcnt = 0;
	repcnt = 0;
	bn = sbn;
	nbc = 0;
	main_loop = 0;
	rbn = 0;
do_i_lp:
	if(main_loop > 9 ) {
		printf("init has looped on the same access with out progress\n");
		printf("for 10 consecutive times.  Do you want to abort? (y or n) <y> ? ");
		if(yes(YES) == YES){
			goto bg_loop;
		} else main_loop = 0;
	}
	if(((ra_ctid>>4) & 0x1f) == RQDX1)
		nb = 64;
	else
		nb = scan_size;			/* needs investigation!! */
	printd4("bn before = %d, nb before = %d\n", bn, nb);
	if((bn + nb) > us)
		nb = (us - bn);
	if((nb + nbc) > nblk)
		nb = nblk - nbc;
	printd4("bn = %d, nb = %d, us = %d, nbc = %d, nblk = %d\n",
		bn, nb, us, nbc, nblk);
	main_loop++;
	rsize = nb * 512;
	lseek(fd, (long)(bn*512), 0);
	ra_badc = RP_AC;
	rcnt = read(fd, (char *)buf, rsize);
	/* the 8 is for forced error */
#define OK (((ra_stat&ST_MSK) == ST_SUC) && ((ra_eflags & EF_BBR) == 0))
	if(rcnt != rsize)
		printd3("blk %d\n",rcnt/512);
	if(((rcnt==rsize)&&OK)||((rcnt/512)&&(rcnt!= -1)&&(rcnt < rsize)&&OK)) {
do_i_lp1:
		main_loop = 0;
		if(rcnt != rsize) {
			bn += rcnt/512;
			nbc += rcnt/512;
		} else {
			bn += nb;
			nbc += nb;
		}
		if(nbc >= nblk) {
bg_loop:
			close_fd(fd);
			printf("\n\n%d blocks checked", nbc);
			printf("\n%d bad blocks found", badcnt);
			printf("\n%d bad blocks replaced\n", repcnt);
			if(repcnt != badcnt)
				printf("%s", bdwarn);
			else
				printf("\n");
			goto retry;
		}
		goto do_i_lp;
	}
	rep_count = 0;
	badcnt++;
	if((ra_stat == 8)&&(!rwfem)&&(rcnt/512==0)&&((ra_eflags&EF_BBR)==0)) {
		notify_fem(bn + rcnt/512);
		rcnt = 512;
		goto do_i_lp1;
	}
do_i_scan:
	if((ra_eflags & EF_BBR) == 0) {
		if((rcnt != -1) && (rcnt < rsize)) {
			rbn = bn + rcnt/512;
		} 
		else
			rbn = bn;
		lseek(fd,(long) rbn * 512, 0);
		ra_badc = RP_AC;
		bad_lbn = 0;
		if((read(fd,buf,512) == 512) && OK) {
				rcnt = 512 * ((rbn + 1) - bn);
			goto do_i_lp1;
		}
	} else 
		rbn=(bad_lbn >= bn) ? bad_lbn :((rcnt/512)+bn);
	bad_lbn = 0;
	rcnt = 0;
	if(rep_count++ > 10) {
		printf("LBN %d has been in a loop for 12 consecutive ", rbn);
		printf("times,\n do you wish to abort (y or n) <y> ? ");
		if (yes(YES)==YES)
			goto bg_loop;
		rep_count = 0;
	}
	printd("\nBLOCK: %d - ", rbn);
	switch(dip->dki_flag) {	/* replacement strategy to use */
	case BB_COOP:
		if((ra_eflags & EF_BBR) || ((command_modifier & 0x200) && (ra_stat == 0xe8))){
			printd("replacement ");
			rp_lbn = rbn;
			goto rp_s1;	/* go replace block */
		} else if(ra_stat == ST_DAT) {	/* FEM */
		/*
		 * need to check that it is really a forced
		 * error indicator, and add edc.  really need a 
		 * "rewrite the block" flag
		 */
			/* data error with force error indicator */
			if(!rwfem) { /* NOT REWRITING FORCED ERRORS */
				printd("%s", feb_nc1); /* warn bad data */
				printd("%s", feb_nc2);
				/* cause increment to next block */
				rcnt = 512 * ((rbn + 1) - bn);
				goto do_i_lp1;
			} else {
				if(fembrw(0, rbn) == YES)
					badcnt--;
			}
			break;
		} else {
			printd("(no bad block reported flag) ");
			printd("cannot be replaced!\n");
			deprnt();
			if(ra_stat & ST_EDC)
				printd("EDC error encountered in LBN %d\n",
					rp_lbn);
			else {
				printd("Init does not know how to deal with ");
				printd("a status code of 0x%x.\n", ra_stat);
			}
			printd("Do you wish to continue the init command (y or n) <y>:");
			if(yes(YES)) {
				rcnt = 512 * ((rbn + 1) - bn);
				goto do_i_lp1;
			}
			else
				goto retry;
			break;
		}
do_i_rg:		/* return here if replacement succeeded */
		repcnt++;
		main_loop = 0;
		printd("SUCCEEDED!\n");
		s = brchk(rbn);	/* read check replacement block */
		if(s == BR_NBBR) {
			rcnt = 512 * ((rbn + 1) - bn);
			goto do_i_lp1;	/* every thing is cool! */
		}
		if(s == BR_FEM) { /* replacement block - forced error */
			if(!rwfem) {
				printd("BLOCK: %d - ", rbn);
				printd("%s", feb_nc1);
				printd("%s", feb_nc2);
				repcnt--;
				/* cause increment to next block */
				rcnt = 512 * ((rbn + 1) - bn);
				goto do_i_lp1;
			}
			else
				if(fembrw(1, rbn) == NO)
					repcnt--;
			break;
		}
		repcnt--;
		printd("BLOCK: %d - ", rbn);
		printd("replacement block read check FAILED!\n");
		switch(s) {
		case BR_UECC:
			printd("%s", rb_uecc);
			deprnt();
			break;
		case BR_BBR:
			goto do_i_scan;
		case BR_NDE:
		default:
			printd("%s", ndemsg);
			deprnt();
			break;
		}
		break;
do_i_rb:		/* return here if replacement failed */
		printf("\nBLOCK: %d - replacement FAILED!\n", rbn);
		main_loop = 0;
		goto do_u_x;
do_i_ra:		/* return here if replacement aborted */
			/* i.e., block was not really bad */
		printd("ABORTED (block not really bad)!\n");
		main_loop = 0;
		lseek(fd, (long)(rbn*512), 0);
		ra_badc = RP_RD;
		read(fd, (char *)buf, 512);
		if(ra_stat != ST_DAT) {
			badcnt--;
			break;	/* NOT FORCED ERROR */
		}
		if(!rwfem) {
			printd("BLOCK: %d - ", rbn);
			printd("%s", feb_nc1);
			printd("%s", feb_nc2);
			/* cause increment to next block */
			rcnt = 512 * ((rbn + 1) - bn);
			goto do_i_lp1;
		}
		else
			if(fembrw(1, rbn) == YES)
				badcnt--;
		break;
	case BB_NONE:
		printd("%s: bad blocks cannot be replaced!\n",
			dip->dki_type);
		deprnt();
		break;
	case BB_RWRT:
		if(ra_eflags & EF_BBR) {
			printd("replacement ");
			switch(brchk(rbn)) {	/* read check */
			case BR_NBBR:
				repcnt++;
				printd("SUCCEEDED!\n");
				break;
			case BR_FEM:
				printd("SUCCEEDED!\n");
				repcnt++;
				if(!rwfem) {
					printd("BLOCK: %d - ", rbn);
					printd("%s", feb_nc1);
					printd("%s", feb_nc2);
					repcnt--;
					/* cause increment to next block */
					rcnt = 512 * ((rbn + 1) - bn);
					goto do_i_lp1;
					break;
				}
				if(fembrw(1, rbn) == NO)
					repcnt--;
				break;
			case BR_UECC:
				printd("FAILED!\n");
				printd("%s", rb_uecc);
				deprnt();
				break;
			case BR_BBR:
				goto do_i_scan;
			case BR_NDE:
			default:
				printd("FAILED!\n");
				printd("%s", ndemsg);
				deprnt();
				break;
			}
		}
		else
			if(ra_stat == ST_DAT) { /* FEM */
				if(!rwfem) { /* NOT REWRITING FORCED ERRORS */
					/* warn bad data */
					printd("%s", feb_nc1);
					printd("%s", feb_nc2);
					/* cause increment to next block */
					rcnt = 512 * ((rbn + 1) - bn);
					goto do_i_lp1;
				}
				else
					if(fembrw(0, rbn) == YES)
						badcnt--;
			}
			else {
				printd("cannot be replaced!\n");
				printd("%s", ndemsg);
				deprnt();
			}
		break;
	}
	goto do_i_lp;

/*
 * MSCP: host initiated bad block replacement algorithm
 *
 * I allow myself one editorial comment before proceeding:
 *
 *	Fred Canter "Good GOD Gertrude, you must be kidding!"
 *
 * Each step of the bad block replacement procedure is labeled
 * so as to correspond to the steps in section 7.5 of the "DSA
 * DISK FORMAT SPECIFICATION" Version 1.4.0.
 */

rp_s1:
#ifdef	DEBUG
	if(rp_bps == 1)
		goto do_u_x;
#endif	DEBUG
/*
 * Step 1 is performed elsewhere in the program. Either the replacement
 * process is initilated by an error with the BAD BLOCK REPORTED flag set,
 * or the process is restarted because it was interrupted by a failure of
 * some type. The latter case is detected when the unit is prought online.
 */
	rp_didit = 0;	/* clear block actually replaced indicator */
			/* in case of interrupted replacement */
	rp_prev = rp_pri = rp_sec = 0;

rp_s2:
#ifdef	DEBUG
	if(rp_bps == 2)
		goto do_u_x;
#endif	DEBUG
/*
 * The DSA spec calls for a soft lock of the unit, but because replacement
 * is only done by this stand-alone program a lock is not necessary.
 */

bbr_recur = 0;		/* initialize the BBR recursion counter to 0 */

rp_s3:
#ifdef	DEBUG
	if(rp_bps == 3)
		goto do_u_x;
#endif	DEBUG
/*
 * The first two steps were easy, but I don't expect this trend to continue.
 */

	if(rp_dirty == 0)
		goto rp_s4;	/* NOT an interrupted replacement! */
/*
 * Interrupted replacement,
 * Re-establish software context for Phase 1
 * and Phase 2, if necessary.
 * Yes, I know that RCT block 0 was read in
 * during the dskopen()!
 */
	printf("\n\7\7\7COMPLETING INTERRUPTED BAD BLOCK REPLACEMENT");
	printf(" (Phase %d)...\n", rp_dirty);
	if(rctmcr(0, bp_rci) < 0) {	/* revector control info */
		rp_rcte(3, RD, 0);
		goto rp_s18;
	}
	if(rctmcr(1, bp_ir) < 0) {	/* bad LBN image */
		rp_rcte(3, RD, 1);
		goto rp_s18;
	}
	rtp = (struct rct_rci *)bp_rci;
	if(rtp->rt_stat & RCI_FE)	/* initial read sucessful or not */
		rp_irs = NO;
	else
		rp_irs = YES;
/*
 * The DSA spec says that the LBN is only valid
 * if the P1 bit is set, but I don't buy it!
 * I never clear the LBN until after phase 2 is
 * complete or a fatal error.
 */
	rp_lbn = rtp->rt_lbn;	/* bad block's LBN */
	printf("\nBLOCK: %d - replacement ", rp_lbn);
	if(rp_dirty == 1)
		goto rp_s7;	/* Interrupted during phase one */
	rp_rbn = rtp->rt_rbn;
	if(rtp->rt_stat & RCI_BR) {	/* previously replaced */
		rp_prev = 1;
		rp_orbn = rtp->rt_brb;
	}
	rp_l = ra_drv[unit].ra_trksz;	/* LBNs per track */
	rp_r = ra_drv[unit].ra_rbns;	/* RBNs per track */
						/* Primary RBN descriptor */
	rp_hblk = (((rp_lbn / rp_l) * rp_r) / 128) + 2;
	rp_hoff = ((rp_lbn / rp_l) * rp_r) % 128;
	rp_blk = (rp_rbn / 128) + 2;		/* Actual RBN descriptor */
	rp_off = rp_rbn % 128;
	if((rp_blk == rp_hblk) && (rp_off == rp_hoff))	/* Primary RBN ? */
		rp_pri = 1;
	else
		rp_sec = 1;	/* YES, I know rp_sec not really used */
	if(rp_prev) {		/* set saved old RBN desc, as best we can */
		rp_orbd = rp_lbn;	/* LBN being replaced */
		rp_oblk = (rp_orbn / 128) + 2;
		rp_ooff = rp_orbn % 128;
		if((rp_oblk == rp_hblk) && (rp_ooff == rp_hoff))
			rp_orbd |= 04000000000;	/* primary RBN */
		else
			rp_orbd |= 06000000000;	/* non-primary RBN */
	}
	if(rctmcr(rp_blk, bp_rct1) < 0) {	/* First RCT block */
		rp_rcte(3, RD, rp_blk);
		goto rp_s18;
	}
	goto rp_s11;		/* finish up phase 2 */

rp_s4:
#ifdef	DEBUG
	if(rp_bps == 4)
		goto do_u_x;
#endif	DEBUG
printd1("4 ");
/*
 * Read the suspected bad block, remember whether or not the read succeeded.
 * Also, remember if the forced error indicator was set.
 */
	rp_irs = NO;
	p = (char *)bp_ir;
	for(i=0; i<512; i++)
		*p++ = 0;
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_RD;
	for (i=0; i < 4; i++)
		if(read(fd, bp_ir, 512) == 512) {
			rp_irs = YES;
			i = 4;	/* end the for loop */
		}
		else {
			if(ra_stat & ST_DAT)
				i = 4; /* end the for loop */
			rp_irs = NO;
		}


rp_s5:
#ifdef	DEBUG
	if(rp_bps == 5)
		goto do_u_x;
#endif	DEBUG
printd1("5 ");
/*
 * Save data read in step 4 in RCT block 1.
 */
	if(rctmcw(1, bp_ir) < 0) {
		rp_rcte(4, WRT, 1);
		goto rp_s18;
	}

rp_s6:
#ifdef	DEBUG
	if(rp_bps == 6)
		goto do_u_x;
#endif	DEBUG
printd1("6 ");
/*
 * Begin phase 1, save info in RCT block 0.
 */
	if(rctmcr(0, bp_rci) < 0) {
		rp_rcte(6, RD, 0);
		goto rp_s18;
	}
	rtp = (struct rct_rci *)bp_rci;
	rtp->rt_lbn = rp_lbn;
	rtp->rt_stat &= ~RCI_FE;
	if(rp_irs == NO)
		rtp->rt_stat |= RCI_FE;
	rtp->rt_stat |= RCI_P1;
	if(rctmcw(0, bp_rci) < 0) {
		rp_rcte(6, WRT, 0);
		goto rp_s17;
	}

rp_s7:
#ifdef	DEBUG
	if(rp_bps == 7)
		goto do_u_x;
#endif	DEBUG
printd1("7 ");
/*
 * Check to see if the block is really bad.
 *
 * Read the block, then write customer data, then write inverted
 * customer data.
 */

	nondataerr = NO;
	rp_wcs7 = YES;
	if(rctmcr(1, bp_tmp) < 0) {	/* Get best guess data from RCT blk 1 */
		rp_rcte(7, RD, 1);
		goto rp_s18;
	}
	for (i = 0; i < 4; i++) {
		/* check for any data error */
		lseek(fd, (long)(rp_lbn*512), 0);
		ra_badc = RP_AC;
		ra_badm = MD_SEC|MD_SREC;
		if ((read(fd, buf, 512) != 512)
			&& ((ra_eflags & EF_BBR) || (ra_eflags & EF_BBU))
			|| (ra_stat & 0xe8)) {
			rp_wcs7 = NO;
			i = 4;
		}
	}
	if (rp_wcs7 == NO && ((ra_stat & 0xe8) != 0xe8)) {
		printd4(" nondata read failure status = 0x%x - aborting\n",
			ra_stat);
		rp_wcs7 = YES;
		nondataerr = YES;
	}
	if (rp_wcs7 == YES && ~nondataerr) {
		bp_tp = &buf[2048];
		bp_tmp = &buf[512];
		tbp_ir = bp_ir;
		for (j= 0; j < 512; j++)
			*bp_tp++ = ~*tbp_ir++;
		for (i = 0; i < 8; i++) {
			if ((rp_wcs7 = blocktest(fd, !rp_irs, bp_ir)) == YES)
				rp_wcs7 = blocktest(fd, 0, bp_tp);
			if (rp_wcs7 == NO)
				i = 8;
		}
	}

rp_s8:
#ifdef	DEBUG
	if(rp_bps == 8)
		goto do_u_x;
#endif	DEBUG
printd1("8 ");
/*
 * Bad block may be good, write saved bad block contents back
 * out to bad block. May be a transient bad block error????
 *
 * NOTE: the "force" variable causes replacement regardless!
 */
	rp_wcs8 = YES;
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_WRT;
	ra_badm = MD_CMP;
	if(rp_irs == NO)
		ra_badm |= MD_ERR;
	if(write(fd, bp_ir, 512) == 512) {
		if(((ra_eflags & EF_BBR) == 0) &&
		    (rp_irs == YES)) {
			if(force || (rp_wcs7 == NO))
				goto rp_s9;
			else
				goto rp_s13;
		}
	}
	if((ra_stat & 0xe8) && 	/* DATA ERROR with FORCED ERROR */
	   (rp_irs == NO) &&
	   ((ra_eflags & EF_BBR) == 0))
			if(force)
				goto rp_s9;
			else
				goto rp_s13;

rp_s9:
#ifdef	DEBUG
	if(rp_bps == 9)
		goto do_u_x;
#endif	DEBUG
printd1("9 ");
/*
 * Seach the RCT for the replacement block number (RBN) and
 * whether or not the bad LBN was previously replaced, if so
 * also get the old RBN.
 */
/* Set initial (hash) RCT block # and offset, init variables */

	if (bbr_recur >= 2) {
		printd("Replacement Command Failure at LBN %d\n", rp_lbn);
		goto rp_s16;
	}
	else
		bbr_recur++;
	firstblk = 0;
	rp_prev = rp_pri = rp_sec = 0;
	rp_l = ra_drv[unit].ra_trksz;	/* LBNs per track */
	rp_r = ra_drv[unit].ra_rbns;	/* RBNs per track */
	rp_hblk = (((rp_lbn / rp_l) * rp_r) / 128) + 2;
	rp_hoff = ((rp_lbn / rp_l) * rp_r) % 128;
	rp_blk = rp_hblk;
	rp_off = rp_hoff;
/* Check for primary RBN descriptor empty or matches this LBN */

	if(rctmcr(rp_blk, bp_rct1) < 0) {
		rp_rcte(9, RD, rp_blk);
		goto rp_s16;
	}
	if(rbempty()) {
		rp_pri++;
		goto rp_s9_f;	/* found RBN */
	}
	if(rbmatch()) {
		rp_prev++;
		rp_orbn = ((rp_blk - 2) * 128) + rp_off;
		rp_orbd = bp_rct1[rp_off].rbnd_l;
	}
/* Start ping-pong search */

	first_level = YES;
	off_bottom = off_top = NO;
	rp_delta = 1;
	do {
		rp_off = rp_hoff + rp_delta;
		if((rp_off >= 0) && (rp_off <= 127) && !rbnull()) {
			if(rbempty()) {
				rp_sec++;
				goto rp_s9_f;		/* found RBN */
			}
			if(rbmatch()) {
				rp_prev++;
				rp_orbn = ((rp_blk - 2) * 128) + rp_off;
				rp_orbd = bp_rct1[rp_off].rbnd_l;
			}
		}
		else {
			if(rp_off > 127)
				off_bottom = YES;
			if(rp_off < 0)
				off_top = YES;
			if((off_top = YES) && (off_top == off_bottom))
				first_level == NO;
		}
		rp_delta = -rp_delta;
		if(rp_delta >= 0)
			rp_delta++;
	}
	while ((first_level) && (!rbnull()));
	if(!rbnull())
		rp_blk++;
	else
		rp_blk = 2;
	firstblk++;
	rp_off = rp_hoff = 0;

/* Start of linear search */

rp_s9_l:
	if(rblast()) {	/* If primary in last RCT block, go to first block */
		rp_blk = 2;
		rp_off = 0;
	}
rp_s9_l1:
	if(rctmcr(rp_blk, bp_rct1) < 0) {
		rp_rcte(9, RD, rp_blk);
		goto rp_s16;
	}
	rp_off = 0;
rp_s9_l2:
	if((rp_blk == rp_hblk) && (!firstblk)) {  /* search failed */
		rp_rcte(9, -1, 0);	/* RP step 9 FAILED! - message */
		printf("\nRCT search failed: no replacement block available!\n");
		goto rp_s16;
	}
	if(rbempty()) {
		rp_sec++;
		goto rp_s9_f;		/* found RBN */
	}
	if(rbmatch()) {
		rp_prev++;
		rp_orbn = ((rp_blk - 2) * 128) + rp_off;
		rp_orbd = bp_rct1[rp_off].rbnd_l;
	}
	if(rbnull()) {
		rp_blk = 2;
		goto rp_s9_l1;
	}
	rp_off++;
	if(rp_off > 127) {
		rp_blk++;
		firstblk = 0;
		goto rp_s9_l1;
	}
	goto rp_s9_l2;
rp_s9_f:
	rp_rbn = ((rp_blk - 2) * 128) + rp_off;

rp_s10:
#ifdef	DEBUG
	if(rp_bps == 10)
		goto do_u_x;
#endif	DEBUG
/*
 * Update RCT block 0 with RBN, BR flag, and say in phase 2.
 */
printd1("10 ");
	if(goforit)
		badsdebug = 7;
	rtp->rt_rbn = rp_rbn;
	if(rp_prev) {
		rtp->rt_brb = rp_orbn;
		rtp->rt_stat |= RCI_BR;
	}
	rtp->rt_stat &= ~RCI_P1;
	rtp->rt_stat |= RCI_P2;
	printd4("rbn = 0x%x, brb = 0x%x, stat = 0x%x\n",
		rtp->rt_rbn, rtp->rt_brb, rtp->rt_stat);
	if(rctmcw(0, bp_rci) < 0) {
		rp_rcte(10, WRT, 0);
		goto rp_s16;
	}

rp_s11:
#ifdef	DEBUG
	if(rp_bps == 11)
		goto do_u_x;
#endif	DEBUG
/*
 * Update RCT to say block has been replaced.
 */
printd1("11 ");
	bp_rct1[rp_off].rbnd_bits.rct_lbn = rp_lbn;
	if(rp_pri)
		bp_rct1[rp_off].rbnd_bits.rct_code = PRIMARY;
	else
		bp_rct1[rp_off].rbnd_bits.rct_code = NON_PRIMARY;
	rp_oblk = (rp_orbn / 128) + 2;
	rp_ooff = rp_orbn % 128;
	if(rp_prev) {
		printd3("previous replace");
		if(rp_blk != rp_oblk) {	/* rbn & old rbn in diff blks */
			if(rctmcr(rp_oblk, bp_rct2) < 0) {
				rp_rcte(11, RD, rp_oblk);
				goto rp_s16;
			}
			printd3(" in different block");
			bp_rct2[rp_ooff].rbnd_bits.rct_code = UNUSABLE;
		} else
			bp_rct1[rp_ooff].rbnd_bits.rct_code = UNUSABLE;
		printd3("\n");
	}
	printd4("rct_code = %d\n", bp_rct1[rp_off].rbnd_bits.rct_code);
	if(rctmcw(rp_blk, bp_rct1) < 0) {
		rp_rcte(11, WRT, rp_blk);
		goto rp_s15;
	}
	if(rp_prev && (rp_oblk != rp_blk)) {
		printd3("writing out second RCT block\n");
		if(rctmcw(rp_blk, bp_rct2) < 0) {
			rp_rcte(11, WRT, rp_blk);
			goto rp_s15;
		}
	}

rp_s12:
#ifdef	DEBUG
	if(rp_bps == 12)
		goto do_u_x;
#endif	DEBUG
/*
 * Use the REPLACE command to cause the controller
 * to replace the bad block.
 * ECO:  If the replace command fails, goto step 15 instead of 17.
 */
printd1("12 ");
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_REP;
	if(rp_pri)
		ra_badm = MD_PRI;
	ra_rbn = rp_rbn;	/* tell driver replacement block # */
	write(fd, bp_ir, 512);	/* really a REPLACE command */
	if((ra_stat & ST_MSK) != ST_SUC) {
		rp_rcte(12, -1, 0);	/* RP step 12 FAILED! - message */
		printd2("ra_stat = 0x%x, ra_ecode = 0x%x, ra_eflags = 0x%x\n",
			ra_stat, ra_ecode, ra_eflags);
		printf("\nMSCP replace command failed!\n");
		goto rp_s17;
	}
	/*
	 * First, read the block and check for the Forced Error flag. 
	 * All unused RBNs are written with the forced error flag, so
	 * it should be set if the replace command worked.
	 */
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_RD;
	read(fd, buf, 512);
	if ((ra_stat & ST_DAT) != ST_DAT) {
		rp_rcte(12, -1, 0);	/* RP step 12 FAILED! - message */
		printd("\nMSCP replace command failed!\n");
		printd2("RBN used for LBN %d did not contain Forced Error indicator.\n", rp_lbn);
		printd2("ra_stat = 0x%x, ra_ecode = 0x%x, ra_eflags = 0x%x\n",
			ra_stat, ra_ecode, ra_eflags);
		goto rp_s17;
	}
	/* 
	 * Read the data to be written to the replacement block
	 * from RCT block 1 -- just to be safe.
	 */
	if (rctmcr(1, bp_ir) < 0) {
		rp_rcte(12, RD, 0);
		goto rp_s18;
	}
	printd4("rp_s12: writing into LBN %d\n", rp_lbn);
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_WRT;
	ra_badm = MD_CMP;
	if(rp_irs == NO)
		ra_badm |= MD_ERR;
	write(fd, bp_ir, 512);	/* write saved -> replacement block */
	printd4("rp_s12: ra_stat = 0x%x\n", ra_stat);
	if((ra_stat & ST_MSK) == ST_SUC) 
		goto rp_s121;
	if((ra_eflags & EF_BBR) != EF_BBR)
		if((ra_stat == ST_DAT) && (rp_irs == NO))
			goto rp_s121;
	/* write - compare failed */
	rp_rcte(12, -1, 0);
	printf("\nWRITE-COMPARE operation on replacement block failed!\n");
	/* 
	 *  If a header compare error or a bad block reported, go back
	 *  to step 9 and try to find another RBN.
	 */
	if((ra_stat != MD_HCE) || (ra_eflags & EF_BBR))
		goto rp_s9;
	else {
		printf("Check the RCT, it MAY be corrupt!!\n");
		if(cmd[0] = 'i')
			goto bg_loop;
		else {
			goto do_u_x;
		}
	}
rp_s121:	/* replacement succeeded */
	rp_didit++;

rp_s13:
#ifdef	DEBUG
	if(rp_bps == 13)
		goto do_u_x;
#endif	DEBUG
/*
 * Update RCT block 0 to indicate replacement
 * no longer in progress.
 */
printd1("13 ");
	if(goforit)
		badsdebug = 3;
	rtp->rt_stat &= RCI_VP;	/* clean out all but VP */
	rtp->rt_lbn = 0;
	rtp->rt_rbn = 0;
	rtp->rt_brb = 0;
	if(rctmcw(0, bp_rci) < 0) {
		rp_rcte(13, WRT, 0);
		goto rp_s17;
	}

rp_s14:
#ifdef	DEBUG
	if(rp_bps == 14)
		goto do_u_x;
#endif	DEBUG
/*
 * Soft lock not used.
 * Replacement completed, return!
 */
	if(cmd[0] == 'i') {
		if(rp_didit)
			goto do_i_rg;
		else
			goto do_i_ra;
	} else {
		if(fd > 0)
			close_fd(fd);
		if(rp_dirty)
			rp_dirty = 0;
		if(rp_didit)
			printd("SUCCEEDED!\n");
		else
			if (nondataerr) {
				printd("ABORTED (nondata error encontered)!\n");
				nondataerr = NO;
			}
			else
				printd("ABORTED (block not really bad)!\n");
		goto retry;
	}

rp_s15:
#ifdef	DEBUG
	if(rp_bps == 15)
		goto do_u_x;
#endif	DEBUG
/*
 * Restore RCT to say new RBN unusable and
 * set bad LBN back to its original state.
 */
printd1("15 ");
	bp_rct1[rp_off].rbnd_bits.rct_code = UNUSABLE;
	if(rp_prev) {
		if(rp_blk != rp_oblk)
			bp_rct2[rp_ooff].rbnd_l = rp_orbd;
		else
			bp_rct1[rp_ooff].rbnd_l = rp_orbd;
	}
	if(rctmcw(rp_blk, bp_rct1) < 0)
		rp_rcte(15, WRT, rp_blk);
	if(rp_prev && (rp_blk != rp_oblk))
		if(rctmcw(rp_oblk, bp_rct2) < 0)
			rp_rcte(15, WRT, rp_oblk);

rp_s16:
#ifdef	DEBUG
	if(rp_bps == 16)
		goto do_u_x;
#endif	DEBUG
/*
 * Write saved date back to the bad LBN
 * with forced error indicator.
 */
printd1("16 ");
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_WRT;
	if(rp_irs == NO)
		ra_badm = MD_ERR;
	if(write(fd, bp_ir, 512) != 512) {
		rp_rcte(16, -1, 0);	/* RP step 16 FAILED! - message */
		printf("\nRewrite of original data back to bad block failed!\n");
	}

rp_s17:
#ifdef	DEBUG
	if(rp_bps == 17)
		goto do_u_x;
#endif	DEBUG
/*
 * Update RCT block 0 to say replacement
 * no longer inprogress.
 */
printd1("17 ");
	if(goforit)
		badsdebug = 7;
	rtp->rt_stat &= RCI_VP;	/* clean out all but VP */
	rtp->rt_lbn = 0;
	rtp->rt_rbn = 0;
	rtp->rt_brb = 0;
	if(rctmcw(0, bp_rci) < 0)
		rp_rcte(17, WRT, 0);

rp_s18:
#ifdef	DEBUG
	if(rp_bps == 18)
		goto do_u_x;
#endif	DEBUG
/*
 * Soft lock not used.
 * Replacement failed, return!
 */
	if(goforit)
		badsdebug = 3;
	if(cmd[0] == 'i')
		goto do_i_rb;
	else {
		if(rp_dirty)
			rp_dirty = 0;
		printf("\n\7\7\7Block %d replacement FAILED!\n", rp_lbn);
		if(fd > 0)
			close_fd(fd);
		goto retry;
	}
}

/*
 *  Write data to a block, and then read it -- thus testing the block
 */

blocktest(fd, ucerr, bp)
int	fd;			/* file descriptor */
char 	*bp;
int	ucerr;			/* uncorrectable ECC error when reading
				 * the block the block to be replaced
				 */
{
	int i, count;
	int	block_good;	/* true if block is thought to be good */

	block_good = YES;
	lseek(fd, (long)(rp_lbn*512), 0);
	ra_badc = RP_WRT;
	ra_badm = (MD_CMP|MD_SEC|MD_SREC | (!ucerr ? 0 : MD_ERR));
	if((((count = write(fd, bp, 512)) != 512) && (ra_eflags & EF_BBR)) ||
		(ra_stat & 0xe8)) {
		block_good = NO;
		printd3("blocktest: write failed, count = %d\n", count);
	}
	if (block_good == YES) {
		for (i = 0; i < 4; i++) {
			lseek(fd, (long)(rp_lbn*512), 0);
			ra_badc = RP_AC;
			ra_badm = MD_SEC|MD_SREC;
			if(((read(fd, buf, 512) != 512) && (ra_eflags & EF_BBR)) ||
				(ra_stat & 0xe8)) {
				block_good = NO;
				i = 4;
			}
		}
	}
	return (block_good);
}

/*
 * RCT read/write fatal error message
 *
 * s	= step in replacement process
 * rw	= failure was on read or write
 *	= -1, print first line then return
 * lbn	= block number
 */

rp_rcte(s, rw, lbn)
int	s;
int	rw;
int	lbn;
{
	printf("\n\7\7\7Bad block replacement process step %d FAILED!", s);
	if(rw == -1)
		return;
	printf("\nMulticopy %s", (rw == WRT) ? "write" : "read");
	printf(" of RCT block %d FAILED!\n", lbn);
	deprnt();	/* print disk error info */
}

/*
 * Open the disk and set file descriptor in fd.
 * mode = how to open, read, write, read/write.
 * err  = ignore errors (see if drive is there).
 *
 * If the disk has an RCT, read RCT block zero
 * and set rp_dirty to the phase that was in
 * progress if the replacement was interrupted.
 * Otherwise, set rp_dirty to zero.
 */
dskopen(err, mode)
int err;
int mode;
{
	register struct rct_rci *rtp;

	if((fd = open(fn, mode)) <= 0) {
		if(err == 0) {
			printf("\nCan't open %s!\n", fn);
			exit(FATAL);
		}
		return;
	}
	/* Oh what a mess */
	dtp = dt;
	*dtp++ = (F_to_C(ra_drv[unit].ra_mediaid,2) | 0x20);
	*dtp++ = (F_to_C(ra_drv[unit].ra_mediaid,1) | 0x20);
	sprintf(dtp, "%d", (ra_drv[unit].ra_mediaid & 0x7f));

	for(dip=dkinfo; dip->dki_type; dip++)
		if(strcmp(dip->dki_type, dt) == 0)
			break;
	if(dip->dki_type == 0) {
		if (F_to_C(ra_drv[unit].ra_mediaid,2) == 'R') 
			printf("\n(%s) - not supported by RABADS!\n", dt);
		return(-1);
	}
	rp_dirty = 0;
	if((ra_drv[unit].ra_rctsz == 0) || (dip->dki_flag != BB_COOP))
		return;
	if(rctmcr(0, bp_rci) < 0) {
		printf("\n\7\7\7WARNING: cannot read RCT block zero for ");
		printf("%s unit %d!\n", ra_dct, unit);
		return;
	}
	rtp = (struct rct_rci *)bp_rci;
	if(rtp->rt_stat & RCI_P1)
		rp_dirty = 1;
	if(rtp->rt_stat & RCI_P2)
		rp_dirty = 2;
	if(rp_dirty == 0)
		return;
	printf("\n\7\7\7\t\t****** WARNING ******\n");
	printf("\nA bad block replacement was interrupted during phase %d.\n",
		rp_dirty);
	if(cmd[0] == 'r')
		return;
	printf("\nAfter the current command completes, execute a replace");
	printf("\ncommand by responding to the rabads < ..... >: prompt");
	printf("\nwith the letter r followed by <RETURN>. This will");
	printf("\ncause the interrupted replacement to be completed.\n");
}

prtdsk()
{
	struct dkinfo *dp;

	printf("\nDisk\tULTRIX\tSize in");
	printf("\nName\tName\tBlocks");
	printf("\n----\t----\t------");
	for(dp=dkinfo; dp->dki_type; dp++){
		printf("\n%s\t", dp->dki_type);
		printf("%s\t", dp->dki_name);
		printf("%d", dp->dki_size);
	}
	printf("\n");
}
/*
 * Drive information pointer set,
 *	pointer into dkinfo table,
 *	unit number,
 *	MSCP controller number,
 *	file name - ??(#,0).
 */
dipset()
{
	register int i, j, k, l;
	int devnum;		/* the device number calculated from the
				   UBA number and the unit number */
	int validinput;		/* flag for indicating valid input chars */
	union cpusid	sid;

dip_x:

	/*  Ask questions based on the cpu type. Only odd one for now is
	 *  8200
 	*/
	sid.cpusid = mfpr(SID);		/* Get cpu type */
	cpu_type = sid.cpuany.cp_type; 	/* Save cpu type */
	switch (cpu_type){

#ifndef MVAX
#if defined(VAX8200)
	case VAX_8200:
re_ask:
		printf ("\nEnter adapter node ID (0 - 15) < %d >:", nodeid);
		getstring(csr);
		if (csr[0] != '\0') {
			csraddr = atol(csr);
			if (csraddr < 0 || csraddr > 15) {
				printf ("Invalid Node ID\n");
				goto re_ask;
				}
			nodeid = csraddr;
			uda_offline = 1;	/* mark uda offline */
			for (k=0; k < MAXDRIVE; k++)
				ra_offline[k] = 1;	/* mark all drives offline */
			udaddr = 0;	/* force new CSR */
			ud_ubaddr = 0;	/* force new unibus address of uda structure */
			}
		saveCSR = (long)NEX8200(nodeid);

		if (!badloc(((caddr_t)saveCSR),sizeof(int))) {

			switch (((struct bi_nodespace *)saveCSR)->biic.biic_typ & BITYP_TYPE) {

			case BI_BUA:
			 /*
			 *  Get the CSR of the controller.  The standalone driver
			 *  only knows about one controller at a time.  We have to fool
			 *  driver by making it open the controller at the CSR we 
			 *  specify.
			 */


csr1_num:
				printf("\nEnter controller CSR (in octal) < %o >: ", udastd[0]);
				getstring(csr);
				if (csr[0] != '\0') {
					for (j = 0; csr[j] != '\0'; j++)
						if (csr[j] > '7') {
							printf (" %s contains an invalid octal digit!\n", csr);
							goto csr1_num;
						}
					csraddr = atox(csr,3);
					if (csraddr > 0177777) {
						printf ("CSR address too large!\n");
						goto csr1_num;
					}
					if (csraddr != 0 && (csraddr > 0177777 || csraddr < 0160010)) {
						printf ("Bad unibus address!\n");
						goto csr1_num;
					}
					udastd[0] = csraddr;
					uda_offline = 1;	/* mark uda offline */
					for (k=0; k < MAXDRIVE; k++)
						ra_offline[k] = 1;	/* mark all drives offline */
					udaddr = 0;	/* force new CSR */
					ud_ubaddr = 0;	/* force new unibus address of uda structure */
				}
				can_bda = 0;	/* Not a BDA */
				ubanum = 0;
				break;
	
			case BI_BDA:
				can_bda = 1;
				ubanum = 0;
				break;

			default:
				printf ("Invalid Node ID - Must be BUA or BDA\n");
				goto re_ask;
				break;

			}
		}
		else {
			printf ("Invalid Node ID - Must be BUA or BDA\n");
			goto re_ask;
		}
		break;

#endif VAX8200

	default:


		can_bda = 0;		/* Not a BDA */
		/*
		 *  Get the UBA number.  uba0 is at TR3 and so on.
		 */
	
		printf ("\nEnter UBA number ( 0 - 7 )  < %d >: ", (ubanum / 8));
		getstring(dn);
		if (dn[0] != '\0') {
			if((strlen(dn) != 1) || (dn[0] < '0') || (dn[0] > '7')) {
				printf("\nUBA numbers 0 -> 7 only!\n");
				goto dip_x;
			}
			ubanum = (dn[0] - '0') * 8;	/* uba offset */
			/*
			 * When switching to another UBA, the UDA structures
			 * must be reinitialized so the driver knows to
			 * start-up the controller.
			 */
			uda_offline = 1;	/* mark uda offline */
			for (k=0; k < MAXDRIVE; k++)
				ra_offline[k] = 1;	/* mark all drives offline */
			udaddr = 0;	/* force new CSR */
			ud_ubaddr = 0;	/* force new unibus address of uda
					   structure */
		}
#else MVAX
default:
		ubanum = 0;
#endif MVAX

	/*
	 *  Get the CSR of the controller.  The standalone driver
	 *  only knows about one controller at a time.  We have to fool
	 *  driver by making it open the controller at the CSR we 
	 *  specify.
	 */

csr_num:
	printf("\nEnter controller CSR (in octal) < %o >: ", udastd[0]);
	getstring(csr);
	if (csr[0] != '\0') {
		for (j = 0; csr[j] != '\0'; j++)
			if (csr[j] > '7') {
				printf (" %s contains an invalid octal digit!\n", csr);
				goto csr_num;
			}
		csraddr = atox(csr,3);
		if (csraddr > 0177777) {
			printf ("CSR address too large!\n");
			goto csr_num;
		}
		if (csraddr != 0 && (csraddr > 0177777 || csraddr < 0160010)) {
			printf ("Bad unibus address!\n");
			goto csr_num;
		}
		udastd[0] = csraddr;
		uda_offline = 1;	/* mark uda offline */
		for (k=0; k < MAXDRIVE; k++)
			ra_offline[k] = 1;	/* mark all drives offline */
		udaddr = 0;	/* force new CSR */
		ud_ubaddr = 0;	/* force new unibus address of uda structure */
	}
	}

	/*
	 *  Get the drive unit number.  This number corresponds to the
	 *  unit plug in the drive.
	 */

unit_num:
	printf("\nEnter unit number ( 0 - 7 )  < %d >: ", unit);
	getstring(dn);
	if (dn[0] != '\0') {
		if((strlen(dn) != 1) || (dn[0] < '0') || (dn[0] > '7')) {
			printf("\nUnits 0 -> 7 only!\n");
			goto unit_num;
		}
		unit = (dn[0] - '0');
	}

	/*
	 *  Calculate the device number, and then build a string to be
	 *  passed to the open routine.  Determine which entry
	 *  in the devsw table to use to get to the driver.
	 */

	devnum = ubanum + unit;
	sprintf(fn, "%s(%d,0)", "ra", devnum);
	for(i=0; devsw[i].dv_name; i++)
		if(strcmp("ra", devsw[i].dv_name) == 0)
			break;
#ifdef	DEBUG

	printf("\nDEBUG: Break Point at replacement step ? ");
	getstring(line);
	if(line[0] == '\0')
		rp_bps = 0;
	else
		rp_bps = atoi(line);

#endif	DEBUG
	return(0);
}

/*
 *  Close the file descriptor
 */

close_fd(fd)
int	fd;		/* file descriptor */
{
	close(fd);
	fd = -1;
	printd("\n");
}

/*
 *  Set the forced error indicator on a single block
 */

set_forced_error()
{
	char	fembuf[512];		/* buffer for sector of data to be written with forced error indicator */
	char	line[15];	
	int	close_file = 0;		/* flag for errors, so the file descriptor gets closed after the error */
	int	bn;			/* block number in which to set the forced error indicator */

	printf("\nPassword: ");
	getstring(line);
	if(strcmp(line, pwstr) == 0) {
		if(dipset() >= 0) {
			if(dip->dki_flag != BB_NONE) {
				dskopen(NOERR, RW);
				if(continu(FRCE) >= 0) {
					if(fd >= 0) {
						if(!rp_dirty) {
							if(ra_drv[unit].ra_rctsz != 0) {
								while((bn = blockno(fd)) < 0)
									;
								lseek(fd, (bn * 512), 0);
								read(fd, fembuf, 512);
								lseek(fd, (bn * 512), 0);
								ra_badc = RP_WRT;
								ra_badm = 010000;	/* MD_ERR (forced error) */
								write(fd, fembuf, 512);
							}
							else {
								printf("\nDisk does not have a Revector Control Table!");
								close_file++;
							}
						}
						else {
							printf("\nForce aborted!\n");
							close_file++;
						}
					}
					else
						printf("\nCan't open %s!\n",
							fn);
				}
			}
			else
				printf("\nCannot replace blocks on an %s disk!\n",
					dip->dki_type);
		}
	}
	else
		printf("\nSorry - reserved for Digital!\n");
	if(close_file > 0) {
		close_fd(fd);
	}
}
										


/* 
 *  get the block number, and do some sanity checking
 */

blockno(fd)
int	fd;		/* file descriptor */
{
	int	bn;	/* block number */

	printf("\nBlock number: ");
	getstring(line);
	if(line[0] == '\0')
		return(-1);
	bn = atol(line);
	if((bn < 0) || (bn >= ra_drv[unit].ra_dsize))
		return(-2);
	return(bn);
}


/*
 * RCT multicopy read,
 *	lbn = sector of RCT to read.
 *	bp = pointer to buffer.
 *	Host area size is added to lbn.
 * fd is the previously opened file descriptor,
 * all are external to main().
 */

rctmcr(lbn, bp)
unsigned lbn;
union rctb *bp;
{
	register int i, n, s;
	daddr_t	bn, boff;

	n = ra_drv[unit].ra_ncopy;
	s = ra_drv[unit].ra_rctsz;
	bn = ra_drv[unit].ra_dsize + lbn;
	for(i=0; i<n; i++) {
		if(rctcopy && (rctcopy != (i+1)))
			continue;
		boff = bn * 512;
		lseek(fd, (long)boff, 0);
		ra_badc = RP_RD;	/* tell driver doing BADS funny business */
		read(fd, (char *)bp, 512);
		if(ra_stat == ST_SUC) {
			return(0);
		}
		bn += s;
	}
	return(-1);
}

/*
 * RCT multicopy write,
 *	lbn = sector of RCT to write.
 *	bp = pointer to buffer.
 *	Host area size is added to lbn.
 * fd is the previously opened file descriptor,
 * all are external to main().
 */

rctmcw(lbn, bp)
unsigned lbn;
union rctb *bp;
{
	register int i, n, s;
	int ec, sc, fatal;
	daddr_t bn, boff;
	int count;

	n = ra_drv[unit].ra_ncopy;
	s = ra_drv[unit].ra_rctsz;
	bn = ra_drv[unit].ra_dsize + lbn;
	ec = fatal = 0;
	for(i=0; i<n; i++, bn += s) {
		boff = bn * 512;
		lseek(fd, (long)boff, 0);
		ra_badc = RP_WRT;	/* tells driver doing BADS funny business */
		ra_badm = MD_CMP ;
	/*	write(fd, (char *)bp, 512); */
		if((count = write(fd, (char *)bp, 512)) != 512)
			printd2("rctmcw: failed, count = %d\n", count);
		if(ra_stat == ST_SUC)
			continue;
		sc = (ra_stat >> 5) & 03777;
		printd2 ("rctmcw: ra_stat = 0x%x, sc = 0x%x\n", ra_stat, sc);
		if((ra_stat & ST_MSK) == ST_DAT) {
			if((sc != 2) && (sc != 3) && (sc != 7))
				continue; /* NOT - HCE, DATA SYNC, UNC ECC */
			ec++;
			lseek(fd, (long)boff, 0);
			ra_badc = RP_WRT;
			ra_badm = MD_ERR;
		/*	write(fd, (char *)bp, 512); */
			if((count = write(fd, (char *)bp, 512)) != 512)
				printf("rctmcw: failed, count = 0x%x\n", count);
			if((ra_stat & ST_MSK) == ST_SUC)
				continue;
			sc = (ra_stat >> 5) & 03777;
			printd2 ("rctmcw: retry, ra_stat = 0x%x, sc = 0x%x\n",
				 ra_stat, sc);
			if((ra_stat & ST_MSK) == ST_DAT) {
				if((sc != 2) && (sc != 3) && (sc != 7))
					continue;
			}
		}
		fatal = -1;
		break;
	}
	if(ec >= n)
		fatal = -1;
	return(fatal);
}

/*
 * Print the disk error information that would
 * have been printed by the driver if the ra_badc
 * flag was not set.
 */
deprnt()
{
	printf("%s unit %d disk error: ",
		ra_dct, unit);
	printf("endcode=%o flags=%o status=%o\n",
		ra_ecode, ra_eflags, ra_stat);
	printf("(FATAL ERROR)\n");
}

rbempty()
{
	if(bp_rct1[rp_off].rbnd_bits.rct_code == 0)
		return(1);
	else
		return(0);
}

rbmatch()
{
	printd3("\nrp_lbn = %d, rct_lbn = %d  ", rp_lbn,
		bp_rct1[rp_off].rbnd_bits.rct_lbn);
	if(bp_rct1[rp_off].rbnd_bits.rct_code & UNUSABLE) {
		printd3("rbmatch: UNUSABLE RBN\n");
		return(0);
	}
	if(rp_lbn == bp_rct1[rp_off].rbnd_bits.rct_lbn) {
		printd3("rbmatch: found match\n");
		return(1);
	}
	printd3("rbmatch: no match");
	return(0);
}

rbnull()
{
	if(bp_rct1[rp_off].rbnd_bits.rct_code == NULL_ENTRY)
		return(1);
	else
		return(0);
}
rblast()
{
	register int i;

	for(i=0; i<128; i++)
		if(bp_rct1[i].rbnd_bits.rct_code == NULL_ENTRY)
			return(1);
	return(0);
}

prfm()
{
	register char c;
		
	printf("\n\nPress <RETURN> for more");
	printf(" (<CTRL/C> to abort): ");
	while((c=getchar()) != '\n') {
		if(c == 03) /* <CTRL/C> */
			return(1);
	}
	return(0);
}

char	ynline[40];

yes(hlp)
{
yorn:
	getstring(ynline);
	if(ynline[0] == '\0')
		return (hlp);
	if((strcmp(ynline, "y") == 0) || (strcmp(ynline, "yes") == 0))
		return(YES);
	if((strcmp(ynline, "n") == 0) || (strcmp(ynline, "no") == 0))
		return(NO);
	if((strcmp(ynline, "?") == 0) || (strcmp(ynline, "help") == 0))
		return(HELP);
	if((strcmp(ynline, "Y") == 0) || (strcmp(ynline, "YES") == 0))
		return(YES);
	if((strcmp(ynline, "N") == 0) || (strcmp(ynline, "NO") == 0))
		return(NO);
	if(strcmp(ynline, "h") == 0)
		return(HELP);
	if((strcmp(ynline, "H") == 0) || (strcmp(ynline, "HELP") == 0))
		return(HELP);
	printf("\nPlease answer yes or no!\n");
	goto yorn;
}

/*
 * Read a suspected bad block and
 * return its status.
 */

brchk(lbn)
daddr_t lbn;
{
	register int i, s;
	int	numofreads;

	if (badsdebug > 2)
		numofreads = 5;
	else
		numofreads = 49;
	for(i = 0; i < numofreads; i++) {
		lseek(fd, (long)(lbn*512), 0);
		ra_badc = RP_AC;
		if(read(fd, (char *)buf, 512) != 512)
			break;
	}
	s = ra_stat & ST_MSK;
	if(s == ST_SUC) {
		if(ra_eflags & EF_BBR)
			return(BR_BBR);
		else
			return(BR_NBBR);
	} else if(s == ST_DAT) {
		if((dip->dki_flag != BB_RWRT) && (ra_eflags & EF_BBR))
			return(BR_DEB);
		switch((ra_stat >> 5) & 03777) {
		case 0:
			return(BR_FEM);
		case 4:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			/* can't happen - not allowing datagrams */
			return(BR_CECC);
		case 7:
			return(BR_UECC);
		default:
			return(BR_NDE);
		}
	} else
		return(BR_NDE);
}

/*
 * Conditionally print BLOCK # header message.
 * Rewite a block that was written with "forced error".
 * Return status of rewrite operation.
 */

fembrw(hm, lbn)
int	hm;
daddr_t	lbn;
{
	if(hm)
		printf("BLOCK: %d - ", lbn);
	printf("rewrite to clear \"forced error\" ");
	lseek(fd, (long)(lbn*512), 0);
	ra_badc = RP_WRT;
	write(fd, (char *)buf, 512);
	lseek(fd, (long)(lbn*512), 0);
	if(read(fd, (char *)buf, 512) == 512) {
		printf("SUCCEEDED!\n");
		return(YES);
	} else {
		printf("FAILED!\n");
		deprnt();	/* print disk error info */
		return(NO);
	}
}

/*
 *  get a string of characters and strip out any Ctrl-S or Ctrl-Q 
 *  characters.  The standalone system doesn't know what to do with
 *  Ctrl-S or Ctrl-Q chars, and just passes them on into the input
 *  string.  
 */

getstring (cp)

char *cp;
{
	char *tp;	/* temporary character pointer */

	tp = cp;
	gets(cp);
	for(; *cp != '\0'; cp++) {
		*tp = *cp;
		if ((*cp != 0x11) && (*cp != 0x13))
			tp++;
		else
			*tp = '\0';
	}
}

/*
 *  print the forced error warning.
 */

notify_fem(block)
int block;
{
	printf("\nBLOCK: %d - ", block);
	printd("%s", feb_nc1); /* warn bad data */
	printd("%s", feb_nc2);
}

/*
 *  Inform the user of what they have told rabads to do,
 *  and ask if they want to continue.
 */

continu(cmdtype)
int cmdtype;
{
	if ((rp_dirty) && ((cmdtype != TABLE) && (cmdtype != STATUS)
				&& (cmdtype != REPLACE))) {
		printf("Please run the \"replace\" command! \n");
		return(-1);
	}
	if (F_to_C(ra_drv[unit].ra_mediaid,2) != 'R') {
		printf("\nYou have selected an unknown or non-existent unit\n");
		return (-1);
		}
	printf("\nYou have selected the %s command ", cmd_mesg[cmdtype]);
	printf("on the %c%c%d at:\n", F_to_C(ra_drv[unit].ra_mediaid,2)
				    , F_to_C(ra_drv[unit].ra_mediaid,1)
				    , ra_drv[unit].ra_mediaid & 0x7f);

	if (cpu_type == VAX_8200) {	/* Handle BI systems */
		printf("\tNode ID\t\t%d\n",nodeid);
		if (can_bda == 0)	/* Print the CSR on the BUA */
			printf("\tCSR\t\t%o\n", udastd[0]);
		printf("\tdrive unit\t%d\n", unit);
		}
	else {
#ifndef MVAX
		printf("\tUBA\t\t%d\n", (ubanum/8));
#endif	MVAX
		printf("\tCSR\t\t%o\n\tdrive unit\t%d\n", udastd[0], unit);
	}
	printf("Do you wish to continue? (y or n) <y>: ");
	/*getstring(line);
	if((*line == 'y') || (*line == '\0'))
	 */
	if(yes(YES))
		return (0);
	else
		return (-1);
}


atox(sp,times)	/* make an octal or hex word from a ascii string */
		/* --doesn't handle overflow!!		  */
short times;	/* number of times to shift left -- 3 for octal, 4 for hex */
char *sp;
{
	int i = 0;

	while (*sp != '\0') {
		if (*sp >= '0' && *sp <= '7')
			i = (i<<times | (*sp++ - '0'));
		else if (*sp >= 'a' && *sp <= 'f')
			i = (i<<times | ((*sp++ - 'a') + 10));
		else if (*sp >= 'A' && *sp <= 'F')
			i = (i<<times | ((*sp++ - 'A') + 10));
		else {
			printf("\nBad digit\n");
		}
	}
	return (i);
}

writeblk()
{
	int	i, block, addr, offset;
	u_char	wbuf[1024];
	char	*lp;

	addr = offset = 0;
	if (dipset() == 0) {
		dskopen(ERR, RW);
		do {
			printf("\nBlock number: ");
			getstring(line);
		}
		while (line[0] == '\0');
		block = atol(line);		/* block number to be written */
		printf("\nStart with a clean block < y or n > ? ");
		getstring(line);
		if (line[0] == 'y')
			for (i = 0; i < 1024; i++)
				wbuf[i] = 0;
		else {
			if(lseek(fd, (block*512), 0) < 0) {
				printf("writeblk: Seek error\n");
				exit(1);
			}
			if(read(fd, wbuf, 512) != 512){
				printf("writeblk: Error reading sector\n");
				exit(1);
			}
		}
		do {
			printf("\nFormat (hex only): blockoffset byte0 byte1 ");
			printf("byte2 ..... byte15\n>: ");
			getstring(line);
			lp = line;
			while(*lp != '\0') {
				while((*lp != '\0') && (*lp != ' ')) {
					if(*lp >= 'a' && *lp <= 'f')
						addr = ((addr<<4) + ((*lp++ - 'a') + 10));
					else
						addr = ((addr << 4) + (*lp++ - '0'));
				}
				while (*lp != '\0') {
					if (*lp == ' ')
						lp++;
					while ((*lp!='\0')&&(*lp!=' '))
					    if((*lp>='a')&&(*lp<='f'))
						wbuf[addr+offset] = ((wbuf[addr+offset] << 4) + ((*lp++ - 'a') + 10));
					    else
						wbuf[addr+offset] = ((wbuf[addr+offset] << 4) + (*lp++ - '0'));
					offset++;
				}
			}
			offset = 0;
		}
		while (!prfm());
		if(lseek(fd, (block*512), 0) < 0) {
			printf("writeblk: Seek error\n");
			exit(1);
		}
#ifdef DEBUG
		printf ("\nwriteblk: block = %d\n", block);
#endif DEBUG
		if(write(fd, wbuf, 512) != 512){
			printf("writeblk: Error writing sector\n");
			exit(1);
		}
#ifdef DEBUG
		printf("writeblk: closing file\n");
#endif DEBUG
		close_fd(fd);
	}
}


getblk()
{
	int	block;
	char	rbuf[512];

	if (dipset() == 0) {
		dskopen(ERR, RD);
		do {
			printf("\nBlock number: ");
			getstring(line);
		}
		while (line[0] == '\0');
		block = atol(line);	/* block number to be dumped */
#ifdef DEBUG
			printf ("getblk: block = %d\n", block);
#endif DEBUG
		if(lseek(fd, (block*512), 0) < 0) {
			printf("getblk: Seek error\n");
			exit(1);
		}
		if(read(fd, rbuf, 512) != 512){
			printf("getblk: Error reading sector\n");
			exit(1);
		}
#ifdef DEBUG
			printf ("getblk: closing file\n");
#endif DEBUG
		close_fd(fd);
	}
	dumpblk(rbuf);
}

/*  It is sometimes desireable to change the default modifier used in
 *  mscp command messages.  One use of this is to lower the drive 
 *  ECC threshold for testing.  This is controlled by the variable
 *  "command_modifier".  This routine provides an easy way to change
 *  the variable "command_modifier".
 *
 *  Note:  The value supplied here is NOT used by the driver when doing
 *         a replace command, and should not be, as the replace command
 *	   does strange things when you mess around with the modifier
 *	   field.
 */

chgthreshold(modptr)
int * modptr;		/* pointer to command_modifier field */
{
	char	instr[10];	/* string to hold keyboard input */

	printf("\nEnter the value command_modifier value in hex <%x>: ",
		*modptr);
	getstring(instr);
	if (instr[0] != '\0')
		*modptr = atox(instr, 4);
}

/*
 *  There are several levels of printouts defined in this program -- all
 *  are controlled by the variable "badsdebug".  This routine prompts
 *  for a verbose level that gets plugged into the variable "badsdebug"
 */

verbose(levelptr)
int * levelptr;		/* level of printout 0 - 5 */
{
	char	instr[10];	/* string to hold keyboard input */

	printf("\nEnter the value of verbose level (0 to 6) <%d>: ",
		*levelptr);
	getstring(instr);
	if (instr[0] != '\0')
		*levelptr = atol(instr);
}

dumpblk(ptr)
char	*ptr;
{
	int	j;
	short	addr, start;
	u_char	*rbuf;

	rbuf = (u_char *)ptr;
	addr = j  = 0;
	do {
		while ((addr < 512) && (j < 16)) {
			if (addr < 0x10)
				printf ("00%x  ", addr);
			else if (addr < 0x100)
				printf ("0%x  ", addr);
			else
				printf ("%x  ", addr);
			for ( start = addr; (start+16) != addr; addr++) {
				if (rbuf[addr] > 0xff)
					printf("XX ");
				else
					if (rbuf[addr] < 0x10)
						printf ("0%x ", rbuf[addr]);
					else
						printf ("%x ", rbuf[addr]);
			}
			addr = start;
			for ( ; (start+16) != addr; addr++)
				if((rbuf[addr] >= 0x20) && (rbuf[addr]<=0x7e))
					printf ("%c", rbuf[addr]);
				else
					printf (" ");
			printf ("\n");
			j++;
		}
		j = 0;
		if (addr != 512)
			if (ask)	/* don't ask abort message */
				if (prfm())
					addr = 512; /* abort rest of printout */
	}
	while (addr < 512);
}
/*
 *	Read and write a block "testno" of times for debuging the
 *	stand alone system write/read bug.
 */

beatblock()

{
	int i, j, count;
	int	block_good;	/* true if block is thought to be good */
	int	testno;		/* number of times to test the block */
	int	blockno;	/* the block under test */
	char	buff[512];	/* temporary buffer */

	block_good = YES;
	if (dipset() == 0) {
		dskopen(ERR, RW);
		do {
			printf("\nBlock number: ");
			getstring(line);
		}
		while (line[0] == '\0');
		blockno = atol(line);	/* block number to be dumped */
		printd3("beatblock: block = %d\n", blockno);
		do {
			printf("Number of tests: ");
			getstring(line);
		}
		while (line[0] == '\0');
		testno = atol(line);	/* number of times to test block */
	}
	else {
		printd("beatblock: dipset failed\n");
		return;
	}
	printf("\nbeating block...");
	for (j = 0; j < testno; j++) {
		for (i = 0; i < 4; i++) {
			/* check for any data error */
			if (lseek(fd, (long)(blockno*512), 0) < 0) {
				printf("beatblock: Seek error\n");
				exit(1);
			}
			ra_badc = RP_RD;
			ra_badm = MD_SEC|MD_SREC;
			if ((read(fd, buff, 512) != 512)
				&& ((ra_eflags & EF_BBR)||(ra_eflags & EF_BBU))
				|| (ra_stat & 0xe8)) {
				block_good = NO;
				i = 4;
			}
		}
		lseek(fd, (long)(blockno*512), 0);
		ra_badc = RP_WRT;
		ra_badm = (MD_CMP);
		if((((count=write(fd,buff,512))!=512)&&(ra_eflags & EF_BBR))||
			(ra_stat & 0xe8)) {
			block_good = NO;
			printd3("beatblock: write failed, count = %d\n", count);
		}
		if (block_good == YES) {
			for (i = 0; i < 4; i++) {
				lseek(fd, (long)(blockno*512), 0);
				ra_badc = RP_AC;
				ra_badm = MD_SEC|MD_SREC;
				if(((read(fd,buff,512)!=512)&&(ra_eflags&EF_BBR)) ||
					(ra_stat & 0xe8)) {
					block_good = NO;
					i = 4;
				}
			}
		}
		if (j % 100 == 0)
			printf ("k");
	}
	if (block_good = NO)
		printf("beatblock: error enountered\n");
	printf("beatblock: completed\n");
	close_fd(fd);
}
