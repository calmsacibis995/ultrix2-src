:
# @(#)uucpsetup.sh	1.10 ULTRIX 4/30/86
#
#									
# 			Copyright (c) 1984 by				
# 		Digital Equipment Corporation, Maynard, MA		
# 			All rights reserved.				
# 									
#	This software is furnished under a license and may be used and	
#	copied  only  in accordance with the terms of such license and	
#	with the  inclusion  of  the  above  copyright  notice.   This	
#	software  or  any  other copies thereof may not be provided or	
#	otherwise made available to any other person.  No title to and	
#	ownership of the software is hereby transferred.			
# 									
#	The information in this software is subject to change  without	
#	notice  and should not be construed as a commitment by Digital	
#	Equipment Corporation.						
# 									
#	Digital assumes no responsibility for the use  or  reliability	
#	of its software on equipment which is not supplied by Digital.	
#
# Purpose:	Handles uucp setup.
# Usage:	uucpsetup
# Environment:	Bourne shell script
# Date:		3/28/84
# Author:	afd
#
# Mods:		30-APR-1986	ccb
#		performance!
# 
# Remarks:
#	Sets up files:
#	L.sys
#	L.cmds
#	USERFILE
#	L-devices
#	L-stat
#	R-stat
#
#	It assumes a simplified view of the world:  only DF02, DF03 and DF112's
#	are supported, calling times are broken down into Any time, Evenings,
#	and Nights, a rather "standard" form is used for the L.sys entries.
PATH="/bin:/etc:/usr/bin:/usr/ucb";export PATH

# Trap ^c signal
trap "" 1 2 3

HOSTNAME=`hostname`

# Set up Shell file parameters.
#	dir path for uucp.
#	2 default directories needed for uucp.
#	default execution access level.
#	default tty num to use for outgoing connections.

LIB=/usr/lib/uucp
#LIB=.
UUHOME=/usr/spool/uucppublic
UUCP=/usr/spool/uucp
DIR1=$UUCP/sys/DEFAULT/D.$HOSTNAME
DIR2=${DIR1}X
DEF_XLEVEL=1
NULL=/dev/null

readonly PATH LIB UUHOME DIR1 DIR2 DEF_XLEVEL UUCP

# Require it to be run by root
[ -w ${LIB}/L.cmds ] ||
{
	echo "Please su to root first."
	exit 1
}


# If this script was called by the installation procedure or if uucp
#  was not set up at installation time (default directories don't exist),
#  then do one-time setups.
#  else, just get new systems for uucp connections.

ONETIME=
ASK=
case "$1" in
install)
	ONETIME=y
	ASK=y
	;;
*)	echo "
	Whenever a default selection is given for a question
	[shown in square brackets]  you  only  need to press
	the <RETURN> key to select the default choice.
"
	[ -d $DIR1 -a -d $DIR2 ] || ONETIME=y
	;;
esac


# See if the system is to be set up for uucp.
case "$ASK" in
y)	while : # thats forever.
	do
		echo -n "
Do you want your system set up for uucp connections (y/n)? "

		read JUNK
		case "$JUNK" in
		[Yy]*)	break
			;;
		[Nn]*)	exit 0
			;;
		esac
	done
	;;
esac

# determine the tty controler type.
#!HACK PT for dmb32.
{ date </dev/tty03 2>&1 > $NULL && TTYDEV=dz; } ||
{ date </dev/ttyh3 2>&1 > $NULL && TTYDEV=dh; } ||
{ date </dev/ttyA3 2>&1 > $NULL && TTYDEV=dhu; } ||
{ date </dev/ttyIa 2>&1 > $NULL && TTYDEV=dmz; }

#   If called from installation script determine whether the system has
#	a dz or a dh terminal device, and set ttynum accordingly.

GO=
case "$ONETIME" in
"y")
	case $TTYDEV in
	dmz)	[ ! -f /dev/ttyd1 ] &&
		{	[ -f /dev/ttyIb ] &&
			{	mv /dev/ttyIb /dev/ttyd1
				mv /dev/ttyIc /dev/ttyd2
				GO=1
			}
			EDLINS="g/ttyIb/d
g/ttyIc/d"
		}
		;;

	dhu)	[ ! -f /dev/ttyd1 ] &&
		{	[ -f /dev/ttyA1 ] &&
			{	mv /dev/ttyA1 /dev/ttyd1
				mv /dev/ttyA2 /dev/ttyd2
				GO=1
			}
			EDLINS="g/ttyA1/d
g/ttyA2/d"
		}
		;;

	dh)	[ ! -f /dev/ttyd1 ] &&
		{	[ -f /dev/ttyh1 ] &&
			{	mv /dev/ttyh1 /dev/ttyd1
				mv /dev/ttyh2 /dev/ttyd2
				GO=1
			}
			EDLINS="g/ttyh1/d
g/ttyh2/d"
		}
		;;

	dz)	[ ! -f /dev/ttyd1 ] &&
		{	[ -f /dev/tty01 ] &&
			{	mv /dev/tty01 /dev/ttyd1
				mv /dev/tty02 /dev/ttyd2
				GO=1
			}
			EDLINS="g/tty01/d
g/tty02/d"
		}
		;;
	#!HACK for new controllers
	esac

	# insure modes on dial-lines
	case "$GO" in
	1)	chmod 666 /dev/ttyd[12]
		;;
	esac
	# edit ttys file
	case "$EDLINS" in
	"")	;;
	*)	echo '$EDLINS
/console/
a
ttyd1	"/etc/getty T1200"	dialup	on modem shared		# dial in/out
ttyd2	"/etc/getty T1200"	dialup	on modem shared		# dial in/out
.
w
q' |ed - /etc/ttys
		;;
	esac


	echo "
UUCP tty ports 1 and 2 on ${TTYDEV}0 have been renamed ttyd1 
and ttyd2 and are the expected UUCP incomming and outgoing tty ports.
"

	# Create default directories with make file, create uustat directories.
	(cd $LIB;make mkdirs 2> $NULL)
	> $LIB/L_stat
	> $LIB/R_stat
	> $UUCP/LOGFILE
	> $UUCP/SYSLOG
	chown uucp $LIB/L_stat $LIB/R_stat $UUCP/LOGFILE $UUCP/SYSLOG 
	chmod 644 $LIB/L_stat $LIB/R_stat $UUCP/LOGFILE $UUCP/SYSLOG 

	#   Set up cron entries for polling systems hourly and
	#	 cleaning up log file daily.
	echo "30 * * * * su uucp < $LIB/uucp.hour
0 6 * * * su uucp < $LIB/uucp.day
0 12 * * * su uucp < $LIB/uucp.noon
0 3 1 * * su uucp < $LIB/uucp.week
\#0 2 * * * su uucp < $LIB/uucp.night
\#30 2 1 * * su uucp < $LIB/uucp.longhall" >> /usr/lib/crontab

	# change LOGFILE var in uucp.hour from /dev/console to a null file
	ed - ${LIB}/uucp.hour <<!
/LOGFILE/
s/console/null/
w
q
!
	;;
esac



# We will now set up for systems you call out to.
#
echo "
	THIS SUBSECTION OF UUCP QUESTIONS DEALS WITH OUTGOING UUCP CONNECTIONS.
"
STRING=ne
while :
do
	echo -n "Enter the name of a remote system to call out to
(<RETURN> only if no$STRING): "

	STRING=" more"
	read RSYSTEM
	case "$RSYSTEM" in
	"")	break
		;;
	esac

	[ -f $LIB/L.sys ] && grep -s "^$RSYSTEM" $LIB/L.sys &&
	{
		echo "
$RSYSTEM is already in $LIB/L.sys file."
		continue
	}
		
	echo "
Next you must enter the times when your system can call system $RSYSTEM.
The four choices are:
	(a) Any time of any day
	(e) Evenings (Mon-Fri 5pm - 8am, Sat & Sun all day )
	(n) Nights   (Mon-Fri 11pm - 8am,  Sat all day  &  Sun until 5pm)
	(x) Never"

	while :
	do
		echo -n "
When do you want your system to call system $RSYSTEM (a/e/n/x) ? "

		read JUNK
		case "$JUNK" in
		a|A|"")	TIME=Any
			;;
		e|E)	TIME="Sa|Su|Wk1705-2359|Wk0000-0755"
			;;
		n|N)	TIME="Sa|Su0000-1655|Wk2305-2359|Wk0000-0755"
			;;
		x|X)	TIME=Never
			;;
		*)	echo "Bad input"
			continue
			;;
		esac
		break
	done	# while :

	# Now get the device entry & set the default class.
	# Set type and line for L.sys
	DEVICE=ACU
	DEF_CLASS=1200

	# Set info for L-devices.
	TYPE=ACU
	LINE=ttyd2

	# Get the class (line speed).  Defaults set in device above.
	echo -n "
Enter the line speed for system $RSYSTEM [$DEF_CLASS] : "

	read CLASS
	case "$CLASS" in
	"")	CLASS=$DEF_CLASS
		;;
	esac

	# Get the phone #.
	while : # eternally
	do
		echo -n "
Enter the phone number for system $RSYSTEM: "
		read PHONE_NUM
		case "$PHONE_NUM" in
		"")	echo ""
			continue
			;;
		esac
		break
	done

	# Get login name for your connection on remote system
	while : # onandon
	do
		echo -n "
Enter your login name on system $RSYSTEM: "
		read LOGIN
		case "$LOGIN" in
		"")	echo ""
			continue
			;;
		esac
		break
	done

	# Get password for your connection on remote system
	while : # true
	do
		echo -n "
Enter your password for login $LOGIN on system $RSYSTEM: "
		read PASSWORD
		case "$PASSWORD" in
		"")	echo ""
			continue
			;;
		esac
		break
	done

	# Append entry to the L.sys file.
	echo "
The entry in $LIB/L.sys will look like:"
	echo "$RSYSTEM $TIME $DEVICE $CLASS $PHONE_NUM ogin:--ogin: $LOGIN ssword: $PASSWORD" | tee -a $LIB/L.sys

	# Call_unit is the same as 'line'
	#  for digital DF02, DF112 or DF03.
	# Speed is the same as 'class'.
	CALL_UNIT=$LINE
	SPEED=$CLASS

	# If line is hard-wired ($phone_num = $device)
	#  use "direct" in brand field.
	#  otherwise, get the brand.

	case "$PHONE_NUM" in
	"$DEVICE")
		BRAND=direct
		;;
	*)	while : # true
		do
			echo -n "
Enter the modem type [DF03] : "
			read JUNK
			BRAND=`echo $JUNK|dd conv=ucase 2> $NULL`
			case $BRAND in
			"")	BRAND="DF03"
				;;
			DF0[23]|DF112|DF[12]24)
				;;
			*)
				echo "
Only DF02, DF03, DF112, DF124 or DF224 are supported."
				continue
				;;
			esac
			break
		done
		;;
	esac

	# Append entry to the L-devices file, if not already there.

	DEVLIN="$TYPE $LINE $CALL_UNIT $SPEED $BRAND"
	[ -f $LIB/L-devices ] && grep -s "DEVLIN" $LIB/L-devices ||
	{
		echo "
The entry in $LIB/L-devices will look like:"
		echo "$DEVLIN" | tee -a $LIB/L-devices
	}
done	# while :





# Get info for systems allowed to make incoming connections.

echo "
	THIS SUBSECTION OF QUESTIONS DEALS WITH UUCP INCOMING CONNECTIONS.
"

# Set up default entries in USERFILE, if not already there.
WRITE=
[ -f $LIB/USERFILE ] &&
{
	grep -s "remote" $LIB/USERFILE ||
		WRITE="remote,	X0	$UUHOME
"
	grep -s "local" $LIB/USERFILE ||
		WRITE="${WRITE}local,	X9	/
"
	echo -n "$WRITE" >> $LIB/USERFILE
}


STRING=ne
while :
do
	echo -n "
Enter the name of a system allowed to establish incoming uucp connections: 
(<RETURN> only if no$STRING): "
	read INSYSTEM
	case "$INSYSTEM" in
	"")	break
		;;
	esac
	case "$STRING" in
	ne)	STRING=" more"
		echo "
For each system you will be asked for:
a  short  comment  for  the  password file,  a login  password,
the execution  access  level  and  the  default directory path.

Enter  these items  one at time, as you are prompted for  them.

The  execution  access  level can  range from  0 to 9.  Where 0
gives no access  to the  remote  system  and 9  gives the  most
access.  In  order  for a remote  system to be  able to execute
a  particular command  on your system,  the remote  system must
have an access level equal to or greater than the corresponding
protection level in the L.cmds file."

		while :
		do
			echo -n "
Do you wish to see the L.cmds file (y/n) ? "
			read JUNK
			case $JUNK in
			[yY]*)	echo
				more $LIB/L.cmds
				;;
			[nN]*)	;;
			*)	echo ""
				continue
				;;
			esac
			break
		done
		;;
	esac

	[ -f $LIB/USERFILE ] && grep -s "^$INSYSTEM" $LIB/USERFILE ||
	{
		echo "
	'$INSYSTEM' is already in $LIB/USERFILE file."
		continue
	}

	# Comment is not necessary, so it need not be entered.
	echo -n "
Enter a short comment for the passwd file: "

	read JUNK
	echo "U$INSYSTEM::4:2:$JUNK:$UUHOME:$LIB/uucico">> /etc/passwd
	echo "
Enter a password for system '$INSYSTEM' for the password file."

	while :
	do
		passwd U$INSYSTEM && break
	done

	# Next set up the USERFILE
	while :
	do
		echo -n "
Enter the execution access level for system '$INSYSTEM' (0-9) [$DEF_XLEVEL]: "

		read XLEVEL
		case "$XLEVEL" in
		"")	XLEVEL=$DEF_XLEVEL
			;;
		[0-9])	;;
		*)	"
	The execution access level must be between 0 and 9."
			continue
			;;
		esac
		break
	done

	# does s/he want callback?
	while :
	do
		echo -n "
If you choose the call back option you will
always pay the phone  bill for connections.
Do you want the call back option for system '$INSYSTEM' (y/n) : "

		read JUNK
		case "$JUNK" in
		[yY]*)	CALLBACK=c
			;;
		[nN]*)	CALLBACK=
			;;
		*)	echo ""
			continue
			;;
		esac
		break
	done

	echo -n "
Enter the directory path for system '$INSYSTEM' [$UUHOME] : "
	read DIRPATH

	case $DIRPATH in
	"")	DIRPATH="$UUHOME"
		;;
	esac

	echo "U$INSYSTEM,$INSYSTEM X$XLEVEL $CALLBACK	$DIRPATH" >> $LIB/USERFILE

	# If $INSYSTEM is not in the L.sys file already, for outgoing
	#  connections, then add it in.

	[ -f $LIB/L.sys ] && grep -s "^$INSYSTEM" $LIB/L.sys ||
		echo "$INSYSTEM incoming" >> $LIB/L.sys

done	# while : # incomming systems

chown uucp $LIB/USERFILE $LIB/L.sys $LIB/L-devices 2> $NULL
chmod 400 $LIB/USERFILE $LIB/L.sys $LIB/L-devices 2> $NULL

echo "
Finished with UUCP setup."
exit 0
