/*
	umount -- system call emulation for 4.2BSD

	last edit:	14-Dec-1983	D A Gwyn
*/

#include	<errno.h>

extern int	_umount(), getuid();

int
umount( spec )
	char	*spec;			/* special file to unmount */
	{
	if ( _umount( spec ) != 0 )
		{
		if ( errno == ENODEV )
			if ( getuid() != 0 )
				errno = EPERM;	/* not super-user */
			else
				errno = ENXIO;	/* spec nonexistent */
		/* other errno values are already okay */

		return -1;
		}

	return 0;
	}
