#ifndef lint
static char *sccsid = "@(#)conf.c	1.46	ULTRIX	2/12/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1987 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/************************************************************************
 *
 * Modification History:
 *
 * 11-Feb-87 -- rafiey (Ali Rafieymehr)
 *	Initialized two new variables for the Vaxstar color.
 *
 * 29-Jan-87 -- rsp (Ricky Palmer)
 *	Added support for its frame buffer driver.
 *
 * 06-Jan-87 -- gmm (George Mathew)
 *	Removed references to sdreset (Vaxstar disk driver does not need a 
 *	reset routine)
 *
 * 26-Sep-86 -- darrell
 *	This change undoes the temporary change put in by Fred Canter
 *	on 15-Sep-16.  The TZK50 driver now handles nbufio.
 *
 * 15-Sep-86 -- fred (Fred Canter)
 *	TEMPORARY CHANGE: remove ststrategy entry from cdevsw, to disable
 *	nbufio for VAXstar TZK50 driver until the driver can be fixed.
 *	This will allow the base-level I TK50 kit to install on VAXstar.
 *
 *  3-Sep-86 -- fred (Fred Canter)
 *	Removed unused sm driver entry points: smread, smwrite, smreset.
 *	Added dummy smcons_init & sgcons_init routines, to allow
 *	the kernel to build if they are not configured.
 *
 * 28-Aug-86 -- prs
 *	Removed declaration of ubdinit. Config declares an empty ubdinit
 *	structure, even if no uba's are in config file.
 *
 * 14-Aug-86 -- fred (Fred Canter)
 *	Changes VAXstar disk slaves from sd to rd/rx.
 *
 * 06-Aug-86 -- jaw	fixed baddaddr to work on running system.
 *
 *  5-Aug-86  -- fred (Fred Canter)
 *	Changed st to stc and misc. other VAXstar changes.
 *
 * 23-Jul-86  -- prs
 *	Added the genericconf table. This table needs to be built here
 *	because only configured devices can be in the table. Also if
 *	no uba's or mba's are configured, then set ubdinit and mbdinit
 *	structure pointers to NULL so swapgeneric, swapboot, and machdep
 *	won't complain.
 *
 *  2-Jul-86  -- fred (Fred Canter)
 *	Added more VAXstar devices to cdevsw[] (sg sm sh).
 *	Added dummy ??driver symbols to satisfy reference in ka630.c
 *	if the device is not configured.
 *
 * 18-Jun-86  -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 15-Apr-86 -- afd
 *	Changed "v_console" to "v_consputc".
 *	Added definition of "v_consgetc" for qvss and qdss input.
 *	These are no longer enclosed by "#ifdef MVAX". Since the symbols
 *	    are referenced in cons.o they must be defined for all cpus.
 *	There is also a dummy "contigphys()" to define the symbol.
 *
 * 02-Apr-86 -- jaw  add support for nautilus console and memory adapter
 *
 * 11-Mar-86
 *	add support for SAS memory driver
 *
 * 11-Mar-86 -- lp
 *	Add strat to entire cdevsw for n-buffered I/O.
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *
 * 01-Mar-86 -- afd  Added DMB32 device to cdevsw.
 *
 * 12-Feb-86 -- jrs
 *	Move defn of null routine for buainit to bi dependency rather
 *	than 8200 cpu dependency.
 *
 * 03-Feb-86 -- jaw  for now define ka8200conf.
 *
 * 16-Jan-86 -- darrell
 *	removed the lines for the ka820machcheck, so it is not made
 *	as a psuedo device and defined it in cpuconf.c
 *
 * 20-Jan-86 -- Barb Glover
 *	Added error logger character device switch
 *
 * 01-Oct-85 -- Larry Cohen
 *	include ioctl.h so that window structure is defined.
 *
 * 03-Sep-85 -- jaw
 *	bi error interrupt added.
 *
 * 03-Aug-85 -- rjl
 *	added qdss and support to the microvax virtual console support.
 *
 * 21-Jun-85 -- jaw
 *	crx changes to cs...
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 20-Mar-85 jaw
 *	add VAX8200 console storage device.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 26 Nov 84 -- rjl
 *	Added the system console as major number 38. Major #0 becomes the
 *	system virtual console. When it's something other than the real
 *	console, the real console can be accessed as 38,0.
 *
 *	14-Nov-84	Stephen Reilly
 * 001- Added ioctl to disk driver which will be used for the disk partitioning
 *	scheme.
 *
 *	conf.c	6.1	83/07/29
 ***********************************************************************/

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/ioctl.h"
#include "../h/tty.h"
#include "../h/conf.h"
#include "../h/errno.h"

#include "../h/types.h"
#include "../vaxuba/ubavar.h"
#include "../sas/vmb.h"

int	nulldev();
int	nodev();

#include "vaxbi.h"
#if NVAXBI == 0
probebi() { /* make locore happy */ }
bierrors() { /* make locore happy */ }
buainit() { /* make locore happy */ }
biclrint() {/*make machdep happy */ }
bisetint() {/*make machdep happy */ }
#endif

#if defined(VAX8200) || defined(VAX8800)
int	cs_size(),cs_ioctl(),cs_open(),cs_close(),cs_strategy(),cs_read(),cs_write();
#else
#define cs_size nodev
#define cs_open nodev
#define cs_close nodev
#define cs_strategy nodev
#define cs_read nodev
#define cs_ioctl nodev
#define cs_write nodev
#endif

#ifndef VAX8200
bimemenable() { /*null */}
bimemerror() { /*null */}
ka820rxcd() { /*null */}
#endif

#include "hp.h"
#if NHP > 0
int	hpopen(),hpstrategy(),hpread(),hpwrite(),hpdump(),hpioctl(),hpsize();
#else
#define hpopen		nodev
#define hpstrategy	nodev
#define hpread		nodev
#define hpwrite 	nodev
#define hpdump		nodev
#define hpioctl 	nodev
#define hpsize		0
#endif

#include "tu.h"
#if NHT > 0
int	htopen(),htclose(),htstrategy(),htread(),htwrite(),htdump(),htioctl();
#else
#define htopen		nodev
#define htclose 	nodev
#define htstrategy	nodev
#define htread		nodev
#define htwrite 	nodev
#define htdump		nodev
#define htioctl 	nodev
#endif

#include "st.h"
#if NST > 0
int	stopen(),stclose(),ststrategy(),stread(),stwrite(),stdump(),stioctl();
#else
#define stopen		nodev
#define stclose 	nodev
#define ststrategy	nodev
#define stread		nodev
#define stwrite 	nodev
#define stdump		nodev
#define stioctl 	nodev
int	stcdriver;
st_start()
{
}
#endif

#include "rk.h"
#if NHK > 0
int	rkopen(),rkstrategy(),rkread(),rkwrite(),rkioctl(),rkintr();
int	rkdump(),rkreset(),rksize();
#else
#define rkopen		nodev
#define rkstrategy	nodev
#define rkread		nodev
#define rkwrite 	nodev
#define rkioctl 	nodev
#define rkintr		nodev
#define rkdump		nodev
#define rkreset 	nodev
#define rksize		0
#endif

#include "te.h"
#if NTE > 0
int	tmopen(),tmclose(),tmstrategy(),tmread(),tmwrite();
int	tmioctl(),tmdump(),tmreset();
#else
#define tmopen		nodev
#define tmclose 	nodev
#define tmstrategy	nodev
#define tmread		nodev
#define tmwrite 	nodev
#define tmioctl 	nodev
#define tmdump		nodev
#define tmreset 	nodev
#endif

#include "ts.h"
#if NTS > 0
int	tsopen(),tsclose(),tsstrategy(),tsread(),tswrite();
int	tsioctl(),tsdump(),tsreset();
#else
#define tsopen		nodev
#define tsclose 	nodev
#define tsstrategy	nodev
#define tsread		nodev
#define tswrite 	nodev
#define tsioctl 	nodev
#define tsdump		nodev
#define tsreset 	nodev
#endif

#include "mu.h"
#if NMT > 0
int	mtopen(),mtclose(),mtstrategy(),mtread(),mtwrite();
int	mtioctl(),mtdump();
#else
#define mtopen		nodev
#define mtclose 	nodev
#define mtstrategy	nodev
#define mtread		nodev
#define mtwrite 	nodev
#define mtioctl 	nodev
#define mtdump		nodev
#endif

#include "ra.h"
#if NUQ > 0
int	udopen(),udstrategy(),udread(),udwrite(),udioctl();
int	udreset(),uddump(),udsize();
#else
#define udopen		nodev
#define udstrategy	nodev
#define udread		nodev
#define udwrite 	nodev
#define udioctl 	nodev
#define udreset 	nulldev
#define uddump		nodev
#define udsize		0
#endif

#include "rd.h"
#include "rx.h"
/*
 * CAUTION:
 *	NRX will be two more than the actual number of
 *	floppy drives, due to the way config counts slaves.
 */
#define	NSD	NRD+NRX
#if NSD > 0
int	sdopen(),sdstrategy(),sdread(),sdwrite(),sdioctl();
int	sddump(),sdsize();
#else
#define sdopen		nodev
#define sdstrategy	nodev
#define sdread		nodev
#define sdwrite 	nodev
#define sdioctl 	nodev
#define sddump		nodev
#define sdsize		0
int	sdcdriver;
sdustart()
{
}
#endif

#include "up.h"
#if NSC > 0
int	upopen(),upstrategy(),upread(),upwrite(),upioctl();
int	upreset(),updump(),upsize();
#else
#define upopen		nodev
#define upstrategy	nodev
#define upread		nodev
#define upwrite 	nodev
#define upioctl 	nodev
#define upreset 	nulldev
#define updump		nodev
#define upsize		0
#endif

#include "tj.h"
#if NUT > 0
int	utopen(),utclose(),utstrategy(),utread(),utwrite(),utioctl();
int	utreset(),utdump();
#else
#define utopen		nodev
#define utclose 	nodev
#define utread		nodev
#define utstrategy	nodev
#define utwrite 	nodev
#define utreset 	nulldev
#define utioctl 	nodev
#define utdump		nodev
#endif

#include "rb.h"
#if NIDC > 0
int	idcopen(),idcstrategy(),idcread(),idcwrite(),idcioctl();
int	idcreset(),idcdump(),idcsize();;
#else
#define idcopen 	nodev
#define idcstrategy	nodev
#define idcread 	nodev
#define idcwrite	nodev
#define idcioctl	nodev
#define idcreset	nulldev
#define idcdump 	nodev
#define idcsize 	0
#endif

#if defined(VAX750) || defined(VAX730)
int	tuopen(),tuclose(),tustrategy();
#else
#define tuopen		nodev
#define tuclose 	nodev
#define tustrategy	nodev
#endif

#include "rx.h"
#if NFX > 0
int	rxopen(),rxstrategy(),rxclose(),rxread(),rxwrite(),rxreset(),rxioctl();
#else
#define rxopen		nodev
#define rxstrategy	nodev
#define rxclose 	nodev
#define rxread		nodev
#define rxwrite 	nodev
#define rxreset 	nulldev
#define rxioctl 	nodev
#endif

#include "uu.h"
#if NUU > 0
int	uuopen(),uustrategy(),uuclose(),uureset(),uuioctl();
#else
#define uuopen		nodev
#define uustrategy	nodev
#define uuclose 	nodev
#define uureset 	nulldev
#define uuioctl 	nodev
int	uu_softc;
int	uudinfo;
int	uudata;
#endif

#include "rl.h"
#if NRL > 0
int	rlopen(),rlstrategy(),rlread(),rlwrite(),rlioctl();
int	rlreset(),rldump(),rlsize();
#else
#define rlopen		nodev
#define rlstrategy	nodev
#define rlread		nodev
#define rlwrite 	nodev
#define rlioctl 	nodev
#define rlreset 	nulldev
#define rldump		nodev
#define rlsize		0
#endif

#include "tms.h"
#if NTMSCP > 0
int	tmscpopen(),tmscpclose(),tmscpstrategy(),tmscpread(),tmscpwrite();
int	tmscpioctl(),tmscpdump(),tmscpreset();
#else
#define tmscpopen	nodev
#define tmscpclose	nodev
#define tmscpstrategy	nodev
#define tmscpread	nodev
#define tmscpwrite	nodev
#define tmscpioctl	nodev
#define tmscpdump	nodev
#define tmscpreset	nodev
#endif

#include "md.h"
#if NMD > 0
int	mdstrategy(),md_size(),mdioctl();
#else
#define mdstrategy	nodev
#define md_size 	nodev
#define mdioctl 	nodev
#endif

int	swstrategy(),swread(),swwrite();

struct bdevsw	bdevsw[] =
{
	{ hpopen,	nulldev,	hpstrategy,	hpdump, 	/*0*/
	  hpsize,	0,	hpioctl },
	{ htopen,	htclose,	htstrategy,	htdump, 	/*1*/
	  0,		B_TAPE, nodev },
	{ upopen,	nulldev,	upstrategy,	updump, 	/*2*/
	  upsize,	0,	nodev },
	{ rkopen,	nulldev,	rkstrategy,	rkdump, 	/*3*/
	  rksize,	0,	rkioctl },
	{ nodev,	nodev,		swstrategy,	nodev,		/*4*/
	  0,		0,	nodev },
	{ tmopen,	tmclose,	tmstrategy,	tmdump, 	/*5*/
	  0,		B_TAPE, nodev },
	{ tsopen,	tsclose,	tsstrategy,	tsdump, 	/*6*/
	  0,		B_TAPE, nodev },
	{ mtopen,	mtclose,	mtstrategy,	mtdump, 	/*7*/
	  0,		B_TAPE, nodev },
	{ tuopen,	tuclose,	tustrategy,	nodev,		/*8*/
	  0,		B_TAPE, nodev },
	{ udopen,	nulldev,	udstrategy,	uddump, 	/*9*/
	  udsize,	0,	udioctl },
	{ utopen,	utclose,	utstrategy,	utdump, 	/*10*/
	  0,		B_TAPE, nodev },
	{ idcopen,	nodev,		idcstrategy,	idcdump,	/*11*/
	  idcsize,	0,	idcioctl },
	{ rxopen,	rxclose,	rxstrategy,	nodev,		/*12*/
	  0,		0,	nodev },
	{ uuopen,	uuclose,	uustrategy,	nodev,		/*13*/
	  0,		0,	nodev },
	{ rlopen,	nodev,		rlstrategy,	rldump, 	/*14*/
	  rlsize,	0,	rlioctl },
	{ tmscpopen,	tmscpclose,	tmscpstrategy,	tmscpdump,	/*15*/
	  0,		B_TAPE, nodev },
	{ cs_open,	cs_close,	cs_strategy,	nodev,		/*16*/
	  cs_size,	0,	cs_ioctl },
	{ nulldev,	nulldev,	mdstrategy,	nodev,		/*17*/
	  md_size,	0,	mdioctl },
	{ stopen,	stclose,	ststrategy,	stdump, 	/*18*/
	  0,		B_TAPE, nodev },
	{ sdopen,	nulldev,	sdstrategy,	sddump, 	/*19*/
	  sdsize,	0,	sdioctl },
};

#include "mba.h"
#if NMBA == 0
struct mba_device *mbdinit = NULL;
#endif

#if NHP > 0
extern struct mba_driver hpdriver;
#endif

#if NRA > 0
extern struct uba_driver uqdriver;
#endif

#if NRB > 0
extern struct uba_driver idcdriver;
#endif

#if NRL > 0
extern struct uba_driver hldriver;
#endif

#if NHK > 0
extern struct uba_driver hkdriver;
#endif

#if NSD > 0
extern struct uba_driver sdcdriver;
#endif

/*
 * Initialize genericconf structure, if the driver is configured.
 */

struct genericconf  genericconf[] =
{
#if NHP > 0
	{ (caddr_t)&hpdriver,	"hp",	makedev(0,0),	BTD$K_MB },
#endif
#if NRA > 0
	{ (caddr_t)&uqdriver,	"ra",	makedev(9,0),	BTD$K_UDA },
	{ (caddr_t)&uqdriver,	"ra",	makedev(9,0),	BTD$K_BDA },
#endif
#if NRB > 0
	{ (caddr_t)&idcdriver,	"rb",	makedev(11,0),	BTD$K_DQ },
#endif
#if NRL > 0
	{ (caddr_t)&hldriver,	"rl",	makedev(14,0),	BTD$K_DL },
#endif
#if NHK > 0
	{ (caddr_t)&hkdriver,	"rk",	makedev(3,0),	BTD$K_DM },
#endif
#if NSD > 0
	{ (caddr_t)&sdcdriver,	"rd",	makedev(19,0),	BTD$K_KA640_DISK }, /* VAXSTAR */
#endif
	{ 0 },
};

int	nblkdev = sizeof (bdevsw) / sizeof (bdevsw[0]);

int	cnopen(),cnclose(),cnread(),cnwrite(),cnioctl();
struct tty cons[];


#include "acc.h"
#if NACC > 0
int	accreset();
#else
#define accreset nulldev
#endif

#include "ct.h"
#if NCT > 0
int	ctopen(),ctclose(),ctwrite();
#else
#define ctopen	nulldev
#define ctclose nulldev
#define ctwrite nulldev
#endif

#include "dh.h"
#if NDH == 0
#define dhopen	nodev
#define dhclose nodev
#define dhread	nodev
#define dhwrite nodev
#define dhioctl nodev
#define dhstop	nodev
#define dhreset nulldev
#define dh11	0
int	nNDH = NDH;
dhtimer(){/* to keep the undefines in locore happy */ }
#else
int	dhopen(),dhclose(),dhread(),dhwrite(),dhioctl(),dhstop(),dhreset();
struct	tty dh11[];
#endif

#include "dhu.h"
#if NDHU == 0
#define dhuopen nodev
#define dhuclose	nodev
#define dhuread nodev
#define dhuwrite	nodev
#define dhuioctl	nodev
#define dhustop nodev
#define dhureset	nulldev
#define dhu11	0
int	nNDHU = NDHU;
dhutimer(){/* to keep the undefines in locore happy */ }
#else NDHU
int	dhuopen(),dhuclose(),dhuread(),dhuwrite(),dhuioctl(),dhustop(),dhureset();
struct	tty dhu11[];
#endif NDHU

#include "dmf.h"
#if NDMF == 0
#define dmfopen nodev
#define dmfclose	nodev
#define dmfread nodev
#define dmfwrite	nodev
#define dmfioctl	nodev
#define dmfstop nodev
#define dmfreset	nulldev
#define dmf_tty 0
#else
int	dmfopen(),dmfclose(),dmfread(),dmfwrite(),dmfioctl(),dmfstop(),dmfreset();
struct	tty dmf_tty[];
#endif

#include "dmb.h"
#if NDMB == 0
#define dmbopen nodev
#define dmbclose	nodev
#define dmbread nodev
#define dmbwrite	nodev
#define dmbioctl	nodev
#define dmbstop nodev
#define dmbreset	nulldev
#define dmb_tty 0
#else
int	dmbopen(),dmbclose(),dmbread(),dmbwrite(),dmbioctl(),dmbstop(),dmbreset();
struct	tty dmb_tty[];
#endif

#include "dmz.h"
#if NDMZ == 0
#define dmzopen nodev
#define dmzclose	nodev
#define dmzread nodev
#define dmzwrite	nodev
#define dmzioctl	nodev
#define dmzstop nodev
#define dmzreset	nulldev
#define dmz_tty 0
#else
int	dmzopen(),dmzclose(),dmzread(),dmzwrite(),dmzioctl(),dmzstop(),dmzreset();
struct	tty dmz_tty[];
#endif

#include "qv.h"
#if NQV == 0
#define qvopen	nodev
#define qvclose nodev
#define qvread	nodev
#define qvwrite nodev
#define qvioctl nodev
#define qvstop	nodev
#define qvreset nulldev
#define qvselect nodev
#define qv_tty	0
#define qvcons_init nulldev
#else NQV
int	qvopen(),qvclose(),qvread(),qvwrite(),qvioctl(),qvstop(),qvreset(),
	qvselect(),qvcons_init();
struct	tty qv_tty[];
#endif NQV

#include "qd.h"
#if NQD == 0
#define qdopen	nodev
#define qdclose nodev
#define qdread	nodev
#define qdwrite nodev
#define qdioctl nodev
#define qdstop	nodev
#define qdreset nulldev
#define qdselect nodev
#define qd_tty	0
#define qdcons_init nulldev
#else NQD
int	qdopen(),qdclose(),qdread(),qdwrite(),qdioctl(),qdstop(),qdreset(),
	qdselect(),qdcons_init();
struct	tty qd_tty[];
#endif NQD

#include "sm.h"
/*
 * NOTE:
 *	VAXstar bitmap driver has a cdevsw[]
 *	entry, but it is never used. All calls
 *	to bitmap driver come from SLU driver (ss.c).
 */
#if NSM == 0
#define smopen	nodev
#define smclose nodev
#define smread	nodev
#define smwrite nodev
#define smioctl nodev
#define smstop	nodev
#define smreset nulldev
#define smselect nodev
smcons_init()
{
	return(ENXIO);
}
int	smdriver;
#else NSM
#define smread	nodev
#define smwrite nodev
int	smopen(),smclose(),smioctl(),smstop(),smreset(),
	smselect();
#endif NSM

#include "sg.h"
#if NSG == 0
#define sgopen	nodev
#define sgclose nodev
#define sgread	nodev
#define sgwrite nodev
#define sgioctl nodev
#define sgstop	nodev
#define sgreset nulldev
#define sgselect nodev
#define sg_tty	0
sgcons_init()
{
	return(ENXIO);
}
int	sgdriver;
#else NSG
int	sgopen(),sgclose(),sgread(),sgwrite(),sgioctl(),sgstop(),sgreset(),
	sgselect();
struct	tty sg_tty[];
#endif NSG

#include "sh.h"
#if NSH == 0
#define shopen nodev
#define shclose	nodev
#define shread nodev
#define shwrite	nodev
#define shioctl	nodev
#define shstop nodev
#define shreset	nulldev
#define sh_tty	0
int	shdriver;
int	nNSH = NSH;
shtimer(){/* to keep the undefines in locore happy */ }
#else NSH
int	shopen(),shclose(),shread(),shwrite(),shioctl(),shstop(),shreset();
struct	tty sh_tty[];
#endif NSH

#include "ss.h"
#if NSS == 0
#define ssopen	nodev
#define ssclose nodev
#define ssread	nodev
#define sswrite nodev
#define ssioctl nodev
#define ssstop	nodev
#define ssselect nodev
#define ss_tty	0
#define sscons_init nulldev
int	nNSS = NSS;
sstimer(){/* to get rid of undefines in locore */ }
int	sspdma;
int	ssdriver;
#else
int	ssopen(),ssclose(),ssread(),sswrite(),ssioctl(),ssstop(),
	ssselect(),sscons_init();
struct	tty ss_tty[];
#endif

#include "se.h"
#if NSE == 0
int	sedriver;
#endif

/*
 * MicroVAX and VAXstar virtual console support
 */
/*
 * Virtual console character display and read routines.
 * The drivers will fill these in as they configure.
 */
int (*v_consputc)() = 0;
int (*v_consgetc)() = 0;

/*
 * VAXstar graphics device driver entry points,
 * can't use cdevsw[] like QVSS & QDSS because the
 * VAXstar console 4 line SLU is shared between the graphics
 * device and the EIA and printer ports.
 * The graphics device drivers fill in the entry
 * points in their ??cons_init() routines.
 * The SLU driver (ss.c) uses these entry points to call
 * the graphics driver if the operation if for the graphics device.
 */
int (*vs_gdopen)() = 0;
int (*vs_gdclose)() = 0;
int (*vs_gdread)() = 0;
int (*vs_gdwrite)() = 0;
int (*vs_gdselect)() = 0;
int (*vs_gdkint)() = 0;
int (*vs_gdioctl)() = 0;
int (*vs_gdstop)() = 0;

/*
 * Console initialization routines
 *
 * These routines are called to set up the system console so
 * that it can be used by cnputc to display characters before
 * the driver probe routines are hit. (catch 22) The search is
 * complete when the list is ended or a driver reports that it
 * setup.
 */

/*
 * CAUTION:	sscons_init must be first so that the serial line cntlr
 *		will be configured as the console on a VAXstar/TEAMmate.
 *		Something in the VAXstar's address space looks like a
 *		QDSS and fools autoconf. If sscons_init is not first, then
 *		qvcons_init and qdcons_init must be modified to return 0
 *		if the CPU is a VAXstar.
 */
(*vcons_init[])() = {
	sscons_init,	/* MUST BE FIRST */
	qdcons_init,
	qvcons_init,
	0
};
/*
 * contigphys is only used by the MicroVAX I. But the routine
 * must be defined for all processors to resolve the reference
 * from uba.c.
 */
#ifndef MVAX
contigphys()
{
}
#endif

#include "lta.h"
#if NLTA == 0
#define ltaopen 	nodev
#define ltaclose	nodev
#define ltaread 	nodev
#define ltawrite	nodev
#define ltaioctl	nodev
#define ltastop 	nodev
#define ltareset	nulldev
#define lata	0
#else
int ltaopen(),ltaclose(),ltaread(),ltawrite(),ltaioctl(),ltastop();
#define ltareset	nulldev
struct tty lata[];
#endif

#if VAX8600
int	crlopen(),crlclose(),crlread(),crlwrite();
#else
#define crlopen 	nodev
#define crlclose	nodev
#define crlread 	nodev
#define crlwrite	nodev
#endif

#if VAX780
int	flopen(),flclose(),flread(),flwrite();
#else
#define flopen	nodev
#define flclose nodev
#define flread	nodev
#define flwrite nodev
#endif

#include "dz.h"
#if NDZ == 0
#define dzopen	nodev
#define dzclose nodev
#define dzread	nodev
#define dzwrite nodev
#define dzioctl nodev
#define dzstop	nodev
#define dzreset nulldev
#define dz_tty	0
int	nNDZ = NDZ;
dztimer(){/* to get rid of undefines in locore */ }
int	dzpdma;
#else
int	dzopen(),dzclose(),dzread(),dzwrite(),dzioctl(),dzstop(),dzreset();
struct	tty dz_tty[];
#endif
#include "lp.h"
#if NLP > 0
int	lpopen(),lpclose(),lpwrite(),lpreset();
#else
#define lpopen		nodev
#define lpclose 	nodev
#define lpwrite 	nodev
#define lpreset 	nulldev
#endif

int	syopen(),syread(),sywrite(),syioctl(),syselect();

int	mmread(),mmwrite();
#define mmselect	seltrue

#include "va.h"
#if NVA > 0
int	vaopen(),vaclose(),vawrite(),vaioctl(),vareset(),vaselect();
#else
#define vaopen		nodev
#define vaclose 	nodev
#define vawrite 	nodev
#define vaopen		nodev
#define vaioctl 	nodev
#define vareset 	nulldev
#define vaselect	nodev
#endif

#include "vp.h"
#if NVP > 0
int	vpopen(),vpclose(),vpwrite(),vpioctl(),vpreset(),vpselect();
#else
#define vpopen		nodev
#define vpclose 	nodev
#define vpwrite 	nodev
#define vpioctl 	nodev
#define vpreset 	nulldev
#define vpselect	nodev
#endif

#include "pty.h"
#if NPTY > 0
int	ptsopen(),ptsclose(),ptsread(),ptswrite(),ptsstop();
int	ptcopen(),ptcclose(),ptcread(),ptcwrite(),ptcselect();
int	ptyioctl();
struct	tty pt_tty[];
#else
#define ptsopen 	nodev
#define ptsclose	nodev
#define ptsread 	nodev
#define ptswrite	nodev
#define ptcopen 	nodev
#define ptcclose	nodev
#define ptcread 	nodev
#define ptcwrite	nodev
#define ptyioctl	nodev
#define pt_tty		0
#define ptcselect	nodev
#define ptsstop 	nulldev
#endif

#include "lpa.h"
#if NLPA > 0
int	lpaopen(),lpaclose(),lparead(),lpawrite(),lpaioctl();
#else
#define lpaopen 	nodev
#define lpaclose	nodev
#define lparead 	nodev
#define lpawrite	nodev
#define lpaioctl	nodev
#endif

#include "dn.h"
#if NDN > 0
int	dnopen(),dnclose(),dnwrite();
#else
#define dnopen		nodev
#define dnclose 	nodev
#define dnwrite 	nodev
#endif

#include "gpib.h"
#if NGPIB > 0
int	gpibopen(),gpibclose(),gpibread(),gpibwrite(),gpibioctl();
#else
#define gpibopen	nodev
#define gpibclose	nodev
#define gpibread	nodev
#define gpibwrite	nodev
#define gpibioctl	nodev
#endif

#include "ik.h"
#if NIK > 0
int	ikopen(),ikclose(),ikread(),ikwrite(),ikioctl(),ikreset();
#else
#define ikopen nodev
#define ikclose nodev
#define ikread nodev
#define ikwrite nodev
#define ikioctl nodev
#define ikreset nodev
#endif

#include "its.h"
#if NITS > 0
int	itsopen(),itsclose(),itsioctl(),itsread();
#else
#define itsopen nodev
#define itsclose nodev
#define itsioctl nodev
#define itsread nodev
#endif

#include "ps.h"
#if NPS > 0
int	psopen(),psclose(),psread(),pswrite(),psioctl(),psreset();
#else
int	nNPS = NPS;
psextsync(){	/* Got to keep locore happy */}
#define psopen nodev
#define psclose nodev
#define psread nodev
#define pswrite nodev
#define psopen nodev
#define psioctl nodev
#define psreset nodev
#endif

#include "ib.h"
#if NIB > 0
int	ibopen(),ibclose(),ibread(),ibwrite(),ibioctl();
#else
#define ibopen	nodev
#define ibclose nodev
#define ibread	nodev
#define ibwrite nodev
#define ibioctl nodev
#endif

#include "ad.h"
#if NAD > 0
int	adopen(),adclose(),adioctl(),adreset();
#else
#define adopen nodev
#define adclose nodev
#define adioctl nodev
#define adreset nodev
#endif

#include "vs.h"
#if NVS > 0
int	vsopen(),vsclose(),vsioctl(),vsreset(),vsselect();
#else
#define vsopen nodev
#define vsclose nodev
#define vsioctl nodev
#define vsreset nodev
#define vsselect nodev
#endif

#include "sys_trace.h"
#if NSYS_TRACE > 0
int	trace_open(),trace_close(),trace_ioctl(),trace_select(),trace_read();
#else NSYS_TRACE
#define	trace_open nodev
#define	trace_close nodev
#define	trace_ioctl nodev
#define	trace_read nodev
#define	trace_select nodev
int syscall_trace(x,y,z){}
#endif NSYS_TRACE

int	ttselect(), seltrue(), asyncsel();
int	erropen(), errclose(), erread(), errwrite(), errioctl(), errsel();

struct cdevsw	cdevsw[] =
{
	cnopen, 	cnclose,	cnread, 	cnwrite,	/*0*/
	cnioctl,	nulldev,	nulldev,	cons,
	ttselect,	nodev,		0,
	dzopen, 	dzclose,	dzread, 	dzwrite,	/*1*/
	dzioctl,	dzstop, 	dzreset,	dz_tty,
	ttselect,	nodev,		0,
	syopen, 	nulldev,	syread, 	sywrite,	/*2*/
	syioctl,	nulldev,	nulldev,	0,
	syselect,	nodev,		0,
	nulldev,	nulldev,	mmread, 	mmwrite,	/*3*/
	nodev,		nulldev,	nulldev,	0,
	mmselect,	nodev,		0,
	hpopen, 	nulldev,	hpread, 	hpwrite,	/*4*/
	hpioctl,	nodev,		nulldev,	0,
	asyncsel,	nodev,		hpstrategy,
	htopen, 	htclose,	htread, 	htwrite,	/*5*/
	htioctl,	nodev,		nulldev,	0,
	asyncsel,	nodev,		htstrategy,
	vpopen, 	vpclose,	nodev,		vpwrite,	/*6*/
	vpioctl,	nulldev,	vpreset,	0,
	vpselect,	nodev,		0,
	nulldev,	nulldev,	swread, 	swwrite,	/*7*/
	nodev,		nodev,		nulldev,	0,
	nodev,		nodev,		0,
	flopen, 	flclose,	flread, 	flwrite,	/*8*/
	nodev,		nodev,		nulldev,	0,
	seltrue,	nodev,		0,
	udopen, 	nulldev,	udread, 	udwrite,	/*9*/
	udioctl,	nodev,		udreset,		0,
	asyncsel,	nodev,		udstrategy,
	vaopen, 	vaclose,	nodev,		vawrite,	/*10*/
	vaioctl,	nulldev,	vareset,	0,
	vaselect,	nodev,		0,
	rkopen, 	nulldev,	rkread, 	rkwrite,	/*11*/
	rkioctl,	nodev,		rkreset,	0,
	seltrue,	nodev,		0,
	dhopen, 	dhclose,	dhread, 	dhwrite,	/*12*/
	dhioctl,	dhstop, 	dhreset,	dh11,
	ttselect,	nodev,		0,
	upopen, 	nulldev,	upread, 	upwrite,	/*13*/
	upioctl,	nodev,		upreset,	0,
	seltrue,	nodev,		0,
	tmopen, 	tmclose,	tmread, 	tmwrite,	/*14*/
	tmioctl,	nodev,		tmreset,	0,
	seltrue,	nodev,		0,
	lpopen, 	lpclose,	nodev,		lpwrite,	/*15*/
	nodev,		nodev,		lpreset,	0,
	seltrue,	nodev,		0,
	tsopen, 	tsclose,	tsread, 	tswrite,	/*16*/
	tsioctl,	nodev,		tsreset,	0,
	asyncsel,	nodev,		tsstrategy,
	utopen, 	utclose,	utread, 	utwrite,	/*17*/
	utioctl,	nodev,		utreset,	0,
	seltrue,	nodev,		0,
	ctopen, 	ctclose,	nodev,		ctwrite,	/*18*/
	nodev,		nodev,		nulldev,	0,
	seltrue,	nodev,		0,
	mtopen, 	mtclose,	mtread, 	mtwrite,	/*19*/
	mtioctl,	nodev,		nodev,		0,
	asyncsel,	nodev,		mtstrategy,
	ptsopen,	ptsclose,	ptsread,	ptswrite,	/*20*/
	ptyioctl,	ptsstop,	nodev,		pt_tty,
	ttselect,	nodev,		0,
	ptcopen,	ptcclose,	ptcread,	ptcwrite,	/*21*/
	ptyioctl,	nulldev,	nodev,		pt_tty,
	ptcselect,	nodev,		0,
	dmfopen,	dmfclose,	dmfread,	dmfwrite,	/*22*/
	dmfioctl,	dmfstop,	dmfreset,	dmf_tty,
	ttselect,	nodev,		0,
	idcopen,	nulldev,	idcread,	idcwrite,	/*23*/
	idcioctl,	nodev,		idcreset,	0,
	seltrue,	nodev,		0,
	dnopen, 	dnclose,	nodev,		dnwrite,	/*24*/
	nodev,		nodev,		nodev,		0,
	seltrue,	nodev,		0,
/* 25-29 reserved to local sites */
	gpibopen,	gpibclose,	gpibread,	gpibwrite,	/*25*/
	gpibioctl,	nulldev,	nodev,		0,
	seltrue,	nodev,		0,
	lpaopen,	lpaclose,	lparead,	lpawrite,	/*26*/
	lpaioctl,	nodev,		nulldev,	0,
	seltrue,	nodev,		0,
	psopen, 	psclose,	psread, 	pswrite,	/*27*/
	psioctl,	nodev,		psreset,	0,
	seltrue,	nodev,		0,
	ibopen, 	ibclose,	ibread, 	ibwrite,	/*28*/
	ibioctl,	nodev,		nodev,		0,
	seltrue,	nodev,		0,
	adopen, 	adclose,	nodev,		nodev,		/*29*/
	adioctl,	nodev,		adreset,	0,
	seltrue,	nodev,		0,
	rxopen, 	rxclose,	rxread, 	rxwrite,	/*30*/
	rxioctl,	nodev,		rxreset,	0,
	seltrue,	nodev,		0,
	ikopen, 	ikclose,	ikread, 	ikwrite,	/*31*/
	ikioctl,	nodev,		ikreset,	0,
	seltrue,	nodev,		0,
	rlopen, 	nodev,		rlread, 	rlwrite,	/*32*/
	rlioctl,	nodev,		rlreset,	0,
	seltrue,	nodev,		0,
/*
 * The DHU driver includes the DHV driver.
 */
	dhuopen,	dhuclose,	dhuread,	dhuwrite,	/*33*/
	dhuioctl,	dhustop,	dhureset,	dhu11,
	ttselect,	nodev,		0,
	dmzopen,	dmzclose,	dmzread,	dmzwrite,	/*34*/
	dmzioctl,	dmzstop,	dmzreset,	dmz_tty,
	ttselect,	nodev,		0,
	qvopen, 	qvclose,	qvread, 	qvwrite,	/*35*/
	qvioctl,	qvstop, 	qvreset,	qv_tty,
	qvselect,	nodev,		0,
	tmscpopen,	tmscpclose,	tmscpread,	tmscpwrite,	/*36*/
	tmscpioctl,	nodev,		tmscpreset,	0,
	asyncsel,	nodev,		tmscpstrategy,
	vsopen, 	vsclose,	nodev,		nodev,		/*37*/
	vsioctl,	nodev,		vsreset,	0,
	vsselect,	nodev,		0,
	cnopen, 	cnclose,	cnread, 	cnwrite,	/*38*/
	cnioctl,	nulldev,	nulldev,	cons,
	ttselect,	nodev,		0,
	ltaopen,	ltaclose,	ltaread,	ltawrite,	/*39*/
	ltaioctl,	ltastop,	ltareset,	lata,
	ttselect,	nodev,		0,
	crlopen,	crlclose,	crlread,	crlwrite,	/*40*/
	nodev,		nodev,		nulldev,	0,
	seltrue,	nodev,		0,
	cs_open,	cs_close,	cs_read,	cs_write,	/*41*/
	cs_ioctl,	nodev,		nulldev,	0,
	seltrue,	nodev,		0,
	qdopen, 	qdclose,	qdread, 	qdwrite,	/*42*/
	qdioctl,	qdstop, 	qdreset,	qd_tty,
	qdselect,	nodev,		0,
	erropen,	errclose,	erread, 	errwrite,	/*43*/
	errioctl,	nodev,		nulldev,	0,
	errsel, 	nodev,		0,
	dmbopen,	dmbclose,	dmbread,	dmbwrite,	/*44*/
	dmbioctl,	dmbstop,	dmbreset,	dmb_tty,
	ttselect,	nodev,		0,
	ssopen, 	ssclose,	ssread, 	sswrite,	/*45*/
	ssioctl,	ssstop, 	nulldev,	ss_tty,
	ssselect,	nodev,		0,
	stopen, 	stclose,	stread, 	stwrite,	/*46*/
	stioctl,	nodev,		nulldev,	0,
	asyncsel,	nodev,		ststrategy,
	sdopen, 	nulldev,	sdread, 	sdwrite,	/*47*/
	sdioctl,	nodev,		nulldev,	0,
	asyncsel,	nodev,		sdstrategy,
	trace_open,	trace_close,	trace_read,	nodev,		/*48*/
	trace_ioctl,	nodev,		nodev,		0,
	trace_select,	nodev,		0,
	smopen, 	smclose,	smread, 	smwrite,	/*49*/
	smioctl,	smstop, 	smreset,	0,
	smselect,	nodev,		0,
	sgopen, 	sgclose,	sgread, 	sgwrite,	/*50*/
	sgioctl,	sgstop, 	sgreset,	sg_tty,
	sgselect,	nodev,		0,
	shopen, 	shclose,	shread, 	shwrite,	/*51*/
	shioctl,	shstop, 	shreset,	sh_tty,
	ttselect,	nodev,		0,
	itsopen,	itsclose,	itsread,	nodev,          /*52*/
	itsioctl,	nodev,		nodev,		0,
	seltrue,	nodev,		0,
};
int	nchrdev = sizeof (cdevsw) / sizeof (cdevsw[0]);

int	mem_no = 3;	/* major device number of memory special file */

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(4, 0);
