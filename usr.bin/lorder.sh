trap "rm -f $$sym?ef; exit" 0 1 2 13 15

case $# in
0)	echo usage: lorder file ...
	exit ;;
1)	case $1 in
	*.o)	set $1 $1
	esac
esac
nm -pg $* | sed '
	/^$/d
	/:$/{
		/\.o:/!d
		s/://
		h
		s/.*/& &/
		p
		d
	}
	/[TD] /{
		s/.* //
		G
		s/\n/ /
		w '$$symdef'
		d
	}
	/C /{
		s/.* //
		G
		s/\n/ /
		w '$$symcef'
		d
	}
	s/.* //
	G
	s/\n/ /
	w '$$symref'
	d
'
sort $$symdef -o $$symdef
sort $$symcef -o $$symcef
sort $$symref -o $$symref
{
	join $$symref $$symdef
	join $$symref $$symcef
	join $$symcef $$symdef
} | sed 's/[^ ]* *//'

#  @(#)lorder.sh	1.2	(ULTRIX)	2/13/86
