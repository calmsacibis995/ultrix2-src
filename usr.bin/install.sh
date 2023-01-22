#! /bin/sh
#
#  @(#)install.sh	1.3	ULTRIX	10/3/86
#
cmd=/bin/mv
strip=""
chmod="/bin/chmod 755"
chown="/etc/chown -f root"
chgrp="/bin/chgrp -f system"
while :
do
	case $1 in
	-s )	strip="/bin/strip"
		;;
	-c )	cmd="/bin/cp"
		;;
	-m )	chmod="/bin/chmod $2"
		shift
		;;
	-o )	chown="/etc/chown -f $2"
		shift
		;;
	-g )	chgrp="/bin/chgrp -f $2"
		shift
		;;
	* )	break
		;;
	esac
	shift
done

case "$2" in
"")	echo "install: no destination specified"
	exit 1
	;;
.|"$1")	echo "install: can't move $1 onto itself"
	exit 1
	;;
esac
case "$3" in
"")	;;
*)	echo "install: too many files specified -> $*"
	exit 1
	;;
esac
if [ -d $2 ]
then	file=$2/$1
else	file=$2
fi
/bin/rm -f $file
$cmd $1 $file
[ $strip ] && $strip $file
$chown $file
$chgrp $file
$chmod $file
