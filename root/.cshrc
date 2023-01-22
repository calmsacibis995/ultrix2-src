#	@(#).cshrc	1.6	ULTRIX	1/15/87
if ($?prompt) then
	umask 022
	set cdpath = ( /sys /usr/spool )
	set notify
	set history = 100
	set path = ( /usr/ucb /bin /usr/bin /etc /usr/etc /usr/local /usr/new /usr/hosts . )
endif
