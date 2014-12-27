#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int fd;
void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd,&key_val,1);
	printf("key_val: 0x%x\n",key_val);
}
int main(int argc, char **argv)
{
	int ret=0;
	int Oflags;
	unsigned char key_val;
	signal(SIGIO,my_signal_fun);
	fd = open("/dev/wq_button",	O_RDWR);//默认以阻塞方式打开
	if(fd<0)
	{
		printf("can't open \n");
	}


	while(1)
	{
		unsigned char key_val;
		ret = read(fd,&key_val,1);
		printf("key_val: 0x%x  ret=%d\n",key_val,ret);
	}
	return 0;
}
