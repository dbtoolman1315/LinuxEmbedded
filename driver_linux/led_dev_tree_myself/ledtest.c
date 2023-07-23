#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd;
	char status;
	if(argc < 1)
	{
		printf("arg is less\n");
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if(fd < 0)
	{
		printf("open failed\n");
		return -1;
	}

	if (0 == strcmp(argv[2], "on"))
	{
		status = 1;
		write(fd, &status, 1);
	}

	else
	{
		status = 0;
		write(fd, &status, 1);
	}

	close(fd);
	
	return 0;
}
