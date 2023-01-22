

#ifndef lint
static	char	*sccsid = "@(#)logname.c	1.1	(ULTRIX)	12/9/84";
#endif lint

# include	"pwd.h"
# include	"sys/types.h"
# include	"macros.h"


char	*logname()
{
	struct passwd *getpwuid();
	struct passwd *log_name;
	int uid;

	uid = getuid();
	log_name = getpwuid(uid);
	endpwent();
	if (! log_name)
		return(0);
	else
		return(log_name->pw_name);
}
