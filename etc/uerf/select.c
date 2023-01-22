#ifndef lint
static char sccsid[]  =  "@(#)select.c	1.9   (ULTRIX)   12/16/86"; 
#endif  lint

/* DEC/CMS REPLACEMENT HISTORY, Element SELECT.C*/
/* *3     8-AUG-1986 14:18:45 ZAREMBA "make local copy of fmt string"*/
/* *2     9-JUN-1986 09:32:38 ZAREMBA "V1_1 changes"*/
/* *1     6-JUN-1986 10:56:48 ZAREMBA "Select module"*/
/* DEC/CMS REPLACEMENT HISTORY, Element SELECT.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element SELECT.C*/
/* *8    16-MAY-1986 14:13:26 ZAREMBA "removed dump routines"*/
/* *7     7-APR-1986 16:15:20 ZAREMBA "add comments to ES$NOT"*/
/* *6     7-APR-1986 16:04:20 ZAREMBA "added else"*/
/* *5    27-MAR-1986 10:42:27 ZAREMBA "Ultrix UERF bug fixes"*/
/* *4    19-MAR-1986 11:12:30 ZAREMBA "removed genmac.h"*/
/* *3    11-MAR-1986 12:15:10 ZAREMBA "added parsing and evalu functions"*/
/* *2    27-FEB-1986 17:53:18 ZAREMBA "Seperate paramters version"*/
/* *1    26-FEB-1986 10:53:07 ZAREMBA "Selection code"*/
/* DEC/CMS REPLACEMENT HISTORY, Element SELECT.C*/
/*
*	.TITLE	SELECT - ERMS SELECT MODULE
*	.IDENT	/1-001/
*
* COPYRIGHT (C) 1986 DIGITAL EQUIPMENT CORP.,
* CSSE SOFTWARE ENGINEERING
* MARLBOROUGH, MASSACHUSETTS
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE FOR USE ONLY ON A 
* SINGLE COMPUTER SYSTEM AND MAY BE COPIED ONLY WITH THE INCLUSION
* OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE,  OR ANY OTHER
* COPIES THEREOF, MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE
* TO ANY OTHER PERSON EXCEPT FOR USE ON SUCH SYSTEM AND TO ONE WHO
* AGREES TO THESE LICENSE TERMS.  TITLE TO AND OWNERSHIP OF THE
* SOFTWARE SHALL AT ALL TIMES REMAIN IN DEC.
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
* NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL
* EQUIPMENT CORPORATION.
*
* DEC ASSUMES NO RESPONSIBILITY FOR THE USE OR RELIABILITY OF
* ITS SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DEC.
*
*++
*
* FACILITY:		[ ERMS - EIMS Record Management System ]
*
* ABSTRACT:
*
*	This module is the ERMS call interface to the SELECT function.
*	It can be called directly from a C program or as the
*	call from the pre-prosessor. 
*	
* ENVIRONMENT:	VAX/VMS C,  ULTRIX-32 C
*
* AUTHOR:  Don Zaremba,  CREATION DATE:  19-Nov-86
*
* MODIFIED BY:
*
* PROGRAM TABLE OF CONTENTS
*  Public
*   bstatus = es$eval(tree)
*   node   = es$parse(stat,selectformat [,argn...])
*   node   = es$mkselectnode(operand,left,right,arguments)
*   args   = es$mkarglist(fldno,fldtype,address)
*   status = es$apparglist(arglist,fldno,fldtype,address)
*
* Private
*   opcode = cvtopc(string)
*   VOID   =  strtoupper(str)
*   strptr = storestring(str)
*   longptr= storelong(long)
*   token  = nexttoken(str,delimiter,rem)
*   str    = trimspaces(str)
*   str    = trimleadingspaces(str)
*   address = cvtparms(ftype,ptype,spar,lpar)
*   VOID   = deletetree(tree)
*   node   = nextinorder(node)
*   node   = firstinorder(tree)
*   VOID   = dumplist(list) #ifdef DEBUG only
*   VOID   = dumptree(tree) #ifdef DEBUG only
*   VOID   = dumptree2(tree) #ifdef DEBUG only
*--
*/

#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include "eims.h"
#include "select.h"
#ifdef DEBUG
#define DUMPVAR(where,var,vtype) printf("where:var = %vtype \n",var)
#define GETVAR(prompt,var,vtype) printf("prompt: "); scanf("%vtype",var)
#endif

/* Local function declarations */
SELNODE *es$mkselectnode();
ARGLIST *es$mkarglist();
long es$apparglist();
char *malloc();

/*
*	.SBTTL	ERMS_EVALUATE - ERMS EVALUATE function
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function evaluates a selection criteria against a
*	standard record and returns ES$SUCC if the record
*	satisifies the criteria, ES$FAIL if not.
*	These evaluations are done using only the address and
*       datatype fields of an arglist. Because of this fact, this
*       function  can be called from a number of different
*       places. The address can point into a standard segment if
*       called from ERMS, or the address could point into a raw
*       buffer if called from ERIT.
*
* FUNCTION DECLARATION
*	long es$eval();
*	
* CALLING SEQUENCE:	status = es$eval(tree)
*
* FORMAL PARAMETERS:		
*
*	SELNODE *tree;	pointer to select tree
*
* IMPLICIT INPUTS: NONE
*
* IMPLICIT OUTPUTS: NONE
*
* FUNCTION VALUE:		integer value
*
*	ES$SUCC - record satisfies selection criteria
*	ES$FAIL - record does not satisfies selection criteria
*	
* SIDE EFFECTS:	NONE
*	
*--
*/
/* DEFFUN */
long
es$eval(tree)
SELNODE *tree;
{
    long answer,ans2;

    switch (tree->operator)
     {
	case ES$AND :
	    answer = es$eval(tree->left);
	    if (answer == ES$FAIL) return(ES$FAIL); /* bail out at first 
						       fail of an AND */
	    ans2 = es$eval(tree->right);
	    answer = ((answer == ES$SUCC) && (ans2 == ES$SUCC))
		     ? ES$SUCC : ES$FAIL;
	    break;
	case ES$OR :
	    answer = es$eval(tree->left);
	    if (answer == ES$SUCC) return(ES$SUCC); /* bail out at first
						       success of an OR */
	    ans2 = es$eval(tree->right);
	    answer = ((answer == ES$SUCC) || (ans2 == ES$SUCC))
		     ? ES$SUCC : ES$FAIL;
	    break;
	case ES$NOT:
	    answer = es$eval(tree->left);
	    answer = (answer == ES$SUCC) ? ES$FAIL : ES$SUCC;
	    break;
	case ES$EQUAL:
	    answer = isequal(tree->operands);
	    break;
	case ES$LE:
	case ES$LT:
	case ES$GE:
	case ES$GT:
	    answer = isinequal(tree->operands,tree->operator);
	    break;
	case ES$RANGE:
	    answer = isrange(tree->operands);
	    break;
     }
    return(answer);
}

long
isrange(args)
ARGLIST *args;
{
    long ans;
    char *varadr;
    long vartype;
    ARGLIST *carg,*carg2;

    carg = args;		/* point to first argument */
    varadr = args->varaddress;
    vartype = args->vartype;
    ans = ES$FAIL;

    carg = carg->next;
    carg2 = carg->next;
    switch (vartype)
      {
	case EM$STRING :
	      ans = ((strcmp(*(char **)varadr,carg2->varaddress) <= 0) &&
		     (strcmp(*(char **)varadr,carg->varaddress) >= 0))
			    ? ES$SUCC : ES$FAIL;
	      break;
	case EM$SHORT :
	case EM$SHORTINDEX :
		ans = ((*(short*)varadr <= *(short*)carg2->varaddress) &&
		     (*(short*)varadr >= *(short*)carg->varaddress))
			    ? ES$SUCC : ES$FAIL;
#ifdef DEBUG2
	      printf("1 adr %x type %d value %d\n",varadr,vartype,*(short*)varadr);
	      printf("2 adr %x type %d value %d\n",carg->varaddress,
		    carg->vartype,*(short*)carg->varaddress);
	      printf("3 adr %x type %d value %d\n",carg2->varaddress,
		    carg2->vartype,*(short*)carg2->varaddress);
#endif
	      break;
	case EM$LONG :
	case EM$LONGINDEX :
	case EM$DATE:
		ans = ((*(long*)varadr <= *(long*)carg2->varaddress) &&
		     (*(long*)varadr >= *(long*)carg->varaddress))
			    ? ES$SUCC : ES$FAIL;
#ifdef DEBUG2
	      printf("1 adr %x type %d value %d\n",varadr,vartype,*(long*)varadr);
	      printf("2 adr %x type %d value %d\n",carg->varaddress,
		    carg->vartype,*(long*)carg->varaddress);
	      printf("3 adr %x type %d value %d\n",carg2->varaddress,
		    carg2->vartype,*(long*)carg2->varaddress);
#endif
	      break;
      }
    return(ans);
}

long
isinequal(args,opcode)
ARGLIST *args;
short opcode;
{
    long ans;
    char *varadr;
    long vartype;
    ARGLIST *carg;

    carg = args;		/* point to first argument */
    varadr = args->varaddress;
    vartype = args->vartype;
    ans = ES$FAIL;

    carg = carg->next;
    switch (vartype)
      {
	case EM$STRING :
	      ans = (strcmp(*(char **)varadr,carg->varaddress) < 0) ? ES$SUCC : ES$FAIL;
#ifdef DEBUG2
	      printf("1 adr %x type %d value [%s]\n",
		    varadr,vartype,*(char**)varadr);
 	      printf("2 adr %x type %d value [%s]\n",carg->varaddress,
		    carg->vartype,carg->varaddress);
#endif
	      break;
	case EM$SHORT :
	case EM$SHORTINDEX :
	    switch (opcode)
	      {
		case ES$LT:
		    ans = (*(short*)varadr < *(short*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$LE:
		    ans = (*(short*)varadr <= *(short*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$GT:
		    ans = (*(short*)varadr > *(short*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$GE:
		    ans = (*(short*)varadr >= *(short*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
	      break;
	  }
	case EM$LONG :
	case EM$LONGINDEX:
	case EM$DATE:
	    switch (opcode)
	      {
		case ES$LT:
		    ans = (*(long*)varadr < *(long*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$LE:
		    ans = (*(long*)varadr <= *(long*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$GT:
		    ans = (*(long*)varadr > *(long*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		case ES$GE:
		    ans = (*(long*)varadr >= *(long*)carg->varaddress) 
			    ? ES$SUCC : ES$FAIL;
		    break;
		} /* end of opcode case */
#ifdef DEBUG2
	      printf("1 adr %x type %d value %d\n",varadr,vartype,*(long*)varadr);
	      printf("2 adr %x type %d value %d\n",carg->varaddress,
		    carg->vartype,*(long*)carg->varaddress);
#endif
	 }
    return(ans);
}
long
isequal(args)
ARGLIST *args;
{
    long ans;
    char *varadr;
    long vartype;
    ARGLIST *carg;

    carg = args;		/* point to first argument */
    varadr = args->varaddress;
    vartype = args->vartype;
    ans = ES$FAIL;

    while ((ans != ES$SUCC) && (carg->next != NULL))
      {
	carg = carg->next;
	switch (vartype)
	 {
	    case EM$STRING :
#ifdef DEBUG2
	      printf("1 adr %x type %d value [%s]\n",
		    varadr,vartype,*(char**)varadr);
 	      printf("2 adr %x type %d value [%s]\n",carg->varaddress,
		    carg->vartype,carg->varaddress);
#endif
	      ans = (strcmp(*(char **)varadr,carg->varaddress) == 0) ? ES$SUCC : ES$FAIL;
	      break;
	    case EM$SHORT :
	    case EM$SHORTINDEX :
	      ans = (*(short*)varadr == *(short*)carg->varaddress) ? ES$SUCC : ES$FAIL;
#ifdef DEBUG2
	      printf("isequal short,short-index\n");
	      printf("1 adr %x type %d value %d\n",varadr,vartype,*(short*)varadr);
	      printf("2 adr %x type %d value %d\n",carg->varaddress,
		    carg->vartype,*(short*)carg->varaddress);
#endif
	      break;
	    default:
	      ans = (*(long*)varadr == *(long*)carg->varaddress) ? ES$SUCC : ES$FAIL;
#ifdef DEBUG2
	      printf("isequal default\n");
	      printf("1 adr %x type %d value %d\n",varadr,vartype,*(long*)varadr);
	      printf("2 adr %x type %d value %d\n",carg->varaddress,
		    carg->vartype,*(long*)carg->varaddress);
#endif
	 }
      }
    return(ans);
}


/*
*	.SBTTL PARSE - Parse Selection Criteria
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function performs the following builds a selection
*	node. In the processes of doing this it will also do
*	the following:
*	    1. Parse the format string into field number and types
*	    2. Perform type conversions when appropriate
*	    3. Validate selection criteria
*
* FUNCTION DECLARATION
*	SELNODE *es$parse();
*	
* CALLING SEQUENCE: node = es$parse(stat,format [,argn...]) 
*
* FORMAL PARAMETERS:		
*
*   long    stat;	    See notes below
*   char    *format;	    See notes below
*  
* SIDE EFFECTS:	Space is allocated for a node and data is
*		stored in that node.
*
* NOTES:
*   The following error code are returned in stat.
*	    ES$SUCC - node created
*	    ES$NOFLD - bad EIMS field number
*	    ES$BADOPCODE  - bad opcode
*	    ES$NOINDEX - no index value found a coded field
*	    ES$BADFMT - bad format string
*
*   format  The format string is similiar to the printf format
*	    string. The only supported formats are %s for strings
*	    and %d for long integers.
*
*	    The select format is 
*		"EIMS_field_name Opcode arg1, arg2 [,argn...]"
*
*		where argn is either a text string or a placeholder
*		  of the form %d (for long) or %s (for string). If
*		  a placeholder is used, then there must be an
*		  additional argument for each placeholder.
* EXAMPLES:
*
*	node = es$parse(stat,"devtype eq rp06, rp07");
*	node = es$parse(stat,"devtype eq %s,%s",par1,par2);
*	node = es$parse(stat,"unitnumber lt 2");
*	node = es$parse(stat,"unitnumber eq %d,%d",u1,2);
*	node = es$parse(stat,"%d eq %s",DD$devtype,p1);
*--
*/
/* DEFFUN */
SELNODE
*es$parse(va_alist)
va_dcl
{

#define FMTBUFSIZE 128

    va_list incr;
    long *statptr,retstat;
    char *fldname, *opc, *nexttoken();
    char *trimspaces(), *trimleadingspaces(), *token;
    short fldno;
    short opcode;
    char *fmt;   /* pointer to local copy of fmt string */
    char *ufmt;  /* user format string */
static char fmtbuf[FMTBUFSIZE]; /* store local fmt string here */

    short ftype,offset; /* Data type and offset of the EIMS field */

    char *spar; /* string paramter */
    long lpar ; /* long paramter */
    short ptype; /* paramter type */
    short argcnt; /* number in arglist */
    char *temp, *cvtparms();

    ARGLIST *args ; /* argument list */
    SELNODE *snode ; /* store the return value here */


    va_start(incr);
    statptr = va_arg(incr,long*);	/* address of the status array */
    ufmt = va_arg(incr,char*);		/* address of selection format */

    fmt = fmtbuf;
    strcpy(fmt,ufmt);		    /* make a local copy */
    fmt = trimleadingspaces(fmt);
 
/* process field name */
    fldname = nexttoken(fmt,' ',&fmt);	/* get token to hold field name */
    while (isspace(*fldname)) fldname++; /* skip leading spaces */
    if (fldname[0] == '%')		 /* check for a placeholder */
     {
	fldno = va_arg(incr,long);	/* pick up field number from args */
     }
    else
     {
        strtoupper(fldname);
	fldno = dd$fldnameno(fldname);
	if (fldno == DD$FAIL) 
	{
	    *statptr = ES$NOFLD;
	    return(NULL);
	}
     }
    retstat = dd$flddata(fldno,&ftype,&offset);  /* get the type and offset */
    if (retstat == DD$FAIL) {
	    *statptr = ES$NOFLD;
	    return(NULL);
	}

/* process opcode */
    fmt = trimleadingspaces(fmt);
    opc = nexttoken(fmt,' ',&fmt);  /* get opcode */
    strtoupper(opc);
    opcode = cvtopc(opc);
    if (opcode == ES$FAIL)
	{
	    *statptr = ES$BADOPCODE;
	    return(NULL);
	}

/* process the remaining paramters
   there must be atleast one */

    argcnt = 1;

#ifdef DEBUG2
    printf("Parse fieldno %d type %d offset %d \n",fldno,ftype,offset);
#endif
    args = es$mkarglist(fldno,ftype,0);
 
    while (fmt != NULL) /* for each token */
	{
	    token = nexttoken(fmt,',',&fmt); /* for each token */
	    token = trimspaces(token);
	    if (strlen(token) != 0)
	      {
		ptype = EM$STRING; /* default to string */
		if (*token == '%')
		{
		    token++;
		    switch(*token)	/* char after % defines the type */
		    {
			case 'd' :
			  lpar = va_arg(incr,long);
			  ptype = EM$LONG;
			  break;
			case 's' :
			  spar = va_arg(incr,char*);
			  ptype = EM$STRING;
			  break;
			default :
			  *statptr = ES$BADFMT;
			  return(NULL);
		    }
	          }
		else
		    spar = token;
	    temp = cvtparms(&retstat,fldno,ftype,ptype,spar,lpar);
	    if (retstat != ES$SUCC)
	      {
		*statptr = retstat;
		return(NULL);
	      }
	    argcnt++;
	    retstat = es$apparglist(args,fldno,ftype,temp);
	  } /* end of test for zero length tokens */
        } /* end of reading tokens */
    
    snode = es$mkselectnode(opcode,NULL,NULL,args);
    *statptr = ES$SUCC;
    return(snode);
}

/* DEFFUN */
/*  This function converts the input paramater to the
*   type of the EIMS field.
*   The function returns the address of the stored paramter 
*/
char
*cvtparms(stat,fldno,ftype,ptype,spar,lpar)
long *stat;
short fldno;
short ftype;
short ptype;
char *spar;
long lpar;
{
    char *ret, *storestring(), *storelong();
    long llong;

    *stat = ES$SUCC;
    ret = NULL;
    switch (ftype)
      {
	case EM$SHORT :  
	case EM$LONG  :   /* these 3 are all done the same way */
	case EM$DATE  :
	    if (ptype == EM$STRING)
		ret = storelong(atol(spar));
	    else
		ret = storelong(lpar);
	    break;
	case EM$STRING :
	    ret = storestring(spar);
	    break;
	case EM$SHORTINDEX :
	case EM$LONGINDEX :
	    if (ptype == EM$STRING)
	     {
		llong = dd$getindex(stat,fldno,spar);
		if (*stat == DD$FAIL)
		  {
		    *stat = ES$TBLERR;
		    return(NULL);
		  }
		*stat = ES$SUCC;
		ret = storelong(llong);
	     }
	    else
	        ret = storelong(lpar);
	    break;
	 default:
	    *stat = ES$TPER;
	    return(NULL);
      }
    return(ret);
}

/* DEFFUN */
char
*storestring(s)
char *s;
{
    char *ret;

    ret = malloc(strlen(s)+1);
    strcpy(ret,s);
    return(ret);
}

/* DEFFUN */
char
*storelong(l)
long l;
{
    long *ret;


    ret = (long *) malloc(sizeof(long));
    *ret = l;
    return((char *)ret);
}

/* DEFFUN */
long
cvtopc(s)
char *s;
{
static char *opcodes[] = { 
	    "",
	    "AND",
	    "OR",
	    "NOT",
	    "EQ",
	    "RANGE",
	    "LT",
	    "LE",
	    "GT",
	    "GE" };
    short low = 1;
    short high = 10;
    short i;

    
    for (i = low; i < high; i++)
	if (strcmp(opcodes[i],s) == 0)
	    return(i);
    return(ES$FAIL);    
}

/* DEFFUN */
strtoupper(s)
char *s;
{
    while (*s != NULL)
      {
	*s = toupper(*s);
	s++;
      }
}

/* DEFFUN */
char
*nexttoken(str,delimiter,rem)
char *str;
int delimiter;
char **rem;
{
    char *ret,*cloc,*strchr();
    
    ret = str;
    cloc = strchr(str,delimiter);
    if (cloc == NULL) 
	{ *rem = NULL; /* set remainder to NULL */
	  return(ret);
	}

    *(cloc) = '\0'; /* terminate the string at the delimiter */
    cloc++;
    *rem = cloc;

    return(ret);      
	
}
/* DEFFUN */
char
*trimspaces(str)
char *str;
{
    char *ret, *strend;

    ret = str;
    while (isspace(*ret) != 0) ret++;
    strend = ret + strlen(ret) -1 ;
    while (isspace(*strend) != 0) strend--;
    strend++;
    *(strend) = '\0';
    return(ret);
}

/* DEFFUN */
char
*trimleadingspaces(str)
char *str;
{
    char *ret, *strend;

    ret = str;
    while (isspace(*ret) != 0) ret++;
    return(ret);
}


/*
*	.SBTTL	Make select node
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function creates a selection node. Space is allocated
*	for the node, a node is created, and the input parameters
*	are stored in the node.
*
* FUNCTION DECLARATION
*	SELNODE *es$mkselectnode();
*	
* CALLING SEQUENCE: node = es$mkselectnode(opr,left,right,args)
*
* FORMAL PARAMETERS:		
*
*   short opr; Must be on of the following:
*		ES$AND ES$OR ES$NOT ES$EQUAL ES$RANGE ES$LT ES$LE ES$GT ES$GE 
*   SELNODE *left;
*   SELNODE *right;
*   ARGLIST *args;
*
* IMPLICIT INPUTS: Paramters passed in the call
*
*	
* SIDE EFFECTS:			
*	Space is allocated for a node and data is
*	stored in that node.
*
* UNDESIRED EVENTS
*	This function returns a NULL pointer if the node cannot
*	be created for some reason. The only know reason is
*	insufficient memory to allocate for the node.
*	
*--
*/
/* DEFFUN */
SELNODE
*es$mkselectnode(opr,lnode,rnode,args)
short opr;
SELNODE *lnode, *rnode;
ARGLIST *args;
{
    SELNODE *answer;

    answer = (SELNODE *) malloc(sizeof(SELNODE));
    answer -> operator = opr;
    answer -> left = lnode;
    answer -> parent = NULL;
    if (lnode != NULL)
	lnode -> parent = answer;
    if (rnode != NULL)
	rnode -> parent = answer;
    answer -> right = rnode;
    answer -> operands = args;
    return(answer);
}

/*
*	.SBTTL	Make new argument list
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function creates a new argument list. The first
*	element of the list is also created.
*
* FUNCTION DECLARATION
*	ARGLIST arglist *es$mkarglist();
*	
* CALLING SEQUENCE: args = es$mkarglist(fldno,ftype,varadr)
*
* FORMAL PARAMETERS:		
*
*   short fldno; EIMS field number
*   short ftype; on of the following EIMS data types
*		    EM$SHORT EM$LONG EM$DATE EM$STRING
*   char *varadr;  address of a variable
*
* IMPLICIT INPUTS: Paramters passed in the call
*
*	
* SIDE EFFECTS:			
*	Space is allocated for the first (top) element of the list.
*
* UNDESIRED EVENTS
*	This function returns a NULL pointer if the list cannot
*	be created for some reason. The only know reason is
*	insufficient memory to allocate for the node.
*	
*--
*/
/* DEFFUN */
ARGLIST
*es$mkarglist(fldno,fldtype,varadr)
short fldno;
short fldtype;
char *varadr;
{
    ARGLIST *answer;

    answer = (ARGLIST *) malloc(sizeof(ARGLIST));
    if (answer == NULL) return(NULL);

    answer -> fldno = fldno;
    answer -> varaddress = varadr;
    answer -> vartype = fldtype;
    answer -> next = NULL;
    return(answer);
}

/*
*	.SBTTL	Append to end of argument list
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function adds a new element to the end of an
*	argument list.
*
* FUNCTION DECLARATION
*	long es$apparglist();
*	
* CALLING SEQUENCE: status = es$apparglist(args,fldno,fldtype,varadr)
*
* FORMAL PARAMETERS:		
*
*   ARGLIST *args ; pointer to the argument list to append to
*   short fldno; EIMS field number
*   short vartype; on of the following EIMS data types
*		    EM$SHORT EM$LONG EM$DATE EM$STRING
*   char *varadr;  address of a user variable
*
* IMPLICIT INPUTS: Paramters passed in the call
*
*	
* SIDE EFFECTS:			
*	Space is allocated for the first (top) element of the list.
*
* FUNCTION VALUE
*	ES$SUCC - element appended to list
*	ES$NOMEM - no room to add element
*
* UNDESIRED EVENTS
*	This function returns ES$NOMEM if the list cannot
*	be created for some reason. The only know reason is
*	insufficient memory to allocate for the node.
*	
*--
*/
/* DEFFUN */
long
es$apparglist(args,fldno,fldtype,varadr)
ARGLIST *args;
short fldno;
short fldtype;
char *varadr;
{
    ARGLIST *curr,*newone;

    curr = args;
    while (curr -> next != NULL)
	    curr = curr -> next;

    newone = (ARGLIST *) malloc(sizeof(ARGLIST));
    if (newone == NULL) return(ES$NOMEM);

    curr -> next = newone;
    newone -> fldno = fldno;
    newone -> varaddress = varadr;
    newone -> vartype = fldtype;
    newone -> next = NULL;
    return(ES$SUCC);
}
/* DEFFUN */
/*  This function deletes and tree and frees up the space */
long
deletetree(tree)
SELNODE *tree;
{
    if (tree != NULL)
      {
	if (tree -> left != NULL) deletetree( tree -> left);
	if (tree -> right != NULL) deletetree( tree -> right);
    	free(tree);
      }
}


/* DEFFUN */
/*  This function returns a pointer to the next node 
    processed inorder. */
SELNODE
*nextinorder(last)
SELNODE *last;
{
    SELNODE *ans,*prev,*firstinorder();

    if (last -> right != NULL)
	return(firstinorder(last -> right)); /* the easy one */
    ans = last -> parent;
    prev = last;
  
    while (ans != NULL)
	{
	   if (prev == ans -> left) return(ans); /* If we came from the left
						    then we now do the parent */
	   prev = ans;
	   ans = ans -> parent; /* back up one parent */
	}
    return(NULL); /* backed up to parent */

}
/* DEFFUN */
/*  This function returns a pointer to the first node 
    processed inorder. */
SELNODE
*firstinorder(top)
SELNODE *top;
{
    SELNODE *ans;

    ans = top;
    while (ans -> left != NULL) ans = ans -> left;
    return(ans);
}


