#ifndef lint
static char *sccsid = "@(#)pstat.c	1.18	ULTRIX	1/29/87";
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

/* ------------------------------------------------------------------------
 * Modification History: /usr/src/etc/pstat.c
 *
 * 29 Jan 87 -- Marc Teitelbaum
 *	Check for INUSE flag on inode, display as "I".  Check for
 *	FBLKINUSE on file flags, display as "B". Add checks for
 *	following tty flags:
 *		TT_STOP  - 'S'
 *		TT_INUSE - 'I'
 *		TT_ONDELAY - 'D'
 *		TT_IGNCAR - 'G'
 *		TT_NBIO  - 'N'
 *		TT_ASYNC - 'Z'
 *	
 *
 * 26 Jan 86 -- depp
 *	Changed method that text swap space is calculated, do to the
 *	changes in text table allocation/deallocation.  The text table
 *	allocation/deallocation changes also made changes to dotext().
 *
 *  4 Dec 86 -- Tim burke
 *	Added support for dmb driver as part of the pstat -t option.
 *
 * 15 Sep 86 -- bglover 
 *	Change refs to x_iptr to x_gptr (text.h change)
 *
 * 22 Aug 86 -- fred (Fred Canter)
 *	Added support for VAXstar and MICROVAX 1800 serial line
 *	device drivers to the pstat -t option. The ss device is
 *	like the dzq11 and the sh is like an 8 line dhu11.
 *
 *  8 Apr 86 -- depp
 *	Removed reference to u.u_exdata, as it no longer exists
 *
 * 14 Oct 85 -- rr
 *	Fixed the handling of the -u ubase option. First problem
 *	was that the assumption was made that the UPAGES+CLSIZE
 *	preallocated 512 byte pages were contiguous and they are not.
 *	Stole the code from ps.c to read in each CLICK (1024 presently)
 *	and modified ps.c to print out the page table page frame
 *	so that all we do in pstat.c is lseek and back up exactly
 *	UPAGES+CLSIZE pte's, read the pte's, and then get each page
 *	one click at a time. Hopefully, this extends the life of this routine.
 *	Also, added new fields to the printout so that all fields are
 *	now dumped by the pstat -u ubase option.
 *
 * 23 Feb 84 -- jmcg
 *	Fixed -k flag handling.  Now that bufpages are allocated
 *	sparsely in virtual space to system buffers, there is no
 *	longer a simple mapping between kernel virtual addresses
 *	and physical memory.  /dev/kmem provides this mapping when
 *	running on a live system, but pstat must decipher the system
 *	page table itself when running against a vmcore.  These changes
 *	were originally made and tested around 24 Jan 84.  They are
 *	almost identical to those provided over the Usenet by A R White
 *	from University of Waterloo (newsgroup net.bugs.4bsd
 *	<6927@watmath.UUCP> 17 Feb 1984).
 *
 * 23 Feb 84 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		pstat.c	4.22 (Berkeley) 6/18/83
 *	with an update obtained by aps from ucbmonet.  He updated the
 *	sccsid to read:
 *	*sccsid = "@(#)pstat.c	1.4 (Ultrix 1.0)  (from 4.24 Berkeley) 2/24/84"
 *	but lost the date.
 *
 * ------------------------------------------------------------------------
 */

/*
 * Print system stuff
 */

#define mask(x) (x&0377)
#define	clear(x) ((int)x&0x7fffffff)

#include <sys/param.h>
#include <sys/dir.h>
#define	KERNEL
#include <sys/file.h>
#undef	KERNEL
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/gnode.h>
#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <nlist.h>
#include <machine/pte.h>

char	*fcore	= "/dev/kmem";
char	*fmem	= "/dev/mem";
char	*fnlist	= "/vmunix";
int	fc, fm;

struct nlist nl[] = {
#define	SGNODE	0
	{ "_gnode" },
#define	STEXT	1
	{ "_text" },
#define	SPROC	2
	{ "_proc" },
#define	SDZ	3
	{ "_dz_tty" },
#define	SNDZ	4
	{ "_dz_cnt" },
#define	SKL	5
	{ "_cons" },
#define	SFIL	6
	{ "_file" },
#define	USRPTMA	7
	{ "_Usrptmap" },
#define	USRPT	8
	{ "_usrpt" },
#define	SWAPMAP	9
	{ "_swapmap" },
#define	SDH	10
	{ "_dh11" },
#define	SNDH	11
	{ "_ndh11" },
#define	SNPROC	12
	{ "_nproc" },
#define	SNTEXT	13
	{ "_ntext" },
#define	SNFILE	14
	{ "_nfile" },
#define	SNGNODE	15
	{ "_ngnode" },
#define	SNSWAPMAP 16
	{ "_nswapmap" },
#define	SPTY	17
	{ "_pt_tty" },
#define	SDMMIN	18
	{ "_dmmin" },
#define	SDMMAX	19
	{ "_dmmax" },
#define	SNSWDEV	20
	{ "_nswdev" },
#define	SSWDEVT	21
	{ "_swdevt" },
#define SSYSMAP	22
	{ "_Sysmap" },
#define SSYSSIZE	23
	{ "_Syssize" },
#define SNPTY	24
	{ "_nNPTY" },
#define SDHU	25
	{ "_dhu11" },
#define SNDHU	26
	{ "_ndhu11"},
#define SDMF	27
	{ "_dmf_tty"},
#define SNDMF	28
	{ "_ndmf" },
#define SDMZ	29
	{ "_dmz_tty" },
#define SNDMZ	30
	{ "_ndmz" },
#define SSS	31
	{ "_ss_tty" },
#define SNSS	32
	{ "_ss_cnt" },
#define SSH	33
	{ "_sh_tty" },
#define SNSH	34
	{ "_sh_cnt" },
#define SDMB	35
	{"_dmb_tty"},
#define SNDMB	36
	{"_ndmb"},
	{ "" }
};

struct ttytype {
	char *name;
	int ttybase;
	int nttys;
} ttytypes[] = {
	{ "dz", SDZ, SNDZ },
	{ "ss", SSS, SNSS },
	{ "dh", SDH, SNDH },
	{ "dhu", SDHU, SNDHU },
	{ "sh", SSH, SNSH },
	{ "dmf", SDMF, SNDMF },
	{ "dmz", SDMZ, SNDMZ },
	{ "dmb", SDMB, SNDMB },
	{ "pty", SPTY, SNPTY },
	{ "" }
};
int	inof;
int	txtf;
int	prcf;
int	ttyf;
int	usrf;
long	ubase;
int	filf;
int	swpf;
int	totflg;
char	partab[1];
struct	cdevsw	cdevsw[1];
struct	bdevsw	bdevsw[1];
int	allflg;
int	kflg;
struct	pte *Usrptma;
struct	pte *usrpt;
struct	pte *Sysmap = 0;
int	sizeSysmap;

main(argc, argv)
char **argv;
{
	register char *argp;
	int allflags;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		argp = *argv++;
		argp++;
		argc--;
		while (*argp++)
		switch (argp[-1]) {

		case 'T':
			totflg++;
			break;

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'k':
			kflg++;
			fcore = fmem = "/vmcore";
			break;

		case 'x':
			txtf++;
			break;

		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (argc == 0)
				break;
			argc--;
			usrf++;
			sscanf( *argv++, "%x", &ubase);
			break;

		case 'f':
			filf++;
			break;
		case 's':
			swpf++;
			break;
		default:
			usage();
			exit(1);
		}
	}
	if (argc>1)
		fcore = fmem = argv[1];
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if ((fm = open(fmem, 0)) < 0) {
		printf("Can't find %s\n", fmem);
		exit(1);
	}
	if (argc>0)
		fnlist = argv[0];
	nlist(fnlist, nl);
	usrpt = (struct pte *)nl[USRPT].n_value;
	Usrptma = (struct pte *)nl[USRPTMA].n_value;
	if (nl[0].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}
	allflags = filf | totflg | inof | prcf | txtf | ttyf | usrf | swpf;
	if (allflags == 0) {
		printf("pstat: one or more of -[aixptfsu] is required\n");
		exit(1);
	}
	if (filf||totflg)
		dofile();
	if (inof||totflg)
		dognode();
	if (prcf||totflg)
		doproc();
	if (txtf||totflg)
		dotext();
	if (ttyf)
		dotty();
	if (usrf)
		dousr();
	if (swpf||totflg)
		doswap();
}

usage()
{

	printf("usage: pstat -[akixptfs] [-u [ubase]] [system] [core]\n");
}

dognode()
{
	register struct gnode *gp;
	struct gnode *xgnode, *agnode;
	register int ngn;
	int ngnode;

	ngn = 0;
	ngnode = getw(nl[SNGNODE].n_value);
	xgnode = (struct gnode *)calloc(ngnode, sizeof (struct gnode));
	klseek(fc, (int)(agnode = (struct gnode *)getw(nl[SGNODE].n_value)), 0);
	read(fc, xgnode, ngnode * sizeof(struct gnode));
	for (gp = xgnode; gp < &xgnode[ngnode]; gp++)
		if (gp->g_count)
			ngn++;
	if (totflg) {
		printf("%4d/%4d\tgnodes\n", ngn, ngnode);
		return;
	}
	printf("%d/%d active gnodes\n", ngn, ngnode);
printf("   LOC      FLAGS     CNT DEVICE  RDC WRC  GNO  MODE  NLK UID   SIZE/DEV\n");
	for (gp = xgnode; gp < &xgnode[ngnode]; gp++) {
		if (gp->g_count == 0)
			continue;
		printf("%8.1x ", agnode + (gp - xgnode));
		putf(gp->g_flag&GLOCKED, 'L');
		putf(gp->g_flag&GUPD, 'U');
		putf(gp->g_flag&GACC, 'A');
		putf(gp->g_flag&GMOUNT, 'M');
		putf(gp->g_flag&GWANT, 'W');
		putf(gp->g_flag&GTEXT, 'T');
		putf(gp->g_flag&GINUSE, 'I');
		putf(gp->g_flag&GCHG, 'C');
		putf(gp->g_flag&GSHLOCK, 'S');
		putf(gp->g_flag&GEXLOCK, 'E');
		putf(gp->g_flag&GLWAIT, 'Z');
		printf("%4d", gp->g_count&0377);
		printf("%4d,%3d", major(gp->g_dev), minor(gp->g_dev));
		printf("%4d", gp->g_shlockc&0377);
		printf("%4d", gp->g_exlockc&0377);
		printf("%6d", gp->g_number);
		printf("%6x", gp->g_mode & 0xffff);
		printf("%4d", gp->g_nlink);
		printf("%4d", gp->g_uid);
		if ((gp->g_mode&GFMT)==GFBLK || (gp->g_mode&GFMT)==GFCHR)
			printf("%6d,%3d", major(gp->g_rdev), minor(gp->g_rdev));
		else
			printf("%10ld", gp->g_size);
		printf("\n");
	}
	free(xgnode);
}

getw(loc)
	off_t loc;
{
	int word;

	klseek(fc, loc, 0);
	read(fc, &word, sizeof (word));
	return (word);
}

putf(v, n)
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

dotext()
{
	register struct text *xp;
	int ntext;
	struct text *xtext, *atext;
	int nrecl=0,nactive=0;

	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	klseek(fc, (int)(atext = (struct text *)getw(nl[STEXT].n_value)), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_gptr!=NULL && (xp->x_flag & XFREE))
			nrecl++;
		if ((xp->x_flag & XFREE) == 0)
			nactive++;
	}
	printf("%4d/%4d/%4d\tactive/reclaimable/total texts\n",
					nactive, nrecl, ntext);
	if (totflg) return;
	printf("   LOC   FLAGS DADDR    CADDR  RSS SIZE   GPTR   CNT CCNT LCNT SLP POIP CMAP\n");
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_gptr == NULL)
			continue;
		printf("%8.1x", atext + (xp - xtext));
		printf(" ");
		putf(xp->x_flag&XPAGI, 'P');
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		if (xp->x_flag & XFREE)
			putf(xp->x_flag&XFREE, 'F');
		else
			putf(xp->x_flag&XWANT, 'w');
		printf("%5x", xp->x_daddr[0]);
		printf("%9.1x", xp->x_caddr);
		printf("%5d", xp->x_rssize);
		printf("%5d", xp->x_size);
		printf("%9.1x", xp->x_gptr);
		printf("%4d", xp->x_count&0377);
		printf("%4d", xp->x_ccount);
		printf("%5d", xp->x_lcount);
		printf("%4d", xp->x_slptime);
		printf("%5d", xp->x_poip);
		printf("%10.1x", xp->x_cmap);
		printf("\n");
	}
	free(xtext);
}

doproc()
{
	struct proc *xproc, *aproc;
	int nproc;
	register struct proc *pp;
	register loc, np;
	struct pte apte;

	nproc = getw(nl[SNPROC].n_value);
	xproc = (struct proc *)calloc(nproc, sizeof (struct proc));
	klseek(fc, (int)(aproc = (struct proc *)getw(nl[SPROC].n_value)), 0);
	read(fc, xproc, nproc * sizeof (struct proc));
	np = 0;
	for (pp=xproc; pp < &xproc[nproc]; pp++)
		if (pp->p_stat)
			np++;
	if (totflg) {
		printf("%4d/%4d\tprocesses\n", np, nproc);
		return;
	}
	printf("%d/%d processes\n", np, nproc);
	printf("   LOC    S    F POIP PRI      SIG  UID SLP TIM  CPU  NI   PGRP    PID   PPID    ADDR   RSS SRSS SIZE    WCHAN    LINK   TEXTP CLKT\n");
	for (pp=xproc; pp<&xproc[nproc]; pp++) {
		if (pp->p_stat==0 && allflg==0)
			continue;
		printf("%8x", aproc + (pp - xproc));
		printf(" %2d", pp->p_stat);
		printf(" %4x", pp->p_flag & 0xffff);
		printf(" %4d", pp->p_poip);
		printf(" %3d", pp->p_pri);
		printf(" %8x", pp->p_sig);
		printf(" %4d", pp->p_uid);
		printf(" %3d", pp->p_slptime);
		printf(" %3d", pp->p_time);
		printf(" %4d", pp->p_cpu&0377);
		printf(" %3d", pp->p_nice);
		printf(" %6d", pp->p_pgrp);
		printf(" %6d", pp->p_pid);
		printf(" %6d", pp->p_ppid);
		klseek(fc, (long)(Usrptma+btokmx(pp->p_addr)), 0);
		read(fc, &apte, sizeof(apte));
		printf(" %8x", apte.pg_pfnum);	/* print page of page table */
		printf(" %4x", pp->p_rssize);
		printf(" %4x", pp->p_swrss);
		printf(" %5x", pp->p_dsize+pp->p_ssize);
		printf(" %7x", clear(pp->p_wchan));
		printf(" %7x", clear(pp->p_link));
		printf(" %7x", clear(pp->p_textp));
		printf("\n");
	}
}

dotty()
{
	struct tty dz_tty[128];
	int ndz;
	register struct tty *tp;
	register char *mesg;
	register struct ttytype *ttype;

	mesg = " # RAW CAN OUT    MODE    ADDR   DEL COL  STATE         PGRP DISC\n";
	printf(mesg);
	klseek(fc, (long)nl[SKL].n_value, 0);
	read(fc, dz_tty, sizeof(dz_tty[0]));
	/*
	 * The console will be on ss line 0 or 3 if the
	 * CPU is a VAXstar or MICROVAX 1800. Don't print
	 * the console line if this is the case.
	 */
	tp = dz_tty;
	if((tp->t_state&TS_ISOPEN) || ((long)nl[SSS].n_type == 0)) {
		printf("\n1 console\n"); /* console */
		ttyprt(&dz_tty[0], 0);
	}
	for (ttype=ttytypes; *ttype->name != '\0'; ttype++) {
		if (nl[ttype->nttys].n_type == 0)
			continue;
		klseek(fc, (long)nl[ttype->nttys].n_value, 0);
		read(fc, &ndz, sizeof(ndz));
		printf("\n%d %s lines\n", ndz, ttype->name);
		klseek(fc, (long)nl[ttype->ttybase].n_value, 0);
		read(fc, dz_tty, ndz * sizeof (struct tty));
		for (tp = dz_tty; tp < &dz_tty[ndz]; tp++)
			ttyprt(tp, tp - dz_tty);
	}
}

ttyprt(atp, line)
struct tty *atp;
{
	register struct tty *tp;

	printf("%2d", line);
	tp = atp;
	switch (tp->t_line) {

/*
	case NETLDISC:
		if (tp->t_rec)
			printf("%4d%4d", 0, tp->t_inbuf);
		else
			printf("%4d%4d", tp->t_inbuf, 0);
		break;
*/

	default:
		printf("%4d", tp->t_rawq.c_cc);
		printf("%4d", tp->t_canq.c_cc);
	}
	printf("%4d ", tp->t_outq.c_cc);
	printf("%8.1x", tp->t_flags);
	printf(" %8.1x", tp->t_addr);
	printf("%3d", tp->t_delct);
	printf("%4d ", tp->t_col);
	putf(tp->t_state&TS_TIMEOUT, 'T');
	putf(tp->t_state&TS_WOPEN, 'W');
	putf(tp->t_state&TS_ISOPEN, 'O');
	putf(tp->t_state&TS_CARR_ON, 'C');
	putf(tp->t_state&TS_BUSY, 'B');
	putf(tp->t_state&TS_ASLEEP, 'A');
	putf(tp->t_state&TS_XCLUDE, 'X');
	putf(tp->t_state&TS_HUPCLS, 'H');
	putf(tp->t_state&TS_TTSTOP, 'S');
	putf(tp->t_state&TS_INUSE, 'I');
	putf(tp->t_state&TS_ONDELAY, 'D');
	putf(tp->t_state&TS_IGNCAR, 'G');
	putf(tp->t_state&TS_NBIO, 'N');
	putf(tp->t_state&TS_ASYNC, 'Z');
	printf("%6d", tp->t_pgrp);
	switch (tp->t_line) {

	case NTTYDISC:
		printf(" ntty");
		break;

	case NETLDISC:
		printf(" net");
		break;

	case HCLDISC:
		printf(" uucp");
		break;
	}
	printf("\n");
}

dousr()
{
	union {
		struct user user;
		char upages[UPAGES+CLSIZE][NBPG];
	} un_user;
#define U un_user.user
	register i, j, *ip, ncl;
	register struct nameidata *nd = &U.u_nd;
	struct pte arguutl[UPAGES+CLSIZE];
	struct ucred ucred;

	/* ubase is the 1st page of the user page table */
	/* We will use this to seek to then back up UPAGES+CLSIZE pte's */
	/* and read in the pte's to get all the pages of the u area */
	klseek(fm,(long)ctob(ubase)-(UPAGES+CLSIZE)*sizeof(struct pte),0);
	if (read(fm, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		printf("pstat: cant read page table for u at addr %x\n",ubase);
		return;
	}
	/* Now figure out the number of clicks to read */
	ncl = (sizeof(struct user) + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
	/* Read them in reverse order into the union which serves */
	/* to let us read an even number of clicks even though the user */
	/* structure may be smaller than that. */
	while (--ncl >= 0) {
		i = ncl * CLSIZE;
		klseek(fm, (long)ctob(arguutl[CLSIZE+i].pg_pfnum), 0);
		if (read(fm, un_user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
			printf("pstat: cant read page %d of u from %s\n",
			    arguutl[CLSIZE+i].pg_pfnum, fmem);
			return;
		}
	}
	printf("pcb (%d bytes)\n",sizeof(struct pcb));
	ip = (int *)&U.u_pcb;
	while (ip < (int *)((int)&U.u_pcb + sizeof(struct pcb))) {
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\t");
		printf("%9.1x ", *ip++);
		if ((ip - (int *)&U.u_pcb) % 4 == 0)
			printf("\n");
	}
	if ((ip - (int *)&U.u_pcb) % 4 != 0)
		printf("\n");
	printf("procp\t%9.1x\n", U.u_procp);
	printf("ar0\t%9.1x\n", U.u_ar0);
	printf("comm\t %s\n", U.u_comm);
	printf("arg");
	for (i=0; i<sizeof(U.u_arg)/sizeof(U.u_arg[0]); i++) {
		if (i%5==0) printf("\t");
		printf("%9.1x", U.u_arg[i]);
		if (i%5==4) printf("\n");
	}
	printf("\n");
	printf("ap\t%9.1x\n", U.u_ap);
	printf("qsave");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0) printf("\t");
		printf("%9.1x", U.u_qsave.val[i]);
		if (i%5==4) printf("\n");
	}
	printf("\n");
	printf("error\t%9.1x\n", U.u_error);
	printf("r_val?\t%9.1x %9.1x\n", U.u_r.r_val1, U.u_r.r_val2);
	printf("eosys\t%9.1x\n", U.u_eosys);
	printf("u_cred\t%x\n",U.u_cred);
	/* credentials is now km_alloc-ed -- have to go get them!!! - rr */
	klseek(fc,U.u_cred,0);
	read(fc,(char *)&ucred,sizeof(struct ucred));
	printf("\tref %d\n",ucred.cr_ref);
	printf("\tuid\t%d\tgid\t%d\n",ucred.cr_uid,ucred.cr_gid);
	printf("\truid\t%d\trgid\t%d\n",ucred.cr_ruid,ucred.cr_rgid);
	printf("\tgroups\t");
	for (i=0;i<NGROUPS;i++) {
		printf("%d",ucred.cr_groups[i]);
		if ((i==(NGROUPS/2 - 1))) printf("\n\t\t");
		else if (i<NGROUPS-1) putchar(',');
	}
	printf("\n");

	printf("sizes\t%d %d %d (clicks)\n", U.u_tsize, U.u_dsize, U.u_ssize);
	printf("dmap (%d bytes)\n\t%x\t%x\t%x\t%x\n",sizeof(struct dmap),
		U.u_dmap,U.u_smap,U.u_cdmap,U.u_csmap);
	printf("ssave");
	for (i=0; i<sizeof(label_t)/sizeof(int); i++) {
		if (i%5==0)
			printf("\t");
		printf("%9.1x", U.u_ssave.val[i]);
		if (i%5==4)
			printf("\n");
	}
	printf("\n");
	printf("u_odsize\t%d\n",U.u_odsize);
	printf("u_ossize\t%d\n",U.u_odsize);
	printf("u_outime\t%d\n",U.u_outime);
	printf("sigs");
	for (i=0; i<NSIG; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%5.1x ", U.u_signal[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (NSIG % 8 != 0) printf("\n");
	printf("sigmask");
	for (i=0; i<NSIG; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%5.1x ", U.u_sigmask[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (NSIG % 8 != 0) printf("\n");
	printf("sigonstack\t%9.1x\n",U.u_sigonstack);
	printf("oldmask   \t%9.1x\n",U.u_oldmask);
	printf("code      \t%9.1x\n", U.u_code);
	printf("sigstack  \t%9.1x\t%9.1x\n",
		U.u_sigstack.ss_sp,U.u_sigstack.ss_onstack);
	printf("file");
	for (i=0; i<NOFILE; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%9.1x", U.u_ofile[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (NOFILE % 8 != 0) printf("\n");
	printf("pofile");
	for (i=0; i<NOFILE; i++) {
		if (i % 8 == 0)
			printf("\t");
		printf("%9.1x", U.u_pofile[i]);
		if (i % 8 == 7)
			printf("\n");
	}
	if (NOFILE % 8 != 0) printf("\n");
	printf("maxfile  \t%d\n", U.u_omax);
	printf("cdir rdir\t%9.1x %9.1x\n", U.u_cdir, U.u_rdir);
	printf("ttyp     \t%9.1x\n", U.u_ttyp);
	printf("ttyd     \t%d,%d\n", major(U.u_ttyd), minor(U.u_ttyd));
	printf("cmask    \t0%o\n", U.u_cmask);
	printf("ru\t");
	ip = (int *)&U.u_ru;
	for (i = 0; i < sizeof(U.u_ru)/sizeof(int); i++) {
		if ( i % 10 == 0 && i ) printf("\n\t");
		printf("%d ", ip[i]);
	}
	if (sizeof(U.u_ru)/sizeof(int) % 10 != 0) printf("\n");
	ip = (int *)&U.u_cru;
	printf("cru\t");
	for (i = 0; i < sizeof(U.u_cru)/sizeof(int); i++) {
		if ( i % 10 == 0 && i ) printf("\n\t");
		printf("%d ", ip[i]);
	}
	if (sizeof(U.u_cru)/sizeof(int) % 10 != 0) printf("\n");
	printf("timers");
	for(i=0;i<sizeof(U.u_timer)/sizeof(struct itimerval);i++) {
		printf("\t%12d %12d %12d %12d\n",
			U.u_timer[i].it_interval.tv_sec,
			U.u_timer[i].it_interval.tv_usec,
			U.u_timer[i].it_value.tv_sec,
			U.u_timer[i].it_value.tv_usec);
	}
/*
 * Nothing now but will handle larger timer structure in the future!
 *	printf("u_XXX[3]\t%x %x %x\n",U.u_XXX[0],U.u_XXX[1],U.u_XXX[2]);
 */
	printf("start    \t%d\n", U.u_start);
	printf("acflag   \t%d\n", U.u_acflag);
	printf("limits   \t");
	for(i=0;i<RLIM_NLIMITS;i++) {
		printf("%d ",U.u_rlimit[i]);
	}
	printf("\n");
	printf("quota    \t%9.1x\n",U.u_quota);
	printf("quotaflag\t%9.1x\n",U.u_qflags);

#ifdef notdef	/* this data element no longer exists -- depp */
	printf("u_exdata \t%9.1x\n",U.u_exdata);
#endif notdef
	printf("smem     \t%9.1x %9.1x %9.1x\n",
		U.u_smsize,U.u_osmsize,U.u_lock);
	printf("prof     \t%9.1x %9.1x %9.1x %9.1x\n",
		U.u_prof.pr_base, U.u_prof.pr_size,
		U.u_prof.pr_off, U.u_prof.pr_scale);
	printf("u_nache  \toff %d ino %d dev %d tim %d\n",
		U.u_ncache.nc_prevoffset,U.u_ncache.nc_inumber,
		U.u_ncache.nc_dev,U.u_ncache.nc_time);
	printf("nameidata\n");
	printf("\tnameiop, error, endoff\t%8x %8d %8d\n",
		nd->ni_nameiop,nd->ni_error,nd->ni_endoff);
	printf("\t   base, count, offset\t%8x %8d %8d\n",
		nd->ni_base,nd->ni_count,nd->ni_offset);
	printf("\tdent ino %d name %.14s\n",
		nd->ni_dent.d_ino,nd->ni_dent.d_name);
	printf("\tsegflg\t%8d\n", nd->ni_segflg);
	printf("u_stack  \t%9.1x\n",U.u_stack[0]);
/*
	i =  U.u_stack - &U;
	while (U[++i] == 0);
	i &= ~07;
	while (i < 512) {
		printf("%x ", 0140000+2*i);
		for (j=0; j<8; j++)
			printf("%9x", U[i++]);
		printf("\n");
	}
*/
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<3) + *s++ - '0';
	return(v);
}

dofile()
{
	int nfile;
	struct file *xfile, *afile;
	register struct file *fp;
	register nf;
	int loc;
	static char *dtypes[] = { "???", "gnode", "socket" };

	nf = 0;
	nfile = getw(nl[SNFILE].n_value);
	xfile = (struct file *)calloc(nfile, sizeof (struct file));
	klseek(fc, (int)(afile = (struct file *)getw(nl[SFIL].n_value)), 0);
	read(fc, xfile, nfile * sizeof (struct file));
	for (fp=xfile; fp < &xfile[nfile]; fp++)
		if (fp->f_count)
			nf++;
	if (totflg) {
		printf("%4d/%4d\tfiles\n", nf, nfile);
		return;
	}
	printf("%d/%d open files\n", nf, nfile);
	printf("   LOC   TYPE    FLG      CNT  MSG    DATA    OFFSET\n");
	for (fp=xfile,loc=(int)afile; fp < &xfile[nfile]; fp++,loc+=sizeof(xfile[0])) {
		if (fp->f_count==0)
			continue;
		printf("%8x ", loc);
		if (fp->f_type <= DTYPE_SOCKET)
			printf("%-8.8s", dtypes[fp->f_type]);
		else
			printf("8d", fp->f_type);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		putf(fp->f_flag&FAPPEND, 'A');
		putf(fp->f_flag&FSHLOCK, 'S');
		putf(fp->f_flag&FEXLOCK, 'X');
		putf(fp->f_flag&FASYNC, 'I');
		putf(fp->f_flag&FBLKINUSE, 'B');
		printf("  %3d", mask(fp->f_count));
		printf("  %3d", mask(fp->f_msgcount));
		printf("  %8.1x", fp->f_data);
		if (fp->f_offset < 0)
			printf("  %x\n", fp->f_offset);
		else
			printf("  %ld\n", fp->f_offset);
	}
}

int dmmin, dmmax, nswdev;

doswap()
{
	struct proc *proc;
	int nproc;
	struct text *xtext;
	int ntext;
	struct map *swapmap;
	int nswapmap;
	struct swdevt *swdevt, *sw;
	register struct proc *pp;
	int nswap, used, tused, free, waste;
	int db, sb;
	register struct mapent *me;
	register struct text *xp;
	int i, j;

	nproc = getw(nl[SNPROC].n_value);
	proc = (struct proc *)calloc(nproc, sizeof (struct proc));
	ntext = getw(nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	nswapmap = getw(nl[SNSWAPMAP].n_value);
	swapmap = (struct map *)calloc(nswapmap, sizeof (struct map));
	nswdev = getw(nl[SNSWDEV].n_value);
	swdevt = (struct swdevt *)calloc(nswdev, sizeof (struct swdevt));
	klseek(fc, nl[SSWDEVT].n_value, L_SET);
	read(fc, swdevt, nswdev * sizeof (struct swdevt));
	klseek(fc, getw(nl[SPROC].n_value), 0);
	read(fc, proc, nproc * sizeof (struct proc));
	klseek(fc, getw(nl[STEXT].n_value), 0);
	read(fc, xtext, ntext * sizeof (struct text));
	klseek(fc, getw(nl[SWAPMAP].n_value), 0);
	read(fc, swapmap, nswapmap * sizeof (struct map));
	swapmap->m_name = "swap";
	swapmap->m_limit = (struct mapent *)&swapmap[nswapmap];
	dmmin = getw(nl[SDMMIN].n_value);
	dmmax = getw(nl[SDMMAX].n_value);
	nswap = 0;
	for (sw = swdevt; sw < &swdevt[nswdev]; sw++)
		nswap += sw->sw_nblks,
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	tused = 0;
	for (xp = xtext; xp < &xtext[ntext]; xp++)
		if ((xp->x_flag & XFREE) == 0) {
			tused += ctod(xp->x_size);
			if (xp->x_flag & XPAGI)
				tused += ctod(ctopt(xp->x_size));
		}
	used = tused;
	waste = 0;
	for (pp = proc; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == 0 || pp->p_stat == SZOMB)
			continue;
		if (pp->p_flag & SSYS)
			continue;
		db = ctod(pp->p_dsize), sb = up(db);
		used += sb;
		waste += sb - db;
		db = ctod(pp->p_ssize), sb = up(db);
		used += sb;
		waste += sb - db;
		if ((pp->p_flag&SLOAD) == 0)
			used += vusize(pp);
	}
	if (totflg) {
#define	btok(x)	((x) / (1024 / DEV_BSIZE))
		printf("%4d/%4d\t00k swap\n",
		    btok(used/100), btok((used+free)/100));
		return;
	}
	printf("%dk used (%dk text), %dk free, %dk wasted, %dk missing\n",
	    btok(used), btok(tused), btok(free), btok(waste),
/* a dmmax/2 block goes to argmap */
	    btok(nswap - dmmax/2 - (used + free)));
	printf("avail: ");
	for (i = dmmax; i >= dmmin; i /= 2) {
		j = 0;
		while (rmalloc(swapmap, i) != 0)
			j++;
		if (j) printf("%d*%dk ", j, btok(i));
	}
	free = 0;
	for (me = (struct mapent *)(swapmap+1);
	    me < (struct mapent *)&swapmap[nswapmap]; me++)
		free += me->m_size;
	printf("%d*1k\n", btok(free));
}

up(size)
	register int size;
{
	register int i, block;

	i = 0;
	block = dmmin;
	while (i < size) {
		i += block;
		if (block < dmmax)
			block *= 2;
	}
	return (i);
}

/*
 * Compute number of pages to be allocated to the u. area
 * and data and stack area page tables, which are stored on the
 * disk immediately after the u. area.
 */
vusize(p)
	register struct proc *p;
{
	register int tsz = p->p_tsize / NPTEPG;

	/*
	 * We do not need page table space on the disk for page
	 * table pages wholly containing text. 
	 */
	return (clrnd(UPAGES +
	    clrnd(ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES)) - tsz));
}

/*
 * Allocate 'size' units from the given
 * map. Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 *
 * Algorithm is first-fit.
 *
 * This routine knows about the interleaving of the swapmap
 * and handles that.
 */
long
rmalloc(mp, size)
	register struct map *mp;
	long size;
{
	register struct mapent *ep = (struct mapent *)(mp+1);
	register int addr;
	register struct mapent *bp;
	swblk_t first, rest;

	if (size <= 0 || size > dmmax)
		return (0);
	/*
	 * Search for a piece of the resource map which has enough
	 * free space to accomodate the request.
	 */
	for (bp = ep; bp->m_size; bp++) {
		if (bp->m_size >= size) {
			/*
			 * If allocating from swapmap,
			 * then have to respect interleaving
			 * boundaries.
			 */
			if (nswdev > 1 &&
			    (first = dmmax - bp->m_addr%dmmax) < bp->m_size) {
				if (bp->m_size - first < size)
					continue;
				addr = bp->m_addr + first;
				rest = bp->m_size - first - size;
				bp->m_size = first;
				if (rest)
					rmfree(mp, rest, addr+size);
				return (addr);
			}
			/*
			 * Allocate from the map.
			 * If there is no space left of the piece
			 * we allocated from, move the rest of
			 * the pieces to the left.
			 */
			addr = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
			}
			if (addr % CLSIZE)
				return (0);
			return (addr);
		}
	}
	return (0);
}

/*
 * Free the previously allocated space at addr
 * of size units into the specified map.
 * Sort addr into map and combine on
 * one or both ends if possible.
 */
rmfree(mp, size, addr)
	struct map *mp;
	long size, addr;
{
	struct mapent *firstbp;
	register struct mapent *bp;
	register int t;

	/*
	 * Both address and size must be
	 * positive, or the protocol has broken down.
	 */
	if (addr <= 0 || size <= 0)
		goto badrmfree;
	/*
	 * Locate the piece of the map which starts after the
	 * returned space (or the end of the map).
	 */
	firstbp = bp = (struct mapent *)(mp + 1);
	for (; bp->m_addr <= addr && bp->m_size != 0; bp++)
		continue;
	/*
	 * If the piece on the left abuts us,
	 * then we should combine with it.
	 */
	if (bp > firstbp && (bp-1)->m_addr+(bp-1)->m_size >= addr) {
		/*
		 * Check no overlap (internal error).
		 */
		if ((bp-1)->m_addr+(bp-1)->m_size > addr)
			goto badrmfree;
		/*
		 * Add into piece on the left by increasing its size.
		 */
		(bp-1)->m_size += size;
		/*
		 * If the combined piece abuts the piece on
		 * the right now, compress it in also,
		 * by shifting the remaining pieces of the map over.
		 */
		if (bp->m_addr && addr+size >= bp->m_addr) {
			if (addr+size > bp->m_addr)
				goto badrmfree;
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
		}
		goto done;
	}
	/*
	 * Don't abut on the left, check for abutting on
	 * the right.
	 */
	if (addr+size >= bp->m_addr && bp->m_size) {
		if (addr+size > bp->m_addr)
			goto badrmfree;
		bp->m_addr -= size;
		bp->m_size += size;
		goto done;
	}
	/*
	 * Don't abut at all.  Make a new entry
	 * and check for map overflow.
	 */
	do {
		t = bp->m_addr;
		bp->m_addr = addr;
		addr = t;
		t = bp->m_size;
		bp->m_size = size;
		bp++;
	} while (size = t);
	/*
	 * Segment at bp is to be the delimiter;
	 * If there is not room for it 
	 * then the table is too full
	 * and we must discard something.
	 */
	if (bp+1 > mp->m_limit) {
		/*
		 * Back bp up to last available segment.
		 * which contains a segment already and must
		 * be made into the delimiter.
		 * Discard second to last entry,
		 * since it is presumably smaller than the last
		 * and move the last entry back one.
		 */
		bp--;
		printf("%s: rmap ovflo, lost [%d,%d)\n", mp->m_name,
		    (bp-1)->m_addr, (bp-1)->m_addr+(bp-1)->m_size);
		bp[-1] = bp[0];
		bp[0].m_size = bp[0].m_addr = 0;
	}
done:
	return;
badrmfree:
	printf("bad rmfree\n");
}

/*---
 *
 * klseek taken from source for ps with slight modifications.
 *
 */

klseek(fd, loc, off)
	int fd;
	long loc;
	int off;
{

	if( kflg && Sysmap == 0)
		{/* initialize Sysmap */

		sizeSysmap = nl[SSYSSIZE].n_value * sizeof( struct pte);
		Sysmap = (struct pte *)calloc( sizeSysmap, 1);
		lseek( fc, clear( nl[SSYSMAP].n_value), 0);
		if( read( fc, Sysmap, sizeSysmap) != sizeSysmap)
			{
			printf( "Can't read system page table from %s\n",
				fcore);
			exit(1);
			}
		}
	if( kflg && (loc&0x80000000))
		{/* do mapping for kernel virtual addresses */
		struct pte *ptep;

		loc &= 0x7fffffff;
		ptep = &Sysmap[btop(loc)];
		if( (char *)ptep - (char *)Sysmap > sizeSysmap)
			{
			printf( "no system pte for %s\n", loc);
			exit(1);
			}
		if( ptep->pg_v == 0)
			{
			printf( "system pte invalid for %x\n", loc);
			exit(1);
			}
		loc = (off_t)((loc&PGOFSET) + ptob(ptep->pg_pfnum));
		}
	(void) lseek(fd, (long)loc, off);
}

