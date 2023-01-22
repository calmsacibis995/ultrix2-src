#! /bin/sh
#  @(#)nfssetup.sh	1.13	ULTRIX	3/3/87
#									
# 			Copyright (c) 1986 by				
# 		Digital Equipment Corporation, Maynard, MA		
# 			All rights reserved.				
# 									
#    This software is furnished under a license and may be used and	
#    copied  only  in accordance with the terms of such license and	
#    with the  inclusion  of  the  above  copyright  notice.    This	
#    software  or  any  other copies thereof may not be provided or	
#    otherwise made available to any other person.   No title to and	
#    ownership of the software is hereby transferred.			
# 									
#    The information in this software is subject to change  without	
#    notice  and should not be construed as a commitment by Digital	
#    Equipment Corporation.						
# 									
#    Digital assumes no responsibility for the use  or  reliability	
#    of its software on equipment which is not supplied by Digital.	
#
# Purpose:	Set up NFS environment
# Usage:	nfssetup [install]
# Environment:	Bourne shell script
# Date:		6/11/86
# Author:	Fred L. Templin
# 
# Remarks:
#    Sets up files:
#	/etc/rc.local
#	/etc/exports
#	/etc/fstab
#
#    Much of this has been borrowed from "netsetup.sh"
#

#
# Set up interrupt handlers:
#
QUIT='
	if [ -r $EXTMP ]
	then
		rm $EXTMP
	fi
	if [ -r $FSTMP ]
	then
		rm $FSTMP
	fi
	echo "Nfssetup terminated with no installations made."
	exit 1
'
#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

EXTMP=/tmp/nfssetup.ex.$$
FSTMP=/tmp/nfssetup.fs.$$
VMUNIX=/vmunix
if [ $DEBUG ]
then
	RCFILE=./rc.tmp
	FSFILE=/dev/null
	EXFILE=/dev/null
	NFILE=/etc/networks
	NFSSETUP=./nfssetup
else
	RCFILE=/etc/rc.local
	FSFILE=/etc/fstab
	EXFILE=/etc/exports
	NFILE=/etc/networks
	NFSSETUP=/etc/nfssetup

	#
	# Require it to be run by root
	#
	if [ \! -w $RCFILE ]
	then
		echo "Please su to root first."
		eval "$QUIT"
	fi
fi

#
# PHASE ONE: Gather data!!
#

echo "Checking kernel configuration..."
nm $VMUNIX | grep -s '_nfs_namei' 
if [ $? -ne 0 ]
then
	echo "
In order to make use of the network file system (NFS) services,
you must first configure the NFS support code into your ULTRIX-32
kernel.  Please consult the System Management Guide for information
on how to configure and bootstrap the new ULTRIX-32 kernel."
	eval "$QUIT"
fi

egrep -s '^#.*%NFSSTART%' $RCFILE
if [ $? -eq 0 ]
then
	echo "
The network file system has already been installed.  Please
examine the file ${RCFILE} to find the current configuration."
	eval "$QUIT"
fi
echo "
The nfssetup command configures the network file system (NFS)
environment for your system.  All systems using NFS facilities
must run the Remote Procedure Call (RPC) port mapper daemon.
An entry for this daemon is placed in the /etc/rc.local file
along with entries for the optional daemons you select.

You will be asked a series of questions about your system.
Default answers are shown in square brackets ([]).  To use a
default answer, press the RETURN key."

#
#   Determine state of this NFS machine.  PURE client (no exports made),
# or exporter.
#
nnfsd=0
nbiod=0
serving=""
again=y
while [ $again ]
do
	again=""
	echo ""
	echo -n "Will you be exporting any directories [n] ? "
	read serving
	case $serving in
	[yY]*)
	    	serving=y
    		;;
	[nN]*|"")
    		serving=n
    		;;
	*)
	    	again=y
	    	;;
	esac
done

if [ $serving = y ]
then
#
# Since we'll be serving up directories, need to put "nfsd"
# in the local rc script along with the others.  We'll ask
# to see if the user wants more or less daemons than usual.
#
	nnfsd=4
	echo ""
	echo "
	Systems that export NFS directories must run /etc/nfsd to
	handle NFS requests from clients.  You can configure up
	to 20 nfsd daemons, but for average workload situations,
	4 is a good number to run."
	flag=y
	while [ $flag ]
	do
		flag=""
		echo ""
		echo -n "	Enter the number of nfsd servers to run [4] : "
		read num
		if [ $num ]
		then
			if [ $num -le 0 ]
			then
				flag=y
				echo "	Number must be greater than zero"
			fi
			if [ $num -ge 20 ]
			then
				nnfsd=20
			else
				nnfsd=$num
			fi
		fi
	done

fi
#
# Ask if he wants any \"biod\" daemons.
#
echo ""
echo "
	NFS clients can use block I/O daemons for buffering
	data transfers, although their use is not required.
	You can configure up to 5 biod daemons on your system
	based upon the workload you expect, but for average
	workload situations, 4 is a good number to run."
flag=y
nbiod=4
while [ $flag ]
do
	flag=""
	echo ""
	echo -n "	Enter the number of block I/O daemons to run [4] : "
	read num
	if [ $num ]
	then
		if [ $num -lt 0 ]
		then
			flag=y
			echo "	Number must be greater than or equal zero"
		fi
		if [ $num -ge 5 ]
		then
			nbiod=5
		else
			nbiod=$num
		fi
	fi
done
#
# Ask if he wants to run the rwalld daemon...
#
echo ""
echo "
	NFS clients that rely heavily on having certain NFS
	directories mounted may wish to be notified in the
	event of NFS servers going down.  In order for users
	on your system to receive notifications, you must run
	the remote wall daemon. (rwalld)"
again=y
while [ $again ]
do
	again=""
	echo ""
	echo -n "	Would you like to run the rwalld daemon [n] ? "
	read rwall
	case $rwall in
	[yY]*)
	    	rwall=y
    		;;
	[nN]*|"")
    		rwall=n
    		;;
	*)
	    	again=y
	    	;;
	esac
done

#
# He's exporting directories.  Find out which ones and validate them
# but don't add them to "/etc/exports" just yet!
#
if [ $serving = y ]
then
	echo "
You are now setting up your directory export list.  Enter the
full pathnames of the directories to be exported.  For each
pathname, enter the network group names and/or machine names to
be given access permission to this directory, or a null list to
indicate general permission.  (Network groups are ONLY available
on machines using Yellow Pages).  This information is placed in the
/etc/exports file.  Press the RETURN key to terminate the pathname
and permissions lists."
	more_paths=y
	while [ $more_paths ]
	do
		more_paths=""
		permlist=""
		echo ""
		echo -n "Enter the directory pathname: "
		read dirname
		if [ $dirname ]
		then
			more_paths=y
			if [ -d $dirname ]
			then
				more_perms=y
				while [ $more_perms ]
				do
					more_perms=""
					echo -n "	Netgroup/Machine name: "
					read permname
					if [ -n "$permname" ]
					then
						more_perms=y
						permlist=`echo $permlist $permname | cat`
					fi
				done
				echo "$dirname		$permlist" >> $EXTMP
				serving=y
			else 
				echo "
The pathname: ${dirname}
is not a valid directory.
"
			fi
		else
		echo "Directory export list complete..."
		fi
	done
fi

#
# Find out which file systems from which machines are to be imported.
#
echo "
You will now be asked to provide information about the remote file
systems you wish to access.  First list the name of the remote host
serving the directories you wish to mount, then give the full directory
pathnames.  Also, for each remote directory, you must specify the full
directory pathname of the mount point on the local machine and whether
the mount is read-only or read-write.  (Nfssetup will create the mount
point directory if it does not already exist.)  Press the RETURN key to
terminate the host and directory pathname lists:"
more_hosts=y
newdirs=""
while [ $more_hosts ]
do
	more_hosts=""
	echo ""
	echo -n "Enter the remote host name: "
	read hostid
	if [ $hostid ]
	then
		more_hosts=y
		more_paths=y
		while [ $more_paths ]
		do
			more_paths=""
			echo -n "
	Enter the remote directory pathname: "
			read rdir
			if [ -n "$rdir" ]
			then
				more_paths=y
				again=y
				while [ $again ]
				do
					again=""
					echo -n "	Enter the local mount point: "
					read ldir
					if [ -z "$ldir" ]
					then
						again=y
					elif [ -f $ldir ]
					then
						echo "
	${ldir}: File exists! Please choose a new mount point."
						again=y
					elif [ \! -d $ldir ]
					then
						echo "
	${ldir}: Directory does not exist, but will be created."
						newdirs=`echo $newdirs $ldir | cat`
					fi
				done

				again=y
				while [ $again ]
				do
					again=""
					echo -n "	Is this a read-only mount [y] ? "
					read readonly
					case $readonly in
					[nN]*)
						echo "${rdir}@${hostid}:${ldir}:rw:0:0:nfs:bg:" >> $FSTMP
						;;
					[yY]*|"")
						echo "${rdir}@${hostid}:${ldir}:ro:0:0:nfs:bg:" >> $FSTMP
						;;
					*)
						again=y
						;;
					esac
				done
			fi
		done
	else
		echo "Remote directory mount list complete..."
	fi
done

#
# Ask user for verification...
#
echo "
Please confirm the following information which you
have entered for your NFS environment:
"
if [ $serving = y ]
then
	echo "	${nnfsd} nfsd daemons"
	echo "	${nbiod} biod daemons"
	if [ $rwall = y ]
	then
		echo "	rwalld daemon installed"
	fi
	if [ -s $EXTMP ]
	then
		echo "
	Directory export list:"
		awk '
		{
			printf "\t\t%s", $1
			if ( NF > 1 ) {
				printf " exported to:"
				for ( i = 2; i <= NF; i++ )
					printf " %s", $i
				printf "\n"
			}
			else
				printf " exported with general permissions\n"
		}' $EXTMP
	else
		echo "
	No directories exported"
	fi
else
	echo "	${nbiod} biod daemons"
	if [ $rwall = y ]
	then
		echo "	rwalld daemon installed"
	fi
	echo "
	No directories exported"
fi
if [ -s $FSTMP ]
then
	echo "
	Remote directory mount list:"
	awk '
	BEGIN { FS = ":" }
	{
		if ( $3 == "ro" )
			printf "\t\t%s mounted on: %s (Read Only)\n", $1, $2
		else
			printf "\t\t%s mounted on: %s\n", $1, $2
	}' $FSTMP
else
	echo "
	No remote directories to mount"
fi
again=y
while [ $again ]
do
	echo -n "
Enter \"c\" to CONFIRM the information, \"q\" to QUIT nfssetup
without making any changes, or \"r\" to RESTART the procedure [no default]: "
	read conf
	case $conf in
		[qQ]*)
		[ -r $EXTMP ] && rm $EXTMP
		eval "$QUIT"
    		;;
		[rR]*)
		[ -r $EXTMP ] && rm $EXTMP
		exec $NFSSETUP $*
		;;
		[cC]*)
		again=""
		;;
		*)
    		again=y
    		;;
	esac
done
#
# PHASE TWO...  Update files!!
#
trap "" 1 2 3 15

echo "
Updating files:"
#
# See if the port mapper is already there...
#
echo "	/etc/rc.local"
egrep -s '[ 	]*/etc/portmap' $RCFILE
if [ $? -ne 0 ]
then
	#
	# Not there...  put it in!!
	#
	echo "#
# RPC port mapper daemon
#
echo -n 'RPC port mapper:'			>/dev/console
if [ -f /etc/portmap ]; then
	/etc/portmap &
	echo ' started.'			>/dev/console
else
	echo ' not found.'			>/dev/console
fi" >> $RCFILE
fi

#
# Put in banner...
#
echo "#
# %NFSSTART% - NFS daemons added by \"nfssetup\"
#
echo -n 'NFS daemons:'						>/dev/console" >> $RCFILE
#
# Put in mountd IF we're serving...
#
if [ $serving = y ]
then
echo "if [ -f /etc/mountd -a -f /etc/portmap -a -s ${EXFILE} ]; then
	/etc/mountd & echo -n ' mountd'		>/dev/console
fi" >> $RCFILE
fi
#
# Install optional daemons...
#
if [ $nnfsd -gt 0 ] && [ $nbiod -gt 0 ]
then
	echo "if [ -f /etc/nfsd -a -f /etc/portmap ]; then
	/etc/nfsd ${nnfsd} & echo -n ' nfsd'				>/dev/console
fi
if [ -f /etc/biod ]; then
	/etc/biod ${nbiod} & echo -n ' biod'				>/dev/console
fi" >> $RCFILE
elif [ $nnfsd -gt 0 ]
then
	echo "if [ -f /etc/nfsd -a -f /etc/portmap ]; then
	/etc/nfsd ${nnfsd} & echo -n ' nfsd'				>/dev/console
fi" >> $RCFILE
elif [ $nbiod -gt 0 ]
then
	echo "if [ -f /etc/biod ]; then
	/etc/biod ${nbiod} & echo -n ' biod'				>/dev/console
fi" >> $RCFILE
fi
if [ $rwall = y ]
then
	echo "if [ -f /usr/etc/rwalld -a -f /etc/portmap ]; then
	/usr/etc/rwalld & echo -n ' rwalld'				>/dev/console
fi" >> $RCFILE
fi
echo "echo '.'							>/dev/console
#
# Mount remote directories...
#
echo -n 'mounting NFS directories:'				>/dev/console
/etc/nfs_umount -b					>/dev/null 2>&1
/etc/mount -a -t nfs					>/dev/console 2>&1
echo ' done.'				>/dev/console
#
# %NFSEND%
#" >> $RCFILE

#
# Update export list
#
if [ -r $EXTMP ]
then
	echo "	/etc/exports"
	cat $EXTMP >> $EXFILE
	rm $EXTMP
fi

#
# Update fstab
#
if [ -r $FSTMP ]
then
	echo "	/etc/fstab"
	cat $FSTMP >> $FSFILE
	rm $FSTMP
fi

#
# Make new local mount point directories...
#
if [ -n "$newdirs" ]
then
	echo ""
	echo "Creating local mount points:"
fi
for dirname in $newdirs
do
	object=""
	for subdirs in `echo $dirname | awk '
	BEGIN { FS = "/" }
	{
		if (substr($0,1,1) != "/")
			print $1
		for (i = 2; i <= NF; ++i) print "/"$i
	}'` 
	do
		object=$object$subdirs
		if [ -f $object ]
		then
			echo "	Can't create ${object}. File exists!" 
			break
		fi
		if [ \! -d $object ]
		then
			mkdir $object 2> /dev/null
			if [ $? -ne 0 ]
			then
				echo "	Can't create ${object}. Mkdir failed!"
				break
			fi
		fi
	done
	echo "	"$dirname
done
echo ""
echo "***** NFSSETUP COMPLETE *****"
