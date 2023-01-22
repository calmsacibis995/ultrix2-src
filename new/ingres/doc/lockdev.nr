.\"		@(#)lockdev.nr	1.2	(ULTRIX)	2/1/85
.ds HE 'HOW TO INSTALL CONCURRENCY DEVICE''PAGE %'
.so nmacs
.ce
Instructions for installing the INGRES lock device in UNIX

.ce
May 19, 1983
.sp 3
INGRES version 7.1 has a fully implemented concurrency control
mechanism.
The INGRES system sets and clears locks by writing to
a server named "ingreslock".
If the socket cannot be opened for writing, INGRES
runs without any concurrency control.

Initially it is advisable to bring up INGRES without
the concurrency device.
If planned usage of the INGRES system requires concurrency
control then the server "ingreslock" must be installed
into your UNIX system.

This document describes the procedure for installing
the UNIX server for
"ingreslock".

.sh 1
Log in as root.
.sh 1
Compiling the concurrency device
.PS
.dn
change directory to ../source/support
.PS
The concurrency device consists of three
files: .../source/support/ildr.c
 .../source/h/ildr.h
and ../source/support/init_socket.c

compile the lock driver
.nf
	make lockd

.fi
.sh
Installation Parameters
.PS
There are several parameters in ildr.h that can be set by
the user.  These include:

.nf
	NLOCKS - number of locks
	PLOCKS - page locks
	RLOCKS - relation locks
	DLOCKS - data base locks

.fi
NLOCKS is the maximum number of locks permissible.
It determines the size of the lock table.
PLOCKS is the maximum number of page, relation and data base
locks.
RLOCKS is the maximum number of relation and data base locks.
DLOCKS is the maximum number of data base locks.  Only
DLOCKS INGRESes can be active at one time.

The following conditions must hold to protect against
deadlock due to lock table overflow:

.nf
	NLOCKS > PLOCKS > RLOCKS > DLOCKS + 6

.fi
The extra number of RLOCKS are required inorder
to be able to run 6-variable queries.
We recommend first setting DLOCKS and then setting the other parameters
as follows:

.nf
	RLOCKS = 2*DLOCKS + 6
	PLOCKS = RLOCKS + 3
	NLOCKS = PLOCKS + 1
.fi

For example, to reduce the size of the lock table, one could set
DLOCKS = 2, RLOCKS = 10, PLOCKS = 13, NLOCKS = 14.
In this case, only 2 people could be running ingres at one time.

As another example, to allow for more concurrent INGRESes, one could set
DLOCKS = 20, RLOCKS = 46, PLOCKS = 49, NLOCKS = 50.
This scenario would allow for twenty simultaneous INGRES users.

The memory required by the lockdriver is 
1008 bytes in the text segment and
36 + 18 * NLOCKS in the data and BSS segments.

.sh
Copy the server
.PS
Copy the lock server to where the servers
are kept on your system.

.nf
	cp lockd  /etc/lockd

.fi
.sh 1
Add the service to /etc/services
.PS
You should have an entry

.nf

	ingreslock	1524/tcp		# Ingres concurrency driver

.fi

.PS
In your /etc/services file. Note that the number (in this case 1524) need only
be unique among your servers, and does NOT have to be 1524 (we use it though).
.sh 1
Put lock server in /etc/rc.local thusly:

.nf
	/etc/lockd & echo -n ' lockd' > /dev/console
.fi

.PS
This ensures that the lock server is always running when the system
is up.
.sh 1
Done.
.PS
INGRES will now use the "ingreslock" service for its
concurrency control.
A simple way to verify this is to try running
(for example) "purge" on a data base while
anyone is running "ingres" on the same data base.
