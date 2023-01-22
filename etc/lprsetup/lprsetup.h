/*	@(#)lprsetup.h	@(#)lprsetup.h	1.1	7/8/86	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 **********************************************************************/


/**************************************
* lprsetup.h
**************************************/
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <time.h>

/*********
* debug
*********/
#define dprintf if (debug) fprintf
extern int debug;

/**************************
* file name defines
**************************/

#ifdef LOCAL
/* SEE ALSO printlib.c for PRINTCAP defined again */ 

#define PRINTCAP "printcap"	/* printcap pathname		*/
#define COPYCAP	"copycap"	/* copy file printcap pathname	*/
#define LOGCAP	"printcap.log"	/* copy file printcap pathname	*/

#else NO LOCAL

#define PRINTCAP "/etc/printcap"	/* printcap pathname		*/
#define COPYCAP	"/etc/pcap.tmp"		/* copy file printcap pathname	*/
#define LOGCAP	"/usr/adm/printcap.log"	/* copy file printcap pathname	*/

#endif LOCAL

#define	DAEMON	"daemon"	/* daemon passwd name	 	*/
#define UNKNOWN "unknown"	/* unknown printer name	 	*/
#define LPF	"/usr/lib/lpf"	/* default output filter	*/
#define	EDTTY	"/bin/ed - /etc/ttys 1>/dev/null 2>/dev/null"	/* edit /etc/ttys file  */
#define	MKDEV	"/dev/MAKEDEV"	/* device MAKEDEV script */

#define	SUPERUSER	0	/* root pid			*/
#define	LEN		256	/* entry string length		*/
#define DIR		0755	/* directory mode		*/

/************************
* getcmd returns
************************/
#define	NO		0
#define	YES		1
#define	NOREPLY		2
#define PRINT		3
#define	HELP		4
#define	GOT_SYMBOL	5	/* add a symbol */
#define	QUIT		6
#define	CTRLD		7
#define USED		8	/* only print used symbols */
#define ALL		9	/* print all symbols */
#define	BOOL		10	/* boolean symbol */
#define	INT		11	/* integer symbol */
#define	STR		12	/* string symbol */
#define OFF		13	/* boolean off */
#define ON		14	/* boolean on */
#define ADD		15	/* add entry */
#define MODIFY		16	/* modify entry */
#define DELETE		17	/* delete entry */
#define LIST		18	/* list all possible symbols */


#define BUF_LINE	250	/* line length buffer */
#define BUF_WORD	40	/* word length buffer */
#define TRUE		1	/* return(...) codes */
#define FALSE		0
#define BAD		-1	/* used in misc: validate() routine */
#define	ERROR		1
#define	OK		0
#define NOT		!

int	modifying;		/* TRUE when modifying in modify() routine */

struct table
{
    char   *name;		/* symbol name goes here */
    char   *svalue;		/* default value of symbol */
    int     stype;		/* type of symbol: BOOL, INT, or STR */
    int     used;		/* True if using symbol for this printcap */
    char   *nvalue;		/* new value of symbol */
    int     nun;		/* true if new value is "none" = delete */
};

struct nameval
{
    char   *name;		/* symbol name */
    char   *svalue;		/* value of symbol */
};

struct cmdtyp
{
    char   *cmd_name;
    int     cmd_id;
};

/*
 * Do not add help codes here
 * without first updating "globals.h".
 */
#define H_af	0
#define H_br	1
#define H_cf	2
#define H_df	3
#define H_dn	4
#define H_du	5
#define H_fc	6
#define H_ff	7
#define H_fo	8
#define H_fs	9
#define H_gf   	10
#define H_ic   	11
#define H_if   	12
#define H_lf   	13
#define H_lo   	14
#define H_lp	15
#define H_mc	16
#define H_mx	17
#define H_nf	18
#define H_of	19
#define H_op	20
#define H_os	21
#define H_pl	22
#define H_pp	23
#define H_pw	24
#define H_px	25
#define H_py	26
#define H_rf	27
#define H_rm	28
#define H_rp	29
#define H_rs	30
#define H_rw	31
#define H_sb	32
#define H_sc	33
#define H_sd	34
#define H_sf	35
#define H_sh	36
#define H_st	37
#define H_tf	38
#define H_tr	39
#define H_ts	40
#define H_vf	41
#define H_xc	42
#define H_xf	43
#define H_xs	44
/* end of help codes */

/* general help messages */
extern char h_help[];
extern char h_helps[];
extern char h_doadd[];
extern char h_dodel[];
extern char h_domod[];
extern char h_synonym[];
extern char h_default[];

/* more specific help messges */
extern char h_af[];
extern char h_br[];
extern char h_cf[];
extern char h_df[];
extern char h_dn[];
extern char h_du[];
extern char h_fc[];
extern char h_ff[];
extern char h_fo[];
extern char h_fs[];
extern char h_gf[];
extern char h_ic[];
extern char h_if[];
extern char h_lf[];
extern char h_lo[];
extern char h_lp[];
extern char h_mc[];
extern char h_mx[];
extern char h_nc[];
extern char h_nf[];
extern char h_of[];
extern char h_op[];
extern char h_os[];
extern char h_pl[];
extern char h_pp[];
extern char h_pw[];
extern char h_px[];
extern char h_py[];
extern char h_rf[];
extern char h_rm[];
extern char h_rp[];
extern char h_rs[];
extern char h_rw[];
extern char h_sb[];
extern char h_sc[];
extern char h_sd[];
extern char h_sf[];
extern char h_sh[];
extern char h_st[];
extern char h_tf[];
extern char h_tr[];
extern char h_ts[];
extern char h_vf[];
extern char h_xc[];
extern char h_xf[];
extern char h_xs[];

/**************************************
* end of lprsetup.h
**************************************/
