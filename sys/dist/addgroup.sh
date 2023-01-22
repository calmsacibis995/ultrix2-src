:
# @(#)addgroup.sh	1.2	10/16/86
#
#									
# 			Copyright (c) 1984 by				
# 		Digital Equipment Corporation, Maynard, MA		
# 			All rights reserved.				
# 									
#    This software is furnished under a license and may be used and	
#    copied  only  in accordance with the terms of such license and	
#    with the  inclusion  of  the  above  copyright  notice.   This	
#    software  or  any  other copies thereof may not be provided or	
#    otherwise made available to any other person.  No title to and	
#    ownership of the software is hereby transferred.			
# 									
#    The information in this software is subject to change  without	
#    notice  and should not be construed as a commitment by Digital	
#    Equipment Corporation.						
# 									
#    Digital assumes no responsibility for the use  or  reliability	
#    of its software on equipment which is not supplied by Digital.	
#
# Purpose:	Add a new group number to /etc/group
# Usage:	addgroup
# Environment:	Bourne shell script
# Date:		3/19/84
# Author:	afd
# 
# Remarks:
#    Returns the group number.  Uses negative return values for errors.
#

# Trap ^c signal

trap "" 2

# Make sure its root.

if test ! -w /etc/group
then
    echo "Please su to root first."
    exit 1
fi

# If group name not supplied (ie not called by adduser),
#   then get group name to add.

if test $1
then
    getgroup=""
    group=$1
else
    getgroup=true
fi

#
# See if this system is a Yellow Pages client.
#
tail -1 /etc/group | grep "^[+-]:" > /dev/null
if [ $? -eq 0 ]
then
	/etc/unlockpw
	echo "
This system makes use of the Yellow Pages service for
group administration.  Please add the new group by
following the steps given in the Overview of Yellow
Pages chapter of the Network Management Guide."
	exit 5
fi

if test $getgroup
then
#   Lock the passwd and group files

    /etc/lockpw
    exstat="$?"
    if test $exstat != 0
    then
	exit $exstat
    fi


#
#   Get new group name

    again=yes
    while test $again
	do
	echo -n "Enter group name for new group: "
	read group

	if test $group
	then
#     	    See if group already exists in passwd file, if so exit.
	    grep "^$group:" /etc/group > /dev/null
	    if test $? = 0
	    then
		/etc/unlockpw
		echo " Group $group already in /etc/group file."
		exit 1
	    else
		again=""
	    fi
	else
	    again=yes
	fi
	done
fi

# Get the group number for the new user.  Sort the group file on gid,
#   get the largest gid, validity check it as a valid number,
#   then add 5 to it to get the new gid.

gid=`sort -nt: +2 -3 /etc/group | tail -1 | awk -F: '{print $3}' | sed -n '/[0-9][0-9]*/p'`

# Check for valid $gid

if test $gid
then
    ok=true
else
    echo ""
    echo "The /etc/group file is corrupt!"
    echo "Exiting $0 script.  New entry not created!"
    /etc/unlockpw
    exit 1
fi
defgid=`expr $gid + 5`

again=yes
while test $again
    do
    again=
    echo ""
    echo -n "Enter group number for new group [$defgid]: "
    read gnum

    if test $gnum
    then
#     	See if group number already exists in group file, if so get another.
	gid=`grep "$gnum:" /etc/group | awk -F: '{print $3}'`
	if test $gid
	then
	    if test $gid = $gnum
	    then
		again=yes
		echo " Group number $gnum is already in /etc/group file."
	    fi
	fi
    else
	gnum=$defgid
    fi
    done

# Add the group to the /etc/group file

echo "$group:*:$gnum:" >> /etc/group

if test $getgroup
then
#   Unlock the password and group files

    /etc/unlockpw
fi

exit $gnum
