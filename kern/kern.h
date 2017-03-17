/*
 * kern.h
 *
 *  Created on: 25 марта 2016 г.
 *      Author: root
 */



#ifndef KERN_H_
#define KERN_H_

#include <linux/ioctl.h>

#define NDRIVES 1
#define DEVICE_NAME "mmaptest"
//#define BUF_SIZE  (NDRIVES * 4096)
#define BUF_SIZE  (4 * 1024)
#define BUF_TEST_SIZE BUF_SIZE
#define MAJOR_NUM 239

#define TIMER_DELAY 500

#define EFD_FIND 1
#define EFD_MMAP_CMD 2
#define EFD_START_TEST_CMD 3
#define EFD_STOP_TEST_CMD 4
#define EFD_EXIT_TEST_CMD 5

#define EFD_MEMORY_READY 6
#define EFD_MEMORY_COPIED 7

/* Set the message of the device driver */
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)
/* _IOR means that we're creating an ioctl command
 * number for passing information from a user process
 * to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device
 * number we're using.
 *
 * The second argument is the number of the command
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from
 * the process to the kernel.
 */

/* Get the message of the device driver */
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)
 /* This IOCTL is used for output, to get the message
  * of the device driver. However, we still need the
  * buffer to place the message in to be input,
  * as it is allocated by the process.
  */
#endif /* KERN_H_ */
