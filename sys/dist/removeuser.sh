#!/bin/sh
#
#  @(#)removeuser.sh	1.3	ULTRIX	10/3/86
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
# Purpose:	To remove a user from the passwd file
# Usage:	removeuser
# Environment:	Bourne shell script
# Date:		3/20/84
# Author:	afd
# 
# Remarks:
#	Removes user from /etc/passwd file, from /etc/group file, and
#	optionally deletes the user's home directory, all sub-directories
#	and files.
#
#	MODS:
#	001	ccb	09-03-86
#		code compression and acceleration
#
# Trap ^c signal
trap "" 1 2 3

# Lock the passwd and group files
/etc/lockpw || exit $?
trap '/etc/unlockpw;exit $STAT' 0
NL="
"
MAIL=/usr/spool/mail
STAT=1	# assume failure

# Get the user's login name
echo -n "${NL}Enter login name for user to be removed: "
read USER
case "$USER" in
"")	STAT=0;exit
	;;
*)	USERENTRY=`grep "^$USER:" /etc/passwd` ||
	{
		echo "User $USER not in /etc/passwd file."
		exit
	}
	;;
esac

# Display user_entry record from passwd file and confirm it with user.
echo "This is what the entry in /etc/passwd looks like:$NL$NL	$USERENTRY"
while :
do
	echo -n "$NL${NL}Is this the entry you wish to delete? "
	read _X_
	case "$_X_" in
	[yY]*)	break
		;;
	[nN]*)	echo "${NL}User $USER not removed."
		STAT=0;exit
		;;
	esac
done

echo "Working ..."

# Remove the user from /etc/passwd
cp /etc/passwd /tmp/passwd
ed - /tmp/passwd <<EOF
/^${USER}:/d
w
q
EOF
mv /tmp/passwd /etc/passwd

# Remove the user /etc/group
cp /etc/group /tmp/group
ed - /tmp/group <<EOF
g/:$USER/s/$USER//
g/:,/s//:/
g/,$USER/s///
w
q
EOF
mv /tmp/group /etc/group

echo "${NL}User $USER removed."

# free up the password file.
/etc/unlockpw
STAT=0	# no failure exits beyond this point
trap '' 0
HOMEPATH=`echo $USERENTRY|awk -F: '{print $6}'`

while :
do
	echo -n "${NL}Do you want to remove $USER's home directory,
all subdirectories and files (y/n)? "
	read _X_
	case $_X_ in
	[yY]*)	while :
		do
			echo -n "
You should have backed up $USER's files if you do not wish to lose them.$NL
Are you sure that you want to remove $USER's files (y/n)? "
			read _X_
			case "$_X_" in
			[yY]*)
				echo "${NL}Deleting $HOMEPATH"
				rm -rf $HOMEPATH $MAIL/$USER 2>&1 > /dev/null
				exit 0
				;;
			[nN]*)	break 2
				;;
			esac
		done
		;;
	[nN]*)	break
		;;
	esac
done

# 'no'
echo "$NL$HOMEPATH not deleted."
