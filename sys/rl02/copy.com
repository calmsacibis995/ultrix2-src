!
! Load 'copy.' to install the ULTRIX-32 operating system.
!
!
SET SNAP OFF		! DISABLE ERROR_HALT snapshots
SET FBOX OFF		! Ultrix will turn on Fbox
INIT			! SRM processor init
UNJAM 			! UNJAM SBIA's and enable master sbia interrupts
INIT/PAMM		! INIT physical address memory map
DEPOSIT CSWP 8		! Turn off the cache - Ultrix will enable cache

LOAD/START:0 COPY.	! Load 'copy.' at memory location 0
START 2			! Start 'copy.' at the address 2
