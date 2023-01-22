/*
	mount -- system call emulation for 4.2BSD

	last edit:	19-Dec-1983	D A Gwyn
*/

#include	<errno.h>

extern int	_mount(), geteuid();

int
mount( spec, dir, rwflag )
	char	*spec;			/* special file being mounted */
	char	*dir;			/* directory serving as root */
	int	rwflag; 		/* low bit 1 iff read-only */
	{
	if ( _mount( spec, dir, rwflag & 1 ) != 0 )
		{
		switch ( errno )
			{
		case ENODEV:
			if ( geteuid() != 0 )
				errno = EPERM;	/* not super-user */
			else
				errno = ENOENT;	/* spec nonexistent */
			break;

		case EPERM:		/* illegal char in `dir' */
		case EROFS:		/* `dir' on read-only FS */
			errno = ENOTDIR;
			break;
/*
		case ENOTBLK:
		case ENXIO:
		case ENOTDIR:
		case EBUSY:
			break;
*/
			}
		return -1;
		}

	return 0;
	}
