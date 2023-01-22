#! /bin/sh
#  @(#)ypsetup.sh	1.8	ULTRIX	3/3/87
#									
# 			Copyright (c) 1986 by				
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
# Purpose:	Set up Yellow Pages
# Usage:	ypsetup [domainname]
# Environment:	Bourne shell script
# Date:		6/30/86
# Author:	Fred L. Templin
# 
# Remarks:
#    Sets up files:
#	/etc/rc.local
#	/usr/lib/crontab
#	/etc/yp/{domainname}/*
#
#

#
# Set up interrupt handlers:
#
QUIT='
	if [ -r $YPTMP ]
	then
		rm $YPTMP
	fi
	echo "Ypsetup terminated with no installations made."
	exit 1
'
QMSG='
	echo "Please clean up ${YPDIR}/${ypdomain} !!"
	eval "$QUIT"
'

#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

YPTMP=/tmp/ypsetup.tmp$$
NETWORKS=/etc/networks
HOSTS=/etc/hosts
AWK=/bin/awk
CAT=/bin/cat
MAKE="make -f /etc/yp/Makefile"
MAKEDBM=/etc/yp/makedbm
YPXFR=/etc/yp/ypxfr
YPLOG=ypxfr.log
NULL=/dev/null
if [ $DEBUG ]
then
	RCFILE=./test/rc.local
	CRONFILE=./test/crontab
	YPSETUP=./ypsetup
	HOSTNAME=./test/hostname
	DOMAINNAME=./test/hostname
	YPDIR=./test
else
	RCFILE=/etc/rc.local
	CRONFILE=/usr/lib/crontab
	YPSETUP=/etc/ypsetup
	HOSTNAME=/bin/hostname
	DOMAINNAME=/bin/domainname
	YPDIR=/etc/yp
	PATH=$PATH:$YPDIR
	export PATH
	DEFMAPS="group.bygid group.byname hosts.byaddr hosts.byname \
mail.aliases netgroup netgroup.byuser netgroup.byhost networks.byaddr \
networks.byname passwd.byname passwd.byuid protocols.byname protocols.bynumber \
services.byname ypservers"

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
# Be sure network has already been set up, and this baby has a name!!
#

no_host=""
hname=`$HOSTNAME`
if [ $? -ne 0 ] || [ \! -d $YPDIR ]
then
	echo "
Please bring the system to multi-user mode before running ypsetup."
	eval "$QUIT"
fi

#
# See if Yellow Pages are already installed
#
egrep -s '^#.*%YPSTART%' $RCFILE
if [ $? -eq 0 ]
then
	echo "
The Yellow Pages environment for this host has already been installed.
Please make any modifications such as adding new maps or changing the
state of this host by hand."
	eval "$QUIT"
fi

#
# PHASE ONE: Gather data!!
#
echo -n "
The ypsetup command configures the Yellow Pages (YP) environment
for your system.  Yellow Pages provides a distributed data lookup
service for sharing information between systems on the network.
Information is kept in database files known as YP maps. These maps
are organized in YP domains which are collections of maps located
on certain systems on the network known as servers. For each domain,
there are three flavors of systems on the network:

	- a master YP server is a system which is responsible for
	  maintaining the master copy of the domain's database.
	  There should be ONLY ONE master server for a domain.

	- a slave YP server is a system which periodically receives
	  updated versions of the master server's maps. The slave
	  server can look up and return information in its private
	  collection of maps, and can take over for the master
	  server in the event of a failure.

	- a YP client is a system which has no local copies of the
	  domain's database, but can look up database information
	  by requesting service from a master or slave server.

[ Press the RETURN key to continue ] : "
read junk
echo "
ypsetup will take you through a configuration process to set the
default YP domain name of your system, determine the flavor of
your Yellow Pages environment and construct the default YP map
files for this YP domain.  Default answers are shown in square
brackets ([]).  To use a default answer, press the RETURN key."

#
# did he specify a new domainname?
#
if [ $1 ]
then
	ypdomain=$1
	echo "
Using \"$ypdomain\" as the default YP domain name for this system."
else
	#
	# Ask him for a name...
	#
	again=y
	echo "
In order to use the Yellow Pages service, your host must have a
default YP domain name."
	while [ $again ]
	do
		echo -n "
Please enter a name to be used as the default YP domain name : "
		read ypdomain
		case $ypdomain in
		"")
			;;
		*)
			again=""
			;;
		esac
	done
fi

if [ -d $YPDIR/$ypdomain ]
then
	echo "
Your system has already been initialized for the YP domain
\"${ypdomain}\". Please restart ypsetup after cleaning out and
removing the ${YPDIR}/${ypdomain} directory."
	eval "$QUIT"
fi

#
# Set domain name
#
$DOMAINNAME $ypdomain

prompt=y
echo "
Will you be configuring your system as a master YP server,
slave YP server, or YP client for the domain \"${ypdomain}\" ?"
while [ $prompt ]
do
	prompt=""
	echo -n "
Enter \"m\" for MASTER, \"s\" for SLAVE, or \"c\" for CLIENT [c]: "
	read flavor
	case $flavor in
	[mM]*)
		echo "
Before configuring your system as a master YP server, you
must first be sure that NO OTHER SYSTEM ON THE NETWORK IS
CONFIGURED AS A MASTER SERVER FOR THIS DOMAIN!! If a master
YP server is already configured, or you are unsure, please
exit ypsetup now."
		chk=y
		while [ $chk ]
		do
			chk=""
			echo -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [c]: "
			read query
			case $query in
			[eE]*)
				eval "$QUIT"
				;;
			[cC]*|"")
				;;
			*)
				chk=y
				;;
			esac
		done
		echo "
As the master YP server for the domain ${ypdomain}, you may choose
to run the yppasswdd(8yp) server daemon to allow remote password
updates to the master copy of the passwd file.  The master copy of
the passwd file will initially be served from \"/etc/passwd\".  If
you intend to add YP escape characters to the master YP server's
\"/etc/passwd\" file, you must copy the original version to a holding
area such as \"/etc/yp/src\" and modify the entry for yppasswdd in the
\"/etc/rc.local\" file accordingly."
		run_yppasswdd=n
		chk=y
		while [ $chk ]
		do
			chk=""
			echo -n "
Would you like to run the yppasswdd daemon [n]? "
			read query
			case $query in
			[yY]*)
				run_yppasswdd=y
				;;
			[nN]*|"")
				;;
			*)
				chk=y
				;;
			esac
		done
		echo "
You will now be asked to list the names of other hosts which will
be configured as servers for maps in this YP domain.  (This host
is included in the list by default). This list will be used to name
the recipients of any updates to your hosts YP maps. Enter only the
names of known hosts which have been initialized in the \"/etc/hosts\"
file and press the RETURN key to terminate the list:
"
		not_done=y
		while [ $not_done ]
		do
			query=y
			#
			# Enter this host first...
			#
			echo $hname > $YPTMP
			while [ $query ]
			do
				echo -n "	Name of host: "
				read servname
				if [ $servname ]
				then
					good=`$CAT $HOSTS | $AWK "
						BEGIN { found = 0 }
						/[ \t]${servname}[ \t]/ { found = 1; print \"y\" }
						END { if ( found == 0 ) print \"n\" }"`
			#
			# had to put this in to recognize hostnames followed by EOL
			#
					good1=`$CAT $HOSTS | $AWK "
						BEGIN { found = 0 }
						/[ \t]${servname}$/ { found = 1; print \"y\" }
						END { if ( found == 0 ) print \"n\" }"`
					if [ $good = y ] || [ $good1 = y ]
					then
						if [ $servname = $hname ]
						then
							echo "	Can't name this host!"
							echo ""
						else 
							echo $servname >> $YPTMP
						fi
					else
						echo "	\"$servname\" NOT a known host!"
						echo ""
					fi
				else
					query=""
				fi
			done
	
			#
			# Ask for verification...
			#
			echo "The list of Yellow Pages servers is:"
			awk '
			{
				printf "\t%s\n", $0 
			}' $YPTMP
				echo "
You may now redo this list, exit the setup procedure, or continue.
If you choose to continue, the default set of YP maps for your host
will now be initialized.  THIS PROCEDURE TAKES TIME!!"
			query=y
			while [ $query ]
			do
				echo -n "
Enter \"r\" to REDO the servers list, \"e\" to EXIT ypsetup
procedure,  or \"c\" to CONTINUE [c]: "
				read resp
				case $resp in
				[cC]*|"")
					not_done=""
					query=""
					;;
				[rR]*)
					query=""
					;;
				[eE]*)
					eval "$QUIT"
					;;
				*)
					;;
				esac
			done
		done
		
		trap 'eval "$QMSG"' 1 2 3 15

		#
		# Create Yellow Pages Domain
		#
		mkdir $YPDIR/$ypdomain
		if [ $? -ne 0 ]
		then
			echo "
Couldn't create the Yellow Pages directory: ${YPDIR}/${ypdomain}."
			eval "$QUIT"
		fi

		#
		# Make default maps...
		#
		echo "
Making default YP maps. Please wait..."
		$MAKEDBM $YPTMP $YPDIR/$ypdomain/ypservers
		if [ $? -ne 0 ]
		then
			echo "
Couldn't make the \"ypservers\" map"
			eval "$QMSG"
		fi
		(cd $YPDIR/$ypdomain; $MAKE NOPUSH=1)
		if [ $? -ne 0 ]
		then
			echo "
Couldn't make the default YP maps"
			eval "$QMSG"
		fi
		echo "Installation of default maps complete..."
		;;
	[sS]*)
		echo "
Before configuring your system as a slave YP server, you must
first know the name of the master YP server for domain \"${ypdomain}\"
and be sure that it is up.  If no master YP server is configured,
you will not be able to retrieve a set of YP maps for your system."
		answer=y
		while [ $answer ]
		do
			echo -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [c] ? "
			read ans
			case $ans in
			[cC]*|"")
				answer=""
				;;
			[eE]*)
				eval "$QUIT"
				;;
			*)
				;;
			esac
		done

		#
		# get master server's name.
		#
		answer=y
		while [ $answer ]
		do
			echo -n "
Please enter the name of the master YP server for domain \"${ypdomain}\": "
			read master
			if [ $master ]
			then
				good=`$CAT $HOSTS | $AWK "
					BEGIN { found = 0 }
					/[ \t]${master}[ \t]/ { found = 1; print \"y\" }
					END { if ( found == 0 ) print \"n\" }"`
				good1=`$CAT $HOSTS | $AWK "
					BEGIN { found = 0 }
					/[ \t]${master}$/ { found = 1; print \"y\" }
					END { if ( found == 0 ) print \"n\" }"`
				if [ $good = y ] || [ $good1 = y ]
				then
					if [ $master = $hname ]
					then
						echo "	Can't name this host!"
						echo ""
					else
						answer=""
					fi
				else
					echo "	\"${master}\" NOT a known host!"
					echo ""
				fi
			fi
		done
	
		trap 'eval "$QMSG"' 1 2 3 15

		#
		# Create Yellow Pages Domain
		#
		mkdir $YPDIR/$ypdomain
		if [ $? -ne 0 ]
		then
			echo "
Couldn't create the Yellow Pages directory: ${YPDIR}/${ypdomain}."
			eval "$QUIT"
		fi

		#
		# Copy master's maps to slave
		#
		echo "
Copying YP maps for this domain from host \"${master}\".
THIS PROCEDURE TAKES TIME!!"
		noerrs=y
		for mname in $DEFMAPS
		do
			$YPXFR -h $master -c -d $ypdomain $mname 2>> $YPDIR/$YPLOG
			if [ $? -ne 0 ]
			then
				echo "	Couldn't transfer map \"${mname}\""
				noerrs=n
			fi
		done
		if [ $noerrs = n ]
		then
			echo "
Some YP maps were not initialized. See the file \"/etc/yp/ypxfr.log\"
for reasons."
		fi
		;;
	[cC]*|"")
		#
		echo "
Before configuring your system as a YP client, you should
first be sure that there IS at least one system on the
network configured as either a master or slave YP server
for this domain!! If no server is configured, you will not
be able to access the YP maps!"
		answer=y
		while [ $answer ]
		do
			echo -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [c] ? "
			read ans
			case $ans in
			[cC]*|"")
				answer=""
				;;
			[eE]*)
				eval "$QUIT"
				;;
			*)
				;;
			esac
		done
		;;
	*)
		#
		# Bad response...
		#
		prompt=y
		;;
	esac
done


#
# PHASE TWO... Update files!!
#

trap "" 1 2 3 15

startup=""
echo "
Installing Yellow Page daemons..."
#
# See if the port mapper is already there...
#
egrep -s '[ 	]*/etc/portmap' $RCFILE
if [ $? -ne 0 ]
then
	#
	# Not there... put it in!!
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
	startup=`echo $startup "/etc/portmap ; " | $CAT`
fi

#
# Banner, domainname, and ypbind...
#
echo "#
# %YPSTART% - Yellow Pages daemons added by \"ypsetup\".
#
/bin/domainname	${ypdomain}
#" >> $RCFILE
case $flavor in
[mM]*)
	echo "echo -n 'YP daemons:'						>/dev/console
if [ -f /etc/portmap -a -f /usr/etc/ypserv ]; then
	/usr/etc/ypserv ; echo -n ' ypserv'			>/dev/console
fi
if [ -f /etc/portmap -a -f /etc/ypbind ]; then
	/etc/ypbind ; echo -n ' ypbind'				>/dev/console
fi" >> $RCFILE
	#
	startup=`echo $startup "/usr/etc/ypserv ; /etc/ypbind ; " | $CAT`
	#
	if [ $run_yppasswdd = y ]
	then
		echo "
if [ -f /etc/portmap -a -f /usr/etc/rpc.yppasswdd ]; then
	/usr/etc/rpc.yppasswdd /etc/passwd -m passwd \\
	DIR=/etc ; echo -n ' yppasswdd' >/dev/console
fi" >> $RCFILE
		#
		startup=`echo $startup "/usr/etc/rpc.yppasswdd /etc/passwd -m passwd DIR=/etc ; " | $CAT`
		#
	fi
	;;
[sS]*)
	echo "echo -n 'YP daemons:'						>/dev/console
if [ -f /etc/portmap -a -f /usr/etc/ypserv ]; then
	/usr/etc/ypserv ; echo -n ' ypserv'			>/dev/console
fi
if [ -f /etc/portmap -a -f /etc/ypbind ]; then
	/etc/ypbind ; echo -n ' ypbind'				>/dev/console
fi" >> $RCFILE
	#
	startup=`echo $startup "/usr/etc/ypserv ; /etc/ypbind ; " | $CAT`
	#
	#
	# Set updating entries into crontab...
	#
	echo "
# Local Yellow Pages environment
30 * * * * sh /etc/yp/ypxfr_1perhour.sh
31 1,13 * * * sh /etc/yp/ypxfr_2perday.sh
32 1 * * * sh /etc/yp/ypxfr_1perday.sh" >> $CRONFILE
	;;
[cC]*|"")
	echo "echo -n 'YP daemons:'						>/dev/console
if [ -f /etc/portmap -a -f /etc/ypbind ]; then
	/etc/ypbind ; echo -n ' ypbind'				>/dev/console
fi" >> $RCFILE
	#
	startup=`echo $startup "/etc/ypbind ; " | $CAT`
	#
	;;
esac

echo "echo '.'							>/dev/console
#
# %YPEND% - Yellow Pages daemons added by \"ypsetup\".
#" >> $RCFILE

echo "
The necessary Yellow Page daemon entries have been placed in the file
\"/etc/rc.local\".  In order to begin using Yellow Pages, you must now
start the daemons and add YP escape characters to the \"/etc\" files
corresponding to the maps for this domain.  You may either allow ypsetup
to start these daemons automatically or invoke them by hand, but in either
case they will be started automatically on subsequent reboots.
"
answer=y
while [ $answer ]
do
	echo -n "
Would you like ypsetup to start the daemons automatically [y]? "
	answer=""
	read ans
	case $ans in
	[yY]*|"")
		if [ $DEBUG ]
		then
			echo $startup
		else
			eval "$startup"
		fi
		;;
	[nN]*)
		;;
	*)
		answer=y
		;;
	esac
done

#
# Clean up
#
if [ -r $YPTMP ]
then
	rm $YPTMP
fi
echo "***** YPSETUP COMPLETE *****"
