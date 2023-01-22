# ULTRIX.k - ULTRIX supported manufacturing key file.
#
# global (product-wide) definitions
NAME='VAX/ULTRIX V2.0'
CODE=ULT
VERS=020
MI=ULTRIX.mi
ROOT=1
# subset definitions follow
%%
ULTBASE020	.	1	'Base System'
ULTACCT020	.	2	'Accounting Software'
ULTBIN020	.	0	'Kernel Configuration Files'
ULTBSC020	.	2	'Bisynchronous Communications'
ULTINET020	.	2	'TCP/IP Networking Utilities'
ULTMOP020	.	2	'DEC Maintenance Operations Protocol'
ULTNFS020	ULTINET020	2	'Network File System Utilities'
ULTUMAIL020	ULTINET020	2	'Extended (Berkeley) Mailer'
ULTCOMM020	ULTINET020	2	'Communications Utilities'
ULTDCMT020	.	2	'Document Preparation Software'
ULTEXER020	.	2	'System Exerciser Package'
ULTF77020	.	2	'Fortran-77 Development Package'
ULTPASCAL020	.	2	'Pascal Development Package'
ULTPGMR020	.	2	'Software Development Utilities'
ULTSCCS020	.	2	'Source Code Control System'
ULTUUCP020	ULTCOMM020	2	'Unix-to-Unix Copy Facility'
ULTLEARN020	.	2	'Computer Aided System Tutor'
ULTMAN020	ULTDCMT020	2	'On Line Manual Pages'
