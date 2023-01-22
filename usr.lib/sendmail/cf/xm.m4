############################################################
############################################################
#####
#####		Mail-11 Mailer
#####
#		@(#)xm.m4	1.2	(ULTRIX)	10/16/86
#####
############################################################
############################################################

MDmail,	P=/usr/bin/mail11, F=mnsF, S=17, R=18, A=mail11 $f $x $h $u

S17
R$+			$:$>18$1			preprocess
R$U::$+			$@$U::$1			ready to go
R$+			$@$U::$1			add our name

S18
R$+<@$-.UUCP>		$:$2!$1				back to old style
R$+<@$-.DNET>		$:$2::$1			convert to dnet style
R$+<@$-.LOCAL>		$:$2::$1			convert to dnet style
R$+<@$=S>		$:$2::$1			convert to dnet style
R$+<@$=S.$D>		$:$2::$1			convert to dnet style
R$=S::$+		$2				strip local names
R$$+::$+		$@$1::$2			already qualified
R$+			$@$U::$1			qualify others


