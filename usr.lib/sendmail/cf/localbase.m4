############################################################
############################################################
#####
#####           local conventions
#
#		@(#)localbase.m4	1.2	(ULTRIX)	12/16/86
#####
#####
#####
############################################################
############################################################

##### special local conversions

# Here you can any special local rewriting rules and keep them all
# in one place. Ruleset 8 is invoked at the beginning of ruleset 3
# and ruleset 6 is invoked at the end. Ruleset 3 always returns
# the result of a call to ruleset 6.

S8
# empty

S6
R$-$+<@$w>		$@$>3$1$2			localhost
R$-$+<@$w.$D>		$@$>3$1$2			localhost
R$-$+<@$w.$=D>		$@$>3$1$2			other local domains
R$+<@$=S>$*		$@$1<@$2.LOCAL>$3		localhost
R$+<@$=S.$D>$*		$@$1<@$2.LOCAL>$3		localhost
R$+<@$=S.$=D>$*		$@$1<@$2.LOCAL>$4		other local domains
R$*<@$+>$*		$@$1<@$2>$3			already ok
