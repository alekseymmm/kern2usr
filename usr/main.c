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

int handle_polling(struct pollfd* pollfd)
{
	uint64_t value = 0;
	int ret = 0;
		//printf("call ioctl to copy data to %p\n", dst_buffer);
		ret = ioctl(chdev_fd, IOCTL_GET_BUFFER, dst_buffer);

		if(ret){
			printf("Error while copying in ioctl : %d\n", ret);
			return -1;//break;
		}
		printf("data copied\n");

	printf("handling_polling done on CPU: %d, PID: %d\n", sched_getcpu(), getpid());	
	return value;
}

int main()
{
	//int result, timeout = 1000000;
	int result, timeout = 10000;
	struct pollfd pollfd;
	struct timespec time_start, time_stop, dtime;
	struct timespec total_time;
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

	efd2 = eventfd(0, 0);
	if(efd2 < 0){
		printf("Unable to create 2nd evenfd. Exiting...\n");
		goto out1;
	}
	printf("Eventfd created efd2=%d pid=%d\n", efd2, getpid());

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

	printf("Start polling...\n");
	fflush(stdout);

	while(!stop_polling){
		result = poll(&pollfd, 1, timeout);

		switch (result) {
		case 0:
			printf("timeout in polling on CPU: %d, PID: %d\n", sched_getcpu(), getpid());			
			break;
		case -1:
			printf("poll error -1. Exiting...\n");
			goto out3;
		default:
			if(pollfd.revents & POLLIN)
			{
				value = handle_polling(&pollfd);
				if(value == EFD_EXIT_TEST_CMD){
					stop_polling = 1;
				}
			}
		}
	}

out3:
	munmap(buffer, BUF_TEST_SIZE);
out2:
	close(chdev_fd);
out1:
	close(efd);
	close(efd2);

	free(dst_buffer);
	printf("done!\n");
	return 0;
}
