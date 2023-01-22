(*#@(#)lib-s.mod	1.1	Ultrix	11/28/84 *)
MODULE testlib;
IMPORT SYSTEM, BITOPERATIONS, IO, Storage, unix, parameters, strings, math;
VAR
    f : IO.FILE;
    i : INTEGER;
    a : SYSTEM.ADDRESS;
BEGIN
    i := unix.open("test",0);
    f := IO.OPEN("test","r");
    Storage.ALLOCATE(a,100);
END testlib.
