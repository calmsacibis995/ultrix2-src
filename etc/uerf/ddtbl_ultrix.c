#ifndef lint
static char sccsid[]  =  "@(#)ddtbl_ultrix.c	1.8   (ULTRIX)   12/16/86"; 
#endif  lint

#include "eims.h"
#include "generic_dsd.h"
long
dd$flddata(fldno,ftype,offset)
short	fldno;
short	*ftype;
short	*offset;
{
static short ftypes[] = { EM$LONG,
	    EM$LONG,EM$SHORTINDEX,
	    EM$DATE,EM$DATE,EM$LONG,
	    EM$SHORTINDEX,EM$STRING,   /* end of EIS */
	    EM$SHORTINDEX,EM$SHORTINDEX,
	    EM$LONGINDEX,EM$SHORTINDEX,EM$SHORT,
	    EM$STRING,EM$STRING};
static short offsets[] = { 16,20,24,28,32,36,12,40,
	    10,12,28,14,16,20,24};

short low = 21;
short high = 36;

    if ((fldno < low) || (fldno > high)) return(DD$FAIL);
    *ftype = ftypes[fldno-low];
    *offset = offsets[fldno-low];
    return(DD$SUCC);    
}
long
dd$getindex(status,fldno,s)
long *status;
short fldno;
char *s;
{
   long ret;
	*status = DD$SUCC;
        if ((ret = get_code_std_item(fldno,s)) == DD$UNKNOWN_CODE)
	    *status = DD$FAIL;
	return(ret);
}

short
dd$fldnameno(fname)
char *fname;
{
short ret;
static char *names[] = { "EVENTTYPE", /* 21 */
	    "RECORDNUMBER",
	    "OSTYPE",
	    "DATETIME", /* 24 */
	    "UPTIME",
	    "SERIALNUMBER",
	    "EVENTCLASS", /* 27 */
	    "HOSTNAME",  /* 28 */
	    "DEVCLASS", /* 29 */
	    "DEVTYPE",
	    "COARSESYNDROME", /* 31 */
	    "CONTROLLER",
	    "UNITNUMBER",
	    "SERIALID",
	    "MEDIAID", /* 35 */
	    "" };
    short low = 21;
    short high = 36;
    short i;

    
    for (i = low; i < high; i++)
	if (strcmp(names[i-low],fname) == 0)
	    return(i);
    ret = DD$FAIL;
    return(ret);    
}

#ifdef STANDALONE
main()
{
    int stat;
    char name[20];

    printf("Field name ");
    scanf("%s",name);
    stat = dd$fldnameno(name);
    printf("Status of name %s is %d\n",name,stat);

}
#endif
