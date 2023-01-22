#!/bin/sh
# sccsid = @(#)calendar.sh	1.2	(ULTRIX)	1/15/87
#
#-----------------------------------------------------------
#	Modification History
#
# 001 - Jan 13, 1987 - Tung-Ning Cherng
#       Fix bug - When the calendar text file has the default symbols such as
#		unix, ultrix .., the result will be interpreted in value 1.
#
# 000 - Base on BSD4.2 82/11/07.
#
PATH=/bin:/usr/bin:
tmp=/tmp/cal$$
trap "rm -f $tmp /tmp/cal2$$"
trap exit 1 2 13 15
/usr/lib/calendar >$tmp
case $# in
0)
	trap "rm -f $tmp ; exit" 0 1 2 13 15
	(/lib/cpp -Uunix -Ubsd4_2 -Uultrix -Uvax calendar | egrep -f $tmp);;
*)
	trap "rm -f $tmp /tmp/cal2$$; exit" 0 1 2 13 15
	sed '
		s/\([^:]*\):.*:\(.*\):[^:]*$/y=\2 z=\1/
	' /etc/passwd \
	| while read x
	do
		eval $x
		if test -r $y/calendar
		then
			(/lib/cpp -Uunix -Ubsd4_2 -Uultrix -Uvax $y/calendar | \
				egrep -f $tmp) 2>/dev/null  > /tmp/cal2$$
			if test -s /tmp/cal2$$
			then
				< /tmp/cal2$$ mail $z
			fi
		fi
	done
esac
