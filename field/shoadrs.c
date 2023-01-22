#ifndef lint
static	char	*sccsid = "@(#)shoadrs.c	1.1	ULTRIX	1/29/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
 /*
  * Modification History
  *
  * 05-Jan-1987   Created original program. swc
  *
  */

#include <stdio.h>
#include <strings.h>

#define USR_LIST 50		/* allowable length of user input list */

int	x,y;		/* index for device table and user list */
unsigned int	adrs_base;	/* floating CSR address */
unsigned int	vec_base;	/* floating vector address */
unsigned int	found_one;	/* flag that a users device is found */

/*
 * The following is the device table. It is based on the current VMS table
 * in the Guide To Device Driver Writing manual. All devices with floating
 * CSR and vector address MUST be in the table regardless of support status. 
 * All numbers MUST be in octal and the 'end' entry MUST be last. Devices which
 * Ultrix-32 does not support MUST have a "" name field. Bit '1' is set in the 
 * flag field of duplicate device names. This is quick/dirty kludge so I
 * didn't have to code a bunch of stuff in list_devices() so it wouldn't
 * print duplicate names. Bit '2' of the flags field is used/set by the
 * program to show a fixed address device has been used so that we will pick
 * the next one if multiple ones have been specified. There can be more than
 * one of some fixed address devices. The 'max_num' field defines how many are
 * allowed. Some of the 'num_regs' fields have a negative number, this is
 * because the device addressing assignment goes backwards from the first one.
 */

struct d {
	char	ctrl[10];  /* name of controller */
	char	name[10];  /*Ultrix man(4) device name,if "" then unsupported*/
	unsigned int	vector; /* 0 if floating, actual vector if fixed */
	unsigned int	num_vect; /* the number of vectors per device */
	unsigned int	vec_align; /* vector alignment boundary */
	unsigned int	csr_adrs; /* 0 if floating, starting adrs if fixed */
	int		num_regs; /* number of registers per device */
	unsigned int	flags;   /* don't print and used flags */
	unsigned int	max_num; /* maximum number of fixed adrs devices */
/*
 *	ctrl     name    vec  #vec align   csr adrs   #reg  flags   maximun
 */
} devtab[] = 
{
	"cr11",  "",     0230, 0,    0,      0777160,    0,    0,	0,
	"rk711", "hk",   0210, 0,    0,      0777440,    0,    0,	0,
	"lp11",  "lp",   0200, 0,    0,      0777514,    0,    0,	0,
	"lp11",  "lp",   0170, 0,    0,      0764004,    0,    01,	0,
	"lp11",  "lp",   0174, 0,    0,      0764014,    0,    01,	0,
	"lp11",  "lp",   0270, 0,    0,      0764024,    0,    01,	0,
	"lp11",  "lp",   0274, 0,    0,      0764034,    0,    01,	0,
	"ts11",  "zs",   0224, 0,    0,      0772520,    0,    0,	0,
	"rx211", "rx",   0264, 0,    0,      0777170,    0,    0,	0,
	"rb730", "idc",  0250, 0,    0,      0775606,    0,    0,	0,
	"uda50", "uq",   0154, 0,    0,      0772150,    0,    0,	0,
	"tu81",  "tmscp",0260, 0,    0,      0774500,    0,    0,	0,
	"deuna", "de",   0120, 0,    0,      0774510,    0,    0,	0,
	"deqna", "de",   0120, 0,    0,      0774440,    0,    0,	0,
	"qvss",  "qv",   060,  0,    0,      0777200,    0,    0,	0,
	"dr11b", "",     0124, 0,    0,      0772410,    0,    0,	0,
	"dc11",  "",     0,    02,   010,    0774000,    04,   0,	040,
	"tu58",  "",     0,    02,   010,    0776500,    04,   0,	020,
	"dp11",  "",     0,    02,   010,    0774400,    -04,  0,	040,
	"dn11",  "",     0,    01,   04,     0775200,    04,   0,	020,
	"dm11b", "",     0,    01,   04,     0770500,    04,   0,	020,
	"dr11c", "",     0,    02,   010,    0767600,    -04,  0,	020,
	"pr611", "",     0,    01,   010,    0772600,    02,   0,	010,
	"pp611", "",     0,    01,   010,    0772700,    02,   0,	010,
	"dt11",  "",     0,    02,   010,    0777420,    01,   0,	010,
	"dx11",  "",     0,    02,   010,    0776200,    020,  0,	02,
	"dl11c", "",     0,    02,   010,    0775610,    04,   0,	037,
	"dj11",	 "",     0,    02,   010,    0,          04,   0,	0,
	"dh11",	 "",     0,    02,   010,    0,          010,  0,	0,
	"gt40",  "",     0,    04,   010,    0772000,    04,   0,	02,
	"lps11", "",     0,    06,   010,    0770400,    0,    0,	0,
	"dq11",	 "",     0,    02,   010,    0,          04,   0,	0,
	"kw11w", "",     0,    02,   010,    0772400,    0,    0,	0,
	"du11",	 "",     0,    02,   010,    0,          04,   0,	0,
	"dup11", "dup",  0,    02,   010,    0,          04,   0,	0,
	"dv11",  "",     0,    03,   010,    0775000,    020,  0,	04,
	"lk11",	 "",     0,    02,   010,    0,          04,   0,	0,
	"dmc11", "dmc",  0,    02,   010,    0,          04,   0,	0,
	"dz11",	 "dz",   0,    02,   010,    0,          04,   0,	0,
	"kmc11", "",     0,    02,   010,    0,          04,   0,	0,
	"lps11", "",     0,    02,   010,    0,          04,   0,	0,
	"vmv21", "",     0,    02,   010,    0,          04,   0,	0,
	"vmv31", "",     0,    02,   010,    0,          010,  0,	0,
	"dwr70", "",     0,    02,   010,    0,          04,   0,	0,
	"rl11",	 "hl",   0,    01,   04,     0,          04,   0,	0,
	"ts11",  "zs",   0,    01,   04,     0772524,    02,   01,	03,
	"lpa11", "",     0,    02,   010,    0770460,    010,  0,	0,
	"lpa11", "",     0,    02,   010,    0,          010,  01,	0,
	"kw11c", "",     0,    02,   010,    0,          04,   0,	0,
	"rsv",	 "",     0,    01,   010,    0,          04,   01,	0,
	"rx211", "rx",   0,    01,   04,     0,          04,   01,	0,
	"dr11w", "",     0,    01,   04,     0,          04,   0,	0,
	"dr11b", "",     0,    01,   04,     0772430,    0,    01,	0,
	"dr11b", "",     0,    01,   04,     0,          04,   01,	0,
	"dmp11", "",     0,    02,   010,    0,          04,   0,	0,
	"dpv11", "dpv",  0,    02,   010,    0,          04,   0,	0,
	"isb11", "",     0,    02,   010,    0,          04,   0,	0,
	"dmv11", "dmv",  0,    02,   010,    0,          010,  0,	0,
	"deuna", "de",   0,    01,   04,    0,          04,   01,	0,
	"uda50", "uq",   0,    01,   04,     0,          02,   01,	0,
	"dmf32", "dmf",  0,    010,  04,     0,          020,  0,	0,
	"kms11", "",     0,    03,   010,    0,          010,  0,	0,
	"pcl11b","",     0,    02,   010,    0764200,    020,  0,	04,
	"vs100", "",     0,    01,   04,     0,          010,  0,	0,
	"tu81",  "tmscp",0,    01,   04,     0,          02,   01,	0,
	"kmv11", "",     0,    02,   010,    0,          010,  0,	0,
	"kct32", "",     0,    02,   010,    0764400,    020,  0,	04,
	"ieq11", "",     0,    02,   010,    0764100,    0,    0,	0,
	"dhv11", "dhv",  0,    02,   010,    0,          010,  0,	0,
	"dmz32", "dmz",  0,    06,   04,    0,          020,  0,	0,
	"cpi32s","",     0,    06,   04,     0,          020,  0,	0,
	"qvss",  "qv",   0,    02,   010,    0,          040,  01,	0,
	"vs31",  "",     0,    01,   04,     0,          04,   0,	0,
	"lnv11", "",     0,    01,   04,     0776200,    0,    0,	0,
	"qpss",  "",     0,    01,   04,     0,          010,  0,	0,
	"dtqna", "",     0,    01,   04,     0772570,    0,    0,	0,
	"dtqna", "",     0,    01,   04,     0,          0,    0,	0,
	"dsv11", "",     0,    01,   04,     0,          04,   0,	0,
	"qsam",  "",     0,    02,   010,    0,          04,   0,	0,
	"adv11c","",     0,    02,   010,    0,          04,   0,	0,
	"aav11c","",     0,    0,    0,      0,          04,   0,	0,
	"axv11c","",     0,    02,   010,    0,          04,   0,	0,
	"kwv11c","",     0,    02,   010,    0,          02,   0,	0,
	"adv11d","",     0,    02,   010,    0,          04,   0,	0,
	"aav11d","",     0,    02,   010,    0,          04,   0,	0,
	"qdss",  "qd",   0,    03,   010,    0777400,    01,   0,	02,
	"drv11j","",     0,    020,  04,     0764120,    -010, 0,	03,
	"drq3b", "",     0,    02,   010,    0,          010,  0,	0,
	"vsv24", "",     0,    01,   04,     0,          04,   0,	0,
	"vsv21", "",     0,    01,   04,     0,          04,   0,       0,
	"ibq01", "",     0,    01,   04,     0,          02,   0,       0,
	"end",   "",     0,    0,    0,      0,          0,    01,	0,
};

/*
 * This is the table that will hold the devices list input by the user.
 */

struct n {
	char	uctrl[10];  /* device name, should match one in devtab */
	char    alias_ctrl[10]; /* see description of aliastab */
	char    alias_name[10]; /* see description of aliastab */
	int	how_many; /* number of devices to configure */
} usr_list[USR_LIST];



/*
 * This table contains alias names for devices and is used by usr_input().
 * Some devices are considered to have equivalent names and are treated the 
 * same when assigning CSR and Vector address. This allows more freedom of user 
 * input and does not require the user to know what device names go together. 
 * The last entry in the list must always be "end". Devices in the list can 
 * be in any order. If the alias device is NOT supported but the devtab
 * device IS then the .dtab_name field is set to "nospt"
 */

struct a {
	char	alias_name[10];
	char	dtab_ctrl[10];
	char	dtab_name[10];
} aliastab[] =
{
/*	alias name	devtab ctrl	devtab name */

	"tk50",		"tu81",		    "",
	"tu80",		"ts11",		    "",
	"dzv11",	"dz11",		    "",
	"dzq11",	"dz11",		    "",
	"dz32",		"dz11",		    "",
	"dhu11",	"dhv11",	    "dhu",
	"dhq11",	"dhv11",	    "dhq",
	"delua",	"deuna",	    "",
	"delqa",	"deqna",	    "",
	"dmr11",	"dmc11",	    "",
	"duv11",        "du11",             "",
	"dlv11e",       "dl11c",            "",
	"dl11e",	"dl11c",            "",
	"dl11d",        "dl11c",            "",
	"lpv11",        "lp11",             "nospt",
	"kda50",	"uda50",	    "",
	"rqdx1",        "uda50",	    "",
	"rqdx2",        "uda50",	    "",
	"rqdx3",        "uda50",	    "",
	"vcb01",        "qvss",             "",
	"vcb02",        "qdss",             "",
	"dt07",         "dt11",             "",
	"vsv11",        "gt40",             "",
	"cpi32a",       "dmz32",            "npspt",
	"drv11b",       "dr11b",            "",
	"drv11c",       "dr11c",            "",
	"end",		"end",	    	    "",
};


/*
 * Some of the 'aliastab' devices that have alias/equivalent names
 * by convention also have an ordering sequence for assigning addresses.
 * For example the DZ11/DZ32 have equivalent names but the DZ11 is
 * assigned an address before (lower numerically) the DZ32.
 * This table maps such devices and is used in usr_input(). The last
 * entry must be "end".
 */

struct o {
	char	first[10];
	char	second[10];
} ordertab[] =
{
/*	first device	second device  */

	    "dz11",         "dz32",
	    "dmc11",        "dmr11",
	    "end",          "end",
};



/***************************************************************/
main (argc, argv)
int	argc;
char	*argv[];
{



 adrs_base = 0760000; /* This must be initialized to 010 less than the real */
		      /* start address (760010) of floating adress space   */
 vec_base = 0300;


 x = 0;
 y = 0;
 found_one = 0;



 if (argc > 2)  /* do a bunch of stuff to handle command line arguements */
    {
      printf("\nUsage: shoadrs [ -hd ]\n");
      return;
    }
 if (argc == 2)
 {
   if (strcmp(argv[1], "-h") == 0)
    {
     help();
     return;
     }
   if (strcmp(argv[1], "-d") == 0)
    {
     list_devices();
     return;
    }
   if (strcmp(argv[1], "-hd") == 0  ||  strcmp(argv[1], "-dh") == 0)
    {
     help();
     list_devices();
     return;
    }
   else
    {
      printf("\nUsage: shoadrs [ -hd ]\n");
      return;
    }
 }

 printf("\n\t\t Ultrix CSR and Vector Address Analizer\n\n");
 printf("\t\t\tuse -h option for help\n\n");
     

 usr_input();   /* go get the user device list */

 /*
  * The man program body will loop through the usr_list completly for
  * each entry in devtab. If there is more that one of the same given
  * device then we back up 1 in the usr_list and try again, this is so
  * we get multiple devices of the same type. An execption to this
  * is when the first device has a fixed CSR address and the others are
  * floating, in this case we go on to the next devtab entry.
  */
 while ((strcmp(devtab[x].ctrl, "end")) != 0)   /* go once through devtab */
 {						/* look through usr_list */
   while (strcmp(usr_list[y].uctrl, "end") != 0)  /* for each devtab entry */
   {
     if ((strcmp(usr_list[y].uctrl, devtab[x].ctrl) == 0) &&    /* match */
	 ((devtab[x].flags & 02) == 0)  &&	 /* with current device  */
	 (usr_list[y].how_many != 0) )		 /* table entry          */
           {	
	     found_one++;
	     print_info(); 	/* found one , print info */
	     set_gap();		/* increment to next device of same type */
	     y--;		/* pick up multiple devices of same type */
	   }

          y++; 		/* check next usr_list device */

   }

   if (found_one)
      {
        x++;		/* check next controller in device table */
	if (strcmp(devtab[x].ctrl, "end") == 0) /* all done, bye-bye */
	    break;
        set_gap();	/* increment base_adrs to next device, align it */
      }
   else
      {
	if (devtab[x].csr_adrs == 0)  /* this is a kludge because UDA and */
	     adrs_base += 02;         /* TU81 break the rules. This forces */
				      /* an address to be aligned always */

        x++;		/* check next controller in device table */
	if (strcmp(devtab[x].ctrl, "end") == 0) /* all done, bye-bye */
	    break;
	if (devtab[x].csr_adrs == 0 &&    /* do address alignment if needed */
	    devtab[x].num_regs != 0 )
	  if ((adrs_base % (2 * devtab[x].num_regs)) != 0) 
		{
		 set_gap();
		}
        if (devtab[x].vec_align != 0)              
          if (vec_base % devtab[x].vec_align != 0) /* align vector address */
	    vec_base += 04;	         /* between different device types */
      }

   found_one = 0; 	/* reset to try some more */
   y = 0;		/* re-search the usr_list */
 }


}

/* Get the device list from the user. The format MUST be
 * <device name><space><quanity>     (example: dmf32 2). The
 * quanity defaults to 1 if none is entered. The last device 
 * input MUST be 'end' or just a cariage return. EOF is of too.
 */

 usr_input()
 {
   int	i,ii;		/* indexes */
   int  first, second;  /* used for sorting */
   char	*a;             /* user input status result */
   char	ctrlname[10];   /* user input buffer */
   char temp_uctrl[10]; /* temp storage for use in swaping */
   char temp_alias_ctrl[10];
   char temp_alias_name[10];
   int  temp_how_many;


   for (i=0;i < USR_LIST; i++)
   {
     usr_list[i].how_many = 1;	/* set default quanity of 1 */
     fputs("DEVICE: ", stderr); 
     a = gets(ctrlname);
     if (strcmp(ctrlname, "") == 0)  /* just a CR used for end of input */
	strcpy(ctrlname, "end");
     if (a == NULL) 			/* an EOF for end of input */
	{
	 strcpy(ctrlname, "end");
	 printf("\n");		/* an EOF would leave you on same line */
	}
     sscanf(ctrlname,"%s%d",usr_list[i].uctrl, &usr_list[i].how_many);

     /* Print_info() always prints from '.alias_ctrl' field so copy inputed
      * name to there. This may then be changed when we check for alias ctrl
      * names. Also init the '.alias_name' field to null so print_info()
      * knows to use a differet one than what is in devtab.
      */
     strcpy(usr_list[i].alias_ctrl, usr_list[i].uctrl);
     strcpy(usr_list[i].alias_name, "");

     if (strcmp(ctrlname, "end") == 0)   /* user input all done */
	  break;


     /* Search through aliastab for each ctrl name as it is entered. If 
      * alias is found then substitute the devtab name to base all 
      * the searching and address figuring on.
      */

     ii = 0;
     while (strcmp(aliastab[ii].alias_name, "end") != 0)
     {
       if (strcmp(usr_list[i].uctrl, aliastab[ii].alias_name) == 0)
	  {
	    strcpy(usr_list[i].uctrl, aliastab[ii].dtab_ctrl);
	    strcpy(usr_list[i].alias_name, aliastab[ii].dtab_name);
	    break;
	  }
	ii++;
      }

   }
   if ( i == USR_LIST)   			/* if user input overflow */
     strcpy(usr_list[i - 1].uctrl, "end");      /* enforce a legal end mark */

   /*
    * Devices that have alias/equivalent names are treated equally when
    * assigning CSR and Vector Addresses. However some are a bit more
    * equal than others and by convention must get the lower numbered 
    * address, for example DZ11/DZ32 are equivalent names but the DZ11 
    * is addressed before the DZ32. So we must order devices in the
    * usr_list accordingly. Also see 'ordertab' structure.
    */

    i = 0;
    while (strcmp(ordertab[i].first, "end") != 0)
    {
      ii = 0;
      first = -1;
      second = -1;
      while (strcmp(usr_list[ii].uctrl, "end") != 0)
      {
	if (strcmp(usr_list[ii].alias_ctrl, ordertab[i].first))
	   first = ii;
	if (strcmp(usr_list[ii].alias_ctrl, ordertab[i].second))
	   second = ii;
        if ( first >= 0  &&  second >= 0  &&  first < second ) 
          {
	    /* swap things around */
	    strcpy(temp_uctrl, usr_list[first].uctrl);
	    strcpy(temp_alias_ctrl, usr_list[first].alias_ctrl);
	    strcpy(temp_alias_name, usr_list[first].alias_name);
	    temp_how_many = usr_list[first].how_many;

	    strcpy(usr_list[first].uctrl, usr_list[second].uctrl);
	    strcpy(usr_list[first].alias_ctrl, usr_list[second].alias_ctrl);
	    strcpy(usr_list[first].alias_name, usr_list[second].alias_name);
	    usr_list[first].how_many = usr_list[second].how_many;

	    strcpy(usr_list[second].uctrl, temp_uctrl);
	    strcpy(usr_list[second].alias_ctrl, temp_alias_ctrl);
	    strcpy(usr_list[second].alias_name, temp_alias_name);
	    usr_list[second].how_many = temp_how_many;

	    ii = 0;		/* check it all over again */
	    first = -1;
	    second = -1;
          }
        else		/* no swaps needed, keep looking */
	  ii++;
      }
      i++;	/* check next ordertab entry */
    }


   return;
 }
  
 print_info()  /* print CSR and vector address info */
 {

   /* If this is a device that can have multiple fixed address than it 
    * will have floating vector address. These type devices MUST have a
    * maximum limit on the number allowed, enforce that limit if the user
    * entered a greater number.
    */
   if (devtab[x].csr_adrs != 0  &&  
       devtab[x].vector == 0    &&
       usr_list[y].how_many > devtab[x].max_num)
            usr_list[y].how_many = devtab[x].max_num;

   if (usr_list[y].how_many > 0)  /* decrement the number left to go */
       --usr_list[y].how_many ;


   printf("Device: %s\t",usr_list[y].alias_ctrl);
   /* if the device is not supported the .name field = "" or the alias_name
    * field = "nospt"
    */
   if (strcmp(devtab[x].name, "") != 0  && 
       strcmp(usr_list[y].alias_name, "nospt") != 0)
      if (strcmp(usr_list[y].alias_name, "") != 0)
         printf("Name: %s\t",usr_list[y].alias_name);
      else
         printf("Name: %s\t",devtab[x].name);
   else
      printf("Name:    \t");
   if (devtab[x].csr_adrs != 0)
      {
        printf("CSR: %o\t",devtab[x].csr_adrs);       /* fixed address */
	if (devtab[x].vector != 0)
	      devtab[x].flags = (devtab[x].flags | 02);  /* flag as used */
      }
   else
      {
	printf("CSR: %o*\t", adrs_base);		/* floating adrs */
      }
   if (devtab[x].vector != 0)
      printf("Vector: %o\t", devtab[x].vector);         /* fixed vector */
   else
      {
        printf("Vector: %o*\t", vec_base);		/* floating vector*/
        vec_base += (4 * devtab[x].num_vect);    /* next vector adrs */
      }
   if (strcmp(devtab[x].name, "") == 0  || 
       strcmp(usr_list[y].alias_name, "nospt") == 0)
         printf("Support: no\n");
   else
       printf("Support: yes\n");
 }

 set_gap()	/* put the appropriate address gap between devices */
 {
   unsigned int	align_error;

   if (devtab[x].csr_adrs == 0) /* floating address device */
       {
   	align_error =  adrs_base % (2 * devtab[x].num_regs);
   	adrs_base += (2 * devtab[x].num_regs) - align_error;
       }
   else
      devtab[x].csr_adrs += (2 * devtab[x].num_regs); /* fixed adrs device */
 }

 help()		/* do stuff if user wants help */
 {

   printf("\n\n  	Ultrix-32 Unibus/QBus CSR and Vector Address Analyzer\n\n");
   printf("This utility will display the CSR and Vector address that Unibus/QBus device\n");
   printf("hardware should be set to for the Ultrix-32 bus autosizing routine to work\n");
   printf("correctly with. This must be done separately for each Unibus/Qbus in the system.\n\n");

   printf("NOTE: Combinations and number of devices are subject to limitations\n"); 
   printf("      such as bandwidth, physical configuration restraints, thermal\n");
   printf("      dissipation, electrical loads/power and software limitations.\n");
   printf("      Reference the Software Product Description (SPD) for more detail.\n\n");

   printf("Respond to the DEVICE: prompt with a device name and quanity.\n");
   printf("Enter 'end', CTRL-D (^D) or just a carriage return as the last device name.\n\n");
   printf("Example:         DEVICE: dmf32 2\n");
   printf("                 DEVICE: end\n\n");
   printf("If no quanity is specified the default is 1.\n\n");
   printf("The maximum number of device types you can input is 50.\n\n");
   printf("Use the -d option to see a list of valid device names.\n\n");
 }

 list_devices()
 {
   int	i,y;

   printf("\n\n  	Ultrix-32 Unibus/QBus CSR and Vector Address Analyzer\n\n");
   printf("\n\n             The following is a list of valid device names:\n\n");

   /* print names in devtab */
   i = 0;
   y = 0;
   while (strcmp(devtab[i].ctrl, "end") != 0)
   {
     if ((devtab[i].flags & 01)  == 0)
     {
       printf("%s\t",devtab[i].ctrl);
       y++;
     }
     i++;
     if ( y == 9)
       {
	printf("\n");
	y =0;
       }
    }

   /* print names in aliastab */
   i = 0;
   while (strcmp(aliastab[i].alias_name, "end") != 0)
   {
     printf("%s\t",aliastab[i].alias_name);
     y++;
     i++;
     if ( y == 9)
       {
	printf("\n");
	y =0;
       }
    }
    printf("\n");
 }

