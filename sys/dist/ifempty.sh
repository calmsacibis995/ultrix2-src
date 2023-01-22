
# ifempty.sh
#
# Purpose:	
# Usage:	ifempty.sh [/path] <directory-list
#		  where: directory-list is output by getstat
# Environment:	Bourne shell script
# Date:		3/26/84
# Author:	Alan Delorey
# 
# Remarks:
#    This script appends the empty directories to the end of the inv
#    list.
#

read size uid gid mode date rev type path links_to

# while not EOF

while test $? = 0
do
    num=`ls $1$path | wc -l`
    if test $num = 0
    then
	echo "$size	$uid	$gid	$mode	$date	$rev	$type	$path	$links_to" >> inv
    fi
    read size uid gid mode date rev type path links_to
done
