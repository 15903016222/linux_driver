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
	fd = open("/dev/wq_button",	O_RDWR|	O_NONBLOCK);//以非阻塞方式打开
	if(fd<0)
	{
		printf("can't open \n");
	}
//	fcntl(fd, F_SETOWN,getpid());//告诉内核发给谁
//	/*改变FASYNC标志，最终会调用驱动的button_dev_fasync函数*/
//	Oflags = fcntl(fd,F_GETFL);//
//	fcntl(fd,F_SETFL,Oflags|FASYNC);//
	while(1)
	{
		unsigned char key_val;
		ret = read(fd,&key_val,1);
		printf("key_val: 0x%x  ret=%d\n",key_val,ret);
		sleep(5);

	}
	return 0;
}
