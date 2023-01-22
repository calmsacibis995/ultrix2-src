#ifndef lint
static char sccsid[]  =  "@(#)dsd_access.c	1.8   (ULTRIX)   12/16/86"; 
#endif  lint
/*
**	.title DSD Table Access Functions
**	.ident / 1.16 /
**
**
**	  File:	dsd_access.c
** Description:	DSD Table Access Functions
**	Author:	Paul Baker
**	  Date:	7-Oct-1986
**
**
**	Copyright 1986, Digital Equipment Corporation
**
**
**++
**	The DSD table access functions interface either directly to the
**	application, or indirectly through the DSD services functions.
**	The DSD table access functions can not read or manipulate the
**	data-segment.
**
**	These functions do not provide simultaneous access to the STD
**	and O/S DSD tables.
**--
*/

/*
**	Functions in this module:  (in this order)
**
**
**	char  *store (oldstring)
**	DD$BYTE *fld_align(pointer, data_type)
**	short get_item_size (data_type, length)
**
**      long get_std_segment_dsd (dsd_context_pointer)
**	long get_next_item_dsd (dsd_context_pointer)
**           find_std_segment_dsd (segment_type, segment_subtype)
**	     find_std_item_dsd (item_id)
**	long get_next_field_dsd (dsd_context_pointer)
**
**      long get_os_record_dsd (dsd_context_pointer)
**	     find_os_record_dsd (record_type, record_subtype)
**	     find_os_segment_dsd (segment_id)
**	     find_os_item_dsd (item_id)
**	long get_item_dsd (dsd_context_pointer, item_id)
**
**      char *decode_std_item (dsd_context_pointer, item_code)
**      short decode_os_item (dsd_context_pointer, item_code)
**      char *decode_register_field (dsd_context_pointer, field_code)
**
**	long dump_ctx (CTX_PTR)
**	long dump_dsd_tables ()
*/


#include <stdio.h>
#include "dsd_switch.h"		/* Compilation control switches */
#include "generic_dsd.h"	/* DSD table structure definitions */
#include "std_dsd.h"		/* Standard event Data Structure Definitions */
#include "os_dsd.h"		/* O/S event Data Structure Definitions */

#define std_segment	1
#define os_record	2

#define MAX_STRING	256	/* Maximum size of a NAME or LABEL string */
#define ALLOCATION_SIZE	1024	/* String buffer allocation increment */
#define MAX_BUFFERS	50	/* Maximum number of string buffers */

/* MISCELLANEOUS MODULE WIDE VARIABLES */

static char *(buffers[MAX_BUFFERS]);	/* Pointers to string buffers */
static long buf_num = 0;		/* Active buffer */
static long space_left = 0;		/* Available characters in buffer */
static char *buf_ptr;			/* Pointer to free space */
static char NUL_STRING = '\0';		/* Common null string */

long	status;			/* Used to store function completion codes */

long    bin_file_format;

float	std_item_dsd_version,
	std_segment_dsd_version,
	std_record_dsd_version;

float	os_item_dsd_version,
	os_segment_dsd_version,
	os_record_dsd_version;

extern	long	std_item_dsd_count;
extern	long	std_segment_dsd_count;
extern	long	std_record_dsd_count;

extern	long	os_item_dsd_count;
extern	long	os_segment_dsd_count;
extern	long	os_record_dsd_count;

extern DD$STD_ITEM_DSD_PTR	std_item_dsd_anchor;
extern DD$STD_SEGMENT_DSD_PTR	std_segment_dsd_anchor;
/* DD$STD_RECORD_DSD_PTR	std_record_dsd_anchor; */

extern DD$OS_ITEM_DSD_PTR	os_item_dsd_anchor;
extern DD$OS_SEGMENT_DSD_PTR	os_segment_dsd_anchor;
extern DD$OS_RECORD_DSD_PTR	os_record_dsd_anchor;

/*
}

/* MISCELLANEOUS SUPPORT FUNCTIONS   cont.*/

/* Function to align pointers depending on data-type and O/S alignment */
DD$BYTE *fld_align(pointer, data_type)
DD$BYTE	*pointer;
short	data_type;
{
    switch (data_type)
	{
	case DT_TINY :
	case DT_TINY_INDEX :
	case DT_BYTE_VECTOR :
	case DT_ASCIZ :
	case DT_BIT_VECTOR :
	    break;			/* No alignment needed! */
	case DT_SHORT :
	case DT_SHORT_INDEX :
	case DT_SHORT_REGISTER :
	case DT_COUNTED_SHORT_VECTOR :
	case DT_SHORT_VECTOR :
	    ALIGN(pointer, ALIGN_S_FLD);
	    break;
	case DT_LONG :
	case DT_INDEXED :
	case DT_REGISTER :
	case DT_DATE :
	case DT_COUNTED_LONG_VECTOR :
	case DT_ADDR_CNT_VECTOR :
	case DT_LONG_VECTOR :
	    ALIGN(pointer, ALIGN_L_FLD);
	    break;
	case DT_VMS_TIME :
	    ALIGN(pointer, ALIGN_Q_FLD);
	    break;
	case DT_STRING :
	    ALIGN(pointer, ALIGN_P_FLD);
	    break;
	default :
	    printf("\nUnknown data-type detected in \"fld_align\"!\n"); 
	    break;
	};

    return pointer;
}


/* MISCELLANEOUS SUPPORT FUNCTIONS  cont. */

/* Function returns the size of a data-item given the data-type */
/*  and length if the data-type is a vector */

short get_item_size (data_type, length)
short data_type;
short length;
{
    short size;

    switch (data_type)
	{
	case DT_TINY :
	case DT_TINY_INDEX :
	    size = 1;
	    break;
	case DT_SHORT :
	case DT_SHORT_INDEX :
	case DT_SHORT_REGISTER :
	    size = 2;
	    break;
	case DT_LONG :
	case DT_INDEXED :
	case DT_REGISTER :
	case DT_DATE :
	    size = 4;
	    break;
	case DT_VMS_TIME :
	    size = 8;
	    break;
	case DT_BYTE_VECTOR :
	case DT_ASCIZ :
	case DT_BIT_VECTOR :
	    size = length;
	    break;
	case DT_COUNTED_SHORT_VECTOR :
	    size = (length + 1) * 2;		/* + 1 is for the length word */
	    break;
	case DT_COUNTED_LONG_VECTOR :
	    size = (length + 1) * 4;		/* + 1 is for the length word */
	    break;
	case DT_ADDR_CNT_VECTOR :
	    size = (length + 2) * 4;		/* +2 for addr & length words */
	    break;
	case DT_STRING :
	    size = 0;
	    break;
	case DT_SHORT_VECTOR :
	    size = length * 2;
	    break;
	case DT_LONG_VECTOR :
	    size = length * 4;
	    break;
	default :
	    printf("\nUnknown data-type detected in \"get_item_size\"!\n"); 
	    size = 0;
	    break;
	};

    return size;
}

/*
**++
**  get_std_segment_dsd
**
**  o Description
**
**      The application fills in the pointer to the data-segemnt in the CTX
**      before the call and the function fills in the information for the
**      segment and its first element.
**
**  o Return values
**
**	DD$SUCCESS		- Function completed successfully.
**	DD$UNKNOWN_SEGMENT	- The "segment_type" / "segment_subtype"
**				  combination doesn't exist in the DSD tables.
**	DD$UNKNOWN_ITEM		- Can not find the definition for an item!
**				  Indicates that the DSD tables are corrupted!
**
**  o Synopses
**
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**
**      long get_std_segment_dsd (dsd_context_pointer)
**      DD$STD_DSD_CTX *dsd_context_pointer;
**
**  o Remarks
**
**	A call must have been made to "init_dsd" to initialize the DSD
**	tables before using this function.
**
**  o Example
**
**--
*/


long
get_std_segment_dsd (dsd_context_pointer)
DD$STD_DSD_CTX *dsd_context_pointer;
{
    DD$STD_ITEM_DSD_PTR	    find_std_item_dsd ();
    DD$STD_SEGMENT_DSD_PTR  find_std_segment_dsd ();

    /* Fill the Context Structure */
    dsd_context_pointer->CTX_type = std_segment;  /* CTX for standard segment */

    /* Set current element */
    dsd_context_pointer->current_element = 1;

    /* Get the segment DSD */
    dsd_context_pointer->segment_DSD_ptr =
	find_std_segment_dsd (dsd_context_pointer->segment_ptr->type,
			      dsd_context_pointer->segment_ptr->subtype);

    if (dsd_context_pointer->segment_DSD_ptr == DD$UNKNOWN_SEGMENT)
        return DD$UNKNOWN_SEGMENT;

    /* Get the data-segment validity code */
    dsd_context_pointer->segment_VALID_code = DD$VALID;

    /* Get the item DSD */
    dsd_context_pointer->item_DSD_ptr =
	find_std_item_dsd (dsd_context_pointer->segment_DSD_ptr->ELEMENT[0]);

    if (dsd_context_pointer->item_DSD_ptr == DD$UNKNOWN_ITEM)
        return DD$UNKNOWN_ITEM;

    /* Create the pointer to the first data-item */
    dsd_context_pointer->item_ptr =
	(DD$BYTE *)dsd_context_pointer->segment_ptr	/* Base address */
 	+ DD$HEADER_BYTES
	+ DD$VALID_BYTES(DD$ELEMENT_COUNT);

    /* Align the pointer depending on the item's data-type and the O/S */
    dsd_context_pointer->item_ptr =
	fld_align (dsd_context_pointer->item_ptr,
		   dsd_context_pointer->item_DSD_ptr->TYPE);

    /* Get the data-item validity code */
    dsd_context_pointer->item_VALID_code = 
				get_validity_code(dsd_context_pointer);


    /* Initialize the register information */
    if (dsd_context_pointer->item_DSD_ptr->TYPE == DT_SHORT_REGISTER
	|| dsd_context_pointer->item_DSD_ptr->TYPE == DT_REGISTER)
    {
	/* Initialize the current field */
	dsd_context_pointer->current_field = 1;
	dsd_context_pointer->field_position = 0;

	/* Get the field DSD */
	dsd_context_pointer->field_DSD_ptr =
	    &dsd_context_pointer->item_DSD_ptr->FIELD[0];
    };

    return DD$SUCCESS;
}
/*
**++
**  get_next_item_dsd
**
**  o Description
**
**	This function updates the data-item fields of the CTX with the
**	definition of the next element in the data-segment.
**
**  o Return values
**
**	DD$SUCCESS		- Function completed successfully.
**	DD$END_OF_SEGMENT	- No more elements in the data-segment.
**	DD$UNKNOWN_ITEM		- Can not find the definition for an item!
**				  Indicates that the DSD tables are corrupted!
**
**  o Synopses
**
**	#include "generic_dsd.h"
**      #include "std_dsd.h"
**
**	long get_next_item_dsd (dsd_context_pointer)
**	DD$STD_DSD_CTX *dsd_context_pointer;
**
**  o Remarks
**
**	A call must have been made to "get_std_segment_dsd" to initialize
**	CTX before using this function.
**
**  o Example
**
**--
*/


long
get_next_item_dsd (dsd_context_pointer)
DD$STD_DSD_CTX *dsd_context_pointer;
{
    DD$STD_ITEM_DSD_PTR	find_std_item_dsd ();


/*
** This code needs cleaning up!
** Perhaps it should be integrated with the alignment code.
** Alternatively something like "sizeof(DT_INDEXED)"
*/
    /* Update the data-item pointer depending on the old data-type */
    switch (dsd_context_pointer->item_DSD_ptr->TYPE)
	{
	default :		/* Maybe this should return an error? */
				/*  i.e.  Not a STD data-type! */
	    dsd_context_pointer->item_ptr += 4;
	    break;
	case DT_SHORT :
	case DT_SHORT_INDEX :
	case DT_SHORT_REGISTER :
	    dsd_context_pointer->item_ptr += 2;
	    break;
	case DT_LONG :
	case DT_INDEXED :
	case DT_REGISTER :
	case DT_DATE :
	    dsd_context_pointer->item_ptr += 4;
	    break;
	case DT_STRING :
	    dsd_context_pointer->item_ptr += 4;
	    break;
	case DT_BYTE_VECTOR :
	    dsd_context_pointer->item_ptr +=
		dsd_context_pointer->item_DSD_ptr->COUNT;
	    break;
	case DT_COUNTED_SHORT_VECTOR :		/* + 1 is for the length word */
	    dsd_context_pointer->item_ptr +=
		(dsd_context_pointer->item_DSD_ptr->COUNT +1) * 2;
	    break;
	case DT_COUNTED_LONG_VECTOR :		/* + 1 is for the length word */
	    dsd_context_pointer->item_ptr +=
		(dsd_context_pointer->item_DSD_ptr->COUNT +1) * 4;
	    break;
	case DT_ADDR_CNT_VECTOR :		/* +2 for addr & length words */
	    dsd_context_pointer->item_ptr +=
		(dsd_context_pointer->item_DSD_ptr->COUNT +2) * 4;
	    break;
	};

    /* Check for end of segment */
    if (dsd_context_pointer->current_element >=
	dsd_context_pointer->segment_DSD_ptr->ELEMENT_COUNT)
       return DD$END_OF_SEGMENT;

    /* Update the Context Structure for the next element */

    /* Get the new item DSD */
    dsd_context_pointer->item_DSD_ptr =
	find_std_item_dsd (dsd_context_pointer
			   ->segment_DSD_ptr
			   ->ELEMENT[dsd_context_pointer->current_element++]);

    if (dsd_context_pointer->item_DSD_ptr == DD$UNKNOWN_ITEM)
        return DD$UNKNOWN_ITEM;

    /* Align the pointer depending on the new data-type and the O/S */
    dsd_context_pointer->item_ptr =
	fld_align (dsd_context_pointer->item_ptr,
		   dsd_context_pointer->item_DSD_ptr->TYPE);

    /* Get the data-item validity code */
    dsd_context_pointer->item_VALID_code = 
				get_validity_code(dsd_context_pointer);


    /* Initialize the register information */
    if (dsd_context_pointer->item_DSD_ptr->TYPE == DT_SHORT_REGISTER
	|| dsd_context_pointer->item_DSD_ptr->TYPE == DT_REGISTER)
    {
	/* Initialize the current field */
	dsd_context_pointer->current_field = 1;
	dsd_context_pointer->field_position = 0;

	/* Get the field DSD */
	dsd_context_pointer->field_DSD_ptr =
	    &dsd_context_pointer->item_DSD_ptr->FIELD[0];
    };

    return DD$SUCCESS;
}

/*
**++
**  find_std_segment_dsd
**
**  o Description
**
**      Searches the STD-SEGMENT-LIST for the requested segment DSD.
**
**	Returns a pointer to the segment DSD, or DD$UNKNOWN_SEGMENT if
**	the segment_type and segment_subtype are not in the DSD tables.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**
**	DD$STD_SEGMENT_DSD_PTR
**      find_std_segment_dsd (segment_type, segment_subtype)
**	DD$STD_HEADER.type segment_type;
**	DD$STD_HEADER.subtype segment_subtype;
**
**  o Remarks
**
**	Internal function!  Not for use by an application!
**
**  o Example
**
**--
*/


DD$STD_SEGMENT_DSD_PTR
find_std_segment_dsd (segment_type, segment_subtype)
short segment_type;
short segment_subtype;
{
    long i;

    for (i=0; i < std_segment_dsd_count; i++)
	{
	 if (std_segment_dsd_anchor[i].TYPE == segment_type
	     && std_segment_dsd_anchor[i].SUBTYPE == segment_subtype)
	    return &std_segment_dsd_anchor[i];
	};

    return DD$UNKNOWN_SEGMENT;
}

/*
**++
**  find_std_item_dsd
**
**  o Description
**
**      Searches the STD-ITEM-LIST for the requested data-item id.
**
**	Returns a pointer to the data-item DSD, or DD$UNKNOWN_ITEM if
**	the data-item id is not found in the DSD table.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**
**	DD$STD_ITEM_DSD_PTR
**      find_std_item_dsd (item_id)
**	short item_id;
**
**  o Remarks
**
**	Should only be used when information about a data-item is required
**	out of the normal data-segment context.
**
**  o Example
**
**--
*/


DD$STD_ITEM_DSD_PTR
find_std_item_dsd (item_id)
short item_id;
{
    long i;

    for (i=0; i < std_item_dsd_count; i++)
	{
	 if (std_item_dsd_anchor[i].ID == item_id)
	    return &std_item_dsd_anchor[i];
	};

    return DD$UNKNOWN_ITEM;
}

/*
**++
**  get_next_field_dsd
**
**  o Description
**
**	This function updates the register field information in the CTX
**	with the definition of the next field in the register currently
**	pointed to by the CTX.
**
** o Return_values
**
**	DD$SUCCESS		- Function completed successfully.
**	DD$END_OF_REGISTER	- No more fields in the register.
**	DD$NOT_A_REGISTER	- The CTX doesn't currently point to a
**				  data-item having a REGISTER or
**				  SHORT_REGISTER data_type.
**
**  o Synopses
**
**	#include "generic_dsd.h"
**      #include "std_dsd.h"
**
**	long get_next_field_dsd (dsd_context_pointer)
**	DD$STD_DSD_CTX *dsd_context_pointer;
**
**  o Remarks
**
**  o Example
**
**--
*/


long
get_next_field_dsd (dsd_context_pointer)
DD$STD_DSD_CTX *dsd_context_pointer;
{
    /* Check that item is a register */
    if (dsd_context_pointer->item_DSD_ptr->TYPE != DT_SHORT_REGISTER
	&& dsd_context_pointer->item_DSD_ptr->TYPE != DT_REGISTER)
    {
	printf("\n\"get_next_field_dsd\" called with non-register item!\n"); 
	return DD$NOT_A_REGISTER;
    };

    if (dsd_context_pointer->current_field >=
	    dsd_context_pointer->item_DSD_ptr->COUNT)
       return DD$END_OF_REGISTER;


    /* Update the Context Structure for the next field */

    dsd_context_pointer->field_position +=
	dsd_context_pointer->field_DSD_ptr->SIZE;

    /* Get the new field DSD */
    dsd_context_pointer->field_DSD_ptr =
	&dsd_context_pointer->item_DSD_ptr->
	    FIELD[dsd_context_pointer->current_field++];

    return DD$SUCCESS;
}

/*
**++
**  get_os_record_dsd
**
**  o Description
**
**      The application fills in the pointer to the O/S record in
**	the CTX before the call and the function fills in the
**	information for the record.  For the O/S tables, the
**	application must call "get_os_item_dsd" with the value of
**	a valid item-id for the selected segment.
**
**	The function returns DD$UNKNOWN_RECORD if the event_type and
**	event_subtype are not found in the DSD tables.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**
**      long get_os_record_dsd (dsd_context_pointer)
**      DD$OS_DSD_CTX *dsd_context_pointer;
**
**  o Remarks
**
**  o Example
**
**--
*/


long
get_os_record_dsd (dsd_context_pointer)
DD$OS_DSD_CTX *dsd_context_pointer;
{
    DD$OS_RECORD_DSD_PTR find_os_record_dsd ();

    /* Fill the Context Structure */
    dsd_context_pointer->CTX_type = os_record;  /* CTX for O/S record */

    /* Set current element */
    dsd_context_pointer->current_element = 0;

    /* Get the record DSD */
    dsd_context_pointer->event_DSD_ptr =
	find_os_record_dsd (dsd_context_pointer->event_type,
			    dsd_context_pointer->event_subtype);

    if (dsd_context_pointer->event_DSD_ptr == DD$UNKNOWN_RECORD)
        return DD$UNKNOWN_RECORD;

    return DD$SUCCESS;
}

/*
**++
**  find_os_record_dsd
**
**  o Description
**
**      Searches the OS-RECORD-LIST for the requested record DSD.
**
**	Returns a pointer to the record DSD, or DD$UNKNOWN_RECORD if
**	the event_type and event_subtype are not in the DSD tables.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**
**	DD$OS_RECORD_DSD_PTR
**	find_os_record_dsd (record_type, record_subtype)
**	DD$OS_DSD_CTX.event_type record_type;
**	DD$OS_DSD_CTX.event_subtype record_subtype;
**
**  o Remarks
**
**  o Example
**
**--
*/


DD$OS_RECORD_DSD_PTR
find_os_record_dsd (record_type, record_subtype)
unsigned short record_type;
long record_subtype;
{
    long i;

    for (i=0; i < os_record_dsd_count; i++)
	{
	 if (os_record_dsd_anchor[i].TYPE == record_type
	     && os_record_dsd_anchor[i].SUBTYPE == record_subtype)
	    return &os_record_dsd_anchor[i];
	};

    return DD$UNKNOWN_RECORD;
}

/*
**++
**  find_os_segment_dsd
**
**  o Description
**
**      Searches the OS-SEGMENT-LIST for the requested data-segment id.
**
**	Returns a pointer to the data-segment DSD, or DD$UNKNOWN_SEGMENT
**	if the data-segment id is not found in the DSD table.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**
**	DD$OS_SEGMENT_DSD_PTR
**      find_os_segment_dsd (segment_id)
**	DD$OS_SEGMENT_DSD.ID segment_id;
**
**  o Remarks
**
**  o Example
**
**--
*/


DD$OS_SEGMENT_DSD_PTR
find_os_segment_dsd (segment_id)
short segment_id;
{
    long i;

    for (i=0; i < os_segment_dsd_count; i++)
	{
	 if (os_segment_dsd_anchor[i].ID == segment_id)
	    return &os_segment_dsd_anchor[i];
	};

    return DD$UNKNOWN_SEGMENT;
}

/*
**++
**  find_os_item_dsd
**
**  o Description
**
**      Searches the OS-ITEM-LIST for the requested data-item id.
**
**	Returns a pointer to the data-item DSD, or DD$UNKNOWN_ITEM if
**	the data-item id is not found in the DSD table.
**
**  o Synopses
**
**      #include "generic_dsd.h"
**
**	DD$OS_ITEM_DSD_PTR
**      find_os_item_dsd (item_id)
**	DD$OS_ITEM_DSD.ID item_id;
**
**  o Remarks
**
**  o Example
**
**--
**
**++
**  Note:
**	The "find_os_item_dsd" function was modified and changed to a
**	#define macro in edit 1.13 in order to speed up OS record
**	processing.
**
**	The os_item_id was bound to the OS-ITEM-LIST index so that it
**	could be directly referenced.  See the note in the description
**	of the "dsd_init" function for additional information.
**--
**  Deleted code:
**
**	DD$OS_ITEM_DSD_PTR
**	find_os_item_dsd (item_id)
**	short item_id;
**	{
**	    long i;
**
**	    for (i=0; i < os_item_dsd_count; i++)
**		{
**		 if (os_item_dsd_anchor[i].ID == item_id)
**		    return &os_item_dsd_anchor[i];
**		};
**
**	    return DD$UNKNOWN_ITEM;
**	}
**
**  New #define macro:
*/

#define	find_os_item_dsd(os_item_id)					\
    (os_item_id <= os_item_dsd_count) ? &os_item_dsd_anchor[os_item_id]	\
				      : (DD$OS_ITEM_DSD_PTR)DD$UNKNOWN_ITEM
/*
**++
**  get_item_dsd
**  
**  o Description
**  
**      This function will update the CTX with the selected element in the
**      current operating system record.
**  
**      The function returns a completion code of DD$NO_ITEM if
**      the item does not exist in the current record.  It may
**	return a completion code of DD$UNKNOWN_SEGMENT if it is
**	unable to locate an O/S segment definition in the tables.
**	This is an internal table access error and either
**	indicates an inconsistancy in the tables, or that the
**	tables have been corrupted.
**
**	The DD$NO_ITEM return indicates that a standard item
**	is not available in the operating system record.  Thus
**	indicating that the validity bit for this item should
**	be set to DD$N_A.
**  
**  o Synopses
**  
**      #include "generic_dsd.h"
**  
**      long get_item_dsd (dsd_context_pointer, item_id)
**      DD$OS_DSD_CTX *dsd_context_pointer;
**	DD$OS_ITEM_DSD.ID item_id;
**  
**  o Remarks
**  
**  o Example
**  
**--
*/

long 
get_item_dsd (dsd_context_pointer, item_id)
DD$OS_DSD_CTX *dsd_context_pointer;
short item_id;
{
    DD$OS_ELEMENT_PTR	list,
			sub_list;
    DD$OS_ITEM_DSD_PTR	dsd_ptr;
    DD$OS_SEGMENT_DSD_PTR segment_DSD_ptr;
    DD$BYTE  *item_ptr;
    short    segment_id,
	     size;
    long     j,
	     k,
	     x,
	     y;


    /* Initialize the pointer etc. */
    item_ptr = (DD$BYTE *)dsd_context_pointer->event_ptr;   /* Base address */
    size = 0;
    list = dsd_context_pointer->event_DSD_ptr->ELEMENT;


    /* Verify that the item is in this record and calculate its position */

    for(j=0,k=0; j < dsd_context_pointer->event_DSD_ptr->ELEMENT_COUNT; j++,k++)
	{
	switch (list[k].OS_EL_FLD1)	/* FILLER_FLAG */
	    {
	    case (-1) :			/* FILLER */
		item_ptr += size;
		switch (size = list[k].OS_EL_FLD2)  /* FILLER_SIZE */
		    {
		    case (1) :
		    default :
			break;		/* No alignment needed! */
		    case (2) :
			ALIGN (item_ptr, ALIGN_S_FLD);
			break;
		    case (4) :
			ALIGN (item_ptr, ALIGN_L_FLD);
			break;
		    };
		break;

	    case (-2) :			/* ALIAS */
		k++;
		dsd_ptr = find_os_item_dsd (list[k].OS_EL_FLD1); /*OS_ITEM_ID */
		if (dsd_ptr == DD$UNKNOWN_ITEM)
		    return DD$UNKNOWN_ITEM;
		if (list[k].OS_EL_FLD2 == item_id) {     /* STD_ITEM_ID */
		    dsd_context_pointer->item_ptr = item_ptr;
		    dsd_context_pointer->item_DSD_ptr = dsd_ptr;
		    return DD$SUCCESS;
		};
		break;

	    case (-4) :			/* BEGIN STRUCTURE */
		item_ptr += size;
		ALIGN (item_ptr, list[k].OS_EL_FLD2);   /* UNITS */
		size = 0;
		break;

	    default :
		item_ptr += size;
		dsd_ptr = find_os_item_dsd (list[k].OS_EL_FLD1); /* OS_ITEM_ID*/
		if (dsd_ptr == DD$UNKNOWN_ITEM)
		    return DD$UNKNOWN_ITEM;
		item_ptr = fld_align (item_ptr, dsd_ptr->TYPE);
		size = get_item_size (dsd_ptr->TYPE, dsd_ptr->COUNT);
		if (list[k].OS_EL_FLD2 == item_id) {    /* STD_ITEM_ID */
		    dsd_context_pointer->item_ptr = item_ptr;
		    dsd_context_pointer->item_DSD_ptr = dsd_ptr;
		    return DD$SUCCESS;
		};
		break;

	    case (-3) :			/* SEGMENT */
		segment_id = list[k].OS_EL_FLD2;   /* SEGMENT_ID */
		segment_DSD_ptr = find_os_segment_dsd(segment_id);

		if (segment_DSD_ptr == DD$UNKNOWN_SEGMENT)
		    return DD$UNKNOWN_SEGMENT;

		sub_list = segment_DSD_ptr->ELEMENT;


		/* See if the item is in this segment */

		for (x=0,y=0; x < segment_DSD_ptr->ELEMENT_COUNT; x++,y++)
		  {
		  switch (sub_list[y].OS_EL_FLD1)  /* FILLER_FLAG */
		    {
		    case (-1) :			/* FILLER */
			item_ptr += size;
			switch (size = sub_list[y].OS_EL_FLD2) /* FILLER_SIZE */
			    {
			    case (1) :
			    default :
				break;		/* No alignment needed! */
			    case (2) :
				ALIGN (item_ptr, ALIGN_S_FLD);
				break;
			    case (4) :
				ALIGN (item_ptr, ALIGN_L_FLD);
				break;
			    };
			break;

		    case (-2) :			/* ALIAS */
			y++;
						/* OS_ITEM_ID */
			dsd_ptr = find_os_item_dsd (sub_list[y].OS_EL_FLD1);
			if (dsd_ptr == DD$UNKNOWN_ITEM)
			    return DD$UNKNOWN_ITEM;
						/* STD_ITEM_ID */
			if (sub_list[y].OS_EL_FLD2 == item_id) {
			    dsd_context_pointer->item_ptr = item_ptr;
			    dsd_context_pointer->item_DSD_ptr = dsd_ptr;
			    return DD$SUCCESS;
			};
			break;

		    case (-4) :			/* BEGIN STRUCTURE */
			item_ptr += size;
			ALIGN (item_ptr,
			       sub_list[y].OS_EL_FLD2);   /* UNITS */
			size = 0;
			break;

		    default :
			item_ptr += size;
						/* OS_ITEM_ID */
			dsd_ptr = find_os_item_dsd (sub_list[y].OS_EL_FLD1);
			if (dsd_ptr == DD$UNKNOWN_ITEM)
			    return DD$UNKNOWN_ITEM;
			item_ptr = fld_align (item_ptr, dsd_ptr->TYPE);
			size = get_item_size (dsd_ptr->TYPE, dsd_ptr->COUNT);
			if (sub_list[y].OS_EL_FLD2 == item_id) {
						/* STD_ITEM_ID */
			    dsd_context_pointer->item_ptr = item_ptr;
			    dsd_context_pointer->item_DSD_ptr = dsd_ptr;
			    return DD$SUCCESS;
			};
			break;
		    };
		  };
		break;			/* End case (-3)  SEGMENT */
	    };
	};
    return DD$NO_ITEM;
}

/*
**++
**  decode_std_item
**  
**  o Description
**  
**      Returns a pointer to the text string corresponding to the
**	standard code for the data-item currently pointed at by the
**	context block.
**
**	Returns DD$UNKNOWN_CODE if the code isn't valid for the item.
**  
**  o Synopses
**  
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**  
**      char *decode_std_item (dsd_context_pointer, item_code)
**      DD$STD_DSD_CTX *dsd_context_pointer;
**      short item_code;
**  
**  o Remarks
**  
**  o Example
**
**--
*/


char *
decode_std_item (dsd_context_pointer, item_code)
DD$STD_DSD_CTX *dsd_context_pointer;
short item_code;
{
    long i;

    for (i=0; i < dsd_context_pointer->item_DSD_ptr->COUNT; i++)
	{
	if (item_code == dsd_context_pointer->item_DSD_ptr->ITEM[i].CODE)
	   return dsd_context_pointer->item_DSD_ptr->ITEM[i].LABEL;
	};

    return DD$UNKNOWN_CODE;
}
/*
**++
**  get_code_std_item
**  
**  o Description
**  
**      Returns the code that corresponds to the text string passed
**	as an argument. This function is called when the text
**	string of a coded field is know and the acutual code value
**	is desired.
**
**	Returns DD$UNKNOWN_ITEM if the data-item id is not found in
**	the DSD table.
**  
**	Returns DD$UNKNOWN_CODE if the code isn't valid for the item.
**
**  o Synopses
**  
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**  
**      short get_code_std_item (item_id, code_str)
**      short item_id;
**      char *code_str;
**  
**  o Remarks
**  
**	This call does not use the data-item currently pointed to by
**	the context block.  The item-id must be supplied.
**
**  o Example
**
**--
*/


short
get_code_std_item (item_id, code_str)
short item_id;
char *code_str;
{
    long i,
	 j,
	 match,
    	 length_1;
    char string_1[MAX_STRING],
	 *string_2;

    DD$STD_ITEM_DSD_PTR wdsd;


    wdsd = find_std_item_dsd(item_id);
    if (wdsd == DD$UNKNOWN_ITEM)  return(DD$UNKNOWN_ITEM);

    length_1 = strlen(code_str);
    for (j=0; j <= length_1; j++)  string_1[j] = tolower(code_str[j]);

    for (i=0; i < wdsd->COUNT; i++)
	{
	string_2 = wdsd->ITEM[i].LABEL;
	if (length_1 != strlen(string_2))  continue;	/* Try the next code */

	match = TRUE;					/* Assume match */
	for (j=0; j < length_1; j++)
	    {
	    if (string_1[j] != tolower(string_2[j]))
		{
		match = FALSE;
		break;					/* Stop on mismatch */
		};
	    };

	if (match == TRUE)  return (wdsd->ITEM[i].CODE);
	};

    return DD$UNKNOWN_CODE;
}

/*
**++
**  decode_os_item
**  
**  o Description
**  
**      Returns the standard code corresponding to an O/S code for the
**	data-item currently pointed at by the context block.
**
**	Returns DD$UNKNOWN_CODE if the code isn't valid for the item.
**  
**  o Synopses
**  
**      #include "generic_dsd.h"
**  
**      short decode_os_item (dsd_context_pointer, item_code)
**      DD$OS_DSD_CTX *dsd_context_pointer;
**      long item_code;
**  
**  o Remarks
**  
**  o Example
**
**--
*/


short
decode_os_item (dsd_context_pointer, item_code)
DD$OS_DSD_CTX *dsd_context_pointer;
long item_code;
{
    long i;

    for (i=0; i < dsd_context_pointer->item_DSD_ptr->COUNT; i++)
	{
	if (item_code == dsd_context_pointer->item_DSD_ptr->MAP[i].OS_CODE)
	   return dsd_context_pointer->item_DSD_ptr->MAP[i].STD_CODE;
	};

    return DD$UNKNOWN_CODE;
}

/*
**++
**  decode_register_field
**  
**  o Description
**  
**      Returns a pointer to the text string corresponding to the
**	code for the register field currently pointed at by the
**	context block.
**
**	Returns DD$NO_TRANSLATION if the code isn't to be translated.
**  
**  o Synopses
**  
**      #include "generic_dsd.h"
**      #include "std_dsd.h"
**  
**      char *decode_register_field (dsd_context_pointer, field_code)
**      DD$STD_DSD_CTX *dsd_context_pointer;
**      long field_code;
**  
**  o Remarks
**  
**  o Example
**
**--
*/


char *
decode_register_field (dsd_context_pointer, field_code)
DD$STD_DSD_CTX *dsd_context_pointer;
long field_code;
{
    long i;

    for (i=0; i < dsd_context_pointer->field_DSD_ptr->CODE_COUNT; i++)
	{
	if (field_code == dsd_context_pointer->field_DSD_ptr->MAP[i].CODE)
	   return dsd_context_pointer->field_DSD_ptr->MAP[i].LABEL;
	};

    return DD$NO_TRANSLATION;
}

/* Set Validity codes

set_validity_code

o Description

    This function is only available to ERIT.  It sets the item validity
    code in the standard data-segment for the current item.

o Synopses

    #include "generic_dsd.h"

    int set_validity_code (std_dsd_context_pointer, item_validity_code)
    DD$STD_DSD_CTX *std_dsd_context_pointer;
    short item_validity_code;

o Remarks

    The "item_validity_code" must be one of the following:

	DD$VALID	The data is valid and available.

	DD$N_A		The data does not exist in this context.

	DD$N_V		The data is invalid, and the bad value is in
			the item. (i.e. The data does not meet the
			RANGE or LIST validity checks in the item's
			DSD)

	DD$N_V$N_A	The data is invalid and not available. (i.e.
			The data does not meet the RANGE or LIST
			validity checks in the item's DSD, and the
			bad value could not be stored in the item.)
			(e.g. ASCII number ==> short  -- and the string
			recieved contains non-numeric characters.)
			
o Example
*/
static unsigned int LBIT[4] = { 1,4,16,64 };
static unsigned int HBIT[4] = { 2,8,32,128};
static unsigned int ORMASK[4] = {0,1,2,3};
static unsigned int LEFTSHIFT[4] = {0,2,4,6};

setnurdle(nur,nn,nv)
unsigned char nur;
short nn;
short nv;
{
    /* first turn off both bits */
    nur &= ~(LBIT[nn-1] | HBIT[nn-1]);
    nur = nur | (ORMASK[nv] << LEFTSHIFT[nn-1]);
    return(nur);
}
getbits(x,p,n)
unsigned x,p,n;
{
    return((x >> (p+1-n)) & ~(~0 << n));
}

int
set_validity_code (std_dsd_context_pointer, item_validity_code)
DD$STD_DSD_CTX *std_dsd_context_pointer;
short item_validity_code;
{
short wbyte,wnurdle;
unsigned char thebyte,newbyte;
    wbyte = (std_dsd_context_pointer->current_element / 4) ;
    wnurdle = (std_dsd_context_pointer->current_element % 4) + 1;
    thebyte = std_dsd_context_pointer->segment_ptr->VALID_byte[wbyte];
    newbyte = setnurdle(thebyte,wnurdle,item_validity_code);    
    std_dsd_context_pointer->segment_ptr->VALID_byte[wbyte] = newbyte;
}

int
get_validity_code (std_dsd_context_pointer)
DD$STD_DSD_CTX *std_dsd_context_pointer;
{
short wbyte,wnurdle;
unsigned char thebyte;

    wbyte = (std_dsd_context_pointer->current_element / 4) ;
    wnurdle = (std_dsd_context_pointer->current_element % 4) + 1;
    thebyte = std_dsd_context_pointer->segment_ptr->VALID_byte[wbyte];
    return(getbits(thebyte,(wnurdle*2)-1,2));
}

/* Future stuff

-------------------------------------------------------------------------------
get_item_id

o Description

    Returns the data-item id that corresponds to the named item.

o Synopses

    #include "generic_dsd.h"

    long get_item_id (item_name)
    char *item_name;

o Remarks

    May be used in the future when the user can interactively type in
    a data-item name for selection.
			
o Example

*/

/* MISCELLANEOUS DEBUG FUNCTIONS */

#if DSD_ACCESS_DEBUG || DSD_BTT_DEBUG

/* FUNCTION TO DUMP THE CTX */

long dump_ctx (CTX_PTR)
DD$STD_DSD_CTX_PTR CTX_PTR;
{
    printf ("\n\n CTX at %u (%x):\n\n", CTX_PTR, CTX_PTR);
    printf ("\t       CTX_type:  %d\n", CTX_PTR->CTX_type);
    printf ("\tcurrent_element:  %d\n", CTX_PTR->current_element);
    printf ("\t    segment_ptr:  %u (%x)\n", CTX_PTR->segment_ptr,
					       CTX_PTR->segment_ptr);
    printf ("\tsegment_DSD_ptr:  %u (%x)\n", CTX_PTR->segment_DSD_ptr,
					       CTX_PTR->segment_DSD_ptr);
    printf ("\t seg_VALID_code:  %d\n", CTX_PTR->segment_VALID_code);
    printf ("\t       item_ptr:  %u (%x)\n", CTX_PTR->item_ptr,
					       CTX_PTR->item_ptr);
    printf ("\t   item_DSD_ptr:  %u (%x)\n", CTX_PTR->item_DSD_ptr,
					       CTX_PTR->item_DSD_ptr);
    printf ("\titem_VALID_code:  %d\n\n", CTX_PTR->item_VALID_code);
}

#endif

/* MISCELLANEOUS DEBUG FUNCTIONS  cont. */

#if DSD_DUMP_TABLES
/* FUNCTION TO DUMP THE DSD TABLES */
long dump_dsd_tables ()
{
    long i,
	 j;

    printf ("\n\n\nDUMP OF DATA STRUCTURE DEFINITION TABLES\n\n");
    printf ("  binary_dsd.bin file format: %d\n", bin_file_format);



    /* Dump the standard data-item table */

    printf("\n\nSTD-ITEM-LIST  (table.version: %g)\n\n", std_item_dsd_version);

    printf (" ID      NAME     TYPE DISPLAY   LABEL             CODES\n");
    for (i = 0; i < std_item_dsd_count; i++) {
	printf ("\n%3d  %-12s %3d   %3d    %-20s",
	    std_item_dsd_anchor[i].ID,
	    std_item_dsd_anchor[i].NAME,
	    std_item_dsd_anchor[i].TYPE,
	    std_item_dsd_anchor[i].DISPLAY,
	    std_item_dsd_anchor[i].LABEL); 

	if (std_item_dsd_anchor[i].TYPE == DT_SHORT_INDEX
	    || std_item_dsd_anchor[i].TYPE == DT_INDEXED)
	{
	  printf("  %d", std_item_dsd_anchor[i].COUNT); 
	  for (j = 0; j < std_item_dsd_anchor[i].COUNT; j++) {
	    printf("\n                               %3d  %-12s %-20s", 
		std_item_dsd_anchor[i].ITEM[j].CODE,
		std_item_dsd_anchor[i].ITEM[j].NAME,
		std_item_dsd_anchor[i].ITEM[j].LABEL);
	  };
	};
    };


    /* Dump the standard data-segment table */

    printf("\n\n\fSTD-SEGMENT-LIST  (table.version: %g)\n\n",
	std_segment_dsd_version);

    printf ("TYPE SUBTYPE  LABEL");
    printf ("                      CNT\n");
    for (i = 0; i < std_segment_dsd_count; i++) {
	printf ("\n%3d  %3d     %-25s %3d\n\t\t\tELEMENTS: ",
	    std_segment_dsd_anchor[i].TYPE,
	    std_segment_dsd_anchor[i].SUBTYPE,
	    std_segment_dsd_anchor[i].LABEL,
	    std_segment_dsd_anchor[i].ELEMENT_COUNT);
	for (j = 0; j < std_segment_dsd_anchor[i].ELEMENT_COUNT; j++) {
	    printf (" %3d", std_segment_dsd_anchor[i].ELEMENT[j]);
	};
    };

/* MISCELLANEOUS DEBUG FUNCTIONS  cont. */

    /* Dump the operating system data-item table */

    printf("\n\n\fOS-ITEM-LIST  (table.version: %g)\n\n",
	os_item_dsd_version);

    printf (" ID  TYPE  CODES\n");
    for (i = 0; i < os_item_dsd_count; i++) {
	printf ("\n%3d  %3d  ",
	    os_item_dsd_anchor[i].ID,
	    os_item_dsd_anchor[i].TYPE);

	if (os_item_dsd_anchor[i].TYPE == DT_TINY_INDEX
	    || os_item_dsd_anchor[i].TYPE == DT_SHORT_INDEX
	    || os_item_dsd_anchor[i].TYPE == DT_INDEXED)
	{
	  printf("  %d", os_item_dsd_anchor[i].COUNT); 
	  for (j = 0; j < os_item_dsd_anchor[i].COUNT; j++) {
	    printf("\n             %3d  %3d", 
		os_item_dsd_anchor[i].MAP[j].OS_CODE,
		os_item_dsd_anchor[i].MAP[j].STD_CODE);
	  };
	};
    };


    /* Dump the operating system data-segment table */

    printf("\n\n\fOS-SEGMENT-LIST  (table.version: %g)\n\n",
	os_segment_dsd_version);

    printf (" ID                 CNT  ELEMENTS\n");
    for (i = 0; i < os_segment_dsd_count; i++) {
	printf ("\n%3d                %3d      ",
	    os_segment_dsd_anchor[i].ID,
	    os_segment_dsd_anchor[i].ELEMENT_COUNT);

	for (j = 0; j < os_segment_dsd_anchor[i].ELEMENT_COUNT; j++) {
	    printf ("\n\t\t\t %3d %d",
		os_segment_dsd_anchor[i].ELEMENT[j].OS_EL_FLD1,  /*OS_ITEM_ID */
		os_segment_dsd_anchor[i].ELEMENT[j].OS_EL_FLD2); /*STD_ITEM_ID*/
	};
    };


    /* Dump the operating system data-record table */

    printf("\n\n\fOS-RECORD-LIST  (table.version: %g)\n\n",
	os_record_dsd_version);

    printf ("type  subtype CNT  ELEMENTS\n");
    for (i = 0; i < os_record_dsd_count; i++) {
	printf ("\n%3d   %3d    %3d      ",
	    os_record_dsd_anchor[i].TYPE,
	    os_record_dsd_anchor[i].SUBTYPE,
	    os_record_dsd_anchor[i].ELEMENT_COUNT);

	for (j = 0; j < os_record_dsd_anchor[i].ELEMENT_COUNT; j++) {
	    printf ("\n\t\t   %3d %d",
		os_segment_dsd_anchor[i].ELEMENT[j].OS_EL_FLD1,  /*OS_ITEM_ID */
		os_segment_dsd_anchor[i].ELEMENT[j].OS_EL_FLD2); /*STD_ITEM_ID*/
	};
    };
}

#endif
