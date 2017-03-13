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


int main()
{
	char *pathname = "/dev/char/mmaptest";
//	/char *usr_str = "String from usr\n";
	char *usr_buf;
	int chdev_fd;
	int efd;
	//int result, timeout = 1000000;
	int result, timeout = 10000;
	struct pollfd pollfd;
	struct timespec time_start, time_stop, dtime;
	struct timespec total_time;

	//create eventfd
	efd = eventfd(0, 0);
	if(efd < 0){
		printf("Unable to create evenfd. Exiting...\n");
		goto out1;
	}
	printf("Eventfd created efd=%d pid=%d\n", efd, getpid());

	pollfd.fd = efd;
	pollfd.events = POLLIN;
	//ioctl_set_msg(fd, NULL);

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
		case 1:
			printf("Got 1 in polling eventfd\n");
			chdev_fd = open(pathname, O_RDWR);
			if(chdev_fd < 0)
			{
				printf("Failed to open %s \n", pathname);
		        printf("Error no is : %d\n", errno);
		        printf("Error description is : %s\n",strerror(errno));
				goto out1;
			}
			printf("Char dev %s opened \n", pathname);

			usr_buf = (char *)mmap(NULL, BUF_TEST_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, chdev_fd, 0);

			if(usr_buf == MAP_FAILED){
				perror("mmap failed, Exiting...\n");
				goto out2;
			}
			printf("Memory from char dev = %d mmaped to %p \n", chdev_fd, usr_buf);
			break;
		default:
			if(pollfd.revents & POLLIN)
			{
				printf("Got POLLIN in revents\n");
				clock_gettime( CLOCK_REALTIME, &time_start);
				sleep(3);
				clock_gettime( CLOCK_REALTIME, &time_stop);
				//ioctl_set_msg(chdev_fd, NULL);
				printf("Calculation done\n");
				dtime = diff(time_start, time_stop);
				printf("Time for this calculation = %ld\n",dtime.tv_sec*1000000000 + dtime.tv_nsec);
			}
		}
	}
//	ioctl_set_msg(fd, (char *)123456);
//	printf("Result: %s \n", str);
//	strcpy(str, usr_str);
//
//	printf("new str = %s", str);
out3:
	munmap(usr_buf, BUF_TEST_SIZE);
out2:
	close(efd);
out1:
	close(chdev_fd);
	printf("done!\n");
	return 0;
}
