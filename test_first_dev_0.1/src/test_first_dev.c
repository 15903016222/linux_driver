#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
/*
 * wq_device <dev> <on|off>
 */
int main(int argc, char **argv)
{
	int fd;
	char * filename;
	int val = 1;

	if(argc != 3)
	{
		printf("Usage :\n");
		printf("%s <dev> <on|off>\n",argv[0]);
		return 0;
	}
	filename = argv[1];
	fd = open(filename,	O_RDWR);
		if(fd < 0)
		{
			printf("can't open %s \n",filename);
		}
	if(strcmp(argv[2],"on") == 0)
	{
		val =1;//ÁÁµÆ
	}
	else if(strcmp(argv[2],"off") == 0)
	{
		val =0;//ÃðµÆ
	}
	else
	{
		printf("plese input <on|off>");
	}
	write(fd,&val,4);
	return 0;
}
