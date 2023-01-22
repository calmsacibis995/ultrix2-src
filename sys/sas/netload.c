/*
 * netload.c
 */
#ifndef lint
static char *sccsid = "@(#)netload.c	1.4	ULTRIX	10/3/86";
#endif lint
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
#include "vmb.h"
#include "../h/param.h"

extern	struct	vmb_info *vmbinfo;

/*
 * Allocate and Initialize param_buf as follows
 *
 *	param_buf.pad = 1;
 *	param_buf.prot = 0x160;
 *
 * Start with a network broadcast address 
 *	0x0000010000ab 
 * 
 *	param_buf.dest[0] = 0xab; 
 *	param_buf.dest[1] = 0x00; 
 *	param_buf.dest[2] = 0x00; 
 *	param_buf.dest[3] = 0x01; 
 *	param_buf.dest[4] = 0x00; 
 *	param_buf.dest[5] = 0x00;
 */
struct {
	u_short	pad;
	u_short	prot;
	u_char	dest[6];
} param_buf = {1, 0x160, 0xab, 0, 0, 1, 0, 0};

struct prog_code {
unsigned int	code:8,
		devtype:8,
#define loadnum devtype
		mopver:8,
#define error mopver
		pgmtyp:8;
unsigned int	swid_form:8,
		proc:8,
		rbufsz_param:16;
unsigned int	sz_field:8,
		rcvbufsz:16,
		:8;
	char	x[500];
};

#define RCV_BUF_SZ 1024

struct recv {
	u_char code;
	u_char loadnum;
	u_char loadaddr[4];
	u_char data[RCV_BUF_SZ];
};

struct bufs {
	u_char h[16];
	struct prog_code x_buf;
	struct recv rcvmsg;
};

int	rcv_cnt;

int	DEBUG=0;

main ()
{
	int (*start)();
	int b_sz;
	int i;

	printf("\nUltrixload (using VMB version %d)\n\n",
		vmbinfo->vmbvers);
	if (DEBUG) {
		printf("DEBUG is enabled.\n");
		printf("`*' means that 25 ~1K byte packets have been loaded\n");
		printf("`R' means that a read error occurred\n");
		printf("`W' means that a write error occurred\n");
		printf("`S' means that the packet rcvd was not the one asked for\n\n");
		printf("All errors are retried\n\n");
	}
	printf("Requesting operating system ...\n");
	for (i = 0; i < 2000000; i++); 	/* Give the host a breather */
	b_sz = RCV_BUF_SZ;
	start = (int(*)()) upload(PGMTYP_SECONDARY, 0, b_sz);
	if (start < 0) {
		printf("Unrecoverable network failure\n");
		stop();
	}
	for (i = 0; i < 2000000; i++); 	/* Give the host a breather */
	printf("Requesting network parameter file ...\n");
	i = upload(PGMTYP_TERTIARY, &vmbinfo->netblk, b_sz);
	if (i < 0) {
		printf("Network parameter file load failed.\n");
		printf("Continuing without network information.\n");
	        for (i = 0; i < 5000000; i++); 	/* Give the user a chance to read this */
	}
	(*start)(vmbinfo);
	stop();
}

upload (prog, addr, bufsz)
int	prog, addr, bufsz;
{
	struct bufs *buffers = (struct bufs *)((char *)vmbinfo-8192);
	int p_len, i, j=0;
	int tot_cnt, wrt_cnt, retry=5;
	int ldnum=1;
	int done=0;
	char *ptr;

	bufsz += 6;			/* allow for header information */
	buffers->rcvmsg.code = 0;
	drvinit();			/* re-init the driver */
	for (;;) {
		switch (buffers->rcvmsg.code) {
		case MEMLD_CODE:
			if (buffers->rcvmsg.loadnum != (u_char)((ldnum - 1) & 0xff)) {
				if (DEBUG)
					printf("S");
				goto rewrite;
			}
			/*
			 * rcv_cnt filled in by the read_net routine
			 * as returned by the VAXstar VMB driver is not
			 * reliable.   Therefore, we must assume the 
			 * maximum size until the ROM driver is fixed.
			 */
			rcv_cnt = RCV_BUF_SZ;
			if (prog == PGMTYP_TERTIARY && rcv_cnt != sizeof vmbinfo->netblk) {
			/*
			 * TURN THIS BACK ON WHEN THE VAXSTAR
			 * ROM IS FIXED TO RETURN THE PROPER
			 * COUNT.
				printf("\nWARNING - Size of network parameter file incorrect.\n");
				printf("            bytes expected = %d, bytes received = %d\n",
					sizeof vmbinfo->netblk, rcv_cnt);
			 */
				rcv_cnt = sizeof vmbinfo->netblk;
			}
			bcopy(buffers->rcvmsg.data,
				addr + *(int *)&buffers->rcvmsg.loadaddr[0],
				rcv_cnt);
			/* print about every 25k bytes */
			if (j++ == 25){
				printf("*");
				j=0;
			}
		case PARAM_CODE:
			buffers->x_buf.code = REQ_LOAD_CODE;
			buffers->x_buf.loadnum = ldnum++;
			if (ldnum > 255) ldnum =0;
			buffers->x_buf.error = 0;
			wrt_cnt = 3;
			break;
		case VOLASS_CODE:
			printf("    Loading from host at address ");
			for (i=0; i<6; i++)
				printf("%x%c", param_buf.dest[i], i<5 ? ':' : '\n');
		default:
			buffers->x_buf.code = REQ_PROG_CODE;
			buffers->x_buf.devtype = NET_QNA;
			buffers->x_buf.mopver = MOP_VERSION;
			buffers->x_buf.pgmtyp = prog;
			buffers->x_buf.swid_form = -1; /* force sys load */
			buffers->x_buf.proc = SYSTEMPROC;
			buffers->x_buf.rbufsz_param = XTRA_BUFSZ;
			buffers->x_buf.sz_field = 2;
			buffers->x_buf.rcvbufsz = bufsz;
			wrt_cnt = sizeof (struct prog_code);
			break;
		}
rewrite:
		while (retry--)
			if (write_net(wrt_cnt,&buffers->x_buf,retry))
				break;
		if (!retry) 	/* if we ran out of retries */
			goto error;
		if (buffers->rcvmsg.code == PARAM_CODE) {
			printf("\n");
			ptr = (char *) &buffers->rcvmsg;
			i = 2;
			while (!done) {
				switch (ptr[i++]) {
				case TRGNAME:
				case TRGADDR:
				case HSTNAME:
				case HSTADDR:
				case HSTTIME:
					p_len = ptr[i++];
					i+=p_len;
					break;
				case ENDMRK:
					done++;
					continue;
				}
			}
			disconnect();		/* shutdown the link */
			return (*(int *)&ptr[i]);
		}
		if (read_net(bufsz,&buffers->rcvmsg,retry) == 0) {
			if (retry--)
				continue;	/* retry */
			goto error;
		}
		retry=5;
	}
error:
	disconnect();			/* shutdown the link */
	return (-1);
}

write_net(size, addr, try)
int	size, addr, try;
{
	int qio_status;

	qio_status = qio(PHYSMODE,IO$_WRITELBLK,&param_buf,size,addr);
	if (qio_status & 1)
		return(1);
	if (DEBUG)
		printf("W");
	else if (!try) 		/* if this is the last try then print */
		printf("write error, %s\n", geterr(qio_status));
	return(0);
}

read_net(size, addr, try)
int	size, addr, try;
{
	int qio_status;

	qio_status = qio(PHYSMODE,IO$_READLBLK,&param_buf,size,addr);
	if (qio_status & 1) {
		rcv_cnt = (qio_status >> 16) - 6;
		return(1);
	}
	if (DEBUG)
		printf("R");
	else if (!try) 		/* if this is the last try then print */
		printf("read error, %s\n", geterr(qio_status));
	return(0);
}
