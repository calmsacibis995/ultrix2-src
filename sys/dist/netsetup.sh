:
#
# @(#)netsetup.sh	1.9	1/22/85
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
# Purpose:	Handles network setup.
# Usage:	netsetup
# Environment:	Bourne shell script
# Date:		3/8/84
# Author:	afd
# 
# Remarks:
#    Sets up files:
#	/etc/hosts
#	/etc/networks
#	/etc/hosts.equiv
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 01	22-Apr-85, Greg Tarsa
#	Added code to check for hostname and attempt to define it
#	from /etc/rc.local if it is not defined.
#

# Trap ^c signal, etc.
trap "" 1 2 3

# Require it to be run by root

if test ! -w /etc/networks
then
    echo "Please su to root first."
    exit 1
fi

#	if /etc/RSHD is there then mv it to /etc/rshd so deamons will start.
if test -f /etc/RSHD
then
	mv /etc/RSHD /etc/rshd
fi

# define: maximum network number, max machine number.
# set defaults for network name, network alias.

MAXNETNUM=127
MAXMACHNUM=1000
DEFNETNAME=ethernet
DEFNETALIAS=deunanetworks

# Retrieve hostname

hostname=`/bin/hostname`
case "${hostname}" in
    "")
	# Scan the first 10 lines of /etc/rc.local for a hostname command
	sed '10q' /etc/rc.local | sed -n '/^\/bin\/hostname/{
		p
		q
	    }' | sh
	;;
esac
case "${hostname}" in
    "")
	hostname=myvax
	echo "
 Your system does not have a name.
 You must use the hostname command to name
 your system.

"
	echo -n "Enter the name for your system: "
	read hostname
	echo
	;;
esac

echo "
Whenever a default selection is given for a question
[shown in square brackets]  you  only  need to press
the <RETURN> key to select the default choice.
"

# If this script was called by the installation procedure or if the network
#    was not set up at installation time (/etc/networks is empty),
#    then do one-time setups.
# else, just get new systems for the network.

onetime=
if test $1 && test $1 = install
then
    onetime=y
else
    if test \! -s /etc/networks
    then
	onetime=y
    fi
fi

if test $onetime
then

#   Get an abbreviation for the hostname

    again=y
    while test $again
	do
	echo ""
	if test -f /.hostabbrev
	then
		exec < /.hostabbrev
		read hostabbrev
		exec < /dev/tty
	else
		echo -n "Enter an abbreviation for $hostname (for example the 1st letter): "
		read hostabbrev
	fi
	if test $hostabbrev
	then again=""
	else again=y
	fi
	done

#   Find out if the system is going on a network

    again=y
    while test $again
	do
	again=""
	echo ""
	echo -n "Are you configuring your system on a network [no] ? "
	read netyn
	case $netyn in
	[yY]*)
	    netyn=y
	    ;;
	[nN]*|"")
	    netyn=n
	    ;;
	*)
	    again=y
	    ;;
	esac
	done

    if test $netyn = n
    then
#	Not on network - setup /etc/hosts accordingly & /etc/networks is empty

	ed /etc/hosts << EOF > /dev/null
	g/^.*localhost.*\$/s//${MAXNETNUM}.0.0.1 $hostname localhost $hostabbrev/
	w
EOF
	cp /dev/null /etc/networks
#	mv /etc/rshd so rc.local will not start deamons
	if test -f /etc/rshd
	then
		mv /etc/rshd /etc/RSHD
	fi

#	EXIT HERE SINCE THE SYSTEM IS NOT GOING ON A NETWORK.
	exit 0

    else
#	On network - set up /etc/hosts, /etc/networks, /etc/hosts.equiv

	ed /etc/hosts << EOF > /dev/null
	g/^.*localhost.*\$/s//${MAXNETNUM}.0.0.1 localhost/
	w
EOF

#	Get the network number for the network.

	again=y
	while test $again
	    do
	    again=""
	    echo ""
	    echo -n "Enter the network number for your network: "
	    read netnum

	    if test $netnum
	    then
		if test $netnum -ge $MAXNETNUM
		then
		    again=y
		    echo ""
		    echo "The network number must be less than $MAXNETNUM."
		else
		    if test $netnum -lt 1
		    then
			again=y
			echo ""
			echo "The network number must be greater than 0."
		    fi
		fi
	    else
		echo ""
		again=y
	    fi
	    done
# turn on the ifconfig line in rc.local
	ed /etc/rc.local << EOF > /dev/null
	g/^#.*NETNUM/s/#//
	g/NETNUM/s/NETNUM/${netnum}/
	w
	q
EOF


#	Get the system number for the host.

	again=y
	while test $again
	    do
	    again=""
	    echo ""
	    echo -n "Enter the system number on the network for $hostname: "
	    read machnum
	    if test $machnum
	    then 
		if test $machnum -ge $MAXMACHNUM
		then
		    echo "Enter a smaller number."
		    again=y
		else
		    if test $machnum -lt 1
		    then
			echo "Enter a larger number."
			again=y
		    fi
		fi
	    else
		again=y
	    fi
	    done
	echo "$netnum.$machnum	$hostname $hostabbrev" >> /etc/hosts

#	Set up /etc/networks here.  Supply a default network name and alias.
#	If user enters a zero length string (<RETURN> only) use default.

	echo ""
	echo -n "Enter the network name [$DEFNETNAME]: "
	read netname

	if test $netname
	then netname=$netname
	else netname=$DEFNETNAME
	fi

	echo ""
	echo -n "Enter the network alias [$DEFNETALIAS]: "
	read netalias

	if test $netalias
	then netalias=$netalias
	else netalias=$DEFNETALIAS
	fi

	echo ""
	echo "Setting up '/etc/networks' file."
	echo ""
	echo "$netname	$netnum	$netalias" >> /etc/networks
	echo "loop	$MAXNETNUM	local" >> /etc/networks
    fi; # if test $netyn = n
else
#   Not one-time. Get network number from /etc/networks if possibly.

    netnum=`grep "$DEFNETNAME" /etc/networks | awk '{print $2}'`
    if test $netnum && test $netnum -gt 0 -a $netnum -lt $MAXNETNUM
    then
	ok=
    else
	again=y
	while test $again
	    do
	    again=""
	    echo ""
	    echo    "Unable  to determine  the network number."
	    echo -n "Enter the network number of your network: "
	    read netnum

	    if test $netnum
	    then
		if test $netnum -ge $MAXNETNUM
		then
		    again=y
		    echo ""
		    echo "The network number must be less than $MAXNETNUM."
		else
		    if test $netnum -lt 1
		    then
			again=y
			echo ""
			echo "The network number must be greater than 0."
		    fi
		fi
	    else
		echo ""
		again=y
	    fi
	    done
    fi
fi; # if test $onetime

# If user is just adding system names to the network, or at install time
#    user selected to put the system on a network, (didn't exit above)
# then add system names to network.
# common to: first time setup and adding system names later.

echo "
Enter  the system  name,  abbreviation, and system  number,  for each
computer on the network.  Enter these on separate lines when prompted.
This information is put into the '/etc/hosts' file.
"

echo "
Enter the name of the 1st system to put on your network"
echo -n "(<RETURN> only if no more to enter): "
read machname

while test $machname
    do
    again=y
    while test $again
	do
	echo -n "Enter the abbreviation for $machname: "
	read machabbrev

	if test $machabbrev
	then again=""
	else
	    echo ""
	    again=y
	fi
	done
    again=y
    while test $again
	do
	again=""
	echo -n "Enter the system number on the network for $machname: "
	read machnum

	if test $machnum
	then
	    if test $machnum -ge $MAXMACHNUM
	    then
		echo "Enter a smaller number."
		again=y
	    else
		if test $machnum -lt 1
		then
		    echo "Enter a larger number."
		    again=y
		fi
	    fi
	else
	    echo ""
	    again=y
	fi
	done
    echo "$netnum.$machnum	$machname $machabbrev" >> /etc/hosts

    echo ""
    echo "Enter the name of the next system to put on your network"
    echo -n "(<RETURN> only if no more to enter): "
    read machname

    done; # while test $machname

#   Set up /etc/hosts.equiv with trusted hosts here.

echo "
Enter  the  names of  'trusted' systems.  Trusted  systems  are
computers considered to be secure. Users on any trusted systems
can login to your system  without password verification on your
system.  Such users must have a valid account on the  'trusted'
system. This information is put in the '/etc/hosts.equiv' file."

echo "
Enter the full name of the first 'trusted' system"
echo -n "(<RETURN> only if none): "
read trustname

while test $trustname
    do
    grep "$trustname" /etc/hosts > /dev/null
    if test $? -ne 0
    then
	echo ""
	echo "System '$trustname' was not entered."
    else
	echo "$trustname" >> /etc/hosts.equiv
    fi
    echo ""
    echo "Enter the full name of the next 'trusted' system"
    echo -n "(<RETURN> only if no more): "
    read trustname
    done

# Invoke MAKEHOSTS to create links from the system abbreviations to rsh

/usr/hosts/MAKEHOSTS

echo "
Network setup is finished."
