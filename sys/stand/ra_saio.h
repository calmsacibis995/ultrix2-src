
/*
 * SCCSID: @(#)ra_saio.h	1.2	6/28/85
 *
 * Common definitions for all standalone programs
 * that must deal with RA type disks.
 *
 * Fred Canter 2/5/84
 *
 * Modification history:
 *
 *	14-Jun-85 -- map
 *		Add BDA50 and RQDX3. Change QDA25 to 17.
*/

#define	UDA50	0
#define	KLESI	1
#define	RUX1	2
#define	UDA50A	6
#define	RQDX1	7
#define	QDA50	13
#define	QDA25	17
#define	BDA50	18
#define	RQDX3	19

#define	RC25	25
#define	RX50	50
#define	RD51	51
#define	RD52	52
#define RD53	53
#define	RA60	60
#define	RA80	80
#define	RA81	81

#define	RA_MAS	1000	/* RA60/RA80/RA81 maintenance area size */
#define	RC_MAS	102	/* RC25 maintenance area size */
#define	RD_MAS	32	/* RD51/RD52 maintenance area size */

struct	ra_drv {		/* RA drive information */
	int	ra_mediaid;	/* media type of disk */
	char	ra_online;	/* Drive on-line flag */
	char	ra_rbns;	/* Number of replacement blocks per track */
	char	ra_ncopy;	/* Number of RCT copies */
	int	ra_rctsz;	/* Number of blocks in each RCT copy */
	int	ra_trksz;	/* Number of LBNs per track */
	daddr_t	ra_dsize;	/* Unit size in LBNs */
};


#define MAXCTRL	3	/* maximum number of MSCP controllers per UBA */
#define MAXDRIVE 8	/* maximum number of drives on an MSCP controller */
