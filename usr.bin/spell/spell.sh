#! /bin/sh
#@(#)spell.sh	1.4	(ULTRIX)	6/28/85

#************************************************************************
#									*
#			Copyright (c) 1984 by				*
#		Digital Equipment Corporation, Maynard, MA		*
#			All rights reserved.				*
#									*
#   This software is furnished under a license and may be used and	*
#   copied  only  in accordance with the terms of such license and	*
#   with the  inclusion  of  the  above  copyright  notice.   This	*
#   software  or  any  other copies thereof may not be provided or	*
#   otherwise made available to any other person.  No title to and	*
#   ownership of the software is hereby transferred.			*
#									*
#   This software is  derived  from  software  received  from  the	*
#   University    of   California,   Berkeley,   and   from   Bell	*
#   Laboratories.  Use, duplication, or disclosure is  subject  to	*
#   restrictions  under  license  agreements  with  University  of	*
#   California and with AT&T.						*
#									*
#   The information in this software is subject to change  without	*
#   notice  and should not be construed as a commitment by Digital	*
#   Equipment Corporation.						*
#									*
#   Digital assumes no responsibility for the use  or  reliability	*
#   of its software on equipment which is not supplied by Digital.	*
#									*
#***********************************************************************/
#
#	Modified By :	Aki Hirai	Digital Equipment Corp.
#			31 -May -1985
#			Add interrupt signal handling
#************************************************************************/
#   
#	@(#)spell.sh	1.3	(Berkeley)	83/09/10
#
: V data for -v, B flags, D dictionary, S stop, H history, F files, T temp
V=/dev/null		B=			F= 
S=/usr/dict/hstop	H=/dev/null		T=/tmp/spell.$$
next="F=$F@"
trap "rm -f $T ${T}a ${T}x ; exit" 0
trap "rm -f $T ${T}a ${T}x ; exit" 2
for A in $*
do
	case $A in
	-v)	B="$B@-v"
		V=${T}a ;;
	-x)	B="$B@-x" ;;
	-b) 	D=${D-/usr/dict/hlistb}
		B="$B@-b" ;;
	-d)	next="D=" ;;
	-s)	next="S=" ;;
	-h)	next="H=" ;;
	-*)	echo "Bad flag for spell: $A"
		echo "Usage:  spell [ -v ] [ -b ] [ -d hlist ] [ -s hstop ] [ -h spellhist ]"
		exit ;;
	*)	eval $next"$A"
		next="F=$F@" ;;
	esac
done
IFS=@
case $H in
/dev/null)	deroff -w $F | sort -u | /usr/lib/spell $S $T |
		/usr/lib/spell ${D-/usr/dict/hlista} $V $B > ${T}x ; cat ${T}x |
		sort -u +0f +0 - $T ;;
*)		deroff -w $F | sort -u | /usr/lib/spell $S $T |
		/usr/lib/spell ${D-/usr/dict/hlista} $V $B > ${T}a ; cat ${T}x |
		sort -u +0f +0 - $T | tee -a $H
		who am i >> $H 2> /dev/null ;;
esac
case $V in
/dev/null)	exit ;;
esac
sed '/^\./d' $V | sort -u +1f +0
