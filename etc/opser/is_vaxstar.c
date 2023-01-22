#include <sys/file.h>
#include <vax/rpb.h>
#include <sys/types.h>
#include <vax/cpu.h>

main(){
    struct rpb info;
    int fd;

    if((fd = open("/dev/kmem", O_RDONLY)) == -1){
	perror("Couldn't open /dev/kmem");
	exit(1);
    }
    lseek(fd, 0x80000000, 0);
    read(fd, &info, sizeof(struct rpb));
    if((info.cpu == MVAX_II) && (info.cpu_subtype == ST_VAXSTAR)) exit(0);
    else exit(1);
}

