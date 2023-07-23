
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
/*
 * ./button_test /dev/100ask_button0
 *
 */
static int fd;

static void sig_fun(int a)
{
	int val;
	read(fd,&val,4);
	printf("get button : 0x%x\n", val);
}
int main(int argc, char **argv)
{

	int val,ret;
	struct pollfd fds[1];
	int timeout = 5000;

	
	/* 1. 判断参数 */
	if (argc != 2) 
	{
		printf("Usage: %s <dev>\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	/*signal(SIGIO,sig_fun);
	fcntl(fd,F_SETOWN,getpid());
	int flags = fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL, flags | FASYNC);
	
	fds[0].fd = fd;
	fds[0].events = POLLIN;*/
	while (1)
	{
		if (read(fd, &val, 4) == 4)
			printf("get button: 0x%x\n", val);
		else
			printf("while get button: -1\n");
	}
	
	close(fd);
	
	return 0;
}


