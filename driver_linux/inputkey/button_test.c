
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/input.h>

/*
 * ./button_test /dev/100ask_button0
 *
 */

static struct input_event inputevent;
int main(int argc, char **argv)
{
	int fd;
	int val;
	int err;
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

	while (1)
	{
		/* 3. 读文件 */
		err = read(fd, &inputevent, sizeof(inputevent));
		if(err > 0)
		{
			switch(inputevent.type)
			{
				case EV_KEY:
					if(inputevent.code < BTN_MISC)
					{
						printf("key %d %d \n",inputevent.code,inputevent.value? 1: 0);
					}
					else
					{
						printf("key %d %d \n",inputevent.code,inputevent.value? 1: 0);
					}
					break;
				default:
					break;
			}
		}
		printf("get button : 0x%x\n", val);
	}
	
	close(fd);
	
	return 0;
}


