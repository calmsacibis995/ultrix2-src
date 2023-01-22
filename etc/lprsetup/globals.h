/* 	 @(#)globals.h	@(#)globals.h	1.1	7/8/86;	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 **********************************************************************/


/***************************************
*  global variables for lprsetup program
***************************************/

char    progname[] = "lprsetup";/* name of program 			*/
char    pnum[LEN];		/* printer number we are working on	*/
char    pname[LEN];		/* printer name		 		*/
char	longname[LEN];		/* printer name and synonyms		*/
char    ptype[LEN];		/* printer type		 		*/
char    symbolname[LEN];	/* symbol just read in; this needs to be*/
				/* large since is also used to hold the	*/
				/* value of the new symbol. 		*/
	/* DEBUG: was char bp[1024]; */
char	jhp[1024];
char    *bp = jhp;		/* return from tgetent	 		*/
char	oldfilter[LEN];		/* print filter before modify		*/

/*
 * This structure holds symbols from the PRINTCAP file.
 * The used flag indicates whether the symbol is in
 * use or not, if so, it changes from NO to YES.
 */

struct table    tab[] =
{
    "af", "/usr/adm/lpacct", STR, NO, 0, 0,
    "br", "1200", INT, NO, 0, 0,
    "cf", "none", STR, NO, 0, 0,
    "df", "none", STR, NO, 0, 0,
    "dn", "/usr/lib/lpd", STR, NO, 0, 0,
    "du", "0", INT, NO, 0, 0,
    "fc", "none", INT, NO, 0, 0,
    "ff", "\\f", STR, NO, 0, 0,
    "fo", "off", BOOL, NO, 0, 0,
    "fs", "none", INT, NO, 0, 0,
    "gf", "none", STR, NO, 0, 0,
    "ic", "off", BOOL, NO, 0, 0,
    "if", "/usr/lib/lpf", STR, NO, 0, 0,
    "lf", "/usr/adm/lperrs", STR, NO, 0, 0,
    "lo", "lock", STR, NO, 0, 0,
    "lp", "/dev/lp", STR, NO, 0, 0,
    "mc", "20", INT, NO, 0, 0,
    "mx", "1000", INT, NO, 0, 0,
    "nf", "none", STR, NO, 0, 0,
    "of", "/usr/lib/lpf",  STR, NO, 0, 0,
    "op", "none", STR, NO, 0, 0,
    "os", "none", STR, NO, 0, 0,
    "pl", "66", INT, NO, 0, 0,
    "pp", "usr/lib/lpdfilters/ln01pp", STR, NO, 0, 0,
    "pw", "132", INT, NO, 0, 0,
    "px", "none", INT, NO, 0, 0,
    "py", "none", INT, NO, 0, 0,
    "rf", "none", STR, NO, 0, 0,
    "rm", "none", STR, NO, 0, 0,
    "rp", "none", STR, NO, 0, 0,
    "rs", "off", BOOL, NO, 0, 0,
    "rw", "off", BOOL, NO, 0, 0,
    "sb", "off", BOOL, NO, 0, 0,
    "sc", "off", BOOL, NO, 0, 0,
    "sd", "/usr/spool/lpd", STR, NO, 0, 0,
    "sf", "off", BOOL, NO, 0, 0,
    "sh", "off", BOOL, NO, 0, 0,
    "st", "none", STR, NO, 0, 0,
    "tf", "none", STR, NO, 0, 0,
    "tr", "none", STR, NO, 0, 0,
    "ts", "none", STR, NO, 0, 0,
    "vf", "none", STR, NO, 0, 0,
    "xc", "none", INT, NO, 0, 0,
    "xf", "none", STR, NO, 0, 0,
    "xs", "none", INT, NO, 0, 0,
    0, 0, 0, 0, 0, 0
};

char h_af[] =
{"\n\
The 'af' parameter is the name of the accounting file used to		\n\
keep track of the number of pages printed by each user for each		\n\
printer.  The name of the accounting file should be unique for		\n\
each printer on your system.						\n\
"};

char h_br[] =
{"\n\
The 'br' parameter specifies the baud rate for the printer.		\n\
The baud rate is dependent upon the printer hardware.			\n\
Consult your printer hardware manual for the correct baud rate.		\n\
"};

char h_cf[] =
{"\n\
The 'cf' parameter specifies the output filter for the                   \n\
cifplot data filter.                                                     \n\
"};

char h_df[] =
{"\n\
The 'df' parameter specifies the output filter for the TeX data 	\n\
filter (DVI format).                                                    \n\
"};

char h_dn[] =
{"\n\
The 'dn' parameter specifies the name of the daemon program to		\n\
invoke each time a print request is made to the printer.  The		\n\
default daemon name is '/usr/lib/lpd', and should not be		\n\
changed.  The 'dn' parameter is available here so that the		\n\
system may support multiple line printer daemons.			\n\
"};

char h_du[] =
{"\n\
The 'du' parameter specifies the daemon UID used by the printer 	\n\
spooler programs.  The default value, (0) should not be changed.	\n\
"};

char h_fc[] =
{"\n\
The 'fc' parameter specifies which terminal flag bits to clear		\n\
when initializing the printer line.  Normally, all of the bits		\n\
should be cleared (fc=0177777 octal) before calling 'fs'.  Refer	\n\
to the discussion of 'sg_flags' on the tty(4) manual page of the	\n\
ULTRIX Programmer's Manual, Vol. 1.					\n\
"};

char h_ff[] =
{"\n\
The 'ff' parameter is the string to send as a form feed to the		\n\
printer.  The default value for this parameter is '\\f'.		\n\
"};

char h_fo[] =
{"\n\
The boolean parameter 'fo' specifies whether a form feed will		\n\
be printed when the device is first opened.  This is in addition	\n\
to the normal form feed which is printed by the driver when the		\n\
device is opened.  To suppress ALL printer induced form feeds,		\n\
use the 'sf' flag, in addition to the 'fo' flag.			\n\
"};

char h_fs[] =
{"\n\
The 'fs' parameter specifies which terminal flag bits to set		\n\
when initializing the printer line.  Normally, all of the bits		\n\
should be cleared (using fc=0177777 octal) and then 'fs' should be	\n\
used to set the specified bits.  Refer to the discussion of		\n\
'sg_flags' on the tty(4) manual page of the ULTRIX Programmer's	        \n\
Manual, Volume 1.							\n\
"};

char h_gf[] =
{"\n\
The 'gf' parameter specifies the graph data filter (plot(3X) format). 	\n\
"};

char h_ic[] =
{"\n\
The 'ic' parameter is the driver that supports (nonstandard)            \n\
ioctl to an independent printout.                                       \n\
"};

char h_if[] =
{"\n\
The 'if' parameter is the name of a text filter which does accounting.	\n\
"};

char h_lf[] =
{"\n\
The 'lf' parameter is the logfile where errors are reported.		\n\
The default logfile, if one is not specified, is '/dev/console'.	\n\
If you have more than one printer on your system, you should give 	\n\
each logfile a unique name.	                                	\n\
"};

char h_lo[] =
{"\n\
The 'lo' parameter is the name of the lock file used by the		\n\
printer daemon to control printing the jobs in each spooling		\n\
directory.  The default value, 'lock', should not be changed.		\n\
"};

char h_lp[] =
{"\n\
The 'lp' parameter is the name of the special file to open for   	\n\
output.  This is normally '/dev/lp' for parallel printers.		\n\
For serial printers, a terminal line (for example, /dev/tty00		\n\
or /dev/tty01) is specified.	                        		\n\
"};

char h_mc[] =
{"\n\
The 'mc' parameter is the maximum number of copies allowed.		\n\
"};

char h_mx[] =
{"\n\
The 'mx' parameter specifies the maximum allowable filesize      	\n\
(in BUFSIZ blocks) printable by each user.  Specifying mx=0     	\n\
removes the filesize restriction entirely, and is equivalent    	\n\
to not specifying 'mx' in /etc/printcap.         			\n\
"};

char h_nf[] =
{"\n\
The 'nf' parameter specifies a ditroff filter. 				\n\
"};

char h_of[] =
{"\n\
The 'of' parameter specifies the output filter to be used with		\n\
the printer.  Filters currently available include:			\n\
\n\
	Filter name:	  		Description:			\n\
	------------	  		------------			\n\
	/usr/lib/lpf	  		line printer filter 		\n\
					   (LP25, LP26, LP27, LG01)	\n\
	/usr/lib/lpf	  		letter quality filter 		\n\
					   (LA210, LQP02, LQP03)	\n\
	/usr/lib/lpdfilters/lqf	  	letter quality filter 		\n\
					   (LQP02, LQP03)		\n\
	/usr/lib/lpdfilters/ln01of   	LN01 Laser Printer filter 	\n\
	/usr/lib/lpdfilters/ln03of   	LN03 Laser Printer filter 	\n\
\n\
"};

char h_op[] =
{"\n\
The 'op' parameter specifies the object port on a LAT terminal server.	\n\
"};

char h_os[] =
{"\n\
The 'os' parameter specifies the object service on a LAT server.	\n\
"};

char h_pl[] =
{"\n\
The 'pl' parameter specifies the page length in lines.  The 		\n\
default page length is 66 lines.					\n\
"};

char h_pp[] =
{"\n\
The 'pp' parameter specifies the print command filter                   \n\
replacement.  Filters currently available include:			\n\
\n\
	Filter name:	  		Description:			\n\
	------------	  		------------			\n\
	/usr/lib/lpdfilters/ln01pp	LN01 Laser Printer filter	\n\
"};

char h_pw[] =
{"\n\
The 'pw' parameter specifies the page width in characters.  The		\n\
default page width is 132 characters, although a page width of		\n\
80 characters is more useful for letter quality printers, whose		\n\
standard paper size is 8 1/2\" x 11\".					\n\
"};

char h_px[] =
{"\n\
The 'px' parameter specifies the page width in pixels.                  \n\
"};

char h_py[] =
{"\n\
The 'py' parameter specifies the page length in pixels.                 \n\
"};

char h_rf[] =
{"\n\
The 'rf' parameter specifies the filter for printing FORTRAN style      \n\
text files.                                                             \n\
"};

char h_rm[] =
{"\n\
The 'rm' parameter specifies the machine name for a remote printer.     \n\
"};

char h_rp[] =
{"\n\
The 'rp' parameter specifies the remote printer name argument.          \n\
"};

char h_rs[] =
{"\n\
The 'rs' parameter restricts the remote users to those with             \n\
local accounts.                                                         \n\
"};

char h_rw[] =
{"\n\
The boolean parameter 'rw' specifies that the printer is to be		\n\
opened for both reading and writing.  Normally, the printer is		\n\
opened for writing only.						\n\
"};

char h_sb[] =
{"\n\
The 'sb' parameter specifies a short banner consisting                  \n\
of one line only.                                                       \n\
"};

char h_sc[] =
{"\n\
The 'sc' parameter suppresses multiple copies.                          \n\
"};

char h_sd[] =
{"\n\
The 'sd' parameter specifies the spooling directory where files		\n\
are queued before they are printed.  Each spooling directory		\n\
should be unique.							\n\
"};

char h_sf[] =
{"\n\
The boolean parameter 'sf' suppresses all printer induced form    	\n\
feeds, except those which are actually in the file.  The 'sf'	        \n\
flag, in conjunction with 'sh', is useful when printing a letter        \n\
on a single sheet of stationery.				        \n\
"};

char h_sh[] =
{"\n\
The boolean parameter 'sh' suppresses printing of the normal	        \n\
burst page header.  This often saves paper, in addition to being        \n\
useful when printing a letter on a single sheet of stationary.	        \n\
"};

char h_st[] =
{"\n\
The 'st' parameter specifies the status file name.                      \n\
"};

char h_tf[] =
{"\n\
The 'tf' parameter specifies a troff data filter.                       \n\
"};

char h_tr[] =
{"\n\
The 'tr' parameter specifies a trailing string to print when	        \n\
the spooling queue empties.  It is generally a series of form    	\n\
feeds, or sometimes an escape sequence, to reset the printer	        \n\
to a known state.						        \n\
"};

char h_ts[] =
{"\n\
The 'ts' parameter specifies a LAT terminal server node name.		\n\
"};

char h_vf[] =
{"\n\
The 'vf' parameter specifies a raster image filter.                     \n\
Filters currently available include:					\n\
\n\
	Filter name:	  		Description:			\n\
	------------	  		------------			\n\
	/usr/lib/lpdfilters/ln01vf	LN01 Laser Printer filter	\n\
"};

char h_xc[] =
{"\n\
The 'xc' parameter specifies the local mode bits to clear	        \n\
when the terminal line is first opened.  Refer to the		        \n\
discussion of the local mode word in the tty(4) manual		        \n\
page in the ULTRIX Programmer's Manual, Volume 1.		        \n\
"};

char h_xf[] =
{"\n\
The 'xf' parameter specifies the pass-thru filter name.	 		\n\
This routine is used when output is already formatted for		\n\
printing and does not require special filtering.			\n\
"};

char h_xs[] =
{"\n\
The 'xs' parameter specifies the local mode bits to set		        \n\
when the terminal line is first opened.  Refer to the	         	\n\
discussion of the local mode word in the tty(4) manual		        \n\
page in the ULTRIX Programmer's Manual, Volume 1.		        \n\
"};


/************************************************
*  This structure holds the correct printcap
*  values for various printers, other values
*  -> default.  The first member of the structure
*  holds the printer name.
*************************************************/
struct 
{
    char *name;
    struct nameval entry[80];
} printer[] = 
{
    /* LA50 */
    {
    	"la50",
	"br", "4800",
	"fc", "0177777",
	"fs", "016620",
	"pl", "66",
	"pw", "80",
	"sh", "none",
	0, 0
    },
    /* LA75 */
    {
    	"la75",
	"br", "4800",
	"fc", "0177777",
	"fs", "016620",
	"pl", "66",
	"pw", "80",
	"sh", "none",
	0, 0
    },
    /* LA100 */
    {
    	"la100",
	"br", "4800",
	"fc", "0177777",
	"fs", "06020",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* LA210 */
    {
    	"la210",
	"br", "4800",
	"fc", "0177777",
	"fs", "023",
	"of", "/usr/lib/lpdfilters/lqf",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* LG01 */
    {
    	"lg01",
	"br", "9600",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpf",
	0, 0
    },
    /* LN01 */
    {
    	"ln01",
	"br", "9600",
	"fc", "0177777",
	"fs", "023",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpdfilters/ln01of",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* LN01S */
    {
    	"ln01s",
	"br", "9600",
	"fc", "0177777",
	"fs", "023",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpdfilters/ln01of",
	"pl", "66",
	"pw", "80",
	"vf", "/usr/lib/lpdfilters/ln01vf",
	0, 0
    },
    /* LN03 */
    {
    	"ln03",
	"br", "9600",
	"fc", "0177777",
	"fs", "023",
	"of", "/usr/lib/lpdfilters/ln03of",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* LP25 */
    {
    	"lp25",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpf",
	0, 0
    },
    /* LP26 */
    {
    	"lp26",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpf",
	0, 0
    },
    /* LP27 */
    {
    	"lp27",
	"lp", "/dev/lp",
	"of", "/usr/lib/lpf",
	0, 0
    },
    /* LQP02 */
    {
    	"lqp02",
	"br", "9600",
	"fc", "0177777",
	"fs", "023",
	"of", "/usr/lib/lpdfilters/lqf",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* LQP03 */
    {
    	"lqp03",
	"br", "9600",
	"fc", "0177777",
	"fs", "023",
	"of", "/usr/lib/lpdfilters/lqf",
	"pl", "66",
	"pw", "80",
	0, 0
    },
    /* REMOTE SYSTEM FOR PRINTER */
    {
	"remote",
	0, 0
    },
    /* NULL */
    {
	0, 0, 0
    }
};

/*************************************************
*  This structure maps a command character string
*  against an integer code
*************************************************/
struct cmdtyp   cmdtyp[] =
{
    "?", HELP,
    "add", ADD,
    "delete", DELETE,
    "exit", QUIT,
    "help", HELP,
    "list", LIST,
    "modify", MODIFY,
    "no", NO,
    "print", PRINT,
    "quit", QUIT,
    "yes", YES,
    0, 0
};

/******************************************
*  the following arrays hold the help text
*  associated with each function and are
*  referred to specifically by the function.
******************************************/
char h_help[] =
{"\n\
This program helps you to set up your printers.  			\n\
									\n\
Select a command from the Main Command Menu by typing one of the	\n\
commands listed (or just the first letter of the command)		\n\
and pressing RETURN.  You will then be prompted to enter		\n\
the number of the printer (for example 0 or 1).  Then you will be	\n\
prompted for other responses, depending on the command. 		\n\
									\n\
For all prompts, a default response value is given in \[ \].  To use	\n\
the default, just press RETURN.  You can always enter '?' for help.	\n\
									\n\
Some knowledge of the printer and its installation is required 		\n\
in order to correctly use this program.  Refer to the passage		\n\
in the System Management Guide for general requirements.		\n\
									\n\
Most of the setup activities require you to enter values for symbols	\n\
in the file /etc/printcap.  Refer to printcap(5) in the			\n\
ULTRIX Programmer's Manual for a summary of the possible symbols	\n\
for /etc/printcap.  This program provides a detailed explanation	\n\
of each printcap symbol in response to '?' after a symbol name		\n\
has been specified and a value for the symbol is prompted.		\n\
"};

char h_doadd[] =
{"\n\
Enter the number of the printer.  If you only have one printer		\n\
on your system, it should be number 0.  Printer number 0 is 		\n\
generally the default line printer.  The number you enter 		\n\
must be in the range 0 to 99.  This number is used by lprsetup		\n\
to identify the printer.  						\n\
									\n\
This number is also a synonym (along with other names if you wish) 	\n\
that identifies	the printer to the print system.  For example, 		\n\
if you enter the number 4 for this printer, then the command 		\n\
'lpr -P4 files ...' will print files on this printer.			\n\
"};

char h_dodel[] =
{"\n\
This section deletes an existing printers.  				\n\
Enter the number of the printer.  The number must be in the range	\n\
0 to 99.  The number of the printer that you enter must match 		\n\
that of a printer in the /etc/printcap file in order to delete it.	\n\
"};

char h_domod[] =
{"\n\
This section modifies one or more existing printcap entries.  		\n\
Enter the number of the printer.  The number must be in the range	\n\
0 to 99.  The number of the printer that you enter must match 		\n\
that of a printer in the /etc/printcap file in order to modify it.	\n\
You will then be prompted to name the symbol in /etc/printcap which 	\n\
you wish to change.  You may instead enter 'q' (for quit) to return 	\n\
to the main Command menu.						\n\
"};

char h_symsel[] =
{"\n\
Enter the name of the printcap symbol you wish to modify.  Other	\n\
valid entries are:							\n\
        'q'     to quit (no more changes)				\n\
        'p'     to print the symbols you have specified so far		\n\
        'l'     to list all of the possible symbols and defaults	\n\
The names of the printcap symbols are:					\n\
\n\
"};

char h_synonym[] =
{"\n\
Enter an alternate name for this printer.  Some examples include	\n\
'draft',  'letter', and 'LA-75 Companion Printer'.  If the name		\n\
contains blanks or tabs it should be last.  You can enter as many 	\n\
alternate names	for a printer as you like, but the total length of 	\n\
the line containing the names must be less than 80 characters.  	\n\
After entering a synonym, you will be prompted again.  If you do not 	\n\
wish to enter any more synonyms, press RETURN to continue.		\n\
									\n\
Each synonym (including	the printer number) identifies the printer 	\n\
to the print system.  For example, if you enter the synonym 'draft'	\n\
for this printer, then the command 'lpr -Pdraft files ...' 		\n\
will print files on this printer.					\n\
"};

char h_type[] =
{"\n\
Printers are listed by type and only supported DIGITAL printers 	\n\
These printers have some default values	already included in the 	\n\
setup program.  							\n\
									\n\
Other printers can be set up by using 'unknown' and then 		\n\
responding to the prompts, using values similar to those for 		\n\
supported printers.							\n\
									\n\
Responding 'remote' allows you to designate a remote system		\n\
for printing.  In this case, only three printcap entries are required:	\n\
rm (name of the remote system), rp (name of the printer on the		\n\
remote system), and sd (pathname of the spooling directory on		\n\
this system.								\n\
\n\
"};

char h_default[] =
{"\n\
Enter a new value, or press RETURN to use the default. 			\n\
"};

char isnotused[] =
{"feature is not used with 						\n\
the parallel line printer interface.  You would only			\n\
specify this symbol for a printer which is connected via		\n\
a serial terminal line. 						\n\
"};

/***********************************
*  end of globals.h
***********************************/
