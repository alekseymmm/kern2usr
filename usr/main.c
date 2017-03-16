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

#include <errno.h>

#include "kern/kern.h"
#include "utils.h"
//#include "engine.h"

int chdev_fd;
int efd;
int efd2;
char *pathname = "/dev/char/mmaptest";;
char *buffer = NULL;
char *dst_buffer = NULL;

int ioctl_set_msg(int file_dsc, char *message)
{
	int ret_val;

	ret_val = ioctl(file_dsc, IOCTL_SET_MSG, message);
	if(ret_val < 0){
		printf("ioctl_set_msg failed %d\n",ret_val);
		return -1;
	}
	return 0;
}

int do_mmap(char *pathname){
	chdev_fd = open(pathname, O_RDWR);
	if(chdev_fd < 0)
	{
		printf("Failed to open %s \n", pathname);
	    printf("Error no is : %d\n", errno);
	    printf("Error description is : %s\n",strerror(errno));
	    return -1;
	}
	else {
		printf("Char dev %s opened \n", pathname);

		buffer = (char *)mmap(NULL, BUF_TEST_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, chdev_fd, 0);

		if(buffer == MAP_FAILED){
			perror("mmap failed, Exiting...\n");
			close(chdev_fd);
			return -1;
		}
		printf("Memory from char dev = %d mmaped to %p \n", chdev_fd, buffer);
	}

	return 0;
}

int handle_polling(struct pollfd* pollfd)
{
	uint64_t value = 0;
	read(pollfd->fd, &value, sizeof(uint64_t));

	printf("In handling_polling value in eventfd = %lu\n", value);
	switch(value){
	case EFD_MMAP_CMD:
		printf("Got mmap command in polling eventfd\n");
		do_mmap(pathname);
		break;
	case EFD_START_TEST_CMD:
		printf("Eventfd reset to 0 after test start\n");
		break;
	case EFD_STOP_TEST_CMD:
		printf("Eventfd reset to 0 after test stop\n");
		break;
	case EFD_MEMORY_READY:
		//printf("Eventfd notified that memory is ready, and set to 0\n");
		memcpy(dst_buffer, buffer, BUF_TEST_SIZE);
		value = EFD_MEMORY_COPIED;

		//write(efd2, &value, sizeof(uint64_t));
		ioctl_set_msg(chdev_fd, NULL);

		printf("Memory successfully copied and %d has been written to efd2\n", EFD_MEMORY_COPIED);
		break;
	case EFD_EXIT_TEST_CMD:
		printf("Stop polling cmd received\n");
		break;
	}
	printf("handling_polling done!\n");
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
	chdev_fd = open("/dev/char/mmaptest", O_RDWR);
	if(chdev_fd < 0)
	{
	    printf("failed to open /dev/char/mmaptest \n");
	    goto out1;
	}
	printf("char dev =% d opened.\n", chdev_fd);

	pollfd.fd = chdev_fd;
	pollfd.events = POLLIN;
	//ioctl_set_msg(fd, NULL);

	buffer = (char *)mmap(NULL, BUF_TEST_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, chdev_fd, 0);

	if(buffer == MAP_FAILED){
		perror("mmap failed, Exiting...\n");
		close(chdev_fd);
		goto out2;
	}
	printf("Memory from char dev = %d mmaped to %p \n", chdev_fd, buffer);

	printf("Start polling...\n");
	fflush(stdout);

	while(1){
		result = poll(&pollfd, 1, timeout);
		switch (result) {
		case 0:
			printf("timeout in polling\n");
			break;
		case -1:
			printf("poll error -1. Exiting...\n");
			goto out3;
		default:
			if(pollfd.revents & POLLIN)
			{
				memcpy(dst_buffer, buffer, BUF_TEST_SIZE);
				ioctl_set_msg(chdev_fd, NULL);
				printf("Calculation done\n");
			}
		}
	}

out3:
	munmap(buffer, BUF_TEST_SIZE);
out2:
	close(chdev_fd);
out1:

	free(dst_buffer);
	printf("done!\n");
	return 0;
}
