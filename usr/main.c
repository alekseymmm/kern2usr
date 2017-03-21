/*
 * main.c
 *
 *  Created on: 24 марта 2016 г.
 *      Author: root
 */


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sched.h>

#include <errno.h>

#include "kern/kern.h"
#include "utils.h"


int chdev_fd;
int efd;
int efd2;
char *pathname = "/dev/char/mmaptest";;
char *buffer = NULL;
char *dst_buffer = NULL;

int handle_polling(uint64_t value)
{
	int ret = 0;

	switch(value){
	case EFD_START_TEST_CMD:
		printf("Test started\n");
		break;
	case EFD_STOP_TEST_CMD:
		printf("Test stoped\n");
		break;
	case EFD_EXIT_TEST_CMD:
		printf("Test exit\n");
		break;
	case EFD_MEMORY_READY:
		ret = ioctl(chdev_fd, IOCTL_GET_BUFFER, dst_buffer);

		if(ret){
			printf("Error while copying in ioctl : %d\n", ret);
			return -1;
		}
		printf("data copied\n");

		printf("handling_polling done on CPU: %d, PID: %d\n", sched_getcpu(), getpid());
		break;
	default:
		printf("Unsupported command.\n");
		break;
	}
	return 0;
}

int main()
{
	//int result, timeout = 1000000;
	int result, timeout = 10000;
	struct pollfd pollfd;
	uint64_t value = 0;
	int stop_polling = 0;

	dst_buffer = malloc(BUF_TEST_SIZE);
	printf("dst_buffer in user space = %p\n", dst_buffer);

	//create eventfd
	efd = eventfd(0, 0);
	if(efd < 0){
		printf("Unable to create evenfd. Exiting...\n");
		goto out1;
	}
	printf("Eventfd created efd=%d pid=%d\n", efd, getpid());

	pollfd.fd = efd;
	pollfd.events = POLLIN;

	chdev_fd = open(pathname, O_RDWR);
	if(chdev_fd < 0)
	{
		printf("Failed to open %s \n", pathname);
	    printf("Error no is : %d\n", errno);
	    printf("Error description is : %s\n",strerror(errno));
	    goto out2;
	}

	printf("Char dev %s opened \n", pathname);

	printf("Start polling on CPU: %d, PID: %d ...\n", sched_getcpu(), getpid());
	fflush(stdout);

	while(!stop_polling){
		result = read(efd, &value, sizeof(uint64_t));

		switch (result) {
		case 0:
			printf("timeout in polling\n");
			break;
		case -1:

			printf("poll error -1. Exiting...\n");
			goto out2;
		default:
			handle_polling(value);
			if(value == EFD_EXIT_TEST_CMD){
				stop_polling = 1;
			}

		}
	}

out2:
	close(chdev_fd);
out1:
	close(efd);

	free(dst_buffer);
	printf("done!\n");
	return 0;
}
