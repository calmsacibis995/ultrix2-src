/*	@(#)netisr.h	1.2	Ultrix	12/10/84	*/

/*
 * The networking code runs off software interrupts.
 *
 * You can switch into the network by doing splnet() and return by splx().
 * The software interrupt level for the network is higher than the software
 * level for the clock (so you can enter the network in routines called
 * at timeout time).
 */
#ifdef vax
#define	setsoftnet()	mtpr(SIRR, 12)
#endif

/*
 * Each ``pup-level-1'' input queue has a bit in a ``netisr'' status
 * word which is used to de-multiplex a single software
 * interrupt used for scheduling the network code to calls
 * on the lowest level routine of each protocol.
 */
#define	NETISR_RAW	0		/* same as AF_UNSPEC */
#define	NETISR_IP	2		/* same as AF_INET */
#define	NETISR_NS	6		/* same as AF_NS */
#define	NETISR_ND	7		/* network disk protocol */
#define NETISR_DN	12		/* same as AF_DECnet */

#define	schednetisr(anisr)	{ netisr |= 1<<(anisr); setsoftnet(); }

#ifndef LOCORE
#ifdef KERNEL
int	netisr;				/* scheduling bits for network */
#endif
#endif
