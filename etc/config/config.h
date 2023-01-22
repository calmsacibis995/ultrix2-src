
/* 	@(#)config.h	1.12	(ULTRIX)	6/5/86 	*/

/************************************************************************
 *									*
 *			Copyright (c) 1983,86 by			*
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
 *		Modification History
 * 15-Apr-86 -- afd
 *	Took out define for MACHINE_MVAX (mvax is now considered 'vax').
 *	Declare "emulation_instr" for vaxen with subset instruction set.
 *
 * 02 Apr 86 -- depp
 *	Added shared memory configurable items 
 *
 * 05-Mar-86 -- jrs
 *	Added support for configuring bi devices.
 *
 * 25-Feb-86 -- jrs
 *	Changed to allow multiple "needs" files per files.* line
 *
 *  4 June 85 -- depp
 *	Added the variables version and release
 *
 ***********************************************************************/
/*
 * Config.
 */
/* @(#)config.h	1.5		8/14/84 */

#include <sys/types.h>

#define	NODEV	((dev_t)-1)
#define	NNEEDS	10

struct file_list {
	struct	file_list *f_next;	
	char	*f_fn;			/* the name */
	short	f_type;			/* see below */
	short	f_flags;		/* see below */
	short	f_special;		/* requires special make rule */
	char	*f_needs[NNEEDS];
	/*
	 * Random values:
	 *	swap space parameters for swap areas
	 *	root device, etc. for system specifications
	 */
	union {
		struct {		/* when swap specification */
			dev_t	fuw_swapdev;
			int	fuw_swapsize;
		} fuw;
		struct {		/* when system specification */
			dev_t	fus_rootdev;
			dev_t	fus_argdev;
			dev_t	fus_dumpdev;
		} fus;
	} fun;
#define	f_swapdev	fun.fuw.fuw_swapdev
#define	f_swapsize	fun.fuw.fuw_swapsize
#define	f_rootdev	fun.fus.fus_rootdev
#define	f_argdev	fun.fus.fus_argdev
#define	f_dumpdev	fun.fus.fus_dumpdev
};

/*
 * Types.
 */
#define DRIVER		1
#define NORMAL		2
#define	PROFILING	3
#define	SYSTEMSPEC	4
#define	SWAPSPEC	5

/*
 * Attributes (flags).
 */
#define	CONFIGDEP	1
#define	INVISIBLE	2
#define	NOTBINARY	8
#define	UNSUPPORTED	0x10
#define	OBJS_ONLY	0x20

struct	idlst {
	char	*id;
	struct	idlst *id_next;
};

struct device {
	int	d_type;			/* CONTROLLER, DEVICE, UBA or MBA */
	struct	device *d_conn;		/* what it is connected to */
	char	*d_name;		/* name of device (e.g. rk11) */
	struct	idlst *d_vec;		/* interrupt vectors */
	int	d_pri;			/* interrupt priority */
	int	d_addr;			/* address of csr */
	int	d_unit;			/* unit number */
	int	d_drive;		/* drive number */
	int	d_slave;		/* slave number */
#define QUES	-1	/* -1 means '?' */
#define	UNKNOWN -2	/* -2 means not set yet */
	int	d_dk;			/* if init 1 set to number for iostat */
	int	d_flags;		/* nlags for device init */
	int	d_adaptor;		/* which bus adaptor we are on */
	int	d_nexus;		/* which nexus on this adaptor */
	int	d_extranum;		/* which hidden uba we are on */
	struct	device *d_next;		/* Next one in list */
};
#define TO_NEXUS	(struct device *)-1

struct config {
	char	*c_dev;
	char	*s_sysname;
};

/*
 * Config has a global notion of which machine type is
 * being used.  It uses the name of the machine in choosing
 * files and directories.  Thus if the name of the machine is ``vax'',
 * it will build from ``makefile.vax'' and use ``../vax/asm.sed''
 * in the makerules, etc.
 */
int	machine;
char	*machinename;
#define	MACHINE_VAX	1
#define	MACHINE_SUN	2

/*
 * For each machine, a set of CPU's may be specified as supported.
 * These and the options (below) are put in the C flags in the makefile.
 */
struct cputype {
	char	*cpu_name;
	struct	cputype *cpu_next;
} *cputype;

/*
 * A set of options may also be specified which are like CPU types,
 * but which may also specify values for the options.
 */
struct opt {
	char	*op_name;
	char	*op_value;
	struct	opt *op_next;
} *opt;

char	*ident;
char	*ns();
char	*tc();
char	*qu();
char	*get_word();
char	*path();
char	*raise();

int	do_trace;

char	*index();
char	*rindex();
char	*malloc();
char	*strcpy();
char	*strcat();
char	*sprintf();

#if MACHINE_VAX
int	seen_mba, seen_uba;
#endif

struct	device *connect();
struct	device *dtab;
dev_t	nametodev();
char	*devtoname();

char	errbuf[80];
int	yyline;

struct	file_list *ftab, *conf_list, **confp;
char	*PREFIX;

int	timezone, hadtz;
int	dst;
int	profiling;

int	maxusers;
int	physmem; /* estimate of megabytes of physical memory */
int	maxuprc; /* max number of processes per user */
int	processors; /* max number of processors in the system */
int	dmmin;	 /* min chunk for virtual memory allocation */
int	dmmax;	 /* max chunk for virtual memory allocation */
double	release; /* Ultrix release number */
int	version; /* Version of that release */
int	highuba; /* highest uba number seen during config file parsing */
int	extrauba; /* number of extra unibuses we need to define */
int	smmin;	 /* minumum shared memory segment size */
int	smmax;	 /* maximum shared memory segment size */
int	smbrk;	 /* number of VAX pages between end of data segment and
		    beginning of first shared memory segment */
int	emulation_instr; /* true if cpu MVAX defined in config file */

#define eq(a,b)	(!strcmp(a,b))

extern	int	source;
extern	char	*tbl_pseudo_uba[];
extern	char	*tbl_must_nexus[];
extern	char	*tbl_is_uq[];
