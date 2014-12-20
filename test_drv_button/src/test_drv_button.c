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
	unsigned char key_vals[4];
	fd = open("/dev/wq_button",	O_RDWR);
	if(fd<0)
	{
		printf("can't open \n");
	}
	while(1)
	{
		read(fd,key_vals,sizeof(key_vals));
		if(!key_vals[0]||!key_vals[1]||!key_vals[2]||!key_vals[3])
		{
			printf("%04d k1 k2 k3 k4  is %d %d %d %d \n",cnt++,key_vals[0],key_vals[1],key_vals[2],key_vals[3]);
		}
	}
	return 0;
}
