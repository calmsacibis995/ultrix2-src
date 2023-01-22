/* sccs id  @(#)68kframe.h	1.1  10/31/83  */

struct frame {
struct 	frame	*fp;
	lispval	(*pc)();
	lispval ap[1];
};
