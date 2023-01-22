/* 	@(#)nm_defines.h	1.1	(ULTRIX)	9/30/85	*/

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

/*  STATE values  */
#define nm_ON				0		
#define nm_OFF				1
#define nm_SERVICE			2
#define nm_SHUT				2			
#define nm_HOLD				2		
#define nm_CLEARED			3
#define nm_RESTRICTED		3
#define nm_REACHABLE 		4
#define nm_UNREACHABLE		5


/*  SUBSTATE, SERVICE SUBSTATE values  */
#define nm_STARTING			0			
#define nm_REFLECTING		1
#define nm_LOOPING			2
#define nm_LOADING			3
#define nm_DUMPING			4
#define nm_TRIGGERING		5
#define nm_AUTOSERVICE		6
#define nm_AUTOLOADING		7
#define nm_AUTODUMPING		8
#define nm_AUTOTRIGGERING	9
#define nm_SYNCHRONIZING	10
#define nm_FAILED			11


/*  SERVICE, SURVEILLANCE, ASSISTANCE, BLOCKING values  */
#define nm_ENABLED			0		
#define nm_LOAD				0
#define nm_DISABLED			1			
#define nm_DUMP				1


/*  TYPE, PROTOCOL values  */
#define nm_DDCMP_POINT		0			 
#define nm_ROUTINE_III		0
#define nm_DDCMP_CONTROL	1
#define nm_NONROUTING_III	1
#define nm_BILATERAL		1
#define nm_DDCMP_TRIBUTARY	2
#define nm_X25				3
#define nm_TYPE_AREA		3			/*  AREA is ENTITY defined as 5  */
#define nm_DDCMP_DMC		4
#define nm_ROUTING_IV		4
#define nm_NONROUTING_IV	5
#define nm_LAPB				5
#define nm_ETHERNET			6
#define nm_CI				7
#define nm_QP2				8
#define nm_BISYNC			9


/*  DUPLEX values  */
#define nm_FULL				0			
#define nm_HALF				1


/*  CONTROLLER values  */
#define nm_NORMAL			0			
#define nm_LOOPBACK			1


/*  CLOCK values  */
#define nm_EXTERNAL			0			
#define nm_INTERNAL 		1


/*  LOOP WITH values  */
#define nm_ZEROES			0			
#define nm_ONES				1		
#define nm_MIXED			2


/*  LOOP HELP values  */
#define nm_TRANSMIT			0			
#define nm_RECEIVE			1
#define nm_LOOP_FULL		2				/*  FULL defined in DUPLEX as 0 */


/*  Entity types  */
#define nm_NODE				0			
#define nm_LINE				1
#define nm_LOGGING			2
#define nm_CIRCUIT			3
#define nm_MODULE			4
#define nm_AREA				5
#define nm_EXECUTOR			6				/*  pseudo-entity */
#define nm_OBJECT			7				/* system specific entity */

/*  POLLING STATE values  */
#define nm_AUTOMATIC		0			
#define nm_ACTIVE			1
#define nm_INACTIVE			2
#define nm_DYING			3
#define nm_DEAD				4


/*  USAGE values  */
#define nm_PERMANENT		0			
#define nm_INCOMING			1
#define nm_OUTGOING			2


/*  CPU values  */
#define nm_PDP8				0			
#define nm_PDP11			1
#define nm_DECSYSTEM1020	2
#define nm_VAX				3


/*  SOFTWARE TYPE values  */
#define nm_SECOND_LOADER 	0		
#define nm_TERT_LOADER		1
#define nm_SYSTEM			2


/*  SERVICE NODE VERSION values  */
#define nm_PHASE_III		0		
#define nm_PHASE_IV			1


/*  OPERATION values  */
#define nm_INITIATED		0			
#define nm_TERMINATED		1

/*  Information Types */
#define nm_SUMMARY			0
#define nm_STATUS			1
#define nm_CHARACTERISTICS	2
#define nm_COUNTERS			3
#define nm_EVENTS			4

/*  Functions  */
#define nm_f_LOAD			15
#define nm_f_DUMP			16
#define nm_TRIGGER			17
#define nm_TEST				18
#define nm_CHANGE			19
#define nm_READ				20
#define nm_ZERO				21
#define nm_SYS_SPEC			22
#define nm_CLEAR			25
#define nm_READ_ZERO		26

/*  Files  */
#define nm_PERMANENT_DB		0
#define nm_LOAD_FILE		1
#define nm_DUMP_FILE		2
#define nm_SECOND_LD		3
#define nm_TERT_LD			4
#define nm_SECOND_DMP		5
#define nm_VOLATILE			6
#define nm_DIAGNOSTIC	 	7

/*  Action routine return values.  */
#define nm_SUCCESS			1
#define nm_FAIL				0

/* NICE Return Codes */
#define nm_SUC_COMPL		1
#define	nm_SUC_SEPDATA		2
#define	nm_SUC_PARTREP		3
#define	nm_DONE				-128
#define nm_NOMORE			0			/*  Mult Resp Indicator from kernel */
#define nm_MORE				1			/*  Mult Resp Indicator from kernel */

/*  Entity identifiers.  */
#define nm_KNOWN			-1
#define nm_ACTIVE_ID		-2
#define nm_LOOP_NODE		-3	
#define nm_ADJ_NODE			-4
#define nm_SIGNIFICANT		-5	


/*  Maximum values  */

#define nm_MAXETHER			6
#define nm_MAXNODES			10
#define nm_MAXNNAME			6
#define nm_MAXIDLEN			32
#define nm_MAXLINE			16
#define nm_MAXOBJNAME			16
#define nm_MAXNICEMSG		300
#define nm_MAXNAME			40
#define nm_MAXERRTXT		255	
#define nm_MAXFIELD			255
#define nm_MAXACCDATA		39

#define nm_MAXDEV			2
#define nm_SYSADDRSIZE		2
#define nm_VERSION_SIZE		3
#define nm_ACCDATA			39

/*  IPC Mechanism  */
#define nm_LOCAL			0
#define nm_REMOTE			1

/*  Flags  */
#define nm_SET_EXEC_BIT		128
#define nm_EXEC_MASK		0200	



/*  Coded Multiple Types  */

#define nm_CM_NODE			0
#define nm_CM_USER			1
#define nm_CM_VERSION			2
#define nm_CM_SUBADDRESSES		3
#define nm_CM_EVENTS			4
#define nm_CM_OBJECT			5
#define nm_CM_NODE_ID			6

/*  System Types  */

#define nm_RSTS				1
#define	nm_RSX				2
#define nm_TOPS				3
#define nm_VMS				4
#define nm_RT				5
#define nm_CT				6
#define nm_COMM_SER			7
#define nm_ULTRIX			8
#define nm_MSDOS			9

/*  Socket type for object */

#define nm_SEQPACK			0
#define nm_STREAM			1

/*  Connection accept  */

#define nm_ACC_IMMED		0
#define nm_ACC_DEFER		1

