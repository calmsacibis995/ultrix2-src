############################################################
############################################################
#####
#####		Local and Program Mailer specification
#####
#		@(#)localm.m4	1.3	(ULTRIX)	10/16/86
#####
############################################################
############################################################

Mlocal,	P=/bin/mail, F=rlsDFmn, S=10, R=10, A=mail -d $u
Mprog,	P=/bin/sh,   F=lsDFRe,   S=10, R=10, A=sh -c $u

S10
R$+<@LOCAL>		$@$1				delete LOCAL
R$+<@$-.LOCAL>		$@$1<@$2>			delete .LOCAL
R@			$@MAILER-DAEMON			errors to mailer-daemon
