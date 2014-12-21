#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
/*
 * wq_device <dev> <on|off>
 */
int main(int argc, char **argv)
{
	int cnt=0;
	int fd;
	unsigned char key_val;
	fd = open("/dev/wq_button",	O_RDWR);
	if(fd<0)
	{
		printf("can't open \n");
	}
	while(1)
	{
		read(fd,&key_val,1);
		printf("key_val = 0x%x\n",key_val);
	}
	return 0;
}
