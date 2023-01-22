:
#
#  @(#)adduser.sh	1.7	ULTRIX	1/9/87
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
# Purpose:	To add new users to the group and passwd file
# Usage:	adduser
# Environment:	Bourne shell script
# Date:		3/15/84
# Author:	afd
# 
# Remarks:
#	Interactive adduser script to ease the process of adding accounts.
#

# Trap ^c signal

trap "" 2
trap "/etc/unlockpw" 0

# Lock the passwd and group files

/etc/lockpw
exstat="$?"
if test $exstat -ne 0
then
    exit $exstat
fi

#
# See if this system is a Yellow Pages client.
#
tail -1 /etc/passwd | grep "^[+-]:" > /dev/null
yp_used="$?"
tail -1 /etc/group | grep "^[+-]:" > /dev/null
if [ $? -eq 0 ] || [ $yp_used -eq 0 ]
then
	/etc/unlockpw
	echo "
This system makes use of the Yellow Pages service for
user account administration.  Please add the new user
by following the steps given in the Overview of Yellow
Pages chapter of the Network Management Guide."
	exit 5
fi

# Get new user's login name

again=yes
while test $again
    do
    echo ""
    echo -n "Enter login name for new user (initials, first or last name): "
    read user

    if test "${user}"
    then
#	See if user already exists in passwd file, if so exit.
	grep "^${user}:" /etc/passwd > /dev/null
	if test $? -eq 0
	then
	    /etc/unlockpw
	    echo " User $user already in /etc/passwd file."
	    exit 5
	else
	    again=""
	fi
    else
	exit 5
    fi
    done

# Get new user's real name

again=yes
while test $again
    do
    echo -n "Enter full name for new user: "
    read name

    if test "$name"
    then
	again=""
    else
	again=yes
    fi
    done

# Get the login group for the new user.

again=yes
while test $again
    do
    echo -n "What login group should this user go into [ users ] ? "
    read loggroup
    echo "Working ..."

    if test $loggroup
    then
	if [ ! -d /usr/$loggroup ]
	then
		echo "The /usr/$loggroup directory was not found, /usr/$loggroup must be
created before users may be added to that login directory."
		/etc/unlockpw
		exit 1
	else
		null=
	fi
    else
	loggroup=users
    fi

#   Get the group ID for the new user

    gid=`grep "^$loggroup:" /etc/group | awk -F: '{print $3}' `
    if test $gid
    then again=""
    else
	echo ""
	echo "  Unknown group: $loggroup."
	echo "Known groups are:"
	/bin/awk -F: '{print $1}' /etc/group | pr -t -l1 -4
	ynagain=yes
	while test $ynagain
	    do
	    ynagain=
	    echo -n "Do you want to add group $loggroup to the /etc/group file [yes] ? "
	    read addgroup
	    case $addgroup in
	    [yY]*|"")
		addgroup=y
		;;
	    [nN]*)
		;;
	    *)
		ynagain=yes
		;;
	    esac
	    done

	if test $addgroup = y
	then
#	    Add new group to group file
	    echo ""
	    echo "Adding new group to /etc/group file..."
	    addgroup $loggroup
#	    Addgroup script returns the gid number.
	    gid=$?
	    again=""
	else
#	    Ask for login group again
	    again=yes
	fi
    fi
    done
loggid=$gid

# Get other groups if this user is to be part of any others

echo "
Enter another group that '$user' should be a member of"
echo -n "(<RETURN> only if none): "
read group

while test $group
    do
    echo "Working ..."
    adduser=
#   Check for group in group table
    gid=`grep "^$group:" /etc/group | awk -F: '{print $3}' `

    if test $gid
    then adduser=y
    else
	echo ""
	echo "  Unknown group: $group."
	echo "Known groups are:"
	/bin/awk -F: '{print $1}' /etc/group | pr -t -l1 -4
	ynagain=yes
	while test $ynagain
	    do
	    ynagain=
	    echo -n "Do you want to add group $group to the /etc/group file [yes] ? "
	    read addgroup
	    case $addgroup in
	    [yY]*|"")
		addgroup=y
		;;
	    [nN]*)
		;;
	    *)
		ynagain=yes
		;;
	    esac
	    done

	if test $addgroup = y
	then
#	    Add new group to group file
	    adduser=y
	    echo ""
	    echo "Adding new group to group file..."
	    addgroup $group
#	    Addgroup script returns the gid number
	    gid=$?
	fi
    fi

    if test $adduser
    then
#	Add the user to each group as it is specified 

	grep "^$group:" /etc/group | grep "$user" > /dev/null
	if test $? -eq 0
	then 
	    echo ""
	    echo "  User $user is already a member of group $group."
	else
	    rm -f /tmp/group > /dev/null
	    cp /etc/group /tmp/group
	    ed /tmp/group > /dev/null << EOF
	    /^$group:/s/\$/,$user/
	    g/:,/s//:/
	    w
	    q
EOF
	    mv /tmp/group /etc/group
	fi
    fi


#   Any more groups for this user?

    echo ""
    echo "Enter another group that '$user' should be a member of"
    echo -n "(<RETURN> only if no more): "
    read group

    done

echo -n "Enter parent directory for ${user} [/usr/users]: "
read parent
case ${parent} in
"") parent=/usr/users
	;;
esac
while test \! -d "${parent}"
do
	echo "${parent} not found"
	echo -n "Enter parent directory for user (or "exit" to exit) [/usr/users] :"
	read parent
	case ${parent} in
	"")
		parent=/usr/users
		;;
	exit)
		/etc/unlockpw
		exit 5
		;;
	*)
		;;
	esac
done

echo "
Adding new user ..."

# Get the user ID for the new user.  Sort the passwd file on uid,
#   get the largest uid, validity check it as a valid number,
#   then add one to it to get the new uid.

uid=`sort -nt: +2 -3 /etc/passwd | tail -1 | awk -F: '{print $3}' | sed -n '/[0-9][0-9]*/p'`

# Check for valid $uid

if test $uid
then
    ok=true
else
    echo ""
    echo "The pass word file (/etc/passwd) may be corrupt!"
    echo "Exiting $0 script.  New entry not created!"
    /etc/unlockpw
    exit 1
fi
uid=`expr $uid + 1`

# Create home and bin directories, and set-up files

if test \! -d /usr/users
then
    mkdir /usr/users
fi
if test \! -d /usr/spool
then
    mkdir /usr/spool
fi
if test \! -d /usr/spool/mail
then
    mkdir /usr/spool/mail
fi

# Add the user to the password file.

echo "$user::$uid:$loggid:$name:${parent}/${user}:/bin/csh" >> /etc/passwd

mkdir ${parent}/${user} ${parent}/${user}/bin
cp /usr/skel/.profile* ${parent}/${user}/.profile
cp /usr/skel/.cshrc* ${parent}/${user}/.cshrc
cp /usr/skel/.login* ${parent}/${user}/.login
> /usr/spool/mail/${user}
chmod 0755 ${parent}/${user} ${parent}/${user}/bin ${parent}/${user}/.cshrc ${parent}/${user}/.login ${parent}/${user}/.profile
chmod 0600 /usr/spool/mail/${user} 2> /dev/null
chgrp $loggroup ${parent}/${user} ${parent}/${user}/bin ${parent}/${user}/.cshrc ${parent}/${user}/.login ${parent}/${user}/.profile /usr/spool/mail/${user}
/etc/chown ${user} ${parent}/${user} ${parent}/${user}/bin ${parent}/${user}/.cshrc ${parent}/${user}/.login ${parent}/${user}/.profile /usr/spool/mail/${user}

echo "
The new user account initially has no password."

# Unlock the password and group files

/etc/unlockpw
exit 0
